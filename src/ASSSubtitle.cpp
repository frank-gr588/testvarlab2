#include "ASSSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <cstring>

// Константы для ограничения количества элементов
#define MAX_ENTRIES 1000
#define MAX_STYLES 100

// Статические массивы для хранения записей и стилей
SubtitleEntry entries[MAX_ENTRIES];
Style styles[MAX_STYLES];

// Счётчики для текущего количества элементов
int entriesCount = 0;
int stylesCount = 0;

// Проверка на наличие UTF-8 BOM
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

// Конструктор
ASSSubtitle::ASSSubtitle() {}

// Парсинг времени из строки
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

// Форматирование времени в строку
std::string ASSSubtitle::formatTime(int64_t ms) const {
    int h = static_cast<int>(ms / 3600000);
    ms %= 3600000;
    int m = static_cast<int>(ms / 60000);
    ms %= 60000;
    int s = static_cast<int>(ms / 1000);
    ms %= 1000;
    int cs = static_cast<int>(ms / 10);

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d:%02d.%02d", h, m, s, cs);
    return std::string(buffer);
}

// Парсинг информации скрипта
void ASSSubtitle::parseScriptInfo(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line.front() == '[') break;

        auto posColon = line.find(':');
        if (posColon != std::string::npos) {
            std::string key = line.substr(0, posColon);
            std::string value = line.substr(posColon + 1);
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

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

// Парсинг стилей
void ASSSubtitle::parseStyles(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line.front() == '[') break;

        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.rfind("Style:", 0) == 0) {
            Style st;
            char name[100]{}, fontname[100]{}, primaryColour[100]{}, secondaryColour[100]{};
            char outlineColour[100]{}, backColour[100]{};

            int parsed = std::sscanf(
                line.c_str(),
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

// Парсинг диалогов
void ASSSubtitle::parseDialogue(const std::string& line) {
    if (line.find("Dialogue:") != 0) return;

    std::string after = line.substr(9);
    after.erase(0, after.find_first_not_of(" \t\r\n"));

    const int maxFields = 10;
    std::string fields[maxFields];
    int fieldCount = 0;

    size_t start = 0, end;
    while ((end = after.find(',', start)) != std::string::npos && fieldCount < maxFields) {
        fields[fieldCount++] = after.substr(start, end - start);
        start = end + 1;
    }
    if (start < after.size() && fieldCount < maxFields) {
        fields[fieldCount++] = after.substr(start);
    }

    if (fieldCount < 10) return;

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

    dlg.text = std::regex_replace(dlg.text, std::regex("\\\\N"), " ");
    dlg.text = std::regex_replace(dlg.text, std::regex("\\{[^}]*\\}"), "");

    int64_t startMs = parseTime(dlg.start);
    int64_t endMs = parseTime(dlg.end);

    if (entriesCount < MAX_ENTRIES) {
        entries[entriesCount++] = {startMs, endMs, dlg.text};
    }
}

// Парсинг событий
void ASSSubtitle::parseEvents(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty() || line.front() == '[') break;

        if (line.find("Dialogue:") == 0) {
            parseDialogue(line);
        }
    }
}

// Чтение субтитров из файла
void ASSSubtitle::read(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    if (hasUtf8BOM(in)) {
        in.seekg(3);
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

// Запись субтитров в файл
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
    for (int i = 0; i < entriesCount; ++i) {
        const SubtitleEntry &e = entries[i];
        out << "Dialogue: 0,"
            << formatTime(e.start_ms) << ","
            << formatTime(e.end_ms)
            << ",Default,,0,0,0,,"
            << e.text
            << "\n";
    }
}