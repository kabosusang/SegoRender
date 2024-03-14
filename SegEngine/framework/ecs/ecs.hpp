#pragma once 
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <functional>
#include <optional>
#include "SparseSets.hpp"


#define assertm(msg,expr) assert(((void)msg,(expr)))

namespace ecs {

    using ComponentID = uint32_t;
    using Entity = uint32_t;

    struct Resource{};
    struct Component{};

    template <typename T>
    class EventStaging final{
    public:
        static bool Has(){
            return data_ != std::nullopt;
        }
        static void Set(const T& t){
            data_ = t;
        }
        static void Set(const T&& t){
            data_ = std::move(t);
        }

        static T& Get(){
            return *data_;
        }
        static void Clear(){
            data_ = std::nullopt;
        }

    private:
        inline static std::optional<T> data_ = std::nullopt;
    };

    template <typename T>
    class EventReader final{
    public:
        bool Has(){
            return EventStaging<T>::Has();
        }
        T Read(){
             return EventStaging<T>::Get();
        }

        void Clear(){
            EventStaging<T>::Clear();
        }

    };

    class World;

    class Events final{
    public:
        friend class World;
        template<typename T>
        friend class EventWriter;

        template<typename T>
        auto Reader();

        template<typename T>
        auto Writer();
    private:
        std::vector<void(*)(void)> removeEventFuncs_;
        std::vector<void(*)(void)> removeOldEventFuncs_;
        std::vector<std::function<void(void)>> addEventFuncs_;

    void addAllEvents(){
        for(auto func : addEventFuncs_){
            func();
        }
        addEventFuncs_.clear();
    }

     void removeOldEvents(){
        for(auto func : removeOldEventFuncs_){
            func();
        }
        removeOldEventFuncs_ = removeEventFuncs_;
        removeEventFuncs_.clear();
    }
};
template <typename T>
    class EventWriter final{
    public:
        EventWriter(Events& e) : events_(e){}
        void Write(const T& t);

    private:
        Events& events_;
    };

template <typename T>
    auto Events::Reader(){
        return EventReader<T>{};
    }

template <typename T>
    auto Events::Writer(){
        return EventWriter<T>{*this};
    }

template <typename T>
void EventWriter<T>::Write(const T& t){
    events_.addEventFuncs_.push_back([=](){
        EventStaging<T>::Set(t);
    });
    events_.removeEventFuncs_.push_back([](){
        EventStaging<T>::Clear();
    });
}


class Commands;
class Resources;
class Queryer;

using UpdateSystem = void(*)(Commands&,Queryer,Resources,Events&);
using StartupSystem = void(*)(Commands&);


    template<typename Category>
    class IndexGetter final {
    public:
        template<typename T>
        static uint32_t Get() {
            static uint32_t id = curIdx_++;
            return id;
        }

    private:
        inline static uint32_t curIdx_ = 0;

    };

    template <typename T, typename = std::enable_if<std::is_integral_v<T>>>
    struct IDGenerator final {
    public:
        static T Gen() {
            return curId_++;
        }
    private:
        inline static T curId_ = {};
    };

using EntityGenerator = IDGenerator<Entity>;

    class World final {
    public:
        friend class Commands;
        friend class Resources;
        friend class Queryer;
        using ComponenContainer = std::unordered_map<ComponentID, void*>;

        World() = default;
        World(const World&) = delete;
        World& operator=(const World&) = delete;

        World& AddStartupSystem(StartupSystem sys){
            startupSystems_.push_back(sys);
            return *this;
        }

        World& AddSystem(UpdateSystem sys){
            updateSystems_.push_back(sys);
            return *this;
        }

        template <typename T>
        World& SetResource(T&& resource);


        void Startup();
        void Update();
        void ShutDown(){
            entities_.clear();
            resources_.clear();
            ComponentMap_.clear();
        }
        
    private:
        struct Pool final {
            std::vector<void*> instance_;
            std::vector<void*> cache;

            using CreateFunc = void* (*)(void);
            using DestroyFunc = void(*)(void*);

            CreateFunc create;
            DestroyFunc destroy;

            Pool(CreateFunc create, DestroyFunc destroy) : create(create), destroy(destroy) {
                assertm("you must give a non-null destroy create",create);
                assertm("you must give a non-null destroy function",destroy);
            }

            void* Create() {
                if (!cache.empty()) {
                    instance_.push_back(cache.back());
                    cache.pop_back();
                }
                else {
                    instance_.push_back(create());
                }
                return instance_.back();
            }

