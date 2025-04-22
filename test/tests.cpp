#include <gtest/gtest.h>
#include "SRTSubtitle.h"
#include "SAMISubtitle.h"
#include "ASSSubtitle.h"
#include "VTTSubtitle.h"
#include <fstream>

// Утилита для сравнения файлов
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }

    std::string line1, line2;
    while (true) {
        bool hasLine1 = static_cast<bool>(std::getline(f1, line1));
        bool hasLine2 = static_cast<bool>(std::getline(f2, line2));

        if (!hasLine1 && !hasLine2) {
            return true; // Оба файла закончились
        }

        if (!hasLine1 || !hasLine2 || line1 != line2) {
            return false; // Линии не совпадают
        }
    }
}

// Тест для проверки чтения и записи SRT
TEST(SRTSubtitleTest, ReadWriteTest) {
    SRTSubtitle srtSubtitle;
    srtSubtitle.read("test/srcSRT/example.srt");
    srtSubtitle.write("test/OutPutSRT/example_out.srt");

    ASSERT_TRUE(compareFiles("test/OutPutSRT/example_out.srt", "test/refSRT/example.srt"))
        << "SRT read/write test failed!";
}

// Тест для проверки сдвига времени
TEST(SRTSubtitleTest, ShiftTimeTest) {
    SRTSubtitle srtSubtitle;
    srtSubtitle.read("test/srcSRT/example.srt");
    srtSubtitle.shiftTime(1000, START_END); // Сдвигаем на 1000 мс

    srtSubtitle.write("test/OutPutSRT/example_shifted.srt");

    ASSERT_TRUE(compareFiles("test/OutPutSRT/example_shifted.srt", "test/refSRT/example_shifted.srt"))
        << "SRT shift time test failed!";
}

// Тест для проверки удаления форматирования
TEST(SRTSubtitleTest, RemoveFormattingTest) {
    SRTSubtitle srtSubtitle;
    srtSubtitle.read("test/srcSRT/example.srt");
    srtSubtitle.removeFormatting(); // Удаляем форматирование

    srtSubtitle.write("test/OutPutSRT/example_no_format.srt");

    ASSERT_TRUE(compareFiles("test/OutPutSRT/example_no_format.srt", "test/refSRT/example_no_format.srt"))
        << "SRT remove formatting test failed!";
}

// Тест для проверки конверсии SRT -> VTT
TEST(SRTtoVTTTest, ConversionTest) {
    SRTSubtitle srtSubtitle;
    VTTSubtitle vttSubtitle;

    srtSubtitle.read("test/srcSRT/example.srt");
    vttSubtitle.getEntries() = srtSubtitle.getEntries(); // Копируем записи
    vttSubtitle.write("test/OutPutVTT/example.vtt");

    ASSERT_TRUE(compareFiles("test/OutPutVTT/example.vtt", "test/refVTT/example.vtt"))
        << "SRT to VTT conversion test failed!";
}

// Тест для проверки конверсии VTT -> ASS
TEST(VTTtoASSTest, ConversionTest) {
    VTTSubtitle vttSubtitle;
    ASSSubtitle assSubtitle;

    vttSubtitle.read("test/srcVTT/example.vtt", true); // Сохраняем заметки
    assSubtitle.getEntries() = vttSubtitle.getEntries(); // Копируем записи
    assSubtitle.write("test/OutPutASS/example.ass");

    ASSERT_TRUE(compareFiles("test/OutPutASS/example.ass", "test/refASS/example.ass"))
        << "VTT to ASS conversion test failed!";
}

// Тест для проверки конверсии SMI -> SRT
TEST(SMItoSRTTest, ConversionTest) {
    SAMISubtitle samiSubtitle;
    SRTSubtitle srtSubtitle;

    samiSubtitle.read("test/srcSMI/example.smi");
    srtSubtitle.getEntries() = samiSubtitle.getEntries(); // Копируем записи
    srtSubtitle.write("test/OutPutSRT/example.srt");

    ASSERT_TRUE(compareFiles("test/OutPutSRT/example.srt", "test/refSRT/example.srt"))
        << "SMI to SRT conversion test failed!";
}

// Точка входа для Google Test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}