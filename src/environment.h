#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <unordered_map>
#include <string>
#include <variant>
#include <stdexcept>

class Environment{
    private:
        std::unordered_map<std::string, std::variant<double, std::string>> values;
    
    public:
        
        void define(const std::string& name, std::variant<double, std::string> value){
            values[name] =  value;
        }

        std::variant<double, std::string>get(const std::string& name){
            if(values.find(name) != values.end()){
                return values[name];
            }
            throw std::runtime_error("Undefined variable '" + name + "'");
        }       

        bool isDefined(const std::string& name) const {
            return values.find(name) != values.end();
        }
};

#endif //ENVIRONMENT_H