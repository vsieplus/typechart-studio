#include "config/chartinfo.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// write prettified JSON to another file
//std::ofstream o("pretty.json");
//o << std::setw(4) << j << std::endl;

ChartInfo::ChartInfo(std::string chartPath) {

}

ChartInfo::ChartInfo(int level, std::string typist, std::string keyboardLayout) : 
        level(level), typist(typist), keyboardLayout(keyboardLayout) {}

void ChartInfo::saveChart(std::string chartPath) {

}