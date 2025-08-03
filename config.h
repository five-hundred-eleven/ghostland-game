#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>

class Config {
public:
    static bool loadFromFile(const std::string& filename);
    static float getFloat(const std::string& key, float defaultValue);
    static int getInt(const std::string& key, int defaultValue);
    static std::string getString(const std::string& key, const std::string& defaultValue);
    
private:
    static std::map<std::string, std::string> values;
    static std::string trim(const std::string& str);
    static bool parseJsonValue(const std::string& line, std::string& key, std::string& value);
};

#endif