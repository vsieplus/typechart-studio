#include "config/notesequenceitem.hpp"


bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return lhs.absBeat < rhs.absBeat;
}