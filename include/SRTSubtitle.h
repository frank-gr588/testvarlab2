#pragma once
#include "SubtitleEntryList.h"
#include <string>

enum TimeShiftType {
    START_END,
    START_ONLY,
    END_ONLY
};

class SRTSubtitle {
private:
    SubtitleEntryList entries;

    static int64_t parseTime(const std::string& timeStr); // "00:01:02,345" -> ms
    static std::string formatTime(int64_t ms);

    void parseEntry(std::ifstream& in);
    void writeEntry(std::ofstream& out, const SubtitleEntry& entry, size_t index) const;

public:
    void read(const std::string& filename);
    void write(const std::string& filename) const;
    SubtitleEntryList& getEntries();

    void removeFormatting();
    void addDefaultStyle(const std::string& style);
    void shiftTime(int64_t delta_ms, TimeShiftType type);
};