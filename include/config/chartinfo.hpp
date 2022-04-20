#ifndef CHARTINFO_HPP
#define CHARTINFO_HPP

#include <string>


struct ChartInfo {
    ChartInfo(std::string chartPath);
    ChartInfo(int level, std::string typist, std::string keyboardLayout);

    void saveChart(std::string chartPath);

    int level;

    float offsetMS = 0.f;

    std::string typist;
    std::string keyboardLayout;
};

#endif // CHARTINFO_HPP