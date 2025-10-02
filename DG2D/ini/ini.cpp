#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace DG2D
{
    namespace INI
    {
        std::string filename = "global.ini";

        bool getvalue(const std::string& key, std::string& value)
        {
            if (filename.empty()) return false;

            std::ifstream filebuf(filename);
            if (!filebuf.is_open()) return false;

            std::string line;
            while (std::getline(filebuf, line))
            {
                std::istringstream iss(line);
                std::string file_key, file_value;

                size_t eq_pos = line.find('=');
                if (eq_pos == std::string::npos) continue;

                file_key = line.substr(0, eq_pos);
                file_value = line.substr(eq_pos + 1);

                if (file_key == key)
                {
                    value = file_value;
                    return true;
                }
            }

            return false;
        }

        bool setvalue(const std::string& key, const std::string& value)
        {
            if (filename.empty()) return false;

            std::ifstream filebuf(filename);
            std::vector<std::string> lines;
            std::string line;
            bool found = false;

            while (std::getline(filebuf, line))
            {
                if (line.find('=') == std::string::npos) {
                    lines.push_back(line);
                    continue;
                }

                std::string file_key = line.substr(0, line.find('='));
                if (file_key == key)
                {
                    lines.push_back(key + "=" + value);
                    found = true;
                }
                else
                {
                    lines.push_back(line);
                }
            }

            if (!found)
                lines.push_back(key + "=" + value); 

            filebuf.close();

            std::ofstream outbuf(filename, std::ios::trunc);
            for (const auto& l : lines)
                outbuf << l << '\n';

            return true;
        }
    }
}
