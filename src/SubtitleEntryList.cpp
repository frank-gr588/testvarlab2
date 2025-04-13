#include "SubtitleEntryList.h"
#include <stdexcept>
#include <cstring>

SubtitleEntryList::SubtitleEntryList() : data(nullptr), size(0), capacity(0) {}

SubtitleEntryList::~SubtitleEntryList() {
    delete[] data;
}

void SubtitleEntryList::resize(size_t new_capacity) {
    SubtitleEntry* new_data = new SubtitleEntry[new_capacity];
    for (size_t i = 0; i < size; ++i)
        new_data[i] = data[i];
    delete[] data;
    data = new_data;
    capacity = new_capacity;
}

void SubtitleEntryList::push_back(const SubtitleEntry& entry) {
    if (size == capacity) {
        size_t new_capacity = capacity == 0 ? 4 : capacity * 2;
        resize(new_capacity);
    }
    data[size++] = entry;
}

SubtitleEntry& SubtitleEntryList::operator[](size_t index) {
    if (index >= size) throw std::out_of_range("Index out of range");
    return data[index];
}

const SubtitleEntry& SubtitleEntryList::operator[](size_t index) const {
    if (index >= size) throw std::out_of_range("Index out of range");
    return data[index];
}

size_t SubtitleEntryList::getSize() const {
    return size;
}

void SubtitleEntryList::clear() {
    delete[] data;
    data = nullptr;
    size = 0;
    capacity = 0;
}