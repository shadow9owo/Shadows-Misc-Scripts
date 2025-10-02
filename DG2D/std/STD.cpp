#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include "../enums.hpp"
#include "STD.hpp"

namespace DG2D {
    namespace SmartTableDatabase {
        const char savefile_signature[4] = { 'S','T','D','\0' };
        std::string filename = "untitled.std";

        bool preparefile() {
            std::fstream file(filename, std::ios::in | std::ios::binary);
            if (file.good()) {
                char sig[4] = {};
                file.read(sig, 4);
                if (file.gcount() < 4 || std::memcmp(sig, savefile_signature, 4) != 0) {
                    file.close();
                    std::ofstream wipe(filename, std::ios::binary | std::ios::trunc);
                    wipe.write(savefile_signature, 4);
                    wipe.close();
                }
            }
            else {
                std::ofstream newfile(filename, std::ios::binary);
                newfile.write(savefile_signature, 4);
                newfile.close();
            }
            return true;
        }


        void writeSpecialSaveFileSymbols(std::vector<char>& buffer, SpecialSaveFileSymbols m) {
            buffer.push_back(static_cast<char>(m));
        }

        void writestring(std::vector<char>& buffer, const std::string& str) {
            buffer.insert(buffer.end(), str.begin(), str.end());
        }

        bool SetValue(std::string Key, std::string Value) {
            preparefile();

            std::ifstream infile(filename, std::ios::binary);
            if (!infile.is_open())
                return false;

            std::vector<char> oldData((std::istreambuf_iterator<char>(infile)),
                std::istreambuf_iterator<char>());
            infile.close();

            std::vector<char> newData;
            if (oldData.size() >= 4 && std::memcmp(&oldData[0], savefile_signature, 4) == 0) {
                newData.insert(newData.end(), oldData.begin(), oldData.begin() + 4);
            }
            else {
                newData.insert(newData.end(), savefile_signature, savefile_signature + 4);
            }

            std::string bufferStr;
            size_t i = 4;
            bool replaced = false;

            while (i < oldData.size()) {
                char c = oldData[i++];
                if (c < 0) {
                    SpecialSaveFileSymbols m = static_cast<SpecialSaveFileSymbols>(c);
                    if (m == SpecialSaveFileSymbols::Newline) {
                        bufferStr.clear();
                        newData.push_back(c);
                    }
                    else if (m == SpecialSaveFileSymbols::Separator) {
                        if (bufferStr == Key && !replaced) {
                            uint32_t oldSize = 0;
                            std::memcpy(&oldSize, &oldData[i], sizeof(uint32_t));
                            i += sizeof(uint32_t) + oldSize;

                            // Append separator safely
                            writeSpecialSaveFileSymbols(newData, SpecialSaveFileSymbols::Separator);

                            uint32_t newSize = static_cast<uint32_t>(Value.size());
                            newData.insert(newData.end(),
                                reinterpret_cast<char*>(&newSize),
                                reinterpret_cast<char*>(&newSize) + sizeof(newSize));
                            newData.insert(newData.end(), Value.begin(), Value.end());

                            replaced = true;
                            bufferStr.clear();
                            continue;
                        }
                        else {
                            newData.push_back(c);

                            uint32_t oldSize = 0;
                            std::memcpy(&oldSize, &oldData[i], sizeof(uint32_t));

                            if (i + sizeof(uint32_t) + oldSize > oldData.size())
                                break; // avoid overflow

                            newData.insert(newData.end(),
                                &oldData[i],
                                &oldData[i + sizeof(uint32_t) + oldSize]);
                            i += sizeof(uint32_t) + oldSize;
                            bufferStr.clear();
                        }
                    }
                    else if (m == SpecialSaveFileSymbols::EndOfFile) {
                        newData.push_back(c);
                        break;
                    }
                }
                else {
                    bufferStr.push_back(c);
                    newData.push_back(c);
                }
            }

            if (!replaced) {
                std::ofstream outfile(filename, std::ios::binary | std::ios::app);
                writeSpecialSaveFileSymbols(newData, SpecialSaveFileSymbols::Newline);
                writestring(newData, Key);
                writeSpecialSaveFileSymbols(newData, SpecialSaveFileSymbols::Separator);

                uint32_t size = static_cast<uint32_t>(Value.size());
                newData.insert(newData.end(),
                    reinterpret_cast<char*>(&size),
                    reinterpret_cast<char*>(&size) + sizeof(size));
                newData.insert(newData.end(), Value.begin(), Value.end());
    
                outfile.write(newData.data(), newData.size());
                outfile.close();
                return true;
            }


            std::ofstream outfile(filename, std::ios::binary | std::ios::trunc);
            outfile.write(newData.data(), newData.size());
            outfile.close();

            return true;
        }

        std::string LoadValue(std::string Key) {
            preparefile();
            std::ifstream file(filename, std::ios::binary);
            file.seekg(4); // skip signature

            std::string bufferStr;
            char c;

            while (file.get(c)) {
                if (c < 0) {
                    SpecialSaveFileSymbols m = static_cast<SpecialSaveFileSymbols>(c);
                    if (m == SpecialSaveFileSymbols::Newline) {
                        bufferStr.clear();
                    }
                    else if (m == SpecialSaveFileSymbols::Separator) {
                        if (bufferStr == Key) {
                            uint32_t size = 0;
                            file.read(reinterpret_cast<char*>(&size), sizeof(size));

                            if (size > 0) {
                                std::string value(size, '\0');
                                file.read(&value[0], size);
                                return value;
                            }
                            return "";
                        }
                        bufferStr.clear();
                    }
                    else if (m == SpecialSaveFileSymbols::EndOfFile)
                        break;
                }
                else {
                    bufferStr.push_back(c);
                }
            }
            return "";
        }

        bool HasKey(std::string Key) {
            return !LoadValue(Key).empty();
        }
    }
}
