#include <string>
#include <iostream>

namespace DG2D
{
    namespace INI
    {
        extern std::string filename;

        extern bool getvalue(const std::string& key, std::string& value); //value is returned into the value buffer
        extern bool setvalue(const std::string& key, const std::string& value);
    }
}