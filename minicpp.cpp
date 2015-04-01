// A mini C++ interpreter.

#include <iostream>
#include <fstream>
#include <new>
#include <stack>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "mccommon.h"

using namespace std;

char *prog;	// current execution point in source code
char *p_buf;	// points to start of program buffer

// This structure encapsulates the info
// associated with variable.
struct var_type{
	char var_name[MAX_ID_LEN + 1];	//name
	token_ireps v_type;	// data type
	int value;	// value
};