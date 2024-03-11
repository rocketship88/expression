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

Or if a failure (bad expression) then status will be 2 for unbalanced parens, 3 for exceded recursion depth maximum (can set using a #define near the top of the file, currently set to 10) or 1000+n where n is the character position of the error token. The two integer versions are just a wrapper around the double version, which then uses lrint and llrint to round to an integer. 

It also contans 2 functions at the bottom, main and etimer which are used to test the program, either interactively or to time an expression run N times. To use as a library, remove the static declaration from the above 3 functions, and compile:

      gcc -O2 -o testexpr expression.c  -lm

Or to include the debugging trace, used in the interactive run mode (included with the test main) add the -D Debug

      gcc -D Debug  -O2 -o testexpr expression.c  -lm

run testexpr with no args to see the command usage.

The main and timer code are supported only on linux, however the rest of the code can also run on windows. 

The expression currently support ()'s + - * / % ^ ** operators (** and ^ are the same, exponentiation). Also supports functions, currently including abs, floor, int, sqrt, sin, cos, round. It's rather easy to add more functions, but they can only have one argument. Operators are a bit trickier to add, but if one studies the code, it's not all that difficult to add more.

There are 2 routines that build up the parsing array data. One, set setupascii, builds the arrays. Once they're built, the routine dump_arrays(filename) can be used to output to a file (and stdout) the arrays as C code which can then be pasted back into the code. There's an #ifdef GENERATE_ARRAYS to see which version is used. The difference is that these arrays wouldn't need to be written to at runtime, so there'd be nothing written other than local variables on the stack, making the code a pure function. Before you can run dump_arrays, setupascii() has to be run.

Note: The test main program can do the array setup and dump to a file, see the command line usasge.

To add operators, would require some changes to those arrays, and so a new set of arrays would need to be built, and the GENERATE_ARRAYS option would need to be turned on. 
