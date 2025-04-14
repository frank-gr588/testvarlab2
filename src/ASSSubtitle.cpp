#include "ASSSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <cstring>

ASSSubtitle::ASSSubtitle() : stylesCount(0) {}

int64_t ASSSubtitle::parseTime(const std::string& timeStr) {
    std::string trimmed = timeStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    int h, m, s, ms;
    int parsed = std::sscanf(trimmed.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms);
    if (parsed != 4) {
        throw std::runtime_error("Invalid time format: " + timeStr);
    }
    return ((h * 60 + m) * 60 + s) * 1000 + ms;
}

std::string ASSSubtitle::formatTime(int64_t ms) const {
    int h = static_cast<int>(ms / 3600000);
    ms %= 3600000;
    int m = static_cast<int>(ms / 60000);
    ms %= 60000;
    int s = static_cast<int>(ms / 1000);
    ms %= 1000;
    int cs = static_cast<int>(ms / 10);

    std::ostringstream oss;
    oss << h << ":"
        << std::setw(2) << std::setfill('0') << m << ":"
        << std::setw(2) << std::setfill('0') << s << "."
        << std::setw(2) << std::setfill('0') << cs;
    return oss.str();
}

void ASSSubtitle::read(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

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

SubtitleEntryList& ASSSubtitle::getEntries() {
    return entries;
}

void ASSSubtitle::parseScriptInfo(std::ifstream& in) {
    std::string line;
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;

        if (line.front() == '[') {
            in.seekg(pos);
            break;
        }
        auto posColon = line.find(':');
        if (posColon != std::string::npos) {
            std::string key = line.substr(0, posColon);
            std::string value = line.substr(posColon + 1);
            auto trimFunc = [](std::string &str) {
                str.erase(0, str.find_first_not_of(" \t\r\n"));
                str.erase(str.find_last_not_of(" \t\r\n") + 1);
            };
            trimFunc(key);
            trimFunc(value);

            if (key == "Title") {
                scriptInfo.title = value;
            } else if (key == "Original Script") {
                scriptInfo.originalScript = value;
            } else if (key == "Original Translation") {
                scriptInfo.originalTranslation = value;
            }
        }
    }
}

void ASSSubtitle::parseStyles(std::ifstream& in) {
    std::string line;
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;
        if (line.front() == '[') {
            in.seekg(pos);
            break;
        }

        std::string trimmed = line;
        auto trimFunc = [](std::string &str) {
            str.erase(0, str.find_first_not_of(" \t\r\n"));
            str.erase(str.find_last_not_of(" \t\r\n") + 1);
        };
        trimFunc(trimmed);

        if (trimmed.rfind("Style:", 0) == 0) {
            Style st;
            char name[100]{}, fontname[100]{}, primaryColour[100]{}, secondaryColour[100]{};
            char outlineColour[100]{}, backColour[100]{};

            int parsed = std::sscanf(
                trimmed.c_str(),
                "Style: %99[^,],%99[^,],%d,%99[^,],%99[^,],%99[^,],%99[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                name, fontname,
                &st.fontsize,
                primaryColour, secondaryColour, outlineColour, backColour,
                &st.bold, &st.italic, &st.underline, &st.strikeOut,
                &st.scaleX, &st.scaleY, &st.spacing, &st.angle,
                &st.borderStyle, &st.outline, &st.shadow, &st.alignment,
                &st.marginL, &st.marginR, &st.marginV, &st.encoding
            );

            if (parsed < 24) {
                continue;
            }

            st.name = name;
            st.fontname = fontname;
            st.primaryColour = primaryColour;
            st.secondaryColour = secondaryColour;
            st.outlineColour = outlineColour;
            st.backColour = backColour;

            if (stylesCount < MAX_STYLES) {
                styles[stylesCount++] = st;
            }
        }
    }
}

void ASSSubtitle::parseDialogue(const std::string &line) {
    if (line.find("Dialogue:") != 0) {
        return;
    }

    std::string after = line.substr(9);
    after.erase(0, after.find_first_not_of(" \t\r\n"));

    std::string fields[MAX_DIALOGUE_FIELDS];
    size_t fieldCount = 0;

    std::stringstream ss(after);
    std::string field;

    while (std::getline(ss, field, ',') && fieldCount < MAX_DIALOGUE_FIELDS) {
        fields[fieldCount++] = field;
    }

    if (fieldCount < 10) {
        return;
    }

    try {
        Dialogue dlg;
        dlg.layer = std::stoi(fields[0]);
        dlg.start = fields[1];
        dlg.end = fields[2];
        dlg.style = fields[3];
        dlg.name = fields[4];
        dlg.marginL = fields[5];
        dlg.marginR = fields[6];
        dlg.marginV = fields[7];
        dlg.effect = fields[8];

        dlg.text = fields[9];
        for (size_t i = 10; i < fieldCount; ++i) {
            dlg.text += "," + fields[i];
        }

        dlg.text = std::regex_replace(dlg.text, std::regex("\\\\N"), " ");
        dlg.text = std::regex_replace(dlg.text, std::regex("\\{[^}]*\\}"), "");

        SubtitleEntry entry;
        entry.start_ms = parseTime(dlg.start);
        entry.end_ms = parseTime(dlg.end);
        entry.text = dlg.text;

        entries.push_back(entry);
    } catch (...) {
    }
}

void ASSSubtitle::parseEvents(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;
        if (line.front() == '[') {
            break;
        }

        if (line.find("Dialogue:") == 0) {
            parseDialogue(line);
        }
    }
}

void ASSSubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot write file: " + filename);
    }

    out << "[Script Info]\n";
    out << "Title: " << scriptInfo.title << "\n";
    out << "Original Script: " << scriptInfo.originalScript << "\n";
    out << "Original Translation: " << scriptInfo.originalTranslation << "\n\n";

    out << "[V4+ Styles]\n";
    for (int i = 0; i < stylesCount; ++i) {
        const Style &st = styles[i];
        out << "Style: "
            << st.name << ","
            << st.fontname << ","
            << st.fontsize << ","
            << st.primaryColour << ","
            << st.secondaryColour << ","
            << st.outlineColour << ","
            << st.backColour << ","
            << st.bold << ","
            << st.italic << ","
            << st.underline << ","
            << st.strikeOut << ","
            << st.scaleX << ","
            << st.scaleY << ","
            << st.spacing << ","
            << st.angle << ","
            << st.borderStyle << ","
            << st.outline << ","
            << st.shadow << ","
            << st.alignment << ","
            << st.marginL << ","
            << st.marginR << ","
            << st.marginV << ","
            << st.encoding
            << "\n";
    }
    out << "\n[Events]\n";
    for (size_t i = 0; i < entries.getSize(); ++i) {
        const auto &e = entries[i];
        out << "Dialogue: 0,"
            << formatTime(e.start_ms) << ","
            << formatTime(e.end_ms)
            << ",Default,,0,0,0,,"
            << e.text
            << "\n";
    }
}

void ASSSubtitle::removeFormatting() {
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); i++) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

void ASSSubtitle::addDefaultStyle(const std::string& styleName) {
    for (size_t i = 0; i < entries.getSize(); i++) {
        entries[i].text = "<" + styleName + ">" + entries[i].text + "</" + styleName + ">";
    }
}

void ASSSubtitle::shiftTime(int64_t deltaMs, TimeShiftType type) {
    for (size_t i = 0; i < entries.getSize(); i++) {
        if (type == START_ONLY || type == START_END) {
            entries[i].start_ms += deltaMs;
        }
        if (type == END_ONLY || type == START_END) {
            entries[i].end_ms += deltaMs;
        }
    }
}