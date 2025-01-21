# AxScript - A Simple Programming Language

AxScript is a simple programming language implemented in C++ that supports basic programming constructs. This project demonstrates the implementation of a lexer, parser, and interpreter for a custom programming language.

## Features

Current features include:
- Variable declarations and initialization
- Basic arithmetic operations (+, -, *, /)
- Print statements
- Number and string literals
- User input
- Control flow
- Loops
- Error handling and recovery
- REPL (Read-Eval-Print Loop) interface

Example code:
```
var x = 42;
print x;
var result = x + 8;
print result;
```

```
var num;
print "Enter a number : ";
input num;
print num;
```
```
var num = 7;

if (num > 5) {
    var num2 = 2;
    print num2;
    print "num is greater than 5";
} 
else if (num < 5) {
    print "num is less than 5";
} 
else {
    print "num is equal to 5";
}
print "end";
```
```
loop i = 1 to 5 {
    print i;
}
```
```
loop i = 5 to 1 down {
    print i;
}
```
## Project Structure

```
.
├── src/
│   ├── ast.h           # Abstract Syntax Tree definitions
│   ├── environment.h   # Variable environment management
│   ├── interpreter.h   # Code interpretation logic
│   ├── lexer.cpp      # Lexical analysis implementation
│   ├── lexer.h        # Lexer header
│   ├── main.cpp       # Entry point
│   ├── parser.h       # Parser implementation
│   ├── tokens.cpp     # Token utilities
│   ├── tokens.h       # Token definitions
│   └── visitor.h      # Visitor pattern implementation
```

## Building the Project

### Prerequisites
- C++ compiler with C++11 support or later
- CMake (version 3.0 or higher)

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/sthaarwin/axscript.git
cd axscript
```

2. Create a build directory:
```bash
mkdir build
cd build
```

3. Generate build files and compile:
```bash
make all
```

4. Run the executable:
```bash
./src/output/axscript
```

## Usage

### Running a Script File
```bash
./axscript script.axp
```

### Interactive Mode (REPL)
```bash
./axscript
>> print "hello world!";
hello world!
```

## Language Syntax

### Variable Declaration
```
var name = value;
```

### Print Statement
```
print expression;
```

### Input Statement
```
input variableName;
```

### If Statement
```
if(condition){}
```

### Else-If Statement
```
else if(condition){}
```

### Else Statement
```
else(condition){}
```

### Loop(increment) Statement
```
loop var=initial-value to max-range{}
```

### Loop(decrement) Statement
```
loop var=initial-value to min-range down{}
```


### Expressions
- Arithmetic: `+`, `-`, `*`, `/`
- Numbers: Integer or floating-point
- Strings: In double quotes
- Variables: Reference by name
- Loop: `loop i = 1 to 5 {}`, `loop i = 5 to 1 down {}`

## Error Handling

The interpreter provides error messages for:
- Syntax errors
- Undefined variables
- Type mismatches
- Missing semicolons
- Invalid expressions

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/YourFeature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to the branch (`git push origin feature/YourFeature`)
5. Open a Pull Request

## Future Improvements

- [x] Control flow (if/else/elseif statements)
- [x] Loops
- [ ] Functions
- [ ] More data types (boolean, arrays)
- [ ] Standard library
- [ ] Better error messages
- [ ] Code optimization


## Acknowledgments

- Inspired by various programming language implementation tutorials and books
- Built using modern C++ features and best practices

## Author

Arwin Shrestha

---
Feel free to open issues or contribute to the project!
