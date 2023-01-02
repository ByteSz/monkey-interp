//Andrew Legg, 5-5-2020
/* language grammar specs EBNF

"...." stands for so on and so forth

//Program tokens
<program>   -> <expr>
<expr>      -> <expr>; { <expr>; } | <decl> | <func_call> | <term> <op> <term>

//Actions
<decl>      -> 'let' <id> '=' <term>
<func_call> -> <id>'(' <term_list> ')'

//Lists
<term_list> -> <term> {',' <term>}

//types
<func>      -> fn'(' <term_list> ')' '{' <expr> '}'
<term>      -> <id> | <const>
<const>     -> <int> | <func>
<op>        -> '+'
<id>        -> 'a' | 'b' | 'x' | 'y' | 'add' | ....
<int>       -> 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | ....
<sep>       -> ';'
*/


//Headers
#include <stdio.h>
#include <stdio.h>
#include <string>
#include <vector>

///////////////////////////////
//ENUM VALUES FOR INTERPRETER//
///////////////////////////////
//Error types
enum Error
{
	NONE = 0,
	DECL_NON_ID,
	DECL_ID_NON_ASSIGN,
	ID_ASSIGN_NON_TERM,
	ID_ASSIGN_REF_NOT_FOUND,
	FUNC_MISSING_OPEN_PAREN,
	FUNC_MISSING_CLOSING_PAREN,
	FUNC_SCOPE_NO_OPENING,
	NON_VALID_TOKEN_STATEMENT,
	OP_ADD_RHS_NOT_ID,
	ARG_INCORRECT_AMOUNT,
	ARG_TYPE_MISMATCH,
	UNKNOWN_ACTION,
	FUNC_NOT_DECL
};

//Token types
enum TokenType
{
	PROGRAM = 0,
	EXPR,
	DECL,
	FUNC_CALL,
	OP,
	TERM_LIST,
	FUNC,
	TERM,
	CONST,
	ID,
	INT,
	SEP,
	SEP_OPEN,
	SEP_CLOSE,
	SC_OPEN,
	SC_CLOSE,
	ASSIGN,
	COMMA
};

//Variable types
enum VarType
{
	INTEGER,
	REFERENCE,
	FUNCTION
};

//Types of actions to evaluate
enum ActionType
{
	ADDITION,
	SUBTRACT,
	MULTIPLY,
	DIVISION,
	FUNCTION_CALL
};

//////////////////////////////////
//STRING RESULTS FOR INTERPRETER//
//////////////////////////////////
//Error strings
char* errorStr[] = {
	"No error.",
	"Can't use \'let\' to declare a non identifier!",
	"Missing \'=\' assignment operator for variable declaration!",
	"Can't assign variable to a non-term (const/int/function)!",
	"Could not find variable to for reference variable!",
	"Function declaration missing opening parenthasis!",
	"Function declaration missing closing parenthasis!",
	"Function declaration missing \'{\' character for scope opening!",
	"Token provided cannot be used to make a statement!",
	"Right hand side of operation was not a variable!",
	"Function was called with an unmatching amount of parameters!",
	"Function was called with argument(s) of incorrect type!",
	"Action to evaluate was unknown!",
	"Function that is attempting to be called was not declared!"
};

//Token strings
char* tokenStr[] = {
	"PROGRAM",
	"EXPR",
	"DECL",
	"FUNC_CALL",
	"OP",
	"TERM_LIST",
	"FUNC",
	"TERM",
	"CONST",
	"ID",
	"INT",
	"SEP",
	"SEP_OPEN",
	"SEP_CLOSE",
	"SC_OPEN",
	"SC_CLOSE",
	"ASSIGN",
	"COMMA"
};

//VarType strings
char* varStr[] = {
	"INTEGER",
	"REFERENCE",
	"FUNCTION"
};

//ActionType strings
char* actStr[] = {
	"ADDITION",
	"FUNCTION_CALL"
};

//////////////////////////////////
//TYPES FOR INTERPRETER LANGUAGE//
//////////////////////////////////
//Token struct, for storing type and value
struct Token
{
	//Token's type
	TokenType type = PROGRAM;
	//Value of the token
	std::string value = "";
};

//Struct for the Int type
struct Variable
{
	VarType type;
	std::string identifier = "";
	void* value = NULL;
};

//Struct for functions
struct Function
{
	//Args to copy over as decls
	std::vector<Variable*> args;

	//The index of the function start token index
	int scopeStartIndex = -1;

	//Return value for our function
	int result = 0;
};