            void Destroy(void* elem) {
                if (auto it = std::find(instance_.begin(), instance_.end(), elem);
                    it != instance_.end()) {
                    cache.push_back(*it);
                    std::swap(*it, instance_.back());
                    instance_.pop_back();
                }else {
                    assertm("you elment is no in pool", false);
                }
            }

        };

        struct ComponentInfo {
            Pool pool;
            SparseSets<Entity, 32> sparseSet;

            ComponentInfo(Pool::CreateFunc create, Pool::DestroyFunc destory) : pool(create, destory) {}
            ComponentInfo() :pool(nullptr, nullptr) {}
        };

        using ComponentPool = std::unordered_map<ComponentID, ComponentInfo>;
        ComponentPool ComponentMap_;
        std::unordered_map<Entity, ComponenContainer> entities_;

        struct ResourceInfo{
            void* resource = nullptr;
            using DestroyFunc = void(*)(void*);
            DestroyFunc destroy = nullptr;

            ResourceInfo() = default;

            ResourceInfo( DestroyFunc destroy) :  destroy(destroy) {
                assertm("you must give a non-null destroy function",destroy);
            }
            ~ResourceInfo(){
                destroy(resource);
            }

        };
        std::unordered_map<ComponentID, ResourceInfo> resources_;
        std::vector<StartupSystem> startupSystems_;
        std::vector<UpdateSystem> updateSystems_;
        Events events_;
    };

    class Commands final {
    public:
        Commands(World& world) :world_(world) {}
        template<typename ... ComponentTypes>
        Commands& Spawn(ComponentTypes&& ... components) {
            SpawnAndReturn<ComponentTypes ...>(std::forward<ComponentTypes>(components)...);
            return *this;
        }

        template<typename ... ComponentTypes>
        Entity SpawnAndReturn(ComponentTypes&& ... components){
            EntitySpawnInfo info;
            info.entity = EntityGenerator::Gen();

            if constexpr (sizeof...(components) != 0) {
                doSpawn(info.entity,info.components, std::forward<ComponentTypes>(components)...);
            }
            spawnEntities_.push_back(info);
            return info.entity;
        }


        Commands& Destory(Entity entity) {
            destroyEntities_.push_back(entity);
            return *this;
        }

        template<typename T>
        Commands& SetResource(T&& resource){
            auto index = IndexGetter<Resource>::Get<T>();
            if(auto it = world_.resources_.find(index);it != world_.resources_.end()){
                assertm("resource already exists",it->second.resource);
                it->second.resource = new T(std::forward<T>(resource));
            }else{
                auto newIt = world_.resources_.emplace(index,World::ResourceInfo([](void* elem) {delete (T*)elem;}));
                
                newIt.first->second.resource = new T;
               *(T*)(newIt.first->second.resource) = std::forward<T>(resource);
            }
            return *this;
        }

        template<typename T>
        Commands& removeResource(){
            auto index = IndexGetter<Resource>::Get<T>();
            destroyResources_.push_back(ResourceDestroyInfo(index,[](void* elem){delete (T*)elem;}));
            return *this;
        }

    void Execute(){
         for(auto info : destroyResources_){
            removeResource(info);
         }

        for(auto e : destroyEntities_){
            destoryEntity(e);
        }
        for(auto& spawnInfo : spawnEntities_){
            auto it = world_.entities_.emplace(spawnInfo.entity, World::ComponenContainer{});
            auto& componentContainer = it.first->second;
            for(auto& componentInfo : spawnInfo.components){
                componentContainer[componentInfo.index] = doSpawnWithoutType(spawnInfo.entity,componentInfo);
            }
        }

    }
        
    private:
        World& world_;

        using DestoryFunc = void(*)(void*);

        using AssignFunc = std::function<void(void*)>;
        using CreateComponentFunc = void*(*)(void);

        struct ResourceDestroyInfo{
            uint32_t index;
            DestoryFunc destory;

            ResourceDestroyInfo(uint32_t index,DestoryFunc destory):index(index),destory(destory){}
        };

        struct ComponentSpawnInfo{
            AssignFunc assign;
            ComponentID index;
            World::Pool::CreateFunc create;
            World::Pool::DestroyFunc destory;
        };

        struct EntitySpawnInfo{
        Entity entity;
        std::vector<ComponentSpawnInfo> components;

        };

        std::vector<Entity> destroyEntities_;
        std::vector<ResourceDestroyInfo> destroyResources_;
        std::vector<EntitySpawnInfo> spawnEntities_;

