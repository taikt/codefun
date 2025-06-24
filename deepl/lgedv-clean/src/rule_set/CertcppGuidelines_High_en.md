### CERT-C++ Coding Standards  (LG Priority: High)

### CTR50-CPP 
**Description**: Guarantee that container indices and iterators are within the valid range 
**Exception**: None

### CTR51-CPP 
**Description**: Use valid references, pointers, and iterators to reference elements of a container 
**Exception**: None

### CTR52-CPP 
**Description**: Guarantee that library functions do not overflow 
**Exception**: None

### CTR53-CPP 
**Description**: Use valid iterator ranges 
**Exception**: None

### CTR54-CPP 
**Description**: Do not subtract iterators that do not refer to the same container 
**Exception**: None

### CTR55-CPP 
**Description**: Do not use an additive operator on an iterator if the result would overflow 
**Exception**: None

### CTR56-CPP 
**Description**: Do not use pointer arithmetic on polymorphic objects 
**Exception**: None

### DCL50-CPP 
**Description**: Do not define a C-style variadic function 
**Exception**: None

### DCL58-CPP 
**Description**: Do not modify the standard namespaces 
**Exception**: None

### DCL60-CPP 
**Description**: Obey the one-definition rule 
**Exception**: None

### ERR56-CPP 
**Description**: Guarantee exception safety 
**Exception**: None

### ERR57-CPP 
**Description**: Do not leak resources when handling exceptions 
**Exception**: None

### EXP50-CPP 
**Description**: Do not depend on the order of evaluation for side effects 
**Exception**: None

### EXP52-CPP 
**Description**: Do not rely on side effects in unevaluated operands 
**Exception**: None

### EXP53-CPP 
**Description**: Do not read uninitialized memory 
**Exception**: None

### EXP54-CPP 
**Description**: Do not access an object outside of its lifetime 
**Exception**: None

### EXP61-CPP 
**Description**: A lambda object must not outlive any of its reference captured objects 
**Exception**: None

### EXP62-CPP 
**Description**: Do not access the bits of an object representation that are not part of the object's value representation 
**Exception**: None

### MEM50-CPP 
**Description**: Do not access freed memory 
**Exception**: None

### MEM51-CPP 
**Description**: Properly deallocate dynamically allocated resources 
**Exception**: None

### MEM52-CPP 
**Description**: Detect and handle memory allocation errors 
**Exception**: None

### MEM53-CPP 
**Description**: Explicitly construct and destruct objects when manually managing object lifetime 
**Exception**: None

### MEM54-CPP 
**Description**: Provide placement new with properly aligned pointers to sufficient storage capacity 
**Exception**: None

### MEM55-CPP 
**Description**: Honor replacement dynamic storage management requirements 
**Exception**: None

### MEM56-CPP 
**Description**: Do not store an already-owned pointer value in an unrelated smart pointer 
**Exception**: None

### MSC54-CPP 
**Description**: A signal handler must be a plain old function 
**Exception**: None

### OOP55-CPP 
**Description**: Do not use pointer-to-member operators to access None
xistent members 
**Exception**: None

### OOP57-CPP 
**Description**: Prefer special member functions and overloaded operators to C Standard Library functions 
**Exception**: None

### STR50-CPP 
**Description**: Guarantee that storage for strings has sufficient space for character data and the null terminator 
**Exception**: None

### STR51-CPP 
**Description**: Do not attempt to create a std::string from a null pointer 
**Exception**: None

### STR52-CPP 
**Description**: Use valid references, pointers, and iterators to reference elements of a basic_string 
**Exception**: None

### STR53-CPP 
**Description**: Range check element access 
**Exception**: None