//Struct for actions to evaluate
struct Action
{
	//Type of action to perform
	ActionType type;

	//Args to use in operation
	std::vector<Variable*> args;

	//Result of action
	int result = 0;

};

//Stores scripts tokens/data
struct Program
{
	//Array of tokens, in order, for script
	std::vector<Token*> tokens = std::vector<Token*>();

	//Array of variables of different types in function
	std::vector<Variable*> variables = std::vector<Variable*>();

	//Array of functions in the program
	std::vector<Function*> functions = std::vector<Function*>();

	//The array of actions to evaluate
	std::vector<Action*> actions = std::vector<Action*>();

	//The result of our program
	int result = 0;
};

//////////////////////////////////
//UTIL FUNCTIONS FOR INTERPRETER//
//////////////////////////////////
//Check if char is not for names
bool CharIsLetter(char c)
{
	//Check if char is outside of uppercase/lowercase letter range (return false)
	if ((c < 65 || c > 90) && (c < 97 || c > 122)) return false;
	//Otherwise true
	return true;
}

bool CharIsNumber(char c)
{
	//Check if char is outside of number range (return false)
	if (c < 48 || c > 57) return false;
	//Otherwise true
	return true;
}

//Get a variable that a token is associated with
Variable* GetVariable(Program* program, Token token)
{
	//Find the variable associated with token value
	for (int i = 0; i < program->variables.size(); i++)
	{
		if (token.value == program->variables[i]->identifier) return program->variables[i];
	}

	//Could not find var
	return NULL;
}

//DECL token goes here, this function checks for creating a variable
Error MakeVariable(Program* program, int& index)
{
	//Make sure next token is ID
	if (program->tokens[index + 1]->type != ID)
	{
		//Return error
		return DECL_NON_ID;
	}

	//Make sure next token is ASSIGN
	if (program->tokens[index + 2]->type != ASSIGN)
	{
		//Return error
		return DECL_ID_NON_ASSIGN;
	}

	//Create the new variable
	Variable* var = new Variable();

	//Find out the type for the variable
	if (program->tokens[index + 3]->type == INT)
	{
		//Set the variable's type
		var->type = INTEGER;
		//Set the identifier for the variable to the ID token's value
		var->identifier = program->tokens[index + 1]->value;
		//Set the pointer to the data to a new integer's address with the value from the INT token
		var->value = (void*)(new int(std::stoi(program->tokens[index + 3]->value)));
		//Put the variable into the program
		program->variables.push_back(var);
	}
	else if (program->tokens[index + 3]->type == ID)
	{
		//Set the variable's type
		var->type = REFERENCE;
		//Set the identifier for the variable to the ID token's value
		var->identifier = program->tokens[index + 1]->value;

		//Check if variable to reference by identifier was declared before
		for (int i = 0; i < program->variables.size(); i++)
		{
			//Check if this is the correct variable by identifier
			if (program->tokens[index + 3]->value == program->variables[i]->identifier)
			{
				//Set the value to "reference" the other variable by setting it to the same point in memory
				var->value = program->variables[i]->value;
			}
		}

		//Check if reference was found, otherwise return error
		if (var->value == NULL) return ID_ASSIGN_REF_NOT_FOUND;

		//Put the variable into the program's variable array
		program->variables.push_back(var);
	}
	else if (program->tokens[index + 3]->type == FUNC)
	{
		//Check syntax of function, make sure the next token is an opening paren
		if (program->tokens[index + 4]->type != SEP_OPEN) return FUNC_MISSING_OPEN_PAREN;

		//Set the variable's type
		var->type = FUNCTION;
		//Set the identifier for the variable to the ID token's value
		var->identifier = program->tokens[index + 1]->value;
		//Set the pointer to the index of the new function in the function array
		var->value = (void*)(new int(program->functions.size()));

		//Find the functions arguments/parameters identifiers, if there are any
		std::vector<Variable*> args = std::vector<Variable*>();

		//Get the args in the parenthasis
		int i;
		for (i = 0; program->tokens[index + 5 + i]->type != SEP_CLOSE; i++)
		{
			//If even, check for ID
			if (i % 2 == 0 && program->tokens[index + 5 + i]->type == ID)
			{
				//Create a new variable and add it to args
				Variable* a = new Variable();
				a->type = INTEGER;
				a->identifier = program->tokens[index + 5 + i]->value;
				args.push_back(a);
			}
			//Otherwise check for comma and skip
			else if (i % 2 == 1 && program->tokens[index + 5 + i]->type == COMMA) continue;
			//Otherwise the function must be missing a closing parenthasis
			else return FUNC_MISSING_CLOSING_PAREN;
		}


		//Check if the starting token of the function is an opening bracket, return error if not
		if (program->tokens[index + 6 + i]->type != SC_OPEN) return FUNC_SCOPE_NO_OPENING;

		//Make the new function for the program
		Function* func = new Function();
		//Set the args to the args we found
		func->args = args;
		//Set the starting token for the function to the opening bracket
		func->scopeStartIndex = index + 6 + i;

		//Put the function into the program
		program->functions.push_back(func);
		//Put the variable into the program
		program->variables.push_back(var);
	}
	else //IF we don't find the type, then throw an error
	{
		return ID_ASSIGN_NON_TERM;
	}

	//Return no error on success
	return NONE;
}