        template<typename T, typename ... Remains>
        void doSpawn(Entity entity, std::vector<ComponentSpawnInfo>& spawnInfo,T&& component ,Remains&& ... remains) {
            ComponentSpawnInfo info;
            info.index = IndexGetter<Component>::Get<T>();
            info.create = [](void)->void* {return new T;};
            info.destory = [](void* elem){delete (T*)elem;};
            info.assign = [=](void* elem){
                static auto com = component;
                *((T*)elem) = com;
                };

            spawnInfo.push_back(info);
            if constexpr (sizeof ... (Remains) != 0){
                doSpawn<Remains ...>(entity,spawnInfo,std::forward<Remains>(remains)...);
            }
        }
        
        void* doSpawnWithoutType(Entity entity,ComponentSpawnInfo& info ) {
            if (auto it = world_.ComponentMap_.find(info.index); it == world_.ComponentMap_.end()) {
                world_.ComponentMap_.emplace(info.index, 
                World::ComponentInfo(info.create,info.destory));
            }
            World::ComponentInfo& componentInfo = world_.ComponentMap_[info.index];
            void* elem = componentInfo.pool.Create();
            info.assign(elem);
            componentInfo.sparseSet.Add(entity);

            return elem;
        }

        void destoryEntity(Entity entity) {
            if (auto it = world_.entities_.find(entity); it != world_.entities_.end()) {
                for (auto [id, component] : it->second) {
                    auto& componentInfo = world_.ComponentMap_[id];
                    componentInfo.pool.Destroy(component);
                    componentInfo.sparseSet.Remove(entity);
                }
                world_.entities_.erase(it);
            }
        }

        void removeResource(ResourceDestroyInfo& info){
            if(auto it = world_.resources_.find(info.index);it != world_.resources_.end()){
                info.destory(it->second.resource);
                it->second.resource = nullptr;
            }
        }

};

class Resources final{
public:
    Resources(World& world): world_(world){}

    template<typename T>
    bool Has(){
        auto index = IndexGetter<Resource>::Get<T>(); //Get ID
        auto it = world_.resources_.find(index);
        return it!= world_.resources_.end() && it->second.resource;
    }

    template<typename T>
    T& Get(){
         auto index = IndexGetter<Resource>::Get<T>(); //Get ID
         return *((T*)world_.resources_[index].resource);
    }

private:
    World& world_;
};

class Queryer final{
public:
    Queryer(World& world):world_(world){}

    template <typename ... Components>
    std::vector<Entity> Query(){
        std::vector<Entity> entities;
        doQuery<Components ...>(entities);
        return entities;
    }

    template <typename T>
    bool Has(Entity entity){
        auto it = world_.entities_.find(entity);
        auto index = IndexGetter<Component>::Get<T>();
        return it != world_.entities_.end() && it->second.find(index) != it->second.end();
    }

    template <typename T>
    T& Get(Entity entity){
        auto index = IndexGetter<Component>::Get<T>();
        return *((T*)world_.entities_[entity][index]);
    }

private:
    World& world_;

    template <typename T,typename ... Remains>
    void doQuery(std::vector<Entity>& outEntities){
        auto index = IndexGetter<Component>::Get<T>();
        World::ComponentInfo& info = world_.ComponentMap_[index];
            for (auto e : info.sparseSet){
                if constexpr (sizeof ... (Remains) != 0){
                   doQueryRemains<Remains ...>(e,outEntities);
                }else{
                outEntities.push_back(e);
                }
        }
    }

    template <typename T,typename ... Remains>
    void doQueryRemains(Entity entity,std::vector<Entity>& outEntities){
        auto index = IndexGetter<Component>::Get<T>();
        auto& componentContainer = world_.entities_[entity];
        if(auto it = componentContainer.find(index);it == componentContainer.end()){
            return ;
        }

        if constexpr (sizeof ... (Remains) == 0){
            outEntities.push_back(entity);
            return ;
        }else{
            return doQueryRemains<Remains ...>(entity,outEntities);
        }
    }
};

inline void World::Startup(){
    std::vector<Commands> commandList;
    for (auto sys : startupSystems_){
        Commands commands{*this};
        sys(commands);
        commandList.push_back(commands);
    }
    for (auto& commands : commandList){
        commands.Execute();
    }
}

inline void World::Update(){
    std::vector<Commands> commandList;
    for (auto sys : updateSystems_){
        Commands commands{*this};
        sys(commands,Queryer{*this},Resources{*this},events_);
        commandList.push_back(commands);
    }
    events_.removeOldEvents();
    events_.addAllEvents();
    

    for (auto& commands : commandList){
        commands.Execute();
    }
}

template <typename T>
World& World::SetResource(T&& resource){
    Commands commands(*this);
    commands.SetResource(std::forward<T>(resource));
    return *this;
}


}