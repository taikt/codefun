# LG Static Analysis Code Rules Guidelines (Impact: High)
 
## ARRAY_VS_SINGLETON
**Description**: ARRAY_VS_SINGLETON reports some cases where a pointer to a single object is incorrectly treated like an array, which causes the program to access invalid memory. This results in either reading garbage/unintended values from memory or writing to unintended memory and corrupting it. ARRAY_VS_SINGLETON also reports cases where an array of base class objects is treated as an array of derived class objects.

Because the type systems in C/C++ do not distinguish between "pointer to one object" and "pointer to array of objects," it is easy to accidentally apply pointer arithmetic to a pointer that only points to a singleton object. ARRAY_VS_SINGLETON checker finds many cases of this error.

## ATOMICITY
**Description**: The ATOMICITY checker reports some cases where the code contains two critical sections in sequence, but it appears that they should be merged into a single critical section because the latter uses data computed in the former, but the data might possibly invalidated in between. This is a form of concurrent race condition.

For C/C++, this checker finds a class of defects in which a critical section is not sized sufficiently to protect a variable. For example, assume that individual reads and writes are protected by locks but that the entire operation is not protected. Such a case can result in inconsistent results.

For C++, this checker also detects issues when std::atomic::compare_exchange_weak() or std::atomic::compare_exchange_string() are used on objects that contain padding. These functions compare bitwise on the entire objects, including on any padding that might be present, so the objects might compare unequal when they are logically equal.

For Java and Kotlin, this checker examines cases where a value, v, is defined in a critical section. If v flows into another critical section (which uses the same lock that was used when v was defined) where v is used, the checker reports a defect. This checker tracks critical sections defined by synchronized methods, synchronized blocks, and java.util.concurrent.locks.Lock objects.

## BAD_ALLOC_ARITHMETIC
**Description**: BAD_ALLOC_ARITHMETIC finds many calls to allocation routines with errant placement of the parenthesis when using - or + operators. It searches for malloc(x)+y or malloc(x)-y where it appears that the user intended to call malloc(x+y) or malloc(x-y). These errors cause under-allocation or over-allocation, and unintended pointer arithmetic, which result in memory corruption or a potential security vulnerability when attempting to copy data into the resultant buffer.

## BAD_ALLOC_STRLEN
**Description**: BAD_ALLOC_STRLEN reports defects when it finds strlen(p+1) used as the size argument of an allocation site. Assuming that the developer intended strlen(p)+1 to indicate the length of p plus an extra byte for a null terminator, strlen(p+1) is undefined when p is zero length and strlen(p)-1 otherwise. The result is a potential buffer overrun on the result of allocation, undefined behavior when p is zero length, or both. This defect almost always results in a buffer overflow when data is copied to the new buffer.

## BAD_FREE
**Description**: BAD_FREE finds many cases where a pointer is freed but not dynamically allocated. Freeing a pointer that does not point to dynamically allocated memory has undefined behavior. The typical effect is to corrupt memory, which later causes the program to crash.

## BUFFER_SIZE
**Description**: BUFFER_SIZE finds many cases of possible buffer overflows due to incorrect size arguments being passed to buffer manipulation functions. These incorrect arguments, when passed to functions such as strncpy() or memcpy(), can cause memory corruption, security defects, and program crashes.

This checker reports a BUFFER_SIZE defect when it finds a function passed a size argument that will overflow the buffer target. It issues a warning, BUFFER_SIZE_WARNING, when the size argument is the exact size of the buffer target, leaving no room for a null terminator.

This checker analyzes calls to the following functions:
strcpy, strncpy, strcat, strncat, memcpy, memmove, wmemmove, memset,
strxfrm, wcxfrm, wcsncpy, wcsncat,
lstrcpyn, strcpynw,
StrCpyN, StrCpyNA, StrCpyNW,
_mbsncpy, _tcsnpy, _mbsncat, _tcsncat, _tcsxfrm

## COM.ADDROF_LEAK
**Description**: COM.ADDROF_LEAK identifies uses of a CComBSTR or CComPtr instance that might cause memory leaks because the value of the pointer that is internal to the instance can be modified through the pointer address.

The checker tracks local non-static CComBSTR and CComPtr variables that have been determined as managing a non-null pointer. When the address of the pointer (obtained through the overloaded operator address-of (&))is passed as an argument to a function call, the pointer value can potentially be overwritten, causing a memory leak.

