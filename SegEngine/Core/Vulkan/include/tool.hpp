#pragma once

#include <fstream>
#include <vector>
#include <iostream>

namespace Sego{

std::vector<char> ReadWholeFile(const std::string& filename);

}