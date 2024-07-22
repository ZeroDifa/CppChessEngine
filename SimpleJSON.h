#ifndef SIMPLE_JSON_H
#define SIMPLE_JSON_H

#include <cctype>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

class JSONValue {
public:
    using JSONObject = std::unordered_map<std::string, JSONValue>;
    using JSONArray = std::vector<JSONValue>;
    using JSONVariant = std::variant<std::nullptr_t, bool, int, double, std::string, JSONObject, JSONArray>;

    JSONValue() = default;
    JSONValue(std::nullptr_t) : value(nullptr), type(0) {}
    JSONValue(bool b) : value(b), type(1) {}
    JSONValue(int i) : value(i), type(2) {}
    JSONValue(double d) : value(d), type(3) {}
    JSONValue(const std::string& s) : value(s), type(4) {}
    JSONValue(const char* s) : value(std::string(s)), type(5) {}
    JSONValue(const JSONObject& obj) : value(obj), type(6) {}
    JSONValue(const JSONArray& arr) : value(arr), type(7) {}

    bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value); }
    bool isInt() const { return std::holds_alternative<int>(value); }
    bool isDouble() const { return std::holds_alternative<double>(value); }
    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isObject() const { return std::holds_alternative<JSONObject>(value); }
    bool isArray() const { return std::holds_alternative<JSONArray>(value); }

    bool asBool() const { return std::get<bool>(value); }
    auto asAuto() const {
        switch (value.index()) {
            case 0: return std::holds_alternative<std::nullptr_t>(value);
            case 1: return std::holds_alternative<bool>(value);
            case 2: return std::holds_alternative<int>(value);
            case 3: return std::holds_alternative<double>(value);
            case 4: return std::holds_alternative<std::string>(value);
            case 5: return std::holds_alternative<JSONObject>(value);
            case 6: return std::holds_alternative<JSONArray>(value);
            default: throw std::runtime_error("Invalid JSON value");
        }
    }
    int asInt() const { return std::get<int>(value); }
    double asDouble() const { return std::get<double>(value); }
    const std::string& asString() const { return std::get<std::string>(value); }
    const JSONObject& asObject() const { return std::get<JSONObject>(value); }
    const JSONArray& asArray() const { return std::get<JSONArray>(value); }

    JSONVariant value;
    int type = 0;
};

class JSONParser {
public:
    static JSONValue parse(const std::string& jsonStr) {
        const char* json = jsonStr.c_str();
        return parseValue(json);
    };
    static std::string stringify(const JSONValue& value) {
        std::ostringstream oss;
        switch (value.value.index()) {
            case 0: oss << "null"; break;
            case 1: oss << (std::get<bool>(value.value) ? "true" : "false"); break;
            case 2: oss << std::get<int>(value.value); break;
            case 3: oss << std::get<double>(value.value); break;
            case 4: oss << "\"" << std::get<std::string>(value.value) << "\""; break;
            case 5: {
                const auto& obj = std::get<JSONValue::JSONObject>(value.value);
                oss << "{";
                for (auto it = obj.begin(); it != obj.end(); ++it) {
                    if (it != obj.begin()) oss << ",";
                    oss << "\"" << it->first << "\":" << stringify(it->second);
                }
                oss << "}";
                break;
            }
            case 6: {
                const auto& arr = std::get<JSONValue::JSONArray>(value.value);
                oss << "[";
                for (auto it = arr.begin(); it != arr.end(); ++it) {
                    if (it != arr.begin()) oss << ",";
                    oss << stringify(*it);
                }
                oss << "]";
                break;
            }
            default: throw std::runtime_error("Invalid JSON value");
        }
        return oss.str();
    };

private:
    static JSONValue parseValue(const char*& json) {
        skipWhitespace(json);
        if (*json == '"') return JSONValue(parseString(json));
        if (*json == '{') return JSONValue(parseObject(json));
        if (*json == '[') return JSONValue(parseArray(json));
        if (*json == 't') { json += 4; return JSONValue(true); }
        if (*json == 'f') { json += 5; return JSONValue(false); }
        if (*json == 'n') { json += 4; return JSONValue(nullptr); }
        return JSONValue(parseNumber(json));
    };
    static JSONValue::JSONObject parseObject(const char*& json) {
        JSONValue::JSONObject object;
        ++json; // Skip opening brace
        skipWhitespace(json);
        while (*json && *json != '}') {
            std::string key = parseString(json);
            skipWhitespace(json);
            if (*json != ':') throw std::runtime_error("Expected ':' in object");
            ++json; // Skip colon
            skipWhitespace(json);
            object[key] = parseValue(json);
            skipWhitespace(json);
            if (*json == ',') ++json; // Skip comma
            skipWhitespace(json);
        }
        ++json; // Skip closing brace
        return object;
    };
    static JSONValue::JSONArray parseArray(const char*& json) {
        JSONValue::JSONArray array;
        ++json; // Skip opening bracket
        skipWhitespace(json);
        while (*json && *json != ']') {
            array.push_back(parseValue(json));
            skipWhitespace(json);
            if (*json == ',') ++json; // Skip comma
            skipWhitespace(json);
        }
        ++json; // Skip closing bracket
        return array;
    };
    static std::string parseString(const char*& json) {
        std::string result;
        ++json; // Skip opening quote
        while (*json && *json != '"') {
            if (*json == '\\') {
                ++json;
                switch (*json) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw std::runtime_error("Invalid escape sequence");
                }
            } else {
                result += *json;
            }
            ++json;
        }
        ++json; // Skip closing quote
        return result;
    };
    static double parseNumber(const char*& json) {
        char* end;
        double result = strtod(json, &end);
        json = end;
        return result;
    };
    static void skipWhitespace(const char*& json) {
        while (std::isspace(*json)) ++json;
    };
};

#endif // SIMPLE_JSON_H
