# MyShell - A Custom Shell with Integrated Virtual Machine

## Overview

MyShell is a sophisticated educational shell implementation that combines process management with a custom virtual machine, assembler, and garbage collector. It features a custom programming language that compiles to bytecode and executes on an integrated VM with support for advanced data structures, function closures, and automatic memory management.

## Project Structure
```
.
├── src/
│   ├── shell/          # Shell implementation
│   │   ├── shell.c     # Main shell loop
│   │   ├── builtins.c  # Built-in commands
│   │   ├── executor.c  # Process execution
│   │   ├── compiler.c  # Source to bytecode compilation
│   │   └── processmanager.c  # Process lifecycle management
│   ├── parser/         # Language parser (Flex/Bison)
│   │   ├── lexer.l     # Lexical analyzer
│   │   ├── parser.y    # Grammar definition
│   │   ├── ast.c       # Abstract Syntax Tree
│   │   └── symbol_table.c  # Symbol management
│   ├── ir/             # Intermediate representation
│   ├── assembler/      # Bytecode assembler
│   ├── VM/             # Virtual machine
│   │   ├── vm.c        # VM core
│   │   ├── stack.c     # Stack operations
│   │   ├── memory.c    # Memory management
│   │   └── instruction.c  # Instruction execution
│   ├── gc/             # Garbage collector
│   │   ├── gc.c        # GC algorithm
│   │   ├── heap.c      # Heap management
│   │   └── object.c    # Object handling
│   ├── debugger/       # Debugging tools
│   │   ├── debugger.c  # Debugger core
│   │   ├── breakpoints.c  # Breakpoint management
│   │   └── entry.c     # Debug entry point
│   └── main.c          # Program entry point
├── include/            # Header files (mirrors src/)
├── tests/              # Test programs (.am files)
├── log/                # Execution logs and output
└── Makefile            # Build configuration
```

## Prerequisites

- **GCC** (C11 compatible compiler)
- **Flex** (Lexical analyzer generator)
- **Bison** (Parser generator)
- **Make** (Build automation)
- **POSIX-compliant operating system** (Linux/Unix/macOS)

## Installation

1. **Clone or download the repository**

2. **Build the project:**
   ```bash
   make
   ```

3. **Run the shell:**
   ```bash
   ./myShell
   ```

4. **Clean build artifacts:**
   ```bash
   make clean
   ```

## Usage

### Shell Commands

MyShell supports the following built-in commands:

#### 1. Submit a Program
```bash
submit <filepath.am>
```
Compiles and submits a program for execution. The program is parsed, compiled to bytecode, and added to the process list.

**Example:**
```bash
myShell> submit tests/test1.am
Process submitted with PID: 1
```

#### 2. View Process Information
```bash
proc_info
```
Displays all processes in the system with their PIDs and current states.

**Example:**
```bash
myShell> proc_info
PID: 0    State: RUNNING      Path: ./myShell
PID: 1    State: SUBMITTED    Path: tests/test1.am
```

#### 3. Run a Process
```bash
run <pid>
```
Executes a submitted process. Output is saved to the `log/` directory.

**Example:**
```bash
myShell> run 1
Process 1 executed successfully
```

#### 4. Debug a Process
```bash
debug <pid>
```
Starts interactive debugging session for a process.

**Debugger Commands:**
- `step` - Execute n instructions (default: 1)
- `continue` - Run until next breakpoint or completion
- `break <address>` - Set breakpoint at bytecode address
- `info` - Display VM state (stack, memory, registers)
- `break_code` - Show source code mapping
- `quit` - Exit debugger

**Example:**
```bash
myShell> debug 1
Debugger started for PID 1
(dbg) break 10
Breakpoint set at address 10
(dbg) continue
...
(dbg) quit
```

#### 5. Exit Shell
```bash
exit
```
Terminates the shell and cleans up all processes.

### Process States

- **SUBMITTED**: Program compiled and ready to run
- **RUNNING**: Currently executing (shell process)
- **TERMINATED**: Execution completed

## Programming Language Syntax

### Variable Declaration and Assignment
```javascript
var x = 10;
var y = 20;
x = x + y;
```

