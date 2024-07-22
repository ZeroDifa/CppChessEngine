#ifndef CONFIG_FLAG_MANAGER
#define CONFIG_FLAG_MANAGER

#include "SimpleJSON.h"
#include <string>
using namespace std;

class ConfigFlagManager {
    public:
        JSONValue::JSONObject config;
        ConfigFlagManager() {}
        void set(string key) {
            config[key] = JSONValue(true);
        }
        void set(string key, JSONValue value) {
            config[key] = value;
        }
        void unset(string key) {
            config[key] = JSONValue(false);
        }
        bool get(string key) {
            if (config.find(key) == config.end()) {
                return false;
            }
            return config[key].asBool();
        }
        JSONValue::JSONObject getWithoutBools() {
            JSONValue::JSONObject newConfig;
            for (auto it = config.begin(); it != config.end(); ++it) {
                if (it->second.isBool()) {
                    continue;
                }
                newConfig[it->first] = it->second;
            }
            return newConfig;
        }
        string toString() {
            return JSONParser::stringify(config);
        }
        void clear() {
            config.clear();
        }
};

#endif // SIMPLE_JSON_H
