# AxScript - A Simple Programming Language

AxScript is a simple yet powerful programming language implemented in C++ that supports a range of programming constructs. This project demonstrates the complete implementation of a lexical analyzer (lexer), parser, and interpreter for a custom programming language.

## Features

### Core Features
- Variable declarations and assignments
- First-class data types: numbers, strings, booleans, and arrays
- Basic arithmetic operations (`+`, `-`, `*`, `/`)
- String concatenation and manipulation
- Print statements and user input
- Control flow with comparison statements and branching
- Logical operations (`AND`, `OR`, `NOT`) with short-circuit evaluation
- Loops with increment, decrement, and custom step values
- Loop control with `break` and `continue`
- Comments (single-line and multi-line)
- Interactive REPL (Read-Eval-Print Loop) interface
- Robust error handling and recovery

### Example Code Snippets

**Variables and Basic Operations:**
```
var x = 42;
print x;
var result = x + 8;
print result;
```

**User Input:**
```
var num;
print "Enter a number: ";
input num;
print "You entered: " + num;
```

**Comparison and Conditional Statements:**
```
var num = 7;

compeq(num, 5) {
    print "num is equal to 5";
} else if compg(num, 5) {
    print "num is greater than 5";
} else {
    print "num is less than 5";
}
```

**Boolean Logic:**
```
var isTrue = true;
var isFalse = false;

compeq(isTrue, true) and compeq(isFalse, false) {
    print "Both conditions are true";
}

compeq(isTrue, false) or compeq(isFalse, false) {
    print "At least one condition is true";
}
```

**Loops:**
```
// Count up
loop i = 1 to 5 {
    print i;
}

// Count down
loop i = 5 to 1 down {
    print i;
}

// Custom step
loop i = 0 to 10 step 2 {
    print i;  // Prints 0, 2, 4, 6, 8, 10
}
```

**Arrays:**
```
// Create an array with mixed types
var mixedArray = [1, 2, "hello", true, 3.14];

// Access array elements
print mixedArray[0];    // Prints 1
print mixedArray[2];    // Prints "hello"

// Modify array elements
mixedArray[1] = 200;
print mixedArray;       // Prints [1, 200, "hello", true, 3.14]

// Loop through array
var numbers = [10, 20, 30, 40, 50];
loop i = 0 to 4 {
    print "Element " + i + ": " + numbers[i];
}

// Combine arrays
var array1 = [1, 2, 3];
var array2 = [4, 5, 6];
var combined = array1 + array2;  // [1, 2, 3, 4, 5, 6]
```

## Project Structure

```
.
├── src/                   # Source code
│   ├── ast.h              # Abstract Syntax Tree definitions
│   ├── environment.h      # Variable environment management
│   ├── interpreter.h      # Code interpretation logic
│   ├── lexer.cpp          # Lexical analysis implementation
│   ├── lexer.h            # Lexer header
│   ├── main.cpp           # Entry point
│   ├── parser.h           # Parser implementation
│   ├── tokens.cpp         # Token utilities
│   ├── tokens.h           # Token definitions
│   └── visitor.h          # Visitor pattern implementation
├── examples/              # Example programs
│   ├── arrays/            # Array examples
│   │   └── basic.axp      # Basic array operations
│   ├── basic/             # Basic language examples
│   │   ├── hello.axp      # Simple hello world program
│   │   ├── input.axp      # User input example
│   │   ├── numbers.axp    # Number manipulation
│   │   └── text.axp       # Text output with escape sequences
│   ├── boolean/           # Boolean examples
│   │   ├── basic.axp      # Basic boolean operations
│   │   └── ...            # More examples
│   ├── comments/          # Comment examples
│   │   └── comments.axp   # Single and multi-line comments
│   ├── control_flow/      # Control flow examples
│   │   ├── and.axp        # Logical AND operations
│   │   ├── else.axp       # If-else statements
│   │   ├── if_statement.axp # Various comparison types
│   │   └── or.axp         # Logical OR operations
│   └── loops/             # Loop examples
│       ├── break.axp      # Breaking out of loops
│       ├── continue.axp   # Continue to next iteration
│       ├── down_loop.axp  # Counting down loop
│       ├── loop.axp       # Basic loop functionality
│       └── step_loop.axp  # Loops with custom step value
├── bin/                   # Compiled binaries
│   └── axscript           # AxScript executable
├── Makefile               # Build configuration
└── README.md              # This file
```

