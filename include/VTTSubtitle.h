#pragma once

#include "SubtitleEntryList.h"
#include "SRTSubtitle.h"
#include <string>
#include <vector>

// Структура для хранения заметок
struct VTTNote {
    std::string content; // Текст заметки
};

class VTTSubtitle {
private:
    SubtitleEntryList entries;
    std::vector<VTTNote> notes; // Список заметок

    static int64_t parseTime(const std::string& timeStr); // Конвертирует строку времени "00:01:02.345" в миллисекунды
    static std::string formatTime(int64_t ms);           // Конвертирует миллисекунды в строку времени "00:01:02.345"

public:
    void read(const std::string& filename);              // Читает VTT-файл
    void write(const std::string& filename) const;       // Пишет VTT-файл
    SubtitleEntryList& getEntries();                     // Возвращает список субтитров
    const std::vector<VTTNote>& getNotes() const;        // Возвращает список заметок

    void removeFormatting();                             // Удаляет HTML-теги из текста субтитров
    void addDefaultStyle(const std::string& style);      // Добавляет стиль к каждому тексту
    void shiftTime(int64_t delta_ms, TimeShiftType type);// Сдвигает временные метки
};