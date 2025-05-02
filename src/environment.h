#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <unordered_map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <memory>
#include <functional>

// Forward declarations
class FunctionStmt;
class Interpreter;

struct ValueImpl;
using Value = std::shared_ptr<ValueImpl>;

// Abstract Callable interface
class Callable {
public:
    virtual ~Callable() = default;
    virtual int arity() const = 0;  // Number of arguments
    virtual Value call(Interpreter* interpreter, const std::vector<Value>& arguments) = 0;
    virtual std::string toString() const = 0;
};

struct ValueImpl {
    enum class Type { NUMBER, STRING, BOOLEAN, ARRAY, FUNCTION };
    Type type;

    double numberVal;
    bool boolVal;
    std::string stringVal;
    std::vector<Value> arrayVal;
    std::shared_ptr<Callable> callableVal;

    ValueImpl(double val) : type(Type::NUMBER), numberVal(val), boolVal(false) {}
    ValueImpl(const std::string& val) : type(Type::STRING), numberVal(0), boolVal(false), stringVal(val) {}
    ValueImpl(bool val) : type(Type::BOOLEAN), numberVal(0), boolVal(val) {}
    ValueImpl(const std::vector<Value>& val) : type(Type::ARRAY), numberVal(0), boolVal(false), arrayVal(val) {}
    ValueImpl(std::shared_ptr<Callable> val) : type(Type::FUNCTION), numberVal(0), boolVal(false), callableVal(val) {}
};

inline Value makeNumber(double val) { return std::make_shared<ValueImpl>(val); }
inline Value makeString(const std::string& val) { return std::make_shared<ValueImpl>(val); }
inline Value makeBoolean(bool val) { return std::make_shared<ValueImpl>(val); }
inline Value makeArray(const std::vector<Value>& val) { return std::make_shared<ValueImpl>(val); }
inline Value makeFunction(std::shared_ptr<Callable> val) { return std::make_shared<ValueImpl>(val); }

inline bool isNumber(const Value& val) { return val->type == ValueImpl::Type::NUMBER; }
inline bool isString(const Value& val) { return val->type == ValueImpl::Type::STRING; }
inline bool isBoolean(const Value& val) { return val->type == ValueImpl::Type::BOOLEAN; }
inline bool isArray(const Value& val) { return val->type == ValueImpl::Type::ARRAY; }
inline bool isFunction(const Value& val) { return val->type == ValueImpl::Type::FUNCTION; }

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
inline std::shared_ptr<Callable> asFunction(const Value& val) {
    if (!isFunction(val)) throw std::runtime_error("Value is not a function");
    return val->callableVal;
}

class Environment {
private:
    std::shared_ptr<Environment> enclosing;
    std::unordered_map<std::string, Value> values;
    
public:
    Environment() : enclosing(nullptr) {}
    
    Environment(std::shared_ptr<Environment> enclosing) 
        : enclosing(enclosing) {}
    
    void define(const std::string& name, Value value) {
        values[name] = value;
    }

    Value get(const std::string& name) {
        if (values.find(name) != values.end()) {
            return values[name];
        }
        
        if (enclosing != nullptr) {
            return enclosing->get(name);
        }
        
        throw std::runtime_error("Undefined variable '" + name + "'");
    }       

    bool isDefined(const std::string& name) const {
        if (values.find(name) != values.end()) {
            return true;
        }
        
        if (enclosing != nullptr) {
            return enclosing->isDefined(name);
        }
        
        return false;
    }

    void assign(const std::string& name, Value value) {
        if (values.find(name) != values.end()) {
            values[name] = value;
            return;
        }
        
        if (enclosing != nullptr) {
            enclosing->assign(name, value);
            return;
        }
        
        throw std::runtime_error("Undefined variable '" + name + "'");
    }
};

// Return exception for unwinding the call stack
class Return : public std::runtime_error {
private:
    Value value;
    
public:
    Return(Value value) : std::runtime_error(""), value(value) {}
    
    Value getValue() const { return value; }
};

// AxScript Function implementation
class AxScriptFunction : public Callable {
private:
    FunctionStmt* declaration;
    std::shared_ptr<Environment> closure;
    
public:
    AxScriptFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure)
        : declaration(declaration), closure(closure) {}
    
    int arity() const override;
    Value call(Interpreter* interpreter, const std::vector<Value>& arguments) override;
    std::string toString() const override;
};

#endif // ENVIRONMENT_H