#pragma once
#include <string>

struct SubtitleEntry {
    int64_t start_ms;
    int64_t end_ms;
    std::string text;
    std::string formatting;  // Новое поле для хранения форматирования
    int x1, x2, y1, y2;      // Новые поля для хранения координат
    bool has_coordinates;    // Флаг для проверки наличия координат

    SubtitleEntry() : x1(0), x2(0), y1(0), y2(0), has_coordinates(false) {}
};