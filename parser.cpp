// Recursive descent parser for integer expressions.
//
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include "mccommon.h"

using namespace std;

// Keyword lookup table.
// Keywords must be entered lowercase.
struct commands{
	char command[20];
	token_ireps tok;
} com_table[] = {
	"if", IF,
	"else", ELSE,
	"for", FOR,
	"do", DO,
	"while", WHILE,
	"char", CHAR,
	"int", INT,
	"return", RETURN,
	"switch", SWITCH,
	"break", BREAK,
	"case", CASE,
	"cout", COUT,
	"cin", CIN,
	"", END	// mark end of table
};

// This structure links a library function name
// with a pointer to that function.
struct intern_func_type{
	char *f_name;	// function name
	int (*p)();		// pointer to the function
}intern_func[] = {
	"getchar", call_getchar,
	"putchar", call_putchar,
	"abs",call_abs,
	"rand", call_rand,
	"", 0	// null terminate the list
};

// Entry point into parser.
void eval_exp(int &value){
	get_token();

	if(!*token){
		throw InterpExc(NO_EXP);
	}

	if(*token == ';'){
		value = 0;	// empty expression
		return;
	}

	eval_exp0(value);

	putback();	// return last token read to input strean
}

// Process an assignment expression.
void eval_exp0(int &value){
	// temp holds name of var receiving the assignment.
	char temp[MAX_ID_LEN + 1];

	tok_types temp_tok;

	if(token_type == IDENTIFIER){
		if(is_var(token)){
			// if a var, see if assignment
			strcpy_s(temp, token);
			temp_tok = token_type;
			get_token();
			if(*token == '='){	// is an assignment
				get_token();
				eval_exp0(value);	// get value to assign
				assign_var(temp, value);	// assign the value
				return;
			}
			else{	// not an assignment
				putback();	// restore original token
				strcpy_s(token, temp);
				token_type = temp_tok;
			}
		}
	}
	eval_exp1(value);
}



