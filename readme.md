# AxScript - A Simple Programming Language

AxScript is a simple programming language implemented in C++ that supports basic programming constructs. This project demonstrates the implementation of a lexer, parser, and interpreter for a custom programming language.

## Features

Current features include:
- Variable declarations and initialization
- Basic arithmetic operations (+, -, *, /)
- Print statements
- Number, string, and boolean literals
- User input
- Control flow
- Loops
- Error handling and recovery
- REPL (Read-Eval-Print Loop) interface
- Logical operations (AND, OR)
- Comparison with else branches
- Short-circuit evaluation for logical operations
- Boolean data type (true/false)

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

compeq (num, 5) {
    print "num is equal to 5";
}

compneq (num, 5) {
    print "num is not equal to 5";
}

compge (num, 5) {
    print "num is greater than or equal to 5";
}

comple (num, 5) {
    print "num is less than or equal to 5";
}
print "end";
```

```
// Boolean example
var isTrue = true;
var isFalse = false;

compeq (isTrue, true) {
    print "isTrue is true";
}

compeq (isFalse, false) {
    print "isFalse is false";
}
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
│   ├── lexer.cpp       # Lexical analysis implementation
│   ├── lexer.h         # Lexer header
│   ├── main.cpp        # Entry point
│   ├── parser.h        # Parser implementation
│   ├── tokens.cpp      # Token utilities
│   ├── tokens.h        # Token definitions
│   └── visitor.h       # Visitor pattern implementation
├── examples/
│   ├── basic/          # Basic language examples
│   │   ├── hello.axp   # Simple hello world program
│   │   ├── input.axp   # User input example
│   │   ├── numbers.axp # Number manipulation
│   │   └── text.axp    # Text output with escape sequences
│   ├── control_flow/   # Control flow examples
│   │   ├── and.axp     # Logical AND operations
│   │   ├── else.axp    # If-else statements
│   │   ├── if_statement.axp # Various comparison types
│   │   └── or.axp      # Logical OR operations
│   └── loops/          # Loop examples
│       ├── break.axp   # Breaking out of loops
│       ├── continue.axp # Continue to next iteration
│       ├── down_loop.axp # Counting down loop
│       ├── loop.axp    # Basic loop functionality
│       └── step_loop.axp # Loops with custom step value
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

2. Generate build files and compile:
```bash
make all
```

3. Run the executable:
```bash
./bin/axscript
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

### Comparison Statements
- `compeq (left, right) { ... }`: Executes the block if `left` is equal to `right`.
- `compneq (left, right) { ... }`: Executes the block if `left` is not equal to `right`.
- `compge (left, right) { ... }`: Executes the block if `left` is greater than or equal to `right`.
- `comple (left, right) { ... }`: Executes the block if `left` is less than or equal to `right`.
- `compg (left, right) { ... }`: Executes the block if `left` is greater than `right`.
- `compl (left, right) { ... }`: Executes the block if `left` is less than `right`.

Comparison statements can include else and else if branches:

Example:
```
var num = 10;

compg(num, 5) {
    print "num is greater than 5";
} else if compeq(num, 5) {
    print "num is equal to 5";
} else {
    print "num is less than 5";
}

compeq(x, 10) {
    print "x is equal to 10";
} else {
    print "x is not equal to 10";
}
```

Basic comparison statement:
```
compeq (x, 10) {
    print "x is 10";
}

compneq (x, 10) {
    print "x is not 10";
}

compge (x, 5) {
    print "x is greater than or equal to 5";
}

comple (x, 15) {
    print "x is less than or equal to 15";
}

compg (x, 7) {
    print "x is greater than 7";
}

compl (x, 20) {
    print "x is less than 20";
}
```

### Loop(increment) Statement
```
loop var=initial-value to max-range{}
```

### Loop(decrement) Statement
```
loop var=initial-value to min-range down{}
```

### Loop with Step Value
```
loop var=initial-value to end-value step stepSize {}
```

### Logical Operations
AxScript supports logical AND and OR operations in comparison statements:

```
// AND operation - both conditions must be true
compeq(a, 2) and compeq(b, 1) {
    print("Both conditions are true");
} else {
    print("AND condition failed");
}

// OR operation - at least one condition must be true
compeq(a, 2) or compeq(b, 3) {
    print("At least one condition is true");
} else {
    print("OR condition failed");
}
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

## Example Organization

The examples are organized into three main categories:

1. **Basic Examples** (`examples/basic/`):
   - Simple program structure and syntax
   - Variable declaration
   - Input/output operations
   - Text processing
   
2. **Control Flow Examples** (`examples/control_flow/`):
   - Conditional statements
   - Comparison operations
   - Logical operators (AND, OR)
   - Multi-branch conditionals (if-else if-else)
   
3. **Loop Examples** (`examples/loops/`):
   - Basic iteration
   - Counting loops (up and down)
   - Loop control (break, continue)
   - Advanced loop features (step size)

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/YourFeature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to the branch (`git push origin feature/YourFeature`)
5. Open a Pull Request

## Future Improvements

- [x] Control flow (comparison statements)
- [x] Loops
- [x] Logical operations
- [x] Boolean data type
- [ ] Functions
- [ ] More data types (arrays)
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
