#include "ASSSubtitle.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <iostream>
#include <cstring> // Для std::memset и std::strncpy при необходимости

// Функция проверки и пропуска (при наличии) BOM
static bool hasUtf8BOM(std::ifstream &file) {
    // Сохраняем текущую позицию в потоке
    std::streampos pos = file.tellg();

    // Читаем первые три байта, чтобы проверить BOM
    unsigned char bom[3];
    file.read(reinterpret_cast<char*>(bom), 3);

    // Если прочитали ровно 3 байта и они соответствуют "EF BB BF"
    if (file.gcount() == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        return true;
    }

    // Не BOM – восстанавливаем позицию в файле
    file.clear();
    file.seekg(pos, std::ios::beg);
    return false;
}

ASSSubtitle::ASSSubtitle() : stylesCount(0) {}

int64_t ASSSubtitle::parseTime(const std::string& timeStr) {
    // Удаляем возможные пробельные символы в начале и конце
    std::string trimmed = timeStr;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
    trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

    int h, m, s, ms;
    int parsed = std::sscanf(trimmed.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms);
    if (parsed != 4) {
        std::cerr << "Ошибка: недопустимый формат времени в ASS: " << timeStr << std::endl;
        throw std::runtime_error("Invalid time format: " + timeStr);
    }
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
    // Формат H:MM:SS.cc (cc – сотые доли секунды)
    oss << std::setfill('0') << std::setw(1) << h << ":"
        << std::setw(2) << m << ":"
        << std::setw(2) << s << "."
        << std::setw(2) << (ms / 10);
    return oss.str();
}

void ASSSubtitle::parseScriptInfo(std::ifstream& in) {
    std::string line;
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;

        // Если начинается с '[', значит, новая секция
        if (line[0] == '[') {
            in.seekg(pos);
            break;
        }

        size_t posColon = line.find(':');
        if (posColon != std::string::npos) {
            std::string key = line.substr(0, posColon);
            std::string value = line.substr(posColon + 1);

            // Удаляем пробелы
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
    std::cout << "Прочитана секция Script Info: Title = " << scriptInfo.title << std::endl;
}

void ASSSubtitle::parseStyles(std::ifstream& in) {
    std::string line;
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;
        if (line[0] == '[') {
            in.seekg(pos);
            break;
        }

        auto trimFunc = [](std::string &str) {
            str.erase(0, str.find_first_not_of(" \t\r\n"));
            str.erase(str.find_last_not_of(" \t\r\n") + 1);
        };
        trimFunc(line);

        // Проверяем, начинается ли линия со "Style:"
        if (line.rfind("Style:", 0) == 0) {
            Style newStyle;
            char name[100]{}, fontname[100]{}, primaryColour[100]{}, secondaryColour[100]{};
            char outlineColour[100]{}, backColour[100]{};
            int parsed = std::sscanf(
                line.c_str(),
                "Style: %99[^,],%99[^,],%d,%99[^,],%99[^,],%99[^,],%99[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                name,
                fontname,
                &newStyle.fontsize,
                primaryColour,
                secondaryColour,
                outlineColour,
                backColour,
                &newStyle.bold,
                &newStyle.italic,
                &newStyle.underline,
                &newStyle.strikeOut,
                &newStyle.scaleX,
                &newStyle.scaleY,
                &newStyle.spacing,
                &newStyle.angle,
                &newStyle.borderStyle,
                &newStyle.outline,
                &newStyle.shadow,
                &newStyle.alignment,
                &newStyle.marginL,
                &newStyle.marginR,
                &newStyle.marginV,
                &newStyle.encoding
            );

            if (parsed < 24) {
                std::cerr << "Внимание: неполное описание стиля в строке: " << line << std::endl;
                continue;
            }
            newStyle.name = name;
            newStyle.fontname = fontname;
            newStyle.primaryColour = primaryColour;
            newStyle.secondaryColour = secondaryColour;
            newStyle.outlineColour = outlineColour;
            newStyle.backColour = backColour;

            if (stylesCount < MAX_STYLES) {
                styles[stylesCount++] = newStyle;
            } else {
                std::cerr << "Внимание: Достигнуто максимальное количество стилей, стиль "
                          << newStyle.name << " будет проигнорирован.\n";
            }
        }
    }
    std::cout << "Прочитано стилей: " << stylesCount << std::endl;
}

void ASSSubtitle::parseEvents(std::ifstream& in) {
    std::string line;
    int dialogueCount = 0;
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;
        if (line[0] == '[') {
            in.seekg(pos);
            break;
        }

        auto trimFunc = [](std::string &str) {
            str.erase(0, str.find_first_not_of(" \t\r\n"));
            str.erase(str.find_last_not_of(" \t\r\n") + 1);
        };
        trimFunc(line);

        // Проверяем, начинается ли линия с "Dialogue:"
        if (line.rfind("Dialogue:", 0) == 0) {
            parseDialogue(line);
            dialogueCount++;
        } else {
            std::cerr << "Внимание: неизвестный Event: " << line << std::endl;
        }
    }
    std::cout << "Прочитано диалогов (Dialogue): " << dialogueCount << std::endl;
}

