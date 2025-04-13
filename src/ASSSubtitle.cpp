#include "ASSSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>

ASSSubtitle::ASSSubtitle() : stylesCount(0) {}

int64_t ASSSubtitle::parseTime(const std::string& timeStr) {
    int h, m, s, ms;
    sscanf(timeStr.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms);
    return ((h * 60 + m) * 60 + s) * 1000 + ms;
}

std::string ASSSubtitle::formatTime(int64_t ms) const {
    int h = ms / 3600000;
    ms %= 3600000;
    int m = ms / 60000;
    ms %= 60000;
    int s = ms / 1000;
    ms %= 1000;
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << h << ":"
        << std::setw(2) << m << ":"
        << std::setw(2) << s << "."
        << std::setw(3) << ms;
    return oss.str();
}

void ASSSubtitle::parseScriptInfo(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') break;

        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (key == "Title") scriptInfo.title = value;
            else if (key == "Original Script") scriptInfo.originalScript = value;
            else if (key == "Original Translation") scriptInfo.originalTranslation = value;
        }
    }
}

void ASSSubtitle::parseStyles(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') break;

        if (line.find("Style:") != std::string::npos) {
            Style& style = styles[stylesCount++];
            char name[100], fontname[100], primaryColour[100], secondaryColour[100], outlineColour[100], backColour[100];
            sscanf(line.c_str(), "Style: %[^,],%[^,],%d,%[^,],%[^,],%[^,],%[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                name, fontname, &style.fontsize, primaryColour, secondaryColour, outlineColour, backColour, &style.bold, &style.italic,
                &style.underline, &style.strikeOut, &style.scaleX, &style.scaleY, &style.spacing, &style.angle, &style.borderStyle,
                &style.outline, &style.shadow, &style.alignment, &style.marginL, &style.marginR, &style.marginV, &style.encoding);
            style.name = name;
            style.fontname = fontname;
            style.primaryColour = primaryColour;
            style.secondaryColour = secondaryColour;
            style.outlineColour = outlineColour;
            style.backColour = backColour;
        }
    }
}

void ASSSubtitle::parseEvents(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line[0] == '[') break;

        if (line.find("Dialogue:") != std::string::npos) {
            parseDialogue(line);
        }
    }
}

void ASSSubtitle::parseDialogue(const std::string& line) {
    Dialogue dlg;
    char start[100], end[100], style[100], name[100], marginL[100], marginR[100], marginV[100], effect[100], text[1000];
    int parsed = sscanf(line.c_str(), "Dialogue: %d,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^\n]",
        &dlg.layer, start, end, style, name, marginL, marginR, marginV, effect, text);

    dlg.start = start;
    dlg.end = end;
    dlg.style = style;
    dlg.name = name;
    dlg.marginL = marginL;
    dlg.marginR = marginR;
    dlg.marginV = marginV;
    dlg.effect = effect;
    dlg.text = text;

    if (!dlg.effect.empty() || dlg.text.find("\\fe") != std::string::npos || dlg.text.find("\\move") != std::string::npos) {
        return;
    }

    int64_t start_ms = parseTime(dlg.start);
    int64_t end_ms = parseTime(dlg.end);
    SubtitleEntry entry = {start_ms, end_ms, dlg.text};
    entries.push_back(entry);
}

void ASSSubtitle::read(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open file: " + filename);

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        if (line == "[Script Info]") {
            parseScriptInfo(in);
        } else if (line == "[V4+ Styles]") {
            parseStyles(in);
        } else if (line == "[Events]") {
            parseEvents(in);
        }
    }
}

void ASSSubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot write file: " + filename);

    out << "[Script Info]\n";
    out << "Title: " << scriptInfo.title << "\n";
    out << "Original Script: " << scriptInfo.originalScript << "\n";
    out << "Original Translation: " << scriptInfo.originalTranslation << "\n";

    out << "\n[V4+ Styles]\n";
    for (int i = 0; i < stylesCount; ++i) {
        const Style& style = styles[i];
        out << "Style: " << style.name << "," << style.fontname << "," << style.fontsize << "," << style.primaryColour << ","
            << style.secondaryColour << "," << style.outlineColour << "," << style.backColour << "," << style.bold << ","
            << style.italic << "," << style.underline << "," << style.strikeOut << "," << style.scaleX << "," << style.scaleY << ","
            << style.spacing << "," << style.angle << "," << style.borderStyle << "," << style.outline << "," << style.shadow << ","
            << style.alignment << "," << style.marginL << "," << style.marginR << "," << style.marginV << "," << style.encoding << "\n";
    }

    out << "\n[Events]\n";
    for (size_t i = 0; i < entries.getSize(); ++i) {
        const SubtitleEntry& e = entries[i];
        out << "Dialogue: 0," << formatTime(e.start_ms) << "," << formatTime(e.end_ms) << ",Default,,0,0,0,," << e.text << "\n";
    }
}

SubtitleEntryList& ASSSubtitle::getEntries() {
    return entries;
}

void ASSSubtitle::removeFormatting() {
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

void ASSSubtitle::addDefaultStyle(const std::string& style) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = "<" + style + ">" + entries[i].text + "</" + style + ">";
    }
}

void ASSSubtitle::shiftTime(int64_t delta_ms, TimeShiftType type) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        if (type == START_END || type == START_ONLY) {
            entries[i].start_ms += delta_ms;
        }
        if (type == START_END || type == END_ONLY) {
            entries[i].end_ms += delta_ms;
        }
    }
}