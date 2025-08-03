#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::map<std::string, std::string> Config::values;

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Warning: Could not open config file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::string key, value;
        if (parseJsonValue(line, key, value)) {
            values[key] = value;
        }
    }
    
    file.close();
    std::cout << "Loaded " << values.size() << " config values from " << filename << std::endl;
    return true;
}

float Config::getFloat(const std::string& key, float defaultValue) {
    auto it = values.find(key);
    if (it != values.end()) {
        try {
            return std::stof(it->second);
        } catch (const std::exception& e) {
            std::cout << "Warning: Invalid float value for key '" << key << "': " << it->second << std::endl;
        }
    }
    return defaultValue;
}

int Config::getInt(const std::string& key, int defaultValue) {
    auto it = values.find(key);
    if (it != values.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            std::cout << "Warning: Invalid int value for key '" << key << "': " << it->second << std::endl;
        }
    }
    return defaultValue;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) {
    auto it = values.find(key);
    if (it != values.end()) {
        return it->second;
    }
    return defaultValue;
}

std::string Config::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool Config::parseJsonValue(const std::string& line, std::string& key, std::string& value) {
    std::string trimmed = trim(line);
    
    // Skip empty lines, comments, and structural characters
    if (trimmed.empty() || trimmed[0] == '{' || trimmed[0] == '}' || trimmed[0] == '/' || trimmed[0] == '*') {
        return false;
    }
    
    // Look for key-value pair: "key": value
    size_t colonPos = trimmed.find(':');
    if (colonPos == std::string::npos) {
        return false;
    }
    
    // Extract key (remove quotes and whitespace)
    std::string rawKey = trim(trimmed.substr(0, colonPos));
    if (rawKey.length() >= 2 && rawKey[0] == '"' && rawKey[rawKey.length()-1] == '"') {
        key = rawKey.substr(1, rawKey.length()-2);
    } else {
        key = rawKey;
    }
    
    // Extract value (remove quotes, whitespace, and trailing comma)
    std::string rawValue = trim(trimmed.substr(colonPos + 1));
    if (!rawValue.empty() && rawValue.back() == ',') {
        rawValue.pop_back();
        rawValue = trim(rawValue);
    }
    
    if (rawValue.length() >= 2 && rawValue[0] == '"' && rawValue[rawValue.length()-1] == '"') {
        value = rawValue.substr(1, rawValue.length()-2);
    } else {
        value = rawValue;
    }
    
    return !key.empty();
}