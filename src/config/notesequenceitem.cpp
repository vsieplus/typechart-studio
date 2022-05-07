#include "config/notesequenceitem.hpp"

bool operator<(const std::shared_ptr<NoteSequenceItem> & lhs, const std::shared_ptr<NoteSequenceItem> & rhs) {
    return lhs && rhs && (*lhs.get()) < (*rhs.get());
}

bool operator<(const NoteSequenceItem & lhs, const NoteSequenceItem & rhs) {
    return lhs.absBeat < rhs.absBeat;
}