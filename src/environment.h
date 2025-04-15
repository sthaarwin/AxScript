#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <unordered_map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <memory>

// Forward declarations
struct ValueImpl;
using Value = std::shared_ptr<ValueImpl>;

// Actual value implementation
struct ValueImpl {
    enum class Type { NUMBER, STRING, BOOLEAN, ARRAY };
    Type type;

    // Use separate variables instead of a union with manual destruction
    double numberVal;
    bool boolVal;
    std::string stringVal;
    std::vector<Value> arrayVal;

    // Constructors for each type
    ValueImpl(double val) : type(Type::NUMBER), numberVal(val), boolVal(false) {}
    ValueImpl(const std::string& val) : type(Type::STRING), numberVal(0), boolVal(false), stringVal(val) {}
    ValueImpl(bool val) : type(Type::BOOLEAN), numberVal(0), boolVal(val) {}
    ValueImpl(const std::vector<Value>& val) : type(Type::ARRAY), numberVal(0), boolVal(false), arrayVal(val) {}

    // No need for a custom destructor - let the compiler handle it properly
};

// Helper functions to create Values of different types
inline Value makeNumber(double val) { return std::make_shared<ValueImpl>(val); }
inline Value makeString(const std::string& val) { return std::make_shared<ValueImpl>(val); }
inline Value makeBoolean(bool val) { return std::make_shared<ValueImpl>(val); }
inline Value makeArray(const std::vector<Value>& val) { return std::make_shared<ValueImpl>(val); }

// Value type testing functions
inline bool isNumber(const Value& val) { return val->type == ValueImpl::Type::NUMBER; }
inline bool isString(const Value& val) { return val->type == ValueImpl::Type::STRING; }
inline bool isBoolean(const Value& val) { return val->type == ValueImpl::Type::BOOLEAN; }
inline bool isArray(const Value& val) { return val->type == ValueImpl::Type::ARRAY; }

// Value accessor functions
inline double asNumber(const Value& val) { 
    if (!isNumber(val)) throw std::runtime_error("Value is not a number");
    return val->numberVal; 
}
inline const std::string& asString(const Value& val) { 
    if (!isString(val)) throw std::runtime_error("Value is not a string");
    return val->stringVal; 
}
inline bool asBoolean(const Value& val) { 
    if (!isBoolean(val)) throw std::runtime_error("Value is not a boolean");
    return val->boolVal; 
}
inline std::vector<Value>& asArray(const Value& val) { 
    if (!isArray(val)) throw std::runtime_error("Value is not an array");
    return val->arrayVal; 
}

class Environment {
private:
    std::unordered_map<std::string, Value> values;
    
public:
    void define(const std::string& name, Value value) {
        values[name] = value;
    }

    Value get(const std::string& name) {
        if (values.find(name) != values.end()) {
            return values[name];
        }
        throw std::runtime_error("Undefined variable '" + name + "'");
    }       

    bool isDefined(const std::string& name) const {
        return values.find(name) != values.end();
    }

    void assign(const std::string& name, Value value) {
        if (values.find(name) != values.end()) {
            values[name] = value;
        } else {
            throw std::runtime_error("Undefined variable '" + name + "'");
        }
    }
};

#endif //ENVIRONMENT_H