void ASSSubtitle::parseDialogue(const std::string& line) {
    Dialogue dlg;
    char start[100]{}, end[100]{}, style[100]{}, name[100]{};
    char marginL[100]{}, marginR[100]{}, marginV[100]{}, effect[100]{};
    char text[2000]{};

    // Очищаем структуру
    std::memset(&dlg, 0, sizeof(dlg));

    int parsed = std::sscanf(
        line.c_str(),
        "Dialogue: %d,%99[^,],%99[^,],%99[^,],%99[^,],%99[^,],%99[^,],%99[^,],%99[^,],%1999[^\n]",
        &dlg.layer,
        start,
        end,
        style,
        name,
        marginL,
        marginR,
        marginV,
        effect,
        text
    );

    if (parsed < 10) {
        std::cerr << "Внимание: не удалось распарсить диалог: " << line << std::endl;
        return;
    }

    dlg.start = start;
    dlg.end = end;
    dlg.style = style;
    dlg.name = name;
    dlg.marginL = marginL;
    dlg.marginR = marginR;
    dlg.marginV = marginV;
    dlg.effect = effect;

    // Заменяем "\N" на пробелы и убираем фигурные скобки со стилями
    std::string cleanedText = std::regex_replace(text, std::regex("\\\\N"), " ");
    dlg.text = std::regex_replace(cleanedText, std::regex("\\{[^}]*\\}"), "");

    try {
        int64_t start_ms = parseTime(dlg.start);
        int64_t end_ms = parseTime(dlg.end);
        SubtitleEntry entry = {start_ms, end_ms, dlg.text};
        entries.push_back(entry);

        std::cout << "Прочитан диалог: Start = " << dlg.start
                  << ", End = " << dlg.end
                  << ", Text = " << dlg.text << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Ошибка парсинга времени для диалога: " << line
                  << "\nИсключение: " << e.what() << std::endl;
    }
}

void ASSSubtitle::read(const std::string& filename) {
    // Открываем файл в двоичном режиме, чтобы была возможность определить BOM
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Не удалось открыть файл: " + filename);
    }

    // Пропускаем BOM (EF BB BF), если он есть
    if (hasUtf8BOM(in)) {
        std::cout << "Обнаружен и пропущен BOM (EF BB BF).\n";
    }

    std::string line;
    // Читаем посрочно
    while (true) {
        std::streampos pos = in.tellg();
        if (!std::getline(in, line)) break;
        if (line.empty()) continue;

        if (line == "[Script Info]") {
            parseScriptInfo(in);
        } else if (line == "[V4+ Styles]") {
            parseStyles(in);
        } else if (line == "[Events]") {
            parseEvents(in);
        } else {
            // Если это неизвестная секция вида "[...]", вернемся назад
            if (line[0] == '[') {
                in.seekg(pos);
                break;
            }
            // Иногда строки "Dialogue:" встречаются вне [Events]
            if (line.find("Dialogue:") != std::string::npos) {
                parseDialogue(line);
            } else {
                std::cerr << "Warning: Unknown section or line: " << line << std::endl;
            }
        }
    }

    std::cout << "Всего распарсенных субтитров (entries): " << entries.getSize() << std::endl;
}

void ASSSubtitle::write(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Не удалось записать в файл: " + filename);
    }

    // [Script Info]
    out << "[Script Info]\n";
    out << "Title: " << scriptInfo.title << "\n";
    out << "Original Script: " << scriptInfo.originalScript << "\n";
    out << "Original Translation: " << scriptInfo.originalTranslation << "\n";

    // [V4+ Styles]
    out << "\n[V4+ Styles]\n";
    for (int i = 0; i < stylesCount; ++i) {
        const Style &s = styles[i];
        out << "Style: " << s.name << "," << s.fontname << "," << s.fontsize << ","
            << s.primaryColour << "," << s.secondaryColour << "," << s.outlineColour << ","
            << s.backColour << "," << s.bold << "," << s.italic << "," << s.underline << ","
            << s.strikeOut << "," << s.scaleX << "," << s.scaleY << "," << s.spacing << ","
            << s.angle << "," << s.borderStyle << "," << s.outline << "," << s.shadow << ","
            << s.alignment << "," << s.marginL << "," << s.marginR << "," << s.marginV << ","
            << s.encoding << "\n";
    }

    // [Events]
    out << "\n[Events]\n";
    for (size_t i = 0; i < entries.getSize(); ++i) {
        const SubtitleEntry &e = entries[i];
        out << "Dialogue: 0," << formatTime(e.start_ms) << "," << formatTime(e.end_ms)
            << ",Default,,0,0,0,," << e.text << "\n";
    }
}

SubtitleEntryList& ASSSubtitle::getEntries() {
    return entries;
}

void ASSSubtitle::removeFormatting() {
    // Пример: удаление HTML-тегов
    std::regex removeTags("<[^>]*>");
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = std::regex_replace(entries[i].text, removeTags, "");
    }
}

void ASSSubtitle::addDefaultStyle(const std::string& style) {
    // Оборачиваем текст в <style>...</style> (пример, если нужно)
    for (size_t i = 0; i < entries.getSize(); ++i) {
        entries[i].text = "<" + style + ">" + entries[i].text + "</" + style + ">";
    }
}

void ASSSubtitle::shiftTime(int64_t delta_ms, TimeShiftType type) {
    // Сдвигаем времена начала или конца, в зависимости от типа
    for (size_t i = 0; i < entries.getSize(); ++i) {
        if (type == START_END || type == START_ONLY) {
            entries[i].start_ms += delta_ms;
        }
        if (type == START_END || type == END_ONLY) {
            entries[i].end_ms += delta_ms;
        }
    }
}