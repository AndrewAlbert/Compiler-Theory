# Compiler-Theory
Compiler written for the University of Cincinnati class EECE6083: Compiler Theory. The goal of this project is to hand build a simple recursive decent (LL1) compiler by hand (not using compiler construction tools such as flex or antlr) utilizing the C++ programming language and LLVM (for the compiler backend).

## Project Milestones
There will be a total of five milestones pertinent to this project's progress
### 1. Scanner
This milestone is complete
  
### 2. Parser
This milestone is complete
  
### 3. Scope and Type Checking
This milestone is complete.

### 4. Code Generation
This milestone has yet to be started

The parsed token will be translated to LLVM's intermediate representation and then use LLVM's backend optimizer and code generator to generate the final code.

### 5. Runtime System
This milestone has yet to be started

The following runtime functions will be implemented. None of the put functions return a value, and all read from / write to standard in and out.

getBool(bool val out)

getInteger(integer val out)

getFloat(float val out)

getString(string val out)

getChar(char val out)

putBool(bool val in)

putInteger(integer val in)

putFloat(float val in)

putString(string val in)

putChar(char val in)
