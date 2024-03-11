# expression
expression string to value
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

Or if a failure (bad expression) then status will be 2 for unbalanced parens, or 1000+n where n is the character position of the error token. The two integer versions are just a wrapper around the double version, which then uses lrint and llrint to round to an integer. 

It also contans 2 function at the bottom, main and etimer which are used to test the program, either interactively or to time an expression run N times. To use as a library, remove the static declaration from the above 3 functions, and compile:

   gcc -O2 -o testexpr expression.c  -lm

Or to include the debugging trace, used in the interactive run mode (including the test main) add the -D Debug

   gcc -D Debug  -O2 -o testexpr expression.c  -lm

run testexpr with no args to see the command usage.

The main and timer code are supported only on linux, however the rest of the code can also run on windows. 