## COM.BAD_FREE
**Description**: COM.BAD_FREE finds many cases where the code violates the Microsoft COM interface convention regarding the lifetime management of pointers to interfaces. COM specifies that this management should be accomplished through the AddRef and Release methods found on every COM interface. It is an error to circumvent the reference counting mechanism by explicitly freeing a pointer to an interface, because other clients might share ownership of the same object. The COM.BAD_FREE checker finds many instances of these explicit frees.

Explicitly freeing a pointer to a COM interface can leave other owners of the instance with dangling pointers. This can possibly result in use-after-free memory errors, including memory corruption and crashes.

## MISMATCHED_ITERATOR
**Description**: This C++ checker reports many occurrences when an iterator from an STL container is incorrectly passed to a function from another container. The checker also reports defects when an iterator from one container is compared to an iterator from a different container.

The following STL containers are supported: vector, list, map, multimap, set, multiset, hash_map, hash_multimap, hash_set, hash_multiset, basic_string, forward_list, unordered_map, unordered_multimap, unordered_set, unordered_multiset.

## MISSING_COPY_OR_ASSIGN
**Description**: MISSING_COPY_OR_ASSIGN reports many cases where a class that owns resources, such as dynamically allocated memory or operating system handles, lacks either a user-written copy constructor or a user-written assignment operator. When this is the case, the compiler will generate the missing operator, but the compiler-generator operator only does a shallow copy. Later, when the copies are destroyed, the owned resource will be destroyed twice, leading to memory corruption. The defects reported by this checker appear as either MISSING_COPY or MISSING_ASSIGN in Coverity Connect. One, the other, or both of these can be reported for any one class.

A class is considered to own resources if its destructor calls freeing functions on fields of this.

To be considered an assignment operator for the purposes of this rule, an assignment operator must be usable to assign entire objects. Private copy constructors or assignment operators are assumed not to be meant for use and are not required to manage resources.

## MISSING_RETURN
**Description**: MISSING_RETURN checker finds cases where a non-void function does not return a value, and optionally, when it returns more than one value.

Not returning a value, or returning multiple values, can result in unpredictable program behavior. Unreachable paths are not reported as a defect, even if there is no return value:

## NEGATIVE_RETURNS
**Description**: NEGATIVE_RETURNS finds many misuses of negative integers. Negative integers and function return values that can potentially be negative must be checked before being used (for example, as array indexes, loop bounds, algebraic expression variables or size/length arguments to a system call).

Negative integer misuses can cause memory corruption, process crashes, infinite loops, integer overflows, and security defects (if a user is able to control improperly checked input).

## NO_EFFECT
**Description**: NO_EFFECT finds many instances of statements or expressions that do not accomplish anything, or statements that perform an action that is not the intended action. Usually, this issue is caused by a typographical error, oversight, or misunderstanding of language rules, such as operator precedence.

## OVERLAPPING_COPY
**Description**: OVERLAPPING_COPY detects cases where data is copied to an overlapping location in an undefined manner, such as with assignment (undefined unless the overlap is exact and the types compatible) or memcpy (always undefined).

## OVERRUN
**Description**: OVERRUN finds many instances where buffers are accessed out of bounds. Improper buffer access can corrupt memory and cause process crashes, security vulnerabilities and other severe system problems. OVERRUN looks for out-of-bounds indexing into both heap buffers and stack buffers.

## PATH_MANIPULATION
**Description**: PATH_MANIPULATION detects many cases in which a filename or path is constructed unsafely.

An attacker who has control over part of the filename or path might be able to maliciously alter the overall path and access, modify, or test the existence of critical or sensitive files. Particular concerns are the ability to perform directory traversal in a path (for example, ../ ) or to specify absolute paths.

These types of vulnerabilities can be prevented by proper input validation. The user input should be allow-listed to contain only the expected values or characters.

The PATH_MANIPULATION checker uses the global trust model to determine whether to trust servlet inputs, the network data, filesystem data, or database information. You can use the --trust-* and/or --distrust-* options to cov-analyze to modify the current settings.

## PW.RETURN_PTR_TO_LOCAL_TEMP
**Description**: Prevents undefined behavior by detecting problems with returning pointers to local temporary variables.

## READLINK
**Description**: READLINK reports many cases where the POSIX readlink() function is used but the program does not safely place a NULL terminator in the result buffer. The readlink() function places the contents of a symbolic link into a buffer of a specified size. The return value of readlink() can be anything between -1 and the size of the buffer, and both endpoints require special handling.

The readlink() function does not append a null character to the buffer, and truncates the contents in case the buffer is too small. The code must manually null-terminate the buffer, but often defects arise when you unsafely use the return value as an index. If using the return value as an index for null termination, either pass one less than the size of the buffer or check that the return value is less than the size of the buffer.

If the code does not null-terminate the resulting buffer, the STRING_NULL checker reports a defect. Also, if the code uses the return value as an index into the buffer without checking it against -1, the NEGATIVE_RETURNS checker reports a defect.

## RESOURCE_LEAK
**Description**: RESOURCE_LEAK looks for cases in which a program fails to guarantee release of system resources as soon as possible. An application that fails to release acquired resources can suffer performance degradation, crashes, denial of service, or the inability to successfully obtain a given resource.

## RETURN_LOCAL
**Description**: RETURN_LOCAL finds many cases where the address of a local variable is returned from a function. The address is invalid as soon as the function returns, so the usual result is memory corruption and unpredictable behavior.

In C and C++, all local variables are lost upon function exit as a stack frame is removed and control is returned to the calling function. Variables that were allocated on the callee's stack are no longer relevant; their memory will be overwritten when a new function is called. Pointers to local stack variables returned to a calling function can cause memory corruption and inconsistent behavior. This checker finds instances where a function returns a pointer to a stack-allocated variable.

## SIZECHECK
**Description**: DEPRECATED as of version 7.0: SIZECHECK finds many instances of pointer assigned memory allocations where the pointer's target type is larger than the block allocated. For example, if a pointer to a long is assigned a block the size of an int. Note that checker options can expand the scope of defects that this checker finds.

If a pointer points to a block which is too small, then attempts to use it could reference out-of-bounds memory, which can potentially cause heap corruption, program crashes, and other serious problems.

SIZECHECK false positives are the result of either an incorrect calculation of the amount of memory allocated, or the amount of memory that should have been allocated. If SIZECHECK incorrectly analyzes a function that returns heap-allocated memory, you can correctly model the function's abstract behavior using a library call. If a misunderstood context-specific property causes a false positive, you can annotate that property with a //coverity comment. See "C/C++ code-line annotations" in Customizing Coverity.

## SIZEOF_MISMATCH
**Description**: SIZEOF_MISMATCH finds combinations of pointers and sizeof expressions that appear to be mismatched. When a pointer and sizeof expression occur together, the sizeof expression is usually the size of memory that the pointer points to.

The checker also reports defects in a limited number of cases where a size_t argument is expected but no sizeof expression is provided. This occurs when the semantics of a function are known and the size_t argument is expected to be the size of memory to which a pointer points, as well as in a small number of cases where it appears that the size_t argument is intended to be the size of some pointed-to memory.

This checker reports the following combinations of pointer and sizeof expressions:
function arguments and return values
extraneous sizeof expression in pointer arithmetic

## SQLI
**Description**: SQLI finds many instances where a method that interprets a string as SQL is given an argument that might be under the control of an attacker.

Interpreting strings from untrusted sources as SQL can allow a malicious user to exfiltrate (that is, steal), corrupt, or destroy information in a database that is being queried. Typically, this error is caused by using unsafe methods to construct SQL statements: concatenating unsanitized strings directly into the query, or sanitizing untrusted input in an incorrect way.

By default, the SQLI checker treats values as though they are tainted if they come from the network (either through sockets or HTTP requests). The checker can also be configured to treat values from the filesystem or the database as though they are tainted (see Options).

## STRING_NULL
**Description**: STRING_NULL finds many cases where non-null-terminated strings (for example, a string contained in a network packet) are used unsafely.

Because they are pointers to blocks of characters in memory, it is imperative that string arguments be null-terminated before functions manipulate them. When passed to functions such as strlen(), non-null terminated strings can cause looping or overflow defects.

## STRING_OVERFLOW
**Description**: STRING_OVERFLOW finds many cases where a string manipulation function (for example, strcpy ) might write past the end of the allocated array. It determines this based on the sizes of the arrays involved at the call site to the string manipulation function. It reports a defect if it finds a buffer copying function where the source string is larger than the destination string. It issues a warning for all other possible string overflows.

String overflows are one of the premier causes of C/C++ memory corruption and security vulnerabilities. Memory corruption occurs when memory outside the bounds of a string buffer is inadvertently overwritten. Buffer overruns are common because languages such as C and C++ are inherently unsafe : their string manipulation routines do not automatically perform bounds-checking, leaving it up to the programmer to perform this task.

STRING_OVERFLOW analyzes calls to the following functions:
strcpy, strcat
wcscpy, wcscat
StrCpy, StrCpyA, StrCpyW, StrCat, StrCatA, StrCatW
OemToChar, OemToCharA, OemToCharW, OemToAnsi, OemToAnsiA, OemToAnsiW
_mbscpy, _mbscat, _tcscat, _tcscpy
lstrcpy, lstrcpyA, lstrcpyW,
lstrcat, lstrcatA, lstrcatW
The STRING_OVERFLOW checker treats strncpy(dst, src, strlen(src)) like a call to strcpy(dst, src), and treats strncat(dst, src, strlen(src)) like a call to strcat(dst, src).

## STRING_SIZE
**Description**: STRING_SIZE finds cases where a string manipulation function (for example, strcpy ) might write past the end of the allocated array. It determines this by following a string from its tainted source, past any length checking routines that might otherwise sanitize it, and to a trusted sink that does not check the length itself. Unlike STRING_OVERFLOW, which also finds array overruns involving strings, STRING_SIZE follows an interprocedural dataflow path from source to sink rather than relying on information locally available at a string manipulation call site. A STRING_SIZE defect can potentially cause buffer overflows, memory corruption, and program crashes.

To fix this defect, you should either length-check strings of arbitrary length before copying them into fixed size buffers, or use safe copying functions (for example: strncpy() instead of strcpy()).

## UNINIT
**Description**: UNINIT finds many instances of variables that are used without being initialized. Stack variables do not have set values unless initialized. Using uninitialized variables can result in unpredictable behavior, crashes, and security holes.

This checker looks for uninitialized stack variables and dynamically allocated heap memory. It tracks primitive type variables, structure fields, and array elements. Note that initialization tracking for a variable stops if the address of the variable is taken.

UNINIT begins tracking a variable when it is declared and follows it down all call chains checking for uninitialized uses. As with DEADCODE, you can use code-line annotations to suppress UNINIT events and eliminate false positives. See "C/C++ code-line annotations" in Customizing Coverity.

## UNLOCKED_ACCESS
**Description**: The UNLOCKED_ACCESS checker reports on cases of thread-shared data that is accessed concurrently without any locking.

While the checker is configured to detect some thread entry points automatically, it will often need configuration. A thread entry point is any function that can be called concurrently with itself (either a function passed as argument to something such as pthread_create, or a library function that is considered to be thread-safe). To configure entry points, use the -co UNLOCKED_ACCESS:entry_point_function:<regex> option, where <regex> needs to match the mangled name of the function in question.

## USE_AFTER_FREE
**Description**: The USE_AFTER_FREE checker finds many cases where memory or a resource is used after it has been freed or closed. The consequence of using memory after freeing it is almost always memory corruption and a later program crash. The consequence of using a resource after closing it depends on the API in use.

## USE_AFTER_MOVE
**Description**: The USE_AFTER_MOVE checker reports when an object is used after it has been moved without being reinitialized.

## VIRTUAL_DTOR
**Description**: VIRTUAL_DTOR looks for cases where the wrong destructor or no destructor is called by the delete operator because an object is upcast before it is deleted and the destructor is not virtual. The checker does not report a defect if the derived class destructor is implicitly defined and does the same thing as the base class destructor.

Undefined behavior only happens if the child class has a destructor that does more than invoke the parent's destructor. VIRTUAL_DTOR considers a class to have a non-trivial destructor if any of the following cases is true:
The destructor was not generated by the compiler; the destructor was explicitly specified by the user.
Any of the fields added in the child class has a destructor.

## WRAPPER_ESCAPE
**Description**: WRAPPER_ESCAPE finds many instances where the internal representation of a string wrapper class (such as CComBSTR, _bstr_t, CString, or std::string ) for a local- or global-scope object escapes the current function. The usual effect is a use-after-free error because the object destroys its internal BSTR upon exit. Whatever uses the escaped pointer now has an invalid pointer.

## FORWARD_NULL
**Description**: FORWARD_NULL finds errors that can result in program termination or a runtime exception. It finds many instances where a pointer or reference is checked against null or assigned null and then later dereferenced. For JavaScript and Python, this checker finds many instances where a value is checked against or assigned null or undefined and later used as an object (that is, by accessing a property of it) or function (that is, by calling it).

## NULL_RETURNS
**Description**: NULL_RETURNS finds errors that can result in program termination or a runtime exception.

Developers sometimes do not test function return values, and instead use the returned values in potentially dangerous ways. Every function that returns a pointer must be checked against null values before it can be considered safe to use. Failing to check the returned pointer against null values can cause crashes due to trying to dereference a null pointer.

## REVERSE_INULL
**Description**: REVERSE_INULL finds many instances of checks against null, Nothing, nil, or undefined values that occur after uses of the value that would have already failed if it were indeed null, Nothing, nil, or undefined. Such uses include dereferences in C/ C++/ C#/ Java/ Kotlin/ Visual Basic, unwrapping in C#, or Visual Basic, or a member or property access in other languages. The checker name derives from the internally inconsistent code, where the check and use appear to be reversed.

## ORDER_REVERSAL
**Description**: ORDER_REVERSAL finds many cases where the program acquires a pair of locks/mutexes in different orders in different places. This issue can lead to a deadlock if two threads simultaneously use the opposite order of acquisition.

Acquiring pairs of locks in the incorrect order can result in the program hanging. Because of thread interleaving, it is possible for two threads to each be waiting on a lock that the other thread has acquired ( deadlock ). Other threads attempting to acquire either of the two locks will also deadlock.
   
## LOCK
**Description**: LOCK finds many cases where a lock/mutex is acquired but not released, or is locked twice without an intervening release. Usually, this issue occurs because an error-handling path fails to release a lock. The result is usually a deadlock.

Two types of locks are supported:
Exclusive: An exclusive lock cannot be acquired recursively and any attempt to do so will deadlock.
Recursive: The same thread can recursively acquire a recursive lock.

A lock can be either a global variable or local to a function.

LOCK reports a defect when the following sequence occurs:

Note: The values in parenthesis, such as (+lock), are a documentation convention used to aid in illustrating the following examples.
1. A variable L is locked (+lock).
2. L is not unlocked (-unlock).

One of the following can now occur:
The path's end is reached (-lock_returned) and L does not appear anywhere in the function's return value or its expression.
L is locked again (+double_lock). (Only for exclusive locks.)

No errors are reported for functions that intentionally lock a function argument.

Defects are also reported when the following sequence occurs:
1. L is unlocked (+unlock).
2. L is passed to a function which asserts that lock L is held (+lockassert).

Forgetting to release an acquired lock can result in the program hanging: subsequent attempts to acquire the lock fail as the program waits for a release that will never occur.

## INFINITE_LOOP
**Description**: INFINITE_LOOP finds many cases of loops or recursion that never terminate, usually resulting in a program hang.

It finds infinite loops by identifying variables that appear in the conditions for continuing a loop. If these variables cannot be updated to falsify these conditions without an overflow or an underflow, the checker reports the loop as an infinite loop.
It finds infinite recursion by identifying function calls where the call target is the same as the caller, and when there is no path that avoids such a call.

## CHECKED_RETURN
**Description**: CHECKED_RETURN finds many cases where the return value of a function is ignored when it should have been checked. For example, it detects the case where the code neglects to handle an error code returned from a system call. This is a statistical type checker: it determines which functions should be checked based on use patterns. For this checker, you establish the pattern as a ratio between the number of times the return value of a function is checked and the total number of times that function is called.

## INCOMPATIBLE_CAST
**Description**: INCOMPATIBLE_CAST finds many instances where an object in memory is accessed via a pointer cast to an incompatible type. Casting a pointer will not change memory layout so the defects reported by INCOMPATIBLE_CAST are potential out-of-bounds memory accesses or dependencies on byte order.

## INTEGER_OVERFLOW
**Description**: INTEGER_OVERFLOW finds many cases of integer overflows and truncations resulting from arithmetic operations. Some forms of integer overflow can cause security vulnerabilities, for example, when the overflowed value is used as the argument to an allocation function. By default, the checker reports defects only when it determines that the operands are tainted sources, some arithmetic operations are performed, and the operation's result goes to a sink. Sinks are memory allocators and certain system calls. You can use checker options to add more sinks. The checker only reports when overflow occurs on the path from a data source to a data sink. By default, a source is a program variable that can be controlled by an external agent (for example, an attacker), and a sink is a value that is trusted from a security perspective (for example, allocation argument). However, there are checker options that will expand the source and sink criteria in order to report more defects.