## Building the Project

### Prerequisites
- C++ compiler with C++11 support or later
- GNU Readline library
- Make build system

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/sthaarwin/axscript.git
cd axscript
```

2. Compile the project:
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
./bin/axscript script.axp
```

### Interactive Mode (REPL)
```bash
./bin/axscript
>> print "hello world!";
hello world!
>> var x = 42;
>> print x;
42
>> exit
Exiting!
```

## Language Syntax Reference

### Comments
```
// This is a single-line comment

/* This is a
   multi-line comment */
```

### Variable Declaration and Assignment
```
var name;                 // Declaration only
var name = value;         // Declaration with initialization
name = newValue;          // Assignment to existing variable
```

### Data Types
- **Numbers**: Integer or floating-point (`42`, `3.14`, `-10`)
- **Strings**: Text in double quotes (`"hello world"`)
- **Booleans**: `true` or `false`
- **Arrays**: Collections of values (`[1, 2, 3]`, `["a", "b", "c"]`, `[1, "mixed", true]`)

### Arrays
```
var arr = [1, 2, 3, 4, 5];    // Create an array
print arr[0];                 // Access element (indexes start at 0)
arr[1] = 42;                  // Modify element
var len = arr.length;         // Get array length
var combined = arr1 + arr2;   // Concatenate arrays
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
- **Equal**: `compeq(left, right) { ... }`
- **Not equal**: `compneq(left, right) { ... }`
- **Greater than or equal**: `compge(left, right) { ... }`
- **Less than or equal**: `comple(left, right) { ... }`
- **Greater than**: `compg(left, right) { ... }`
- **Less than**: `compl(left, right) { ... }`

### Conditional Statements
```
compeq(x, 10) {
    // Execute if x equals 10
} else if compg(x, 10) {
    // Execute if x is greater than 10
} else {
    // Execute otherwise
}
```

### Logical Operations
```
// AND operation (both conditions must be true)
compeq(a, 1) and compeq(b, 2) {
    print "Both conditions are true";
}

// OR operation (at least one condition must be true)
compeq(a, 1) or compeq(b, 2) {
    print "At least one condition is true";
}
```

### Loops
```
// Count up
loop i = 1 to 5 {
    // Loop body
}

// Count down
loop i = 10 to 1 down {
    // Loop body
}

// Custom step
loop i = 0 to 10 step 2 {
    // Loop body (increments by 2)
}
```

### Loop Control
```
// Break out of a loop
loop i = 1 to 10 {
    compeq(i, 5) {
        break;      // Exit the loop when i equals 5
    }
}

// Skip current iteration
loop i = 1 to 5 {
    compeq(i, 3) {
        continue;   // Skip the rest of the loop body when i equals 3
    }
    print i;
}
```

## Error Handling

The AxScript interpreter provides detailed error messages for:
- Syntax errors
- Undefined variables
- Type mismatches
- Missing semicolons
- Array index out of bounds
- Division by zero
- Invalid operations

Examples:
```
>> print(x);
Runtime error: Undefined variable 'x'

>> var x = "hello" - 5;
Runtime error: Operands must be two numbers.

>> var arr = [1, 2, 3]; print arr[10];
Runtime error: Array index out of bounds: 10
```

## Future Improvements

- [x] Control flow (comparison statements)
- [x] Loops
- [x] Logical operations (AND, OR)
- [x] Boolean data type
- [x] Arrays
- [ ] Functions with parameters and return values
- [ ] User-defined classes and objects
- [ ] Standard library (math, string, file I/O)
- [ ] Better error messages with line number reporting
- [ ] Code optimization
- [ ] Modules and imports

## Contributing

Contributions are welcome! To contribute to AxScript:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/awesome-feature`
3. Commit your changes: `git commit -m 'Add awesome feature'`
4. Push to the branch: `git push origin feature/awesome-feature`
5. Open a Pull Request

Please ensure your code follows the existing style and includes appropriate tests.

## Acknowledgments

- Inspired by various programming language implementation tutorials and books
- Built using modern C++ features and best practices

## Author

Arwin Shrestha

## License

This project is open-source and available under the MIT License.

---
Feel free to open issues or contribute to the project!