//////////////////////////////////
//STAGE FUNCTIONS OF INTERPRETER//
//////////////////////////////////
//Lexically analyze a script from the file
Error LexProgram(std::string script, Program* program)
{
	//Tokenize the script
	for (int i = 0; i < script.length(); i++)
	{
		//Character skips
		if (script[i] == ' ') continue; //Skip spaces
		if (script[i] == '\t') continue; //Skip tabs
		if (script[i] == '\n') continue; //Skip new lines

		//Tokenize the expression, get it's token type
		Token* token = new Token();

		//Set the value pre-emptively
		token->value = script[i];

		//Set the token type
		//Single character tokens
		if      (script[i] == '{')   token->type = SC_OPEN;
		else if (script[i] == '}')   token->type = SC_CLOSE;
		else if (script[i] == '(')   token->type = SEP_OPEN;
		else if (script[i] == ')')   token->type = SEP_CLOSE;
		else if (script[i] == '+')   token->type = OP;
		else if (script[i] == '-')   token->type = OP;
		else if (script[i] == '*')   token->type = OP;
		else if (script[i] == '/')   token->type = OP;
		else if (script[i] == '=')   token->type = ASSIGN;
		else if (script[i] == ';')   token->type = SEP;
		else if (script[i] == ',')   token->type = COMMA;
		//Numbers
		else if (CharIsNumber(script[i]))
		{
			//Find the whole number to tokenize
			std::string num = "";
			while(CharIsNumber(script[i]))
			{
				num += script[i];
				i++;
			}

			i--;

			//Set the type to INT
			token->type = INT;

			//Set the value to the number
			token->value = num;
		}
		//Multi-character tokens
		else
		{
			//Find lexeme to tokenize
			std::string lex = "";
			while(CharIsLetter(script[i]) || CharIsNumber(script[i]))
			{
				lex += script[i];
				i++;
			}

			//Move cursor back one so next char isn't missed
			i--;

			//Set the type to correct type
			if (lex == "fn")       token->type = FUNC;
			else if (lex == "let") token->type = DECL;
			else                   token->type = ID;

			//Set the value to the lexeme
			token->value = lex;
		}

		//Store token
		program->tokens.push_back(token);
	}

	//Return success
	return NONE;
}

//Parse through a tokenized script, check for errors
Error ParseProgram(Program* program)
{
	//Go through and create usable data by parsing the tokens
	for (int i = 0; i < program->tokens.size(); i++)
	{
		//Make the type of data we can use for evaluation
		if (program->tokens[i]->type == DECL)
		{
			//Make the variable with the token index for this program
			Error err = MakeVariable(program, i);

			//If there is an error, return it
			if (err != NONE) return err;
		}
		//Function call handling
		else if (program->tokens[i]->type == ID && program->tokens[i+1]->type == SEP_OPEN)
		{
			//Parse the function call and set it to be done on evaluation
			//Make action
			Action* act = new Action();
			//Set action type to function
			act->type = FUNCTION_CALL;
			//Use to see if we find function
			act->result = -1;
			//Check if function variable to call by identifier was declared before
			for (int j = 0; j < program->variables.size(); j++)
			{
				//Check if this is the correct variable by identifier
				if (program->tokens[i]->value == program->variables[j]->identifier)
				{
					//Store function location in result
					act->result = *((int*)program->variables[j]->value);
				}
			}

			//Check that we found function
			if (act->result == -1) return FUNC_NOT_DECL;

			//Make arguments for action
			for(int j = 0; j < program->functions[act->result]->args.size(); j++)
			{
				//Make variable
				Variable* var = new Variable();
				//Set variable type
				var->type = INTEGER;
				//Set variable's value
				var->value = GetVariable(program, *program->tokens[((j + 1) * 2) + i])->value;
				//Put in args
				act->args.push_back(var);
			}

			//Put the action in the array
			program->actions.push_back(act);
		}
		//Operation call handling
		else if (program->tokens[i]->type == ID && program->tokens[i+1]->type == OP)
		{
			//Get the next char
			if (program->tokens[i+2]->type != ID) return OP_ADD_RHS_NOT_ID;

			//Otherwise make action and add to list
			Action* act = new Action();

			//Check type of operation
			if (program->tokens[i+1]->value == "+")
			{
				act->type = ADDITION;
			}
			else if (program->tokens[i+1]->value == "-")
			{
				act->type = SUBTRACT;
			}
			else if (program->tokens[i+1]->value == "*")
			{
				act->type = MULTIPLY;
			}
			else if (program->tokens[i+1]->value == "/")
			{
				act->type = DIVISION;
			}

			//Push first two variables to add
			//Check if variable to reference by identifier was declared before
			for (int j = 0; j < program->variables.size(); j++)
			{
				//Check if this is the correct variable by identifier
				if (program->tokens[i]->value == program->variables[j]->identifier)
				{
					act->args.push_back(program->variables[j]);
				}
			}

			//Check if second variable to reference by identifier was declared before
			for (int j = 0; j < program->variables.size(); j++)
			{
				//Check if this is the correct variable by identifier
				if (program->tokens[i+2]->value == program->variables[j]->identifier)
				{
					act->args.push_back(program->variables[j]);
				}
			}

			//Add action to array of actions
			program->actions.push_back(act);
		}
		//Skipping section
		else if (program->tokens[i]->type == SEP) continue;
		else if (program->tokens[i]->type == SC_CLOSE) continue;
		//Skip scope opening bracket and go to right before scope closing bracket
		else if (program->tokens[i]->type == SC_OPEN) while (program->tokens[i+1]->type != SC_CLOSE) i++;
		//Error section
		else
		{
			//Return error
			return NON_VALID_TOKEN_STATEMENT;
		}

		//Skip to the end of the statement
		while (program->tokens[i]->type != SEP) i++;
	}

	//Return success
	return NONE;
}

//Evaluate a parsed and tokenized script
Error EvalProgram(Program* program)
{
	//Go through actions in order and do them
	for (int i = 0; i < program->actions.size(); i++)
	{
		if (program->actions[i]->type == ADDITION)
		{
			//Set the result back to 0
			program->actions[i]->result = 0;
			//Add each arg into result
			for (int j = 0; j < program->actions[i]->args.size(); j++)
			{
				if (program->actions[i]->args[j]->value == NULL) printf("Arg %i is NULL!", j);
				//printf("Arg %i val: %i\n", j, *((int*)(program->actions[i]->args[j]->value)));
				program->actions[i]->result += *((int*)(program->actions[i]->args[j]->value));
			}
			//Set program result
			program->result = program->actions[i]->result;
		}
		else if (program->actions[i]->type == SUBTRACT)
		{
			//Set the result back to first arg
			program->actions[i]->result = *((int*)(program->actions[i]->args[0]->value));
			//Subtract each arg from result
			for (int j = 1; j < program->actions[i]->args.size(); j++)
			{
				if (program->actions[i]->args[j]->value == NULL) printf("Arg %i is NULL!", j);
				//printf("Arg %i val: %i\n", j, *((int*)(program->actions[i]->args[j]->value)));
				program->actions[i]->result -= *((int*)(program->actions[i]->args[j]->value));
			}
			//Set program result
			program->result = program->actions[i]->result;
		}
		else if (program->actions[i]->type == MULTIPLY)
		{
			//Set the result back to first arg
			program->actions[i]->result = *((int*)(program->actions[i]->args[0]->value));
			//Multiply each arg into result
			for (int j = 1; j < program->actions[i]->args.size(); j++)
			{
				if (program->actions[i]->args[j]->value == NULL) printf("Arg %i is NULL!", j);
				//printf("Arg %i val: %i\n", j, *((int*)(program->actions[i]->args[j]->value)));
				program->actions[i]->result *= *((int*)(program->actions[i]->args[j]->value));
			}
			//Set program result
			program->result = program->actions[i]->result;
		}
		else if (program->actions[i]->type == DIVISION)
		{
			//Set the result back to first arg
			program->actions[i]->result = *((int*)(program->actions[i]->args[0]->value));
			//Add each arg into result
			for (int j = 1; j < program->actions[i]->args.size(); j++)
			{
				if (program->actions[i]->args[j]->value == NULL) printf("Arg %i is NULL!", j);
				//printf("Arg %i val: %i\n", j, *((int*)(program->actions[i]->args[j]->value)));
				program->actions[i]->result /= *((int*)(program->actions[i]->args[j]->value));
			}
			//Set program result
			program->result = program->actions[i]->result;
		}
		else if (program->actions[i]->type == FUNCTION_CALL)
		{
			//Get function location from result
			int func = program->actions[i]->result;
			//Set the result back to 0
			program->actions[i]->result = 0;

			//Make sure we have the same amount of args
			if (program->actions[i]->args.size() != program->functions[func]->args.size()) return ARG_INCORRECT_AMOUNT;
			//Check that args match function args, and set function args equal to values of args
			for (int j = 0; j < program->actions[i]->args.size(); j++)
			{
				//Check to make sure args are the same type
				if (program->actions[i]->args[j]->type != program->functions[func]->args[j]->type) return ARG_TYPE_MISMATCH;

				//Set arg equal to other arg value
				program->functions[func]->args[j]->value = program->actions[i]->args[j]->value;
			}

			//Make a new sub program for function
			Program* subprogram = new Program();
			//Make the variables for function program
			subprogram->variables = program->functions[func]->args;
			//Give tokens for function to the function program
			int index = program->functions[func]->scopeStartIndex + 1;
			while (program->tokens[index]->type != SC_CLOSE)
			{
				subprogram->tokens.push_back(program->tokens[index]);
				index++;
			}

			//Parse sub program
			Error err = ParseProgram(subprogram);

			//Check for sub program parse errors
			if (err != 0)
			{
				printf("Function parsing error!\n");
				return err;
			}

			//Evaluate sub program
			err = EvalProgram(subprogram);

			//Check for sub program eval errors
			if (err != 0)
			{
				printf("Function evaluation error!\n");
				return err;
			}

			//Set the result to the sub program's result
			program->result = subprogram->result;
		}
		else
		{
			return UNKNOWN_ACTION;
		}
	}

	//Return success
	return NONE;
}

