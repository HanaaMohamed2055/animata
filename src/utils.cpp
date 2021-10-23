#include "utils.h"

#include <fstream>
#include <sstream>

namespace utils
{
    std::string ReadFile(const char* path)
    {
        std::ifstream file;
        std::string result;
        if (file.good())
        {
            file.open(path);

            std::stringstream fileContent;
            fileContent << file.rdbuf();

            result = fileContent.str();
        }
        file.close();
        return result;
    }
}