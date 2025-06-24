### CERT-C Coding Standards (LG Priority: High)

### ARR30-C 
**Description**: Do not form or use out-of-bounds pointers or array subscripts 
**Exception**: None

### ARR32-C 
**Description**: Ensure size arguments for variable length arrays are in a valid range 
**Exception**: None

### ARR36-C 
**Description**: Do not subtract or compare two pointers that do not refer to the same array 
**Exception**: None

### ARR38-C 
**Description**: Guarantee that library functions do not form invalid pointers 
**Exception**: None

### ARR39-C 
**Description**: Do not add or subtract a scaled integer to a pointer 
**Exception**: None

### DCL30-C 
**Description**: Declare objects with appropriate storage durations 
**Exception**: None

### ENV30-C 
**Description**: Do not modify the object referenced by the return value of certain functions 
**Exception**: None

### ENV33-C 
**Description**: Do not call system() 
**Exception**: None

### ENV34-C 
**Description**: Do not store pointers returned by certain functions 
**Exception**: None

### ERR33-C 
**Description**: Detect and handle standard library errors 
**Exception**: None

### EXP30-C 
**Description**: Do not depend on the order of evaluation for side effects 
**Exception**: None

### EXP33-C 
**Description**: Do not read uninitialized memory 
**Exception**: None

### EXP34-C 
**Description**: Do not dereference null pointers 
**Exception**: None

### EXP44-C 
**Description**: Do not rely on side effects in operands to sizeof, _Alignof, or _Generic 
**Exception**: None

### FIO30-C 
**Description**: Exclude user input from format strings 
**Exception**: None

### FIO34-C 
**Description**: Distinguish between characters read from a file and EOF or WEOF 
**Exception**: None

### FIO37-C 
**Description**: Do not assume that fgets() or fgetws() returns a None
mpty string when successful 
**Exception**: None

### FIO38-C 
**Description**: Do not copy a FILE object 
**Exception**: None

### FIO42-C 
**Description**: Close files when they are no longer needed 
**Exception**: None

### FIO45-C 
**Description**: Avoid TOCTOU race conditions while accessing files 
**Exception**: None

### FIO46-C 
**Description**: Do not access a closed file 
**Exception**: None

### FIO47-C 
**Description**: Use valid format strings 
**Exception**: None

### FLP30-C 
**Description**: Do not use floating-point variables as loop counters 
**Exception**: None

### INT30-C 
**Description**: Ensure that unsigned integer operations do not wrap 
**Exception**: None

### INT31-C 
**Description**: Ensure that integer conversions do not result in lost or misinterpreted data 
**Exception**: None

### INT32-C 
**Description**: Ensure that operations on signed integers do not result in overflow 
**Exception**: None

### INT33-C 
**Description**: Ensure that division and remainder operations do not result in divide-by-zero errors 
**Exception**: None

### MEM30-C 
**Description**: Do not access freed memory 
**Exception**: None

### MEM31-C 
**Description**: Free dynamically allocated memory when no longer needed 
**Exception**: None

### MEM34-C 
**Description**: Only free memory allocated dynamically 
**Exception**: None

### MEM35-C 
**Description**: Allocate sufficient memory for an object 
**Exception**: None

### MSC33-C 
**Description**: Do not pass invalid data to the asctime() function 
**Exception**: None

### MSC37-C 
**Description**: Ensure that control never reaches the end of a non-void function 
**Exception**: None

### MSC41-C 
**Description**: Never hard code sensitive information 
**Exception**: None

### POS30-C 
**Description**: Use the readlink() function properly 
**Exception**: None

### POS34-C 
**Description**: Do not call putenv() with a pointer to an automatic variable as the argument 
**Exception**: None

### POS35-C 
**Description**: Avoid race conditions while checking for the existence of a symbolic link 
**Exception**: None

### POS36-C 
**Description**: Observe correct revocation order while relinquishing privileges 
**Exception**: None

### POS37-C 
**Description**: Ensure that privilege relinquishment is successful 
**Exception**: None

### POS54-C 
**Description**: Detect and handle POSIX library errors 
**Exception**: None

### PRE31-C 
**Description**: Avoid side effects in arguments to unsafe macros 
**Exception**: None

### SIG30-C 
**Description**: Call only asynchronous-safe functions within signal handlers 
**Exception**: None

### SIG31-C 
**Description**: Do not access shared objects in signal handlers 
**Exception**: None

### STR31-C 
**Description**: Guarantee that storage for strings has sufficient space for character data and the null terminator 
**Exception**: None

### STR32-C 
**Description**: Do not pass a non-null-terminated character sequence to a library function that expects a string 
**Exception**: None

### STR37-C 
**Description**: Arguments to character-handling functions must be representable as an unsigned char 
**Exception**: None

### STR38-C 
**Description**: Do not confuse narrow and wide character strings and functions 
**Exception**: None

### API02-C 
**Description**: Functions that read or write to or from an array should take an argument to specify the source or target size 
**Exception**: None

### API05-C 
**Description**: Use conformant array parameters 
**Exception**: None

### ARR00-C 
**Description**: Understand how arrays work 
**Exception**: None

### ARR01-C 
**Description**: Do not apply the sizeof operator to a pointer when taking the size of an array 
**Exception**: None

### DCL11-C 
**Description**: Understand the type issues associated with variadic functions 
**Exception**: None

### ENV01-C 
**Description**: Do not make assumptions about the size of an environment variable 
**Exception**: None

### EXP08-C 
**Description**: Ensure pointer arithmetic is used correctly 
**Exception**: None

### EXP09-C 
**Description**: Use sizeof to determine the size of a type or variable 
**Exception**: None

### EXP15-C 
**Description**: Do not place a semicolon on the same line as an if, for, or while statement 
**Exception**: None

### EXP20-C 
**Description**: Perform explicit tests to determine success, true and false, and equality 
**Exception**: None

### FIO01-C 
**Description**: Be careful using functions that use file names for identification 
**Exception**: None

### FIO20-C 
**Description**: Avoid unintentional truncation when using fgets() or fgetws() 
**Exception**: None

### MSC15-C 
**Description**: Do not depend on undefined behavior 
**Exception**: None

### MSC17-C 
**Description**: Finish every set of statements associated with a case label with a break statement 
**Exception**: None

### MSC24-C 
**Description**: Do not use deprecated or obsolescent functions 
**Exception**: None

### POS02-C 
**Description**: Follow the principle of least privilege 
**Exception**: None

### PRE09-C 
**Description**: Do not replace secure functions with deprecated or obsolescent functions 
**Exception**: None

### PRE10-C 
**Description**: Wrap multistatement macros in a do-while loop 
**Exception**: None

### PRE11-C 
**Description**: Do not conclude macro definitions with a semicolon 
**Exception**: None

### SIG00-C 
**Description**: Mask signals handled by noninterruptible signal handlers 
**Exception**: None

### SIG02-C 
**Description**: Avoid using signals to implement normal functionality 
**Exception**: None

### STR00-C 
**Description**: Represent characters using an appropriate type 
**Exception**: None

### STR02-C 
**Description**: Sanitize data passed to complex subsystems 
**Exception**: None

### STR06-C 
**Description**: Do not assume that strtok() leaves the parse string unchanged 
**Exception**: None

### STR07-C 
**Description**: Use the bounds-checking interfaces for string manipulation 
**Exception**: None

### WIN01-C 
**Description**: Do not forcibly terminate execution 
**Exception**: None

### WIN02-C 
**Description**: Restrict privileges when spawning child processes 
**Exception**: None