//Gets string for given error
std::string ReportError(Error error)
{
	return errorStr[error];
}

//Main function that takes arguments
int main(int argc, char* argv[])
{
	//Initialize variable names array

	//Interpret all monkey files given to us
	for (int i = 1; i < argc; i++)
	{
		//Show the user that we are interpreting their script
		printf("Interpreting script %i: %s\n", i, argv[i]);

		//Create script object, and error object
		Error error = NONE;
		Program program;

		//Open the script as a file
		FILE* file = fopen(argv[i], "r");
		//Get the entire script's text
		std::string script = "";
		char c = fgetc(file);
		while (c != EOF)
		{
			script += c;
			c = fgetc(file);
		}

		//Lexically analyze script
		error = LexProgram(script, &program);
		if (error)
		{
			//Print the error, report the interpretor stopping
			printf("Lexical Error: %s\n", ReportError(error).c_str());
			printf("Stopping interpretor for script.\n");
			//Skip script, advance loop
			continue;
		}
		//Print lexed token results
		/*for (int i = 0; i < program.tokens.size(); i++)
		{
			if (program.tokens[i]->type == SEP)           printf("<%s>\n", tokenStr[SEP]);
			else if (program.tokens[i]->type == SC_OPEN)  printf("\n<%s>\n", tokenStr[SC_OPEN]);
			else if (program.tokens[i]->type == SC_CLOSE) printf("<%s>\n", tokenStr[SC_CLOSE]);
			else                                          printf("<%s>", tokenStr[program.tokens[i]->type]);
		}*/

		//Parse the script
		error = ParseProgram(&program);
		if (error)
		{
			//Print the error, report the interpretor stopping
			printf("Parsing Error: %s\n", ReportError(error).c_str());
			printf("Stopping interpretor for script.\n");
			//Skip script, advance loop
			continue;
		}
		//Print list of actions
		/*for (int i = 0; i < program.actions.size(); i++)
		{
			printf("Actions %i type: %s\n", i, actStr[program.actions[i]->type]);
		}*/

		//Evaluate the script
		error = EvalProgram(&program);
		if (error)
		{
			//Print the error, report the interpretor stopping
			printf("Evaluation Error: %s\n", ReportError(error).c_str());
			printf("Stopping interpretor for script.\n");
			//Skip script, advance loop
			continue;
		}

		//Print the program result
		printf("Result => %i\n", program.result);
	}
}
