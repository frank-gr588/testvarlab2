#include "SRTSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>

int64_t SRTSubtitle::parseTime(const std::string& timeStr) {
    int h, m, s, ms;
    sscanf(timeStr.c_str(), "%d:%d:%d,%d", &h, &m, &s, &ms);
    return ((h * 60 + m) * 60 + s) * 1000 + ms;
}

std::string SRTSubtitle::formatTime(int64_t ms) {
    int h = ms / 3600000;
    ms %= 3600000;
    int m = ms / 60000;
    ms %= 60000;
    int s = ms / 1000;
    ms %= 1000;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":"
        << std::setw(2) << m << ":"
        << std::setw(2) << s << ","
        << std::setw(3) << ms;
    return oss.str();
}

void SRTSubtitle::parseEntry(std::ifstream& in) {
    std::string line;
    if (!std::getline(in, line) || line.empty()) return; // Пропускаем номер строки

    std::string timeLine;
    if (!std::getline(in, timeLine)) return;

    size_t arrow = timeLine.find("-->");
    if (arrow == std::string::npos) throw std::runtime_error("Invalid time format in SRT file");

    std::string startStr = timeLine.substr(0, arrow - 1);
    std::string endStr = timeLine.substr(arrow + 4);

    int64_t start_ms = parseTime(startStr);
    int64_t end_ms = parseTime(endStr);

    SubtitleEntry entry;
    entry.start_ms = start_ms;
    entry.end_ms = end_ms;

    size_t coordPos = timeLine.find("X1:");
    if (coordPos != std::string::npos) {
        sscanf(timeLine.c_str() + coordPos, "X1:%d X2:%d Y1:%d Y2:%d", &entry.x1, &entry.x2, &entry.y1, &entry.y2);
        entry.has_coordinates = true;
    }

    std::string text;
    while (std::getline(in, line) && !line.empty()) {
        if (!text.empty()) text += "\n";
        text += line;
    }

    entry.text = text;
    entries.push_back(entry);
}

void SRTSubtitle::writeEntry(std::ofstream& out, const SubtitleEntry& entry, size_t index) const {
    out << (index + 1) << "\n";
    out << formatTime(entry.start_ms) << " --> " << formatTime(entry.end_ms);
    if (entry.has_coordinates) {
        out << " X1:" << entry.x1 << " X2:" << entry.x2 << " Y1:" << entry.y1 << " Y2:" << entry.y2;
    }
    out << "\n";
    out << entry.text << "\n\n";
}

void SRTSubtitle::read(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open file: " + filename);

    while (!in.eof()) {
        parseEntry(in);
    }
}

void SRTSubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot write file: " + filename);

    for (size_t i = 0; i < entries.getSize(); ++i) {
        writeEntry(out, entries[i], i);
    }
}

SubtitleEntryList& SRTSubtitle::getEntries() {
    return entries;
}

void SRTSubtitle::removeFormatting() {
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

void SRTSubtitle::addDefaultStyle(const std::string& style) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = "<" + style + ">" + entries[i].text + "</" + style + ">";
    }
}

void SRTSubtitle::shiftTime(int64_t delta_ms, TimeShiftType type) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        if (type == START_END || type == START_ONLY) {
            entries[i].start_ms += delta_ms;
        }
        if (type == START_END || type == END_ONLY) {
            entries[i].end_ms += delta_ms;
        }
    }
}