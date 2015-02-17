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

