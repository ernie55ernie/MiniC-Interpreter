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

// Process ralational operators.
void eval_exp1(int &value){
	int partial_value;
	char op;
	char relops[] = {
		LT, LE, GT, GE, EQ, NE, 0
	};

	eval_exp2(value);
	op = *token;
	if(strchr(relops, op)){
		get_token();
		eval_exp2(partial_value);

		switch(op){	// perform the relational operation
		case LT:
			value = value < partial_value;
			break;
		case LE:
			value = value <= partial_value;
			break;
		case GT:
			value = value > partial_value;
			break;
		case GE:
			value = value >= partial_value;
			break;
		case EQ:
			value = value == partial_value;
			break;
		case NE:
			value = value != partial_value;
			break;
		}
	}
}

// Add or substract two terms.
void eval_exp2(int &value){
	char op;
	int partial_value;
	char okops[] = {
		'(', INC, DEC, '-', '+', 0
	};

	eval_exp3(value);

	while((op = *token) == '+' || op == '-'){
		get_token();

		if(token_type == DELIMITER && 
			!strchr(okops, *token))
			throw InterpExc(SYNTAX);

		eval_exp3(partial_value);

		switch(op){	// add or substract
		case '-':
			value = value - partial_value;
			break;
		case '+':
			value = value + partial_value;
			break;
		}
	}
}

// Multiply or divide two factors.
void eval_exp3(int &value){
	char op;
	int partial_value, t;
	char okops[] = {
		'(', INC, DEC, '-', '+', 0
	};

	eval_exp4(value);

	while((op = *token) == '*' || op == '/'
		|| op == '%'){
		get_token();

		if(token_type == DELIMITER &&
			!strchr(okops, *token))
			throw InterpExc(SYNTAX);

		eval_exp4(partial_value);

		switch(op){	// mul, div, or modulus
		case '*':
			value = value * partial_value;
			break;
		case '/':
			if(partial_value == 0)
				throw InterpExc(DIV_BY_ZERO);
			value = (value) / partial_value;
			break;
		case '%':
			t = (value) / partial_value;
			value = value - (t * partial_value);
			break;
		}
	}
}

// Is a unary +, -, ++, or --.
void eval_exp4(int & value){
	char op;
	char temp;

	op = '\0';
	if(*token == '+' || *token == '-' ||
		*token == INC || *token == DEC){
			temp = *token;
			op = *token;
			get_token();
			if(temp == INC)
				assign_var(token, find_var(token) + 1);
			if(temp == DEC)
				assign_var(token, find_var(token) - 1);
	}

	eval_exp5(value);
	if(op == '-') value = -(value);
}

// Process parenthesized expression,
void eval_exp5(int &value){
	if(*token == '('){
		get_token();

		eval_exp0(value);	// get subexpression

		if(*token != ')')
			throw InterpExc(PAREN_EXPECTED);
		get_token();
	}
	else
		atom(value);
}

// Find value of number, variable, or function.
void atom(int &value){
}

