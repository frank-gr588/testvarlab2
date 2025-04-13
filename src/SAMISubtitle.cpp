#include "SAMISubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <iostream> // Для диагностики

int64_t SAMISubtitle::parseTime(const std::string& timeStr) {
    try {
        return std::stoll(timeStr);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: invalid argument for stoll: '" << timeStr << "'\n";
        throw;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: out of range argument for stoll: '" << timeStr << "'\n";
        throw;
    }
}

std::string SAMISubtitle::formatTime(int64_t ms) {
    return std::to_string(ms);
}

void SAMISubtitle::read(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) throw std::runtime_error("Cannot open file: " + filename);

    std::string line;
    int64_t previous_end_ms = 0; // Для хранения времени окончания предыдущей строки
    while (std::getline(in, line)) {
        if (line.empty()) continue; // пропускаем пустые строки

        // Пропускаем секции <HEAD>, <STYLE>, и другие
        if (line.find("<HEAD>") != std::string::npos || 
            line.find("<STYLE") != std::string::npos || 
            line.find("<SAMIParam>") != std::string::npos ||
            line.find("<TITLE>") != std::string::npos ||
            line.find("</HEAD>") != std::string::npos ||
            line.find("</STYLE>") != std::string::npos ||
            line.find("</SAMIParam>") != std::string::npos ||
            line.find("</TITLE>") != std::string::npos) {
            continue;
        }

        // Найти <SYNC Start=32940><P>1st block of Taito district, Shin-Ueno line.</P></SYNC>
        size_t syncPos = line.find("<SYNC");
        if (syncPos != std::string::npos) {
            size_t startPos = line.find("Start=", syncPos) + 6;
            size_t endPos = line.find(" ", startPos);
            std::string startStr = line.substr(startPos, endPos - startPos);

            int64_t start_ms = parseTime(startStr);
            int64_t end_ms = 0;

            size_t endSyncPos = line.find("End=", syncPos);
            if (endSyncPos != std::string::npos) {
                endSyncPos += 4;
                size_t endSyncEndPos = line.find(">", endSyncPos);
                std::string endStr = line.substr(endSyncPos, endSyncEndPos - endSyncPos);
                end_ms = parseTime(endStr);
            } else {
                // Если метки End нет, используем начало следующей строки в качестве конца
                end_ms = start_ms;
            }

            size_t pStart = line.find("<P>", endPos) + 3;
            size_t pEnd = line.find("</P>", pStart);
            std::string text = line.substr(pStart, pEnd - pStart);

            // Устанавливаем время окончания предыдущей строки, если оно было равно 0
            if (previous_end_ms == 0) {
                previous_end_ms = start_ms;
            }

            SubtitleEntry entry;
            entry.start_ms = previous_end_ms;
            entry.end_ms = start_ms;
            entry.text = text;

            entries.push_back(entry);
            previous_end_ms = end_ms;
        }
    }

    // Устанавливаем время окончания для последней строки, если необходимо
    if (!entries.getSize() == 0 && previous_end_ms != 0) {
        entries[entries.getSize() - 1].end_ms = previous_end_ms;
    }
}

void SAMISubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot write file: " + filename);

    // Восстанавливаем структуру SAMI файла
    out << "<SAMI>\n<HEAD>\n<TITLE>file</TITLE>\n<SAMIParam>\n  Metrics {time:ms;}\n  Spec {MSFT:1.0;}\n</SAMIParam>\n";
    out << "<STYLE TYPE=\"text/css\">\n<!--\n  P { font-family: Arial; font-weight: normal; color: white; background-color: black; text-align: center; }\n  .ENUSCC { name: English; lang: en-US ; SAMIType: CC ; }\n-->\n</STYLE>\n</HEAD>\n<BODY>\n";
    for (size_t i = 0; i < entries.getSize(); ++i) {
        const SubtitleEntry& e = entries[i];
        out << "<SYNC Start=" << formatTime(e.start_ms) << " End=" << formatTime(e.end_ms) << "><P>" << e.text << "</P></SYNC>\n";
    }
    out << "</BODY>\n</SAMI>\n";
}

SubtitleEntryList& SAMISubtitle::getEntries() {
    return entries;
}

void SAMISubtitle::removeFormatting() {
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

void SAMISubtitle::addDefaultStyle(const std::string& style) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = "<" + style + ">" + entries[i].text + "</" + style + ">";
    }
}

void SAMISubtitle::shiftTime(int64_t delta_ms, TimeShiftType type) {
    for (size_t i = 0; i < entries.getSize(); ++i) {
        if (type == START_END || type == START_ONLY) {
            entries[i].start_ms += delta_ms;
        }
        if (type == START_END || type == END_ONLY) {
            entries[i].end_ms += delta_ms;
        }
    }
}