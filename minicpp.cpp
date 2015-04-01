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

// This vector holds info for global variables.
vector<var_type> global_vars;

// This vector holds info for local variables
// and parameters
vector<var_type> local_variable_stack;

// This structure encapsulates function info.
struct func_type{
	char func_name[MAX_ID_LEN + 1];	// name
	token_ireps ret_type;	// return type
	char *loc;	// location of entry point in program
};

// This vector holds info about functions.
vector<func_type> func_table;

// Stack for managing function scopes.
stack<int> func_call_stack;

// Stack for managing nested scopes.
stack<int> nect_scope_stack;

char token[MAX_T_LEN + 1];	// current token
tok_types token_type;	// token type
token_ireps tok;	// internal representation

int ret_value;	// function return value

bool breakfound = false;	// true if break encountered
/*
int main(int argc, char *argv[]){
}*/