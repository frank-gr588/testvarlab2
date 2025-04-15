#include "SRTSubtitle.h"
#include "SAMISubtitle.h"
#include "ASSSubtitle.h"
#include "VTTSubtitle.h"
#include <iostream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: converter_subs <in_file> <out_file>\n";
        return 1;
    }

    std::string inFile = argv[1];
    std::string outFile = argv[2];

    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Input file path: " << inFile << std::endl;

    try {
        SRTSubtitle srtSubs;
        SAMISubtitle samiSubs;
        ASSSubtitle assSubs;
        VTTSubtitle vttSubs;

        std::string inExtension = inFile.substr(inFile.find_last_of(".") + 1);
        std::string outExtension = outFile.substr(outFile.find_last_of(".") + 1);

        if (inExtension == "srt") {
            srtSubs.read(inFile);
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
            vttSubs.read(inFile);
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