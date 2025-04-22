#include "SRTSubtitle.h"
#include "SAMISubtitle.h"
#include "ASSSubtitle.h"
#include "VTTSubtitle.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <stdexcept>


int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: converter_subs <in_file> <out_file> [options]\n";
        std::cerr << "Options:\n";
        std::cerr << "  --shift-time <ms>        Shift subtitles by <ms> milliseconds.\n";
        std::cerr << "  --remove-formatting      Remove formatting from subtitles.\n";
        std::cerr << "  --add-style <styleName>  Add a style to the subtitles.\n";
        return 1;
    }

    std::string inFile = argv[1];
    std::string outFile = argv[2];

    int64_t shiftTimeMs = 0;
    bool removeFormatting = false;
    std::string addStyle;

    // Parse optional arguments
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "--shift-time" && i + 1 < argc) {
            shiftTimeMs = std::stoll(argv[++i]);
        } else if (std::string(argv[i]) == "--remove-formatting") {
            removeFormatting = true;
        } else if (std::string(argv[i]) == "--add-style" && i + 1 < argc) {
            addStyle = argv[++i];
        }
    }

    try {
        SRTSubtitle srtSubs;
        SAMISubtitle samiSubs;
        ASSSubtitle assSubs;
        VTTSubtitle vttSubs;

        std::string inExtension = inFile.substr(inFile.find_last_of(".") + 1);
        std::string outExtension = outFile.substr(outFile.find_last_of(".") + 1);

        bool keepNotes = (inExtension == "vtt" && outExtension == "vtt");

        // Determine the input format
        if (inExtension == "srt") {
            srtSubs.read(inFile);

            // Apply optional operations only if specified
            if (shiftTimeMs != 0) {
                srtSubs.shiftTime(shiftTimeMs, START_END);
            }
            if (removeFormatting) {
                srtSubs.removeFormatting();
            }
            if (!addStyle.empty()) {
                srtSubs.addDefaultStyle(addStyle);
            }

            // Write output
            if (outExtension == "smi") {
                samiSubs.getEntries() = srtSubs.getEntries();
                samiSubs.write(outFile);
            } else if (outExtension == "ass" || outExtension == "ssa") {
                assSubs.getEntries() = srtSubs.getEntries();
                assSubs.write(outFile);
            } else if (outExtension == "vtt") {
                vttSubs.getEntries() = srtSubs.getEntries();
                vttSubs.write(outFile);
            } else {
                srtSubs.write(outFile);
            }
        } else if (inExtension == "smi") {
            samiSubs.read(inFile);

            // Apply optional operations only if specified
            if (shiftTimeMs != 0) {
                samiSubs.shiftTime(shiftTimeMs, START_END);
            }
            if (removeFormatting) {
                samiSubs.removeFormatting();
            }
            if (!addStyle.empty()) {
                samiSubs.addDefaultStyle(addStyle);
            }

            // Write output
            if (outExtension == "srt") {
                srtSubs.getEntries() = samiSubs.getEntries();
                srtSubs.write(outFile);
            } else if (outExtension == "ass" || outExtension == "ssa") {
                assSubs.getEntries() = samiSubs.getEntries();
                assSubs.write(outFile);
            } else if (outExtension == "vtt") {
                vttSubs.getEntries() = samiSubs.getEntries();
                vttSubs.write(outFile);
            } else {
                samiSubs.write(outFile);
            }
        } else if (inExtension == "ass" || inExtension == "ssa") {
            assSubs.read(inFile);

            // Apply optional operations only if specified
            if (shiftTimeMs != 0) {
                assSubs.shiftTime(shiftTimeMs, START_END);
            }
            if (removeFormatting) {
                assSubs.removeFormatting();
            }
            if (!addStyle.empty()) {
                assSubs.addDefaultStyle(addStyle);
            }

            // Write output
            if (outExtension == "srt") {
                srtSubs.getEntries() = assSubs.getEntries();
                srtSubs.write(outFile);
            } else if (outExtension == "smi") {
                samiSubs.getEntries() = assSubs.getEntries();
                samiSubs.write(outFile);
            } else if (outExtension == "vtt") {
                vttSubs.getEntries() = assSubs.getEntries();
                vttSubs.write(outFile);
            } else {
                assSubs.write(outFile);
            }
        } else if (inExtension == "vtt") {
            vttSubs.read(inFile, keepNotes);

            // Apply optional operations only if specified
            if (shiftTimeMs != 0) {
                vttSubs.shiftTime(shiftTimeMs, START_END);
            }
            if (removeFormatting) {
                vttSubs.removeFormatting();
            }
            if (!addStyle.empty()) {
                // Only allow certain styles in VTT
                if (addStyle == "b" || addStyle == "i" || addStyle == "u" || addStyle == "c") {
                    vttSubs.addDefaultStyle(addStyle);
                } else {
                    std::cerr << "Warning: Style '" << addStyle
                              << "' is not supported in WebVTT. Allowed: b, i, u, c\n";
                }
            }

            // Write output
            if (outExtension == "srt") {
                srtSubs.getEntries() = vttSubs.getEntries();
                srtSubs.write(outFile);
            } else if (outExtension == "smi") {
                samiSubs.getEntries() = vttSubs.getEntries();
                samiSubs.write(outFile);
            } else if (outExtension == "ass" || outExtension == "ssa") {
                assSubs.getEntries() = vttSubs.getEntries();
                assSubs.write(outFile);
            } else {
                vttSubs.write(outFile);
            }
        } else {
            throw std::runtime_error("Unsupported input file format: " + inExtension);
        }

        std::cout << "Conversion complete.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}