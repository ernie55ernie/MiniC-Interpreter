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
vector<var_type> local_var_stack;

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
stack<int> nest_scope_stack;

char token[MAX_T_LEN + 1];	// current token
tok_types token_type;	// token type
token_ireps tok;	// internal representation

int ret_value;	// function return value

bool breakfound = false;	// true if break encountered

int main(int argc, char *argv[]){
	if(argc != 2){
		cout << "Usage; mincpp <filename>\n";
		return 1;
	}

	// Allocate memory for the program.
	try{
		p_buf = new char[PROG_SIZE];
	}catch(bad_alloc exc){
		cout << "Could Not Allocate Porgram Buffer\n";
		return 1;
	}

	// Load the program to execute.
	if(!load_program(p_buf, argv[1])) return 1;

	// Set program pointer to start of program buffer.
	prog = p_buf;

	try{
		// Find the location of all function
		// and local variables in the program.
		prescan();

		// Nect, set up the call to main().

		// Find program starting point.
		prog = find_func("main");

		// Check for incorrect or missing main() function.
		if(!prog){
			cout << "main() Not Found\n";
			return 1;
		}

		// Back up to opening (.
		prog--;

		// Set the first token to main
		strcpy_s(token, "main");

		// Call main() ti start interpreting.
		call();
	}catch(InterpExc exc){
		sntx_err(exc.get_err());
		return 1;
	}catch(bad_alloc exc){
		cout << "Out Of Memory\n";
		return 1;
	}

	return ret_value;
}

// Load a program.
bool load_program(char *p, char *fname){
	int i = 0;
	ifstream in(fname, ios::in | ios::binary);
	if(!in){
		cout << "Caonnot Open file.\n";
		return false;
	}

	do{
		*p = in.get();
		p++; i++;
	}while(!in.eof() && i < PROG_SIZE);

	if(i == PROG_SIZE){
		cout << "Program Too Big\n";
		return false;
	}

	// Null terminate the program. Skip any EOF
	// mark if present in the file.
	if(*(p - 2) == 0x1a) *(p - 2) = '\0';
	else *(p - 1) = '\0';

	in.close();

	return true;
}

// Find the location of all functions in the program
// and store global variables.
void prescan(){
	char *p, *tp;
	char temp[MAX_ID_LEN + 1];
	token_ireps datatype;
	func_type ft;

	// Wheb brace is 0, the current source position
	// is outside of any function.
	int brace = 0;

	p = prog;

	do{
		// Bypass code inside functions
		while(brace){
			get_token();
			if(tok == END) throw InterpExc(UNBAL_BRACES);
			if(*token == '{') brace++;
			if(*token == '}') brace--;
		}

		tp = prog; //Save current position
		get_token();

		// See if global var type of function return type.
		if(tok == CHAR || tok == INT){
			datatype = tok;	// save data type

			get_token();

			if(token_type == IDENTIFIER){
				strcpy_s(temp, token);
				get_token();

				if(*token != '('){	// must be global var
					prog = tp;	// return to start of declaration
					decl_global();
				}else if(*token == '('){	// must be a function	
					
					// See if function already defined.
					for(unsigned i = 0; i < func_table.size(); i++)
						if(!strcmp(func_table[i].func_name, temp))
							throw InterpExc(DUP_FUNC);

					ft.loc = prog;
					ft.ret_type = datatype;
					strcpy_s(ft.func_name, temp);
					func_table.push_back(ft);

					do{
						get_token();
					}while(*token != ')');
					// Next token will now be opening curly
					// brace of function
				}else putback();
			}
		}else{
			if(*token == '{') brace++;
			if(*token == '}') brace--;
		}
	} while(tok != END);
	if(brace) throw InterpExc(UNBAL_BRACES);
	prog = p;
}

// Interpret a single statement of block of code. When
// interp() returns from its inital call, the final
// brace ( or a return) in main() has been encountered.
void interp(){
	int value;
	int block = 0;

	do{
		// Don't interpret until break is handled.
		if(breakfound) return;

		token_type = get_token();

		// See what kind of token is up.
		if(token_type == IDENTIFIER ||
			*token == INC || *token ==DEC){
			// Not a keyword, so process expression.
				putback();	// restore token to input stream for
							// further processing  by eval_exp()
				eval_exp(value);	// process the expression
				if(*token != ';') throw InterpExc(SEMI_EXPECTED);
		}
		else if(token_type == BLOCK){	// block delimiter ?
			if(*token == '{'){	// is a block
				block = 1;	// interpreting block, not statement
				// Record nested scope.
				nest_scope_stack.push(local_var_stack.size());
			}else{	// is a }, so reset scope and return
				// Reset nested scope.
				local_var_stack.resize(nest_scope_stack.top());
				nest_scope_stack.pop();
				return;
			}
		}else // is keyword
			switch(tok){
			case CHAR:
			case INT:	// declare local variables
				putback();
				decl_local();
				break;
			case RETURN:	// return from function call
				func_ret();
				return;
			case IF:	// processa an if statement
				exec_if();
				break;
			case ELSE:	// process an else statement
				find_eob();	// find end of else block
							// and continue execution
				break;
			case WHILE:	// process a while loop
				exec_while();
				break;
			case DO:	// process a do-while loop
				exec_do();
				break;
			case FOR:	// process a for loop
				exec_for();
				break;
			case BREAK:	// handle break
				breakfound = true;

				// Reset nested scope.
				local_var_stack.resize(nest_scope_stack.top());
				nest_scope_stack.pop();
				return;
			case SWITCH:	// handle a switch statement
				exec_switch();
				break;
			case COUT:	// handle console output
				exec_cout();
				break;
			case CIN:	// handle console input
				exec_cin();
				break;
			case END:
				exit(0);
		}
	}while(tok != END && block);
	return;
}

// Return the entry point of the specified function.
// Return NULL if not found.
//char *find_func(char *name){
//}