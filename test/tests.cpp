#include <gtest/gtest.h>
#include "SRTSubtitle.h"
#include "SAMISubtitle.h"
#include "ASSSubtitle.h"
#include "VTTSubtitle.h"
#include <fstream>

// Utility to compare two files line by line
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1);
    std::ifstream f2(file2);

    if (!f1.is_open() || !f2.is_open()) {
        return false; // Return false if either file cannot be opened
    }

    std::string line1, line2;
    while (true) {
        bool hasLine1 = static_cast<bool>(std::getline(f1, line1));
        bool hasLine2 = static_cast<bool>(std::getline(f2, line2));

        if (!hasLine1 && !hasLine2) {
            return true; // Both files ended, files are identical
        }

        if (!hasLine1 || !hasLine2 || line1 != line2) {
            return false; // Lines differ
        }
    }
}

// ==== SRT ====

TEST(SubtitleTest, Test13_SRT) {
    SRTSubtitle sub;
    sub.read("../../test/srcSUBs/Test13.srt");
    sub.write("../../test/OutPutSUBs/Test13_out.srt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test13_out.srt", "../../test/refSUBs/Test13.srt"));
}

TEST(SubtitleTest, Test14_SRT) {
    SRTSubtitle sub;
    sub.read("../../test/srcSUBs/Test14.srt");
    sub.write("../../test/OutPutSUBs/Test14_out.srt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test14_out.srt", "../../test/refSUBs/Test14.srt"));
}

TEST(SubtitleTest, Test15_SRT) {
    SRTSubtitle sub;
    sub.read("../../test/srcSUBs/Test15.srt");
    sub.write("../../test/OutPutSUBs/Test15_out.srt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test15_out.srt", "../../test/refSUBs/Test15.srt"));
}

TEST(SubtitleTest, Test16_SRT) {
    SRTSubtitle sub;
    sub.read("../../test/srcSUBs/Test16.srt");
    sub.write("../../test/OutPutSUBs/Test16_out.srt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test16_out.srt", "../../test/refSUBs/Test16.srt"));
}

// ==== VTT ====

TEST(SubtitleTest, Test2_VTT) {
    VTTSubtitle sub;
    sub.read("../../test/srcSUBs/Test2.vtt", true);
    sub.write("../../test/OutPutSUBs/Test2_out.vtt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test2_out.vtt", "../../test/refSUBs/Test2.vtt"));
}

TEST(SubtitleTest, Test3_VTT) {
    VTTSubtitle sub;
    sub.read("../../test/srcSUBs/Test3.vtt", true);
    sub.write("../../test/OutPutSUBs/Test3_out.vtt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test3_out.vtt", "../../test/refSUBs/Test3.vtt"));
}

TEST(SubtitleTest, Test4_VTT) {
    VTTSubtitle sub;
    sub.read("../../test/srcSUBs/Test4.vtt", true);
    sub.write("../../test/OutPutSUBs/Test4_out.vtt");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test4_out.vtt", "../../test/refSUBs/Test4.vtt"));
}

// ==== SAMI (.smi) ====

TEST(SubtitleTest, Test5_SMI) {
    SAMISubtitle sub;
    sub.read("../../test/srcSUBs/Test5.smi");
    sub.write("../../test/OutPutSUBs/Test5_out.smi");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test5_out.smi", "../../test/refSUBs/Test5.smi"));
}

TEST(SubtitleTest, Test6_SMI) {
    SAMISubtitle sub;
    sub.read("../../test/srcSUBs/Test6.smi");
    sub.write("../../test/OutPutSUBs/Test6_out.smi");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test6_out.smi", "../../test/refSUBs/Test6.smi"));
}

TEST(SubtitleTest, Test7_SMI) {
    SAMISubtitle sub;
    sub.read("../../test/srcSUBs/Test7.smi");
    sub.write("../../test/OutPutSUBs/Test7_out.smi");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test7_out.smi", "../../test/refSUBs/Test7.smi"));
}

TEST(SubtitleTest, Test8_SMI) {
    SAMISubtitle sub;
    sub.read("../../test/srcSUBs/Test8.smi");
    sub.write("../../test/OutPutSUBs/Test8_out.smi");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test8_out.smi", "../../test/refSUBs/Test8.smi"));
}

// ==== ASS ====

TEST(SubtitleTest, TestRFormat10_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestRFormat10.ass");
    sub.write("../../test/OutPutSUBs/TestRFormat10_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestRFormat10_out.ass", "../../test/refSUBs/TestRFormat10.ass"));
}

TEST(SubtitleTest, TestRFormat11_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestRFormat11.ass");
    sub.write("../../test/OutPutSUBs/TestRFormat11_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestRFormat11_out.ass", "../../test/refSUBs/TestRFormat11.ass"));
}

TEST(SubtitleTest, TestRFormat12_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestRFormat12.ass");
    sub.write("../../test/OutPutSUBs/TestRFormat12_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestRFormat12_out.ass", "../../test/refSUBs/TestRFormat12.ass"));
}

TEST(SubtitleTest, TestTime_5s_9_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestTime_5s_9.ass");
    sub.write("../../test/OutPutSUBs/TestTime_5s_9_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestTime_5s_9_out.ass", "../../test/refSUBs/TestTime_5s_9.ass"));
}

TEST(SubtitleTest, TestTime_5s_10_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestTime_5s_10.ass");
    sub.write("../../test/OutPutSUBs/TestTime_5s_10_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestTime_5s_10_out.ass", "../../test/refSUBs/TestTime_5s_10.ass"));
}

TEST(SubtitleTest, TestTime_5s_11_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestTime_5s_11.ass");
    sub.write("../../test/OutPutSUBs/TestTime_5s_11_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestTime_5s_11_out.ass", "../../test/refSUBs/TestTime_5s_11.ass"));
}

TEST(SubtitleTest, TestTime_5s_12_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestTime_5s_12.ass");
    sub.write("../../test/OutPutSUBs/TestTime_5s_12_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestTime_5s_12_out.ass", "../../test/refSUBs/TestTime_5s_12.ass"));
}

TEST(SubtitleTest, Test9_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/Test9.ass");
    sub.write("../../test/OutPutSUBs/Test9_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test9_out.ass", "../../test/refSUBs/Test9.ass"));
}

TEST(SubtitleTest, Test10_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/Test10.ass");
    sub.write("../../test/OutPutSUBs/Test10_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test10_out.ass", "../../test/refSUBs/Test10.ass"));
}

TEST(SubtitleTest, Test11_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/Test11.ass");
    sub.write("../../test/OutPutSUBs/Test11_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test11_out.ass", "../../test/refSUBs/Test11.ass"));
}

TEST(SubtitleTest, Test12_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/Test12.ass");
    sub.write("../../test/OutPutSUBs/Test12_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/Test12_out.ass", "../../test/refSUBs/Test12.ass"));
}

TEST(SubtitleTest, TestFormat9_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestFormat9.ass");
    sub.write("../../test/OutPutSUBs/TestFormat9_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestFormat9_out.ass", "../../test/refSUBs/TestFormat9.ass"));
}

TEST(SubtitleTest, TestRFormat9_ASS) {
    ASSSubtitle sub;
    sub.read("../../test/srcSUBs/TestRFormat9.ass");
    sub.write("../../test/OutPutSUBs/TestRFormat9_out.ass");
    ASSERT_TRUE(compareFiles("../../test/OutPutSUBs/TestRFormat9_out.ass", "../../test/refSUBs/TestRFormat9.ass"));
}

// Entry point for Google Test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
