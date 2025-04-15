#pragma once
#include "SubtitleEntryList.h"
#include "SRTSubtitle.h"
#include <string>

class SAMISubtitle {
private:
    SubtitleEntryList entries;

    static int64_t parseTime(const std::string& timeStr); // "00:00:32,940" -> ms
    static std::string formatTime(int64_t ms);

public:
    void read(const std::string& filename);
    void write(const std::string& filename) const;
    SubtitleEntryList& getEntries();

    void removeFormatting();
    void addDefaultStyle(const std::string& style);
    void shiftTime(int64_t delta_ms, TimeShiftType type);
};