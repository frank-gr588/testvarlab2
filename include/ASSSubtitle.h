#pragma once
#include "SRTSubtitle.h"
#include "SAMISubtitle.h"
#include "SubtitleEntryList.h"
#include <string>

class ASSSubtitle {
public:
    ASSSubtitle();

    void read(const std::string& filename);
    void write(const std::string& filename) const;

    SubtitleEntryList& getEntries();
    void removeFormatting();
    void addDefaultStyle(const std::string& style);
    void shiftTime(int64_t delta_ms, TimeShiftType type);

private:
    int64_t parseTime(const std::string& timeStr);
    std::string formatTime(int64_t ms);

    void parseScriptInfo(std::ifstream& in);
    void parseStyles(std::ifstream& in);
    void parseEvents(std::ifstream& in);
    void parseDialogue(const std::string& line);

    struct ScriptInfo {
        std::string title;
        std::string originalScript;
        std::string originalTranslation;
        // Добавить другие необходимые поля
    } scriptInfo;

    struct Style {
        std::string name;
        std::string fontname;
        int fontsize;
        std::string primaryColour;
        std::string secondaryColour;
        std::string outlineColour;
        std::string backColour;
        int bold;
        int italic;
        int underline;
        int strikeOut;
        int scaleX;
        int scaleY;
        int spacing;
        int angle;
        int borderStyle;
        int outline;
        int shadow;
        int alignment;
        int marginL;
        int marginR;
        int marginV;
        int encoding;
    } styles[100]; // Максимальное количество стилей
    int stylesCount;

    struct Dialogue {
        int layer;
        std::string start;
        std::string end;
        std::string style;
        std::string name;
        std::string marginL;
        std::string marginR;
        std::string marginV;
        std::string effect;
        std::string text;
    };

    SubtitleEntryList entries;
};