### Object Creation and Manipulation
```javascript
// Create a pair object (heap-allocated)
obj a = new;

// Set left and right values
a.set_left = 100;
a.set_right = 200;

// Get values
var left_val = a.get_left;
var right_val = a.get_right;

// Remove values (sets to null/0)
a.remove_left;
a.remove_right;
```

### Control Flow
```javascript
// While loop
var x = 0;
var y = 10;

while (x < y) {
    x = x + 1;
}
```

### Comparison Operators
```javascript
x < y   // Less than
x <= y  // Less than or equal
x > y   // Greater than
x >= y  // Greater than or equal
x == y  // Equal to
```

### Arithmetic Operations
```javascript
var result = a + b;  // Addition
result = a - b;      // Subtraction
result = a * b;      // Multiplication
result = a / b;      // Division
```

### Print Output
```javascript
print(x);  // Print integer value
```

### Function Declarations (In Development)
```javascript
// Function declaration
def add(var x, var y);

// Function definition
add(var x, var y) {
    var result = x + y;
    return result;
}

// Function call
var sum = add(10, 20);
```

## Example Programs

### Example 1: Simple Loop
```javascript
// tests/test1.am
var x = 0;
var y = 3;

while (x < y) {
    x = x + 1;
}
```

### Example 2: Object Manipulation
```javascript
// tests/test5.am
var x = 10;
obj a = new;
a.set_left = x;
var b = a.get_right;
```

### Example 3: Arithmetic
```javascript
var a = 5;
var b = 10;
var sum = a + b;
var product = a * b;
print(sum);
print(product);
```

## Virtual Machine Details

### Instruction Set

#### Stack Operations
- `PUSH <value>` - Push value onto stack
- `POP` - Remove top value from stack
- `DUP` - Duplicate top stack value

#### Arithmetic Operations
- `ADD` - Addition (pop two values, push result)
- `SUB` - Subtraction
- `MUL` - Multiplication
- `DIV` - Division

#### Comparison Operations
- `LT` - Less than
- `LTE` - Less than or equal
- `GT` - Greater than
- `GTE` - Greater than or equal
- `EQ` - Equal to

#### Control Flow
- `JMP <address>` - Unconditional jump
- `JZ <address>` - Jump if zero (top of stack)
- `JNZ <address>` - Jump if not zero

#### Memory Operations
- `STORE <address>` - Store top of stack to memory
- `LOAD <address>` - Load from memory to stack

#### Object Operations
- `NEW_PAIR` - Allocate new pair object on heap
- `SET_LEFT` - Set left value of pair
- `SET_RIGHT` - Set right value of pair
- `GET_LEFT` - Get left value of pair
- `GET_RIGHT` - Get right value of pair
- `REMOVE_LEFT` - Remove left value
- `REMOVE_RIGHT` - Remove right value

#### Function Operations
- `MAKE_FUNC <label>` - Create function object
- `MAKE_CLOSURE` - Create closure
- `CALL_CLOSURE` - Call closure
- `CALL <address>` - Call function
- `RET` - Return from function

#### System Operations
- `GC` - Trigger garbage collection
- `PRINT` - Print top of stack
- `BREAK` - Debugger breakpoint
- `HALT` - Stop VM execution

### Memory Model

- **Stack**: 1024 value slots for computation
- **Call Stack**: Separate stack for function calls
- **Memory**: 1024 addressable memory slots
- **Heap**: Dynamic allocation for objects (pairs)

### Garbage Collection

The VM implements a mark-and-sweep garbage collector:
1. **Mark Phase**: Traverses reachable objects from roots (stack, memory, call stack)
2. **Sweep Phase**: Deallocates unreachable objects
3. **Compaction**: Optional heap compaction (future work)

## Debugging

### Using the Debugger

1. Submit and start debugging:
```bash
myShell> submit tests/test1.am
myShell> debug 1
```

2. Set breakpoints:
```bash
(dbg) break 15    # Break at bytecode address 15
(dbg) break 32    # Break at bytecode address 32
```

3. Step through execution:
```bash
(dbg) step        # Execute one instruction
```

4. View state:
```bash
(dbg) regs        # Show stack, PC, memory
(dbg) break_code      # Show source code mapping
(dbg) vm_state
```

5. Continue execution:
```bash
(dbg) continue    # Run until next breakpoint
```

### Output Logs

All program output is saved to `log/` directory:
- `log/history.txt` - Command history
- `log/output.asm` - Generated assembly code
- Additional per-process logs

## Development

### Current Development Status

**Implemented:**
-  Shell infrastructure and command processing
-  Process management system
-  Lexer and parser (Flex/Bison)
-  AST generation
-  Basic IR generation
-  Assembler (assembly to bytecode)
-  Virtual machine core
-  Stack-based execution
-  Memory operations
-  Arithmetic and comparison operators
-  Control flow (loops, conditionals)
-  Object system (pairs)
-  Garbage collector
-  Debugger with breakpoints
-  Print functionality

**In Development:**
- Complete function system with closures
- Additional comparison operators
- Advanced heap allocation features
- Enhanced error handling
- More comprehensive test coverage

**Planned:**
- Return values for functions
- More data structures (arrays, hash tables)
- String support
- File I/O operations
- Standard library
- Optimization passes

### Building from Source

The build process uses Flex and Bison to generate the lexer and parser:

```bash
# Generate lexer
flex -o src/lex.yy.c src/parser/lexer.l

# Generate parser
bison -d -o src/parser.tab.c src/parser/parser.y

# Compile all sources
gcc -Wall -Wextra -std=c11 -Iinclude [sources...] -o myShell
```

### Adding New Instructions

1. **Add opcode to** `include/VM/instruction.h`:
   ```c
   typedef enum {
       // ...
       MY_NEW_OP = 0x90,
   } opcode;
   ```

2. **Implement in** `src/VM/vm.c`:
   ```c
   case MY_NEW_OP:
       // Implementation
       break;
   ```

3. **Add to opcode_to_string** in `src/VM/instruction.c`:
   ```c
   case MY_NEW_OP: return "MY_NEW_OP";
   ```

4. **Update assembler** in `src/assembler/assembler.c`

5. **Update language parser** if needed

## Testing

### Running Tests

```bash
# Submit and run individual tests
./myShell
myShell> submit tests/test1.am
myShell> run 1

# Or use the provided test suite
for test in tests/*.am; do
    echo "Testing $test"
    # Add automation here
done
```

## Additional Documentation

### Memory Safety

- All heap allocations are tracked by the garbage collector
- Stack overflow protection (fixed size stacks)
- Memory bounds checking on array accesses
- Type safety in variable operations

### Performance Considerations

- Stack-based VM for efficient execution
- Minimal memory allocations during runtime
- Efficient bytecode format
- Optional benchmarking mode for profiling

### Error Handling

The system provides error messages for:
- Syntax errors during parsing
- Type mismatches
- Undefined variables
- Invalid operations
- Stack overflows/underflows
- Memory access violations
- Process management errors

## 👥 Contributors

This project is a collaborative educational effort by:
- **Monit**: Shell + process manager, garbage collection, VM, Debugger
- **Anunay**: Language defintion, parser, AST to assembly, Assembler.

## License

Educational project - refer to your institution's policies on code sharing and reuse.

## Acknowledgments

Built with:
- [Flex](https://github.com/westes/flex) - Lexical analyzer generator
- [GNU Bison](https://www.gnu.org/software/bison/) - Parser generator
- GCC - GNU Compiler Collection

---

## 🔍 Quick Reference

### Essential Commands
| Command | Description |
|---------|-------------|
| `submit <file>` | Compile and submit a program |
| `proc_info` | List all processes |
| `run <pid>` | Execute a process |
| `debug <pid>` | Debug a process |
| `exit` | Exit shell |

### Language Syntax Quick Reference
```javascript
var x = 10;              // Variable
obj a = new;             // Object
a.set_left = x;          // Set property
while (x < 10) { ... }   // Loop
print(x);                // Output
```

### VM Instruction Quick Reference
```assembly
PUSH 10      // Push value
STORE 0      // Store to memory
LOAD 0       // Load from memory
ADD          // Add top two stack values
JZ label     // Jump if zero
HALT         // Stop execution
```

---

For more information or to report issues, please consult the source code documentation or contact the development team.
