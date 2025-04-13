#pragma once
#include <string>
struct SubtitleEntry {
    int64_t start_ms;
    int64_t end_ms;
    std::string text;
    std::string formatting;  // Поле для хранения форматирования
    int x1, x2, y1, y2;      // Поля для хранения координат
    bool has_coordinates;    // Флаг для проверки наличия координат

    // Конструктор по умолчанию
    SubtitleEntry()
        : start_ms(0), end_ms(0), x1(0), x2(0), y1(0), y2(0),
          has_coordinates(false) {}

    // Конструктор с параметрами
    SubtitleEntry(int64_t start, int64_t end, const std::string& txt)
        : start_ms(start), end_ms(end), text(txt),
          x1(0), x2(0), y1(0), y2(0), has_coordinates(false) {}
};