## REVERSE_NEGATIVE
**Description**: REVERSE_NEGATIVE finds many cases where an integer is used as an array index, but then later checked to determine if it is negative. If the integer could be negative, the check takes place too late. If it could not be negative, the check is unnecessary.

During development, correctly bounds-checking an integer before a potentially dangerous use is often overlooked. Mishandling of negative integers can cause hard-to-find problems from memory corruption to security defects. This checker finds instances of dangerous integer use followed by a check against NEGATIVE. Two situations could cause this scenario:
- The programmer "knows" the integer cannot be negative, in which case the check is unnecessary and should be removed as it indicates to other programmers that the integer could be negative.
- The integer could actually be negative, and the check needs to occur before the dangerous use.

REVERSE_NEGATIVE can report false positives if it incorrectly determines that:
- An integer is compared against a negative value.
- A potentially negative integer is used in a dangerous way.

## AUDIT.SPECULATIVE_EXECUTION_DATA_LEAK
**Description**: AUDIT.SPECULATIVE_EXECUTION_DATA_LEAK finds code patterns that might cause a sensitive process (such as a kernel or virtual machine) to be vulnerable to the "Spectre" atttack. The code patterns that this checker finds might allow an attacker to read the process memory.

The checker looks for cases where a value V is compared against another value; an attacker could use V as an index to access some memory. The result from accessing that memory could then be used to access even more memory. Specifically, the checker looks for cases where a value V is checked to be within bounds. The check could take the form of an equality check, an inequality check, or a check for the return value of a function call.

If value V can be controlled by an attacker, the attacker can use it to expose data because the comparison might be speculatively ignored by the CPU when a cache miss prevents immediately evaluating the comparison. This means the attacker can potentially access any value W in the process address space. When that W is then used to access more memory, its effect on the CPU cache might then be used to retrieve information about W.

## BAD_CHECK_OF_WAIT_COND
**Description**: BAD_CHECK_OF_WAIT_COND finds cases where a thread calls a wait function (for example, Object.wait() for Java) on a mutex without properly checking a wait condition.

In many APIs, the wait function doesn't take the desired condition as a parameter; therefore, the programmer needs to ensure that the wait condition is false before waiting, and true after waiting. Checking the condition needs to take place inside the region locked by the mutex, before waiting. Otherwise, the wait condition might become true between the check and the wait, causing the program to wait unnecessarily. Condition variables are typically susceptible to "spurious wakeups," where a wait function successfully returns before it receives notification that the wait condition has been satisfied with a call to a notification function (such as Object.notify() or Object.notifyAll() for Java). Thus, it is also necessary to check the status of the wait condition inside a loop, which allows the thread to wait again if a spurious wakeup occurred. This checker finds situations where such checks are done inappropriately.

## MULTIPLE_INIT_SMART_PTRS
**Description**: MULTIPLE_INIT_SMART_PTRS reports cases where a raw pointer is used to initialize two different smart pointers, or where using the tracked pointer from one smart pointer is used to initialize another one.

This works currently for standard C++ smart pointers: std::unique_ptr, std::shared_ptr and std::weak_ptr.

## OS_CMD_INJECTION
**Description**: OS_CMD_INJECTION detects many instances in which an OS command is executed on the server and the command or its arguments can be manipulated by an attacker. An attacker who has control over part of the command string might be able to maliciously alter the intent of the operating system command. This change might result in the disclosure, destruction, or modification of sensitive data or operating system resources.

By default, the OS_CMD_INJECTION checker treats values as though they are tainted if they come from the network (either through sockets or incoming HTTP requests). The checker can also be configured to treat values from the file system or the database as though they are tainted (see Options).

Note that the checker always reports a defect when tainted data flows to an OS command no matter what sanitization was done. Coverity suggests the following workflow:
1. Follow Coverity remediation advice: validate, sanitize, and use the array forms of methods rather than the string forms.
For JavaScript, sanitize the data before using inside a sensitive function call. If possible, use a safer library or API call instead
2. If you continue to use an OS command to achieve your goal, get a security expert to review the code and mark the defect as Intentional in Coverity Connect if the expert is satisfied that the behavior of the code is safe.

