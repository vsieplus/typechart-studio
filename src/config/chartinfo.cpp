#include "config/chartinfo.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

ChartInfo::ChartInfo(std::string chartPath) {

}

ChartInfo::ChartInfo(int level, std::string typist, std::string keyboardLayout) : 
        level(level), typist(typist), keyboardLayout(keyboardLayout) {}

void ChartInfo::saveChart(std::string chartPath) {
    
}