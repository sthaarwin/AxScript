#include "environment.h"
#include "ast.h"
#include "interpreter.h"

int AxScriptFunction::arity() const {
    return static_cast<int>(declaration->parameters.size());
}

Value AxScriptFunction::call(Interpreter* interpreter, const std::vector<Value>& arguments) {
    // Create a new environment for the function, with the closure as its enclosing scope
    auto environment = std::make_shared<Environment>(closure);
    
    // Bind arguments to parameters
    for (size_t i = 0; i < declaration->parameters.size(); i++) {
        environment->define(declaration->parameters[i].lexeme, arguments[i]);
    }
    
    try {
        // Execute the function body in the new environment
        interpreter->executeBlock(declaration->body, environment);
    } catch (const Return& returnValue) {
        // Catch return statements and pass the value back up
        return returnValue.getValue();
    }
    
    // If no return statement was encountered, return nil
    return makeNumber(0); // Default return value
}

std::string AxScriptFunction::toString() const {
    return "<function " + declaration->name.lexeme + ">";
}