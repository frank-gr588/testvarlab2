#include <gtest/gtest.h>
#include "ASSSubtitle.h"
#include "SAMISubtitle.h"
#include "SRTSubtitle.h"
#include "SubtitleEntry.h"
#include "SubtitleEntryList.h"
#include "VTTSubtitle.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Helper function to create a temporary test file
void createTempFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    outfile << content;
    outfile.close();
}

// Helper function to read the content of a file into a string
std::string readFileContent(const std::string& filename) {
    std::ifstream infile(filename);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    return buffer.str();
}

// ------------------------- ASSSubtitle Tests -------------------------

TEST(ASSSubtitleTest, ReadValidFile) {
    std::string testFile = "test.ass";
    std::string fileContent =
        "[Script Info]\n"
        "Title: Test Subtitle\n"
        "[Events]\n"
        "Dialogue: 0,0:00:01.00,0:00:03.00,Default,,0,0,0,,Hello, World!\\NHow are you?\n";

    createTempFile(testFile, fileContent);

    ASSSubtitle subtitle;
    ASSERT_NO_THROW(subtitle.read(testFile));

    const auto& entries = subtitle.getEntries();
    ASSERT_EQ(entries.getSize(), 1);
    EXPECT_EQ(entries[0].text, "Hello, World!\nHow are you?");

    // Cleanup
    std::remove(testFile.c_str());
}

TEST(ASSSubtitleTest, ReadInvalidFormat) {
    std::string testFile = "invalid.ass";
    std::string content = "INVALID DATA";

    createTempFile(testFile, content);

    ASSSubtitle subtitle;
    EXPECT_THROW(subtitle.read(testFile), std::runtime_error);

    // Cleanup
    std::remove(testFile.c_str());
}

TEST(ASSSubtitleTest, WriteValidFile) {
    std::string outputFilename = "output.ass";

    ASSSubtitle subtitle;
    SubtitleEntry entry = {1000, 3000, "Test text"};
    subtitle.getEntries().push_back(entry);

    ASSERT_NO_THROW(subtitle.write(outputFilename));
    std::string writtenContent = readFileContent(outputFilename);

    EXPECT_NE(writtenContent.find("Test text"), std::string::npos);

    // Cleanup
    std::remove(outputFilename.c_str());
}

TEST(ASSSubtitleTest, RemoveFormatting) {
    ASSSubtitle subtitle;

    SubtitleEntry entry1 = {0, 2000, "Hello, {\\b1}World!\\NHow are you?"};
    SubtitleEntry entry2 = {2000, 4000, "This is a {\\i1}test{\\i0}."};
    subtitle.getEntries().push_back(entry1);
    subtitle.getEntries().push_back(entry2);

    subtitle.removeFormatting();

    EXPECT_EQ(subtitle.getEntries()[0].text, "Hello, World! How are you?");
    EXPECT_EQ(subtitle.getEntries()[1].text, "This is a test.");
}

TEST(ASSSubtitleTest, ShiftTimeForward) {
    ASSSubtitle subtitle;

    SubtitleEntry entry1 = {1000, 3000, "Text 1"};
    SubtitleEntry entry2 = {4000, 6000, "Text 2"};
    subtitle.getEntries().push_back(entry1);
    subtitle.getEntries().push_back(entry2);

    subtitle.shiftTime(1000, START_END);

    EXPECT_EQ(subtitle.getEntries()[0].start_ms, 2000);
    EXPECT_EQ(subtitle.getEntries()[0].end_ms, 4000);
    EXPECT_EQ(subtitle.getEntries()[1].start_ms, 5000);
    EXPECT_EQ(subtitle.getEntries()[1].end_ms, 7000);
}

TEST(ASSSubtitleTest, ShiftTimeBackward) {
    ASSSubtitle subtitle;

    SubtitleEntry entry1 = {2000, 4000, "Text 1"};
    SubtitleEntry entry2 = {5000, 7000, "Text 2"};
    subtitle.getEntries().push_back(entry1);
    subtitle.getEntries().push_back(entry2);

    subtitle.shiftTime(-1000, START_END);

    EXPECT_EQ(subtitle.getEntries()[0].start_ms, 1000);
    EXPECT_EQ(subtitle.getEntries()[0].end_ms, 3000);
    EXPECT_EQ(subtitle.getEntries()[1].start_ms, 4000);
    EXPECT_EQ(subtitle.getEntries()[1].end_ms, 6000);
}

// ------------------------- SAMISubtitle Tests -------------------------

TEST(SAMISubtitleTest, ReadValidFile) {
    SAMISubtitle sami;
    std::string testFile = "test.smi";
    std::string content =
        "<SAMI>\n"
        "<BODY>\n"
        "<SYNC Start=0><P>Hello</P></SYNC>\n"
        "<SYNC Start=1000><P>World</P></SYNC>\n"
        "</BODY>\n"
        "</SAMI>";

    createTempFile(testFile, content);
    ASSERT_NO_THROW(sami.read(testFile));

    const auto& entries = sami.getEntries();
    ASSERT_EQ(entries.getSize(), 2);
    EXPECT_EQ(entries[0].text, "Hello");
    EXPECT_EQ(entries[1].text, "World");

    // Cleanup
    std::remove(testFile.c_str());
}

TEST(SAMISubtitleTest, RemoveFormatting) {
    SAMISubtitle sami;

    SubtitleEntry entry1 = {0, 2000, "<b>Hello</b> World!"};
    SubtitleEntry entry2 = {2000, 4000, "This is a <i>test</i>."};
    sami.getEntries().push_back(entry1);
    sami.getEntries().push_back(entry2);

    sami.removeFormatting();

    EXPECT_EQ(sami.getEntries()[0].text, "Hello World!");
    EXPECT_EQ(sami.getEntries()[1].text, "This is a test.");
}

// ------------------------- SRTSubtitle Tests -------------------------

TEST(SRTSubtitleTest, ShiftTime) {
    SRTSubtitle srt;
    std::string testFile = "test.srt";
    std::string content =
        "1\n"
        "00:00:01,000 --> 00:00:02,000\n"
        "Hello\n\n"
        "2\n"
        "00:00:03,000 --> 00:00:04,000\n"
        "World\n";

    createTempFile(testFile, content);
    ASSERT_NO_THROW(srt.read(testFile));

    const auto& entries = srt.getEntries();
    ASSERT_EQ(entries.getSize(), 2);
    EXPECT_EQ(entries[0].text, "Hello");
    EXPECT_EQ(entries[1].text, "World");

    srt.shiftTime(1000, START_END);
    EXPECT_EQ(entries[0].start_ms, 2000);
    EXPECT_EQ(entries[1].start_ms, 4000);

    // Cleanup
    std::remove(testFile.c_str());
}

// ------------------------- VTTSubtitle Tests -------------------------

TEST(VTTSubtitleTest, ReadWrite) {
    VTTSubtitle vtt;
    std::string testFile = "test.vtt";
    std::string content =
        "WEBVTT\n\n"
        "00:00:01.000 --> 00:00:02.000\n"
        "Hello\n\n"
        "00:00:03.000 --> 00:00:04.000\n"
        "World\n";

    createTempFile(testFile, content);
    ASSERT_NO_THROW(vtt.read(testFile));

    const auto& entries = vtt.getEntries();
    ASSERT_EQ(entries.getSize(), 2);
    EXPECT_EQ(entries[0].text, "Hello");
    EXPECT_EQ(entries[1].text, "World");

    // Cleanup
    std::remove(testFile.c_str());
}

// ------------------------- Main -------------------------

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}