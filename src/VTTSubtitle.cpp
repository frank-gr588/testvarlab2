#include "VTTSubtitle.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <iomanip> // Для setw и setfill

// Конвертирует строку времени в миллисекунды
int64_t VTTSubtitle::parseTime(const std::string& timeStr) {
    std::string trimmed = timeStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    int h = 0, m = 0, s = 0, ms = 0;
    int parsed = 0;

    // Попробуем сначала формат hh:mm:ss.sss
    parsed = std::sscanf(trimmed.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms);

    if (parsed < 4) {
        // Если не получилось, попробуем формат mm:ss.sss (без часов)
        h = 0; // Сбросим часы
        parsed = std::sscanf(trimmed.c_str(), "%d:%d.%d", &m, &s, &ms);

        if (parsed < 3) {
            throw std::runtime_error("Invalid time format: " + timeStr);
        }
    }

    return ((h * 60 + m) * 60 + s) * 1000 + ms;
}

// Конвертирует миллисекунды в строку времени
std::string VTTSubtitle::formatTime(int64_t ms) {
    int h = static_cast<int>(ms / 3600000);
    ms %= 3600000;
    int m = static_cast<int>(ms / 60000);
    ms %= 60000;
    int s = static_cast<int>(ms / 1000);
    ms %= 1000;

    std::ostringstream oss;
    oss << h << ":"
        << std::setw(2) << std::setfill('0') << m << ":"
        << std::setw(2) << std::setfill('0') << s << "."
        << std::setw(3) << std::setfill('0') << ms;
    return oss.str();
}

// Чтение VTT-файла
void VTTSubtitle::read(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    std::getline(in, line);
    if (line != "WEBVTT") {
        throw std::runtime_error("Invalid VTT file: Missing WEBVTT header");
    }

    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.empty()) continue;

        // Обработка заметок (NOTE)
        if (line.substr(0, 4) == "NOTE") {
            VTTNote note;
            note.content = line.substr(4); // Сохраняем всё, что после "NOTE"

            // Считываем многострочную заметку
            while (std::getline(in, line) && !line.empty()) {
                note.content += "\n" + line;
            }

            notes.push_back(note);
            continue;
        }

        // Обработка субтитров (временные метки и текст)
        if (line.find("-->") != std::string::npos) {
            std::string startStr = line.substr(0, line.find("-->"));
            std::string endStr = line.substr(line.find("-->") + 3);

            startStr.erase(0, startStr.find_first_not_of(" \t\r\n"));
            startStr.erase(startStr.find_last_not_of(" \t\r\n") + 1);
            endStr.erase(0, endStr.find_first_not_of(" \t\r\n"));
            endStr.erase(endStr.find_last_not_of(" \t\r\n") + 1);

            int64_t startMs = parseTime(startStr);
            int64_t endMs = parseTime(endStr);

            std::string text;
            while (std::getline(in, line) && !line.empty()) {
                if (!text.empty()) text += "\n";
                text += line;
            }

            SubtitleEntry entry;
            entry.start_ms = startMs;
            entry.end_ms = endMs;
            entry.text = text;

            entries.push_back(entry);
        }
    }
}

// Запись VTT-файла
void VTTSubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot write file: " + filename);
    }

    out << "WEBVTT\n\n";

    // Сначала записываем заметки
    for (const auto& note : notes) {
        out << "NOTE " << note.content << "\n\n";
    }

    // Затем записываем субтитры
    for (size_t i = 0; i < entries.getSize(); ++i) {
        const auto& entry = entries[i];
        out << formatTime(entry.start_ms) << " --> " << formatTime(entry.end_ms) << "\n";
        out << entry.text << "\n\n";
    }
}

// Возвращает список субтитров
SubtitleEntryList& VTTSubtitle::getEntries() {
    return entries;
}

// Возвращает список заметок
const std::vector<VTTNote>& VTTSubtitle::getNotes() const {
    return notes;
}

// Удаляет HTML-теги из текста субтитров
void VTTSubtitle::removeFormatting() {
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

// Добавляет стиль к каждому тексту
void VTTSubtitle::addDefaultStyle(const std::string& style) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = "<" + style + ">" + entries[i].text + "</" + style + ">";
    }
}

// Сдвигает временные метки
void VTTSubtitle::shiftTime(int64_t delta_ms, TimeShiftType type) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        if (type == START_ONLY || type == START_END) {
            entries[i].start_ms += delta_ms;
        }
        if (type == END_ONLY || type == START_END) {
            entries[i].end_ms += delta_ms;
        }
    }
}