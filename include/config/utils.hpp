#ifndef UTILS_HPP
#define UTILS_HPP

#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "config/beatpos.hpp"
#include "config/timeinfo.hpp"

class EditAction;

namespace utils {

void emptyActionStack(std::stack<std::shared_ptr<EditAction>> & stack);

void HelpMarker(const char* desc);
bool showEditableText(const char * label, char * text, size_t bufSize, bool & editingText, std::string & savedText);

BeatPos calculateBeatpos(double absBeat, int currentBeatsplit, const std::vector<Timeinfo> & timeinfo);
std::pair<int, double> splitSecsbyMin(double seconds);

bool cmpSecond(const std::pair<std::string, int> & l, const std::pair<std::string, int> & r);

}

#endif // UTILS_HPP
