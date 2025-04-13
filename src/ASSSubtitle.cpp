#include "ASSSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <iostream>
#include <cstring>

static bool hasUtf8BOM(std::ifstream &file) {
    std::streampos pos = file.tellg();
    unsigned char bom[3];
    file.read(reinterpret_cast<char*>(bom), 3);

    if (file.gcount() == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        return true;
    }
    file.clear();
    file.seekg(pos, std::ios::beg);
    return false;
}

ASSSubtitle::ASSSubtitle() : stylesCount(0) {}

int64_t ASSSubtitle::parseTime(const std::string& timeStr) {
    std::cerr << "[DEBUG] parseTime: raw time string = '" << timeStr << "'" << std::endl;

    std::string trimmed = timeStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    int h, m, s, ms;
    int parsed = std::sscanf(trimmed.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms);
    if (parsed != 4) {
        std::cerr << "[DEBUG] parseTime: failed to parse time string '" << trimmed << "'" << std::endl;
        throw std::runtime_error("Invalid time format: " + timeStr);
    }
    int64_t totalMs = ((h * 60 + m) * 60 + s) * 1000 + ms;
    std::cerr << "[DEBUG] parseTime: " << h << ":" << m << ":" << s << "." << ms
              << " => " << totalMs << " ms" << std::endl;
    return totalMs;
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

void ASSSubtitle::parseScriptInfo(std::ifstream& in) {
    std::cerr << "[DEBUG] parseScriptInfo: entering section [Script Info]" << std::endl;

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

            std::cerr << "[DEBUG] parseScriptInfo: key='" << key << "', value='" << value << "'" << std::endl;

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
    std::cerr << "[DEBUG] parseStyles: entering section [V4+ Styles]" << std::endl;

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

            std::cerr << "[DEBUG] parseStyles: found style line = '" << trimmed << "'" << std::endl;

            if (parsed < 24) {
                std::cerr << "[DEBUG] parseStyles: incomplete style string '" << trimmed << "'" << std::endl;
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
            else {
                std::cerr << "[DEBUG] parseStyles: reached MAX_STYLES limit" << std::endl;
            }
        }
    }
}

void ASSSubtitle::parseDialogue(const std::string &line) {
    if (line.find("Dialogue:") != 0) {
        return; // Игнорируем строки, которые не начинаются с "Dialogue:"
    }

    std::string after = line.substr(9); // Пропускаем "Dialogue:"
    after.erase(0, after.find_first_not_of(" \t\r\n")); // Убираем пробелы и переносы

    std::vector<std::string> fields;
    std::stringstream ss(after);
    std::string field;

    // Разделяем строку по запятым
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }

    if (fields.size() < 10) {
        return; // Пропускаем строки с недостаточным количеством полей
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

        // Текст - всё, что идёт после 9-го поля
        dlg.text = fields[9];
        for (size_t i = 10; i < fields.size(); ++i) {
            dlg.text += "," + fields[i];
        }

        // Убираем форматирующие теги ASS
        dlg.text = std::regex_replace(dlg.text, std::regex("\\\\N"), " ");       // Убираем разрывы строк
        dlg.text = std::regex_replace(dlg.text, std::regex("\\{[^}]*\\}"), ""); // Убираем теги формата ASS

        // Конвертируем время
        int64_t startMs = parseTime(dlg.start);
        int64_t endMs = parseTime(dlg.end);

        SubtitleEntry entry;
        entry.start_ms = startMs;
        entry.end_ms = endMs;
        entry.text = dlg.text;

        entries.push_back(entry);

        // Выводим только успешный текст
        std::cerr << dlg.text << std::endl;
    } catch (...) {
        // Игнорируем любые ошибки в парсинге
    }
}

void ASSSubtitle::parseEvents(std::ifstream& in) {
    std::cerr << "[DEBUG] parseEvents: entering section [Events]" << std::endl;

    std::string line;
    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;
        if (line.front() == '[') {
            std::cerr << "[DEBUG] parseEvents: Exiting section due to new block: " << line << std::endl;
            break;
        }

        if (line.find("Dialogue:") == 0) {
            parseDialogue(line);
        } else {
            std::cerr << "[DEBUG] parseEvents: Skipping non-dialogue line: " << line << std::endl;
        }
    }
}

void ASSSubtitle::read(const std::string& filename) {
    std::cerr << "[DEBUG] read: opening file '" << filename << "'" << std::endl;

    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    if (hasUtf8BOM(in)) {
        std::cerr << "[DEBUG] UTF-8 BOM detected and skipped." << std::endl;
    }

    std::string line;
    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        std::cerr << "[DEBUG] read: line='" << line << "'" << std::endl;

        if (line == "[Script Info]") {
            parseScriptInfo(in);
        } else if (line == "[V4+ Styles]") {
            parseStyles(in);
        } else if (line == "[Events]") {
            parseEvents(in);
        }
    }

    std::cerr << "[DEBUG] read: finished, total entries=" << entries.getSize() << std::endl;
    if (entries.getSize() == 0) {
        std::cerr << "[DEBUG] WARNING: No entries parsed. Check the file format or content." << std::endl;
    }
}
void ASSSubtitle::write(const std::string& filename) const {
    std::cerr << "[DEBUG] write: writing file '" << filename << "'" << std::endl;
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot write file: " + filename);
    }
    std::cerr << "[DEBUG] write: entries size = " << entries.getSize() << std::endl;

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

SubtitleEntryList& ASSSubtitle::getEntries() {
    return entries;
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