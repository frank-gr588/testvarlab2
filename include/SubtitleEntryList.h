#pragma once
#include "SubtitleEntry.h"
#include <cstddef>

class SubtitleEntryList {
private:
    SubtitleEntry* data;
    size_t size;
    size_t capacity;

    void resize(size_t new_capacity);

public:
    SubtitleEntryList();
    ~SubtitleEntryList();

    void push_back(const SubtitleEntry& entry);
    SubtitleEntry& operator[](size_t index);
    const SubtitleEntry& operator[](size_t index) const;
    size_t getSize() const;
    void clear();
};