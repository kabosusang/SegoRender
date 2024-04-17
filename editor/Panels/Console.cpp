#include "pch.h"
#include "Console.hpp"
#include <imgui.h>

namespace Sego{

void Console::OnImGuiRender(){
    std::vector<std::string> lastest_logs = Log::getLastestLogs();
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_HorizontalScrollbar;

    if (!ImGui::Begin("Console",nullptr,window_flags)){
        ImGui::End();
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    auto boldFont = io.Fonts->Fonts[1];
    ImGui::PushFont(boldFont);

    ImGuiListClipper clipper;
    clipper.Begin(lastest_logs.size());
    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
        {
            ShowLogText(lastest_logs[row]);
        }
    }

    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::PopFont();
    ImGui::End();
}


size_t find_nth(const std::string& haystack, const std::string& needle, size_t nth)
{
    size_t pos = -1;
    int cnt = 0;

    while (cnt != nth)
    {
        pos = haystack.find(needle, ++pos);
        if (pos == std::string::npos)
        {
            return -1;
        }
        ++cnt;
    }
    return pos;
}


void Console::ShowLogText(const std::string& log){
    static std::map<ELogLevel, ImVec4> k_log_colors = {
			{ ELogLevel::Debug, ImVec4(0.5f, 0.5f, 0.5f, 1.0f) },
			{ ELogLevel::Info, ImVec4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ ELogLevel::Warning, ImVec4(1.0f, 1.0f, 0.0f, 1.0f) },
			{ ELogLevel::Error, ImVec4(1.0f, 0.0f, 0.0f, 1.0f) },
			{ ELogLevel::Fatal, ImVec4(1.0f, 0.0f, 1.0f, 1.0f) }
		};

    static std::map<ELogLevel, const char*> k_log_strs = {
        { ELogLevel::Debug, "debug" },
        { ELogLevel::Info, "info" },
        { ELogLevel::Warning, "warning" },
        { ELogLevel::Error, "error" },
        { ELogLevel::Fatal, "fatal" }
    };

		for (int i = 0; i <= (int)(ELogLevel::Fatal); ++i)
		{
			ELogLevel log_level = (ELogLevel)i;
			size_t log_str_len = std::string(k_log_strs[log_level]).length();
			if (std::string(log.begin() + 1, log.begin() + log_str_len + 1) == k_log_strs[log_level])
			{
				ImGui::PushStyleColor(ImGuiCol_Text, k_log_colors[log_level]);
				ImGui::Text((std::string("[") + k_log_strs[log_level] + "]").c_str());
				ImGui::PopStyleColor();

				ImGui::SameLine();

				size_t second_right_bracket_pos = find_nth(log, "]", 2);
				ImGui::Text(std::string(log.begin() + second_right_bracket_pos + 2, log.end()).c_str());
			}
		}
}



}


