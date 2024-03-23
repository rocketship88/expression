# expression
expression.c C code for string expression to value

Three functions:

      status = evaluate_d_expression(expr, &ansd);
      status = evaluate_l_expression(expr, &ansl);
      status = evaluate_ll_expression(expr, &ansll);

status will be either 0 for good and the answer will be placed in a variable

      char* expr; // input

      // output, one of 

      double ansd;
      int ansl;
      long long ansll;

Or if a failure (bad expression) then status will be 2 for unbalanced parens, 3 for exceded recursion depth maximum (can set using a #define near the top of the file, currently set to 10) or 1000+n where n is the character position of the error token. The two integer versions are just a wrapper around the double version, which then uses lrint and llrint to round to an integer. However, the long long version has its own version which can be  enabled, see below.

It also contans 2 functions at the bottom, main and etimer which are used to test the program, either interactively or to time an expression run N times. To include the main and build a runnable program:

      gcc -D Main -O2 -o testexpr expression.c  -lm

To also include the debugging trace (written to stdout), used in the interactive run mode (included with the test main) add the -D Debug

      gcc -D Main -D Debug  -O2 -o testexpr expression.c  -lm

To include as a library object, compile, but don't link to create expression.o, only the 3 main functions listed above are external, all the others are internal static:

      gcc -c -O2  expression.c 

A separate function is now included that will be precise with long long integers. It is enabled with the -D Separate_ll_function, as for example:
      
      gcc -c -D Separate_ll_function -O2  expression.c 
      
With the built in main() run testexpr with no args to see the command usage.

The main and timer code are supported only on linux, however the rest of the code can also run on windows. 

Note there is a #define of "windows", near the top of the file that defines the 3 types slightly different. It needs to remain undefined on linux.

The expression currently support ()'s + - * / % ^ ** & ^ << >> |operators (& and | are bitwise and or, ^ is exclusive or, ** is exponention << left shift >> right shift). Also supports functions, currently including abs, floor, int, sqrt, sin, cos, round, and rad (degrees to radians). It's rather easy to add more functions, but they can only have one argument. Operators are a bit trickier to add, but if one studies the code, it's not all that difficult to add more.

The evaluator uses recursion for ()'s and function()'s and there is a #define RECURSION_MAX to return an error if its limit is reached (currently set to 10). The amount of stack space per recursion level is output when run in debug mode (with the main program) and is of the order of 400 bytes, or 4k bytes maximum when set to 10.

There are 2 routines that build up the parsing array data. One, set setupascii, builds the arrays. Once they're built, the routine dump_arrays(filename) can be used to output to a file (and stdout) the arrays as C code which can then be pasted back into the code. There's an #ifdef GENERATE_ARRAYS to see which setup is used. The difference is that these arrays wouldn't need to be written to at runtime, so there'd be nothing written other than local variables on the stack, making the code a pure function. Before you can run dump_arrays, setupascii() has to be run.

Note: The test main program can do the array setup and dump to a file, see the command line usasge.

To add operators, would require some changes to those arrays, and so a new set of arrays would need to be built, and the GENERATE_ARRAYS option can be turned on during testing. Once a new set is built, one uses the built in main to dump out a new set of the arrays with their initializers to replace the current set in the program.  

The algorithm used herein was originally found in an answer to a question posted here: https://stackoverflow.com/questions/60246999/is-there-any-c-function-that-can-evalute-a-string-expression-defined-in-the-stan

