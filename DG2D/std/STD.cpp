#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include "enums.hpp"
#include "STD.hpp"
#include <cstring>

namespace DG2D {
namespace SmartTableDatabase {

    std::string signature = "STD_";
    std::string filename = "untitled.std";

    bool preparefile() {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            std::string sig(4, '\0');
            file.read(&sig[0], 4);
            if (sig != signature) {
                std::ofstream wipe(filename, std::ios::binary | std::ios::trunc);
                wipe.write(signature.data(), 4);
            }
        } else {
            std::ofstream newfile(filename, std::ios::binary);
            newfile.write(signature.data(), 4);
        }
        return true;
    }

    bool SetValue(std::string Key, std::string Value) {
        preparefile();

        std::ifstream infile(filename, std::ios::binary);
        if (!infile.is_open())
            return false;

        std::string oldData((std::istreambuf_iterator<char>(infile)),
                             std::istreambuf_iterator<char>());

        std::string newData;
        if (oldData.size() >= 4 && oldData.substr(0, 4) == signature) {
            newData = oldData.substr(0, 4);
        } else {
            newData = signature;
        }

        std::string token;
        size_t i = 4;
        bool replaced = false;

        while (i < oldData.size()) {
            char c = oldData[i];
            i++;

            if (c < 0) {
                auto m = static_cast<SpecialSaveFileSymbols>(c);

                if (m == SpecialSaveFileSymbols::Newline) {
                    token.clear();
                    newData.push_back(c);
                } else if (m == SpecialSaveFileSymbols::Separator) {
                    if (token == Key && !replaced) {
                        if (i + sizeof(uint32_t) > oldData.size())
                            break;

                        uint32_t oldSize = 0;
                        std::memcpy(&oldSize, oldData.data() + i, sizeof(uint32_t));
                        i += sizeof(uint32_t) + oldSize;

                        newData.push_back(static_cast<char>(SpecialSaveFileSymbols::Separator));
                        uint32_t newSize = static_cast<uint32_t>(Value.size());
                        newData.append(reinterpret_cast<char*>(&newSize), sizeof(newSize));
                        newData.append(Value);
                        replaced = true;
                        token.clear();
                    } else {
                        newData.push_back(c);

                        if (i + sizeof(uint32_t) > oldData.size())
                            break;

                        uint32_t oldSize = 0;
                        std::memcpy(&oldSize, oldData.data() + i, sizeof(uint32_t));

                        if (i + sizeof(uint32_t) + oldSize > oldData.size())
                            break;

                        newData.append(oldData.data() + i, sizeof(uint32_t) + oldSize);
                        i += sizeof(uint32_t) + oldSize;
                        token.clear();
                    }
                } else if (m == SpecialSaveFileSymbols::EndOfFile) {
                    newData.push_back(c);
                    break;
                }
            } else {
                token.push_back(c);
                newData.push_back(c);
            }
        }

        if (!replaced) {
            newData.push_back(static_cast<char>(SpecialSaveFileSymbols::Newline));
            newData.append(Key);
            newData.push_back(static_cast<char>(SpecialSaveFileSymbols::Separator));
            uint32_t size = static_cast<uint32_t>(Value.size());
            newData.append(reinterpret_cast<char*>(&size), sizeof(size));
            newData.append(Value);
        }

        std::ofstream outfile(filename, std::ios::binary | std::ios::trunc);
        outfile.write(newData.data(), newData.size());

        return true;
    }

    std::string LoadValue(std::string Key, std::string def) {
        preparefile();

        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open())
            return def;

        file.seekg(4);

        std::string token;
        char c;

        while (file.get(c)) {
            if (c < 0) {
                auto m = static_cast<SpecialSaveFileSymbols>(c);

                if (m == SpecialSaveFileSymbols::Newline) {
                    token.clear();
                } else if (m == SpecialSaveFileSymbols::Separator) {
                    if (token == Key) {
                        uint32_t size = 0;
                        file.read(reinterpret_cast<char*>(&size), sizeof(size));
                        if (size > 0) {
                            std::string value(size, '\0');
                            file.read(&value[0], size);
                            return value;
                        }
                        return def;
                    }
                    token.clear();
                } else if (m == SpecialSaveFileSymbols::EndOfFile) {
                    break;
                }
            } else {
                token.push_back(c);
            }
        }

        return def;
    }

    bool HasKey(std::string Key) {
        return !LoadValue(Key, "").empty();
    }

}}