## SENSITIVE_DATA_LEAK
**Description**: SENSITIVE_DATA_LEAK finds code that stores, transmits, or logs sensitive data without protecting it adequately, before use, with encryption. Showing exception stack traces or other program information on the UI can reveal data about an application, which makes it more susceptible to attacks. The SENSITIVE_DATA_LEAK checker then reports these sensitive data leaks.

## UNENCRYPTED_SENSITIVE_DATA
**Description**: The UNENCRYPTED_SENSITIVE_DATA checker detects cases where sensitive data (for example, a password or a cryptographic key) is received via an unencrypted transmission or loaded from unencrypted storage. Storing or transmitting sensitive data without encrypting it allows an attacker to steal it or tamper with it. Fixing such a defect requires changes to all endpoints.

## WEAK_GUARD
**Description**: WEAK_GUARD finds comparisons of unreliable data (for example, host names, IP addresses, and so on) to constant strings. Code that uses such a comparison instead of an authorization or authentication check is vulnerable to exploitation by attacks such as DNS poisoning, or IP spoofing. Coverity refers to such a check as a "weak guard".

## WEAK_PASSWORD_HASH
**Description**: WEAK_PASSWORD_HASH finds code that applies a cryptographic hash function (also known as a one-way hash function) to password data in a cryptographically weak manner. In such cases, the computational effort required to retrieve passwords from their hashes might be insufficient to deter large-scale, password-cracking attacks. Examples of weak hashing include the following:

- Using a hashing algorithm that is cryptographically weak (such as MD5).
- Hashing without iterating the hash function a large number of times.
- Hashing without using a salt as part of the input.
- Hashing with a salt that is not random and uniquely chosen for each password.

The recommended method of hashing sensitive password data is to generate a random sequence of bytes (a "salt") for each password that you intend to hash, to hash the password and the salt with an adaptive hash function such as bcrypt, scrypt, and PBKDF2 (Password-Based Key Derivation Function 2), and then to store the hash and the salt for subsequent password checks.

## WRITE_CONST_FIELD
**Description**: The WRITE_CONST_FIELD checker points out issues when a function writes to a structure, class, or union and that write modifies a const-qualified field in the aggregate.

## Y2K38_SAFETY
**Description**: The Y2K38_SAFETY checker is intended to point out two potential issues with the rollover of the 32-bit signed integer counter of seconds since epoch in the UNIX time_t type. Software that deals with dates, and which uses time_t to represent those dates, is susceptible to data corruption due to this rollover. Such corruption begins not on the day the counter rolls over, but when the software in question first attempts to represent a future time that lies beyond that rollover date and time.

The two defect types reported are:
- A time_t value is stored in an (32-bit) integer variable, rather than in another time_t or a 64-bit width integer. This includes passing a time_t variable as a parameter to a function that takes an 32-bit integer argument, with an exception made when the checker can determine that this is being used to seed a random number generator, in which case the loss of bits is not important.
- A declaration involving a time_t typedef is made when that typedef is less than 64 bits wide. Note that if a program makes use only of struct timeval, the definition of that struct in terms of time_t ensures that defects will still be reported.
   
## LOCK_EVASION
**Description**: LOCK_EVASION finds many instances in which the code evades the acquisition or sufficient holding of a lock that is protecting against modification of thread-shared data by checking a piece of that thread-shared data. The evasion might consist of not acquiring the lock or of releasing the lock early (leaving the modification itself unguarded). Evasion of holding a lock in this way can allow operations to be interleaved or reordered at runtime, such that data races occur.

## INEFFICIENT_RESERVE
**Description**: The INEFFICIENT_RESERVE checker detects a particular unsafe pattern in the reserve() method on std::vector. The danger arises from the fact that, unlike the resize() method, the reserve() method makes no effort to adjust the capacity of the internal buffer in a way that minimizes copying of data.

With resize(), if the programmer simply keeps adding 1 object to the vector in a loop, the backing store will actually not increment by one element each time, but will increase, typically, by doubling. The size and the capacity of the vector are separate fields, and while the size may increment, the capacity increases geometrically. The reason for this is that a capacity increase typically cannot increase the size of the buffer in place, but must create a new, larger buffer, and then copy all of the data from the old to the new buffer.

The reserve() method evades this doubling rule. The result is, in the worst-case, an order N-squared algorithm that can cause significant slowdowns.
	
## BAD_SHIFT
**Description**: BAD_SHIFT finds bit shift operations where the value or the range of possible values for the shift amount (the right operand) is such that the operation might invoke undefined behavior or might not produce the expected result.

Specifically, the checker finds cases where:
- For a left bit shift ( <<, or shl for Kotlin ), the shift amount is greater than or equal to the size, in bits, of the type to which the left operand is promoted.
- For a right bit shift ( >>, also >>> for Java and shr/ushr for Kotlin), the shift amount is greater than or equal to the size, in bits, of the (unpromoted) left operand.
- The shift amount is negative.
	
## CONSTANT_EXPRESSION_RESULT
**Description**: CONSTANT_EXPRESSION_RESULT finds many cases where an expression always evaluates to one particular value, but it looks like it is intended to evaluate to different values because it involves at least one variable. For example, the fragment if (x|1) appears to be trying to test the least significant bit of x, but the code uses bitwise OR rather than AND, so the condition always evaluates to true. The checker is tuned to avoid reporting false positives in code that uses conditional compilation, but the associated heuristics can be controlled through checker options.

[C++, CUDA, and Objective-C++] CONSTANT_EXPRESSION_RESULT also issues a defect when the C++20 function std::is_constant_evaluated always returns true or false in the context where it is used.
	
## DC.STREAM_BUFFER 
**Description**: DC.STREAM_BUFFER detects calls to unsafe I/O functions that are accessing a stream buffer (specifically, scanf, fscanf, gets ) that could cause buffer overflow

## DC.STRING_BUFFER
**Description**: DC.STRING_BUFFER detects calls to functions that are accessing string buffers ( sprintf, sscanf, strcat, strcpy, and __builtin___sprintf_chk ) that could cause buffer overflow.
	
## DC.WEAK_CRYPTO
**Description**: DC.WEAK_CRYPTO detects calls to functions that produce unsafe sequences of pseudorandom numbers. The functions match these patterns: initstate, lcong48, rand, random, seed48, setstate, and [dejlmn]rand48 (includes a regular expression that identifies six separate functions). This checker also detects these two LibTomCrypt calls: yarrow_start and rc4_start. These functions set up a pseudorandom number generator but do not seed it.

The functions that DC.WEAK_CRYPTO detects should not be used for encryption, because it is too easy to break the encryption.
	
## DIVIDE_BY_ZERO
**Description**: The DIVIDE_BY_ZERO checker finds instances in which an arithmetic division or modulus occurs when the divisor is zero. The result of such an operation is undefined but typically results in program termination. A defect is only reported if the evidence shows that a divisor can be exactly zero along a particular path.

## FORMAT_STRING_INJECTION
**Description**: FORMAT_STRING_INJECTION finds a security vulnerability that occurs when an unscrutinized value from an untrusted source is used to construct a format string.

## OVERFLOW_BEFORE_WIDEN
**Description**: OVERFLOW_BEFORE_WIDEN finds cases in which the value of an arithmetic expression might overflow before the result is widened to a larger data type. The checker only reports cases where the program already contains a widening operation, but that operation is performed too late. In most cases, the solution is to widen at least one operand to a wider type before performing the arithmetic operation.

The defect indicates that the value of an arithmetic expression is subject to overflow due to a failure to cast operands to a larger data type before performing arithmetic. The failure to cast can yield an unexpected value. For example, an operation that multiplies two integers might yield a value other than a product that results from using arbitrary precision integers (for example, a value other than what a calculator produces when you multiply the two numbers). The checker reports a defect only if it finds some indication that the extra precision is desired.

## PRINTF_ARGS## 
**Description**: PRINTF_ARGS reports invalid printf format strings or invalid arguments to those strings.

## SIGN_EXTENSION## 
**Description**: SIGN_EXTENSION finds many cases where a value is sign-extended when converting from a smaller data type to a larger data type, but it appears that sign extension is not intended because the quantity is essentially unsigned. Most commonly, this happens in code that does a 32-bit endian swap, storing the result in a 64-bit data type. The intermediate 32-bit result must be explicitly cast to an unsigned type in order to suppress the sign extension.

Specifically, it finds cases where the following is true:
- An unsigned quantity is implicitly promoted to a wider signed quantity.
- An arithmetic operation is performed (such as a left-shift) that might lead to the sign bit being set to 1.
- That value is implicitly converted to an even wider type, which causes a possibly unintended sign extension.
The consequence of a sign extension is the result value has all of its high bits set to 1, and consequently is interpreted as a very large value. If this is not intended, then the code will likely misbehave in some application-specific way.