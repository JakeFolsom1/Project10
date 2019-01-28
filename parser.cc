/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include "parser.h"
#include <map>

using namespace std;
struct stmt* head = new stmt;
Token token;
LexicalAnalyzer lexer;
int nextAvailable = 0;
map<string, int> memory;
int memoryStack[100];
int inputStack[100];
int inputAvailable = 0;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    if (token.token_type != expected_type)
        syntax_error();
	token = lexer.GetToken();
    return token;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

//Level -1 :P
void Parser::follow() {
	for (int i = 0; i < sizeof(memoryStack); i++)
	{
		memoryStack[i] = 0;
	}		//Initializing my memory to 0
	head->next = NULL; //Set next = to NULL
	head->stmt_type = NULL; //Used in order to replace head later on, I just wanted to make sure the memory is intact before I reallocate
	parse_input();
}

//Top layer (Level 0)
void Parser::parse_input()
{
	if (token.token_type == NUM)
	{
		parse_inputs();
	}
	else
	{
		parse_program();
	}
}

//Level 1
void Parser::parse_program() {
	if (token.token_type == MAIN)
	{
		parse_main();
	}
	else if (token.token_type == PROC)
	{
		parse_decl_section();
		parse_main();
	}
	else
	{
		syntax_error();
	}

}

//Level 1
void Parser::parse_inputs() {
	inputStack[inputAvailable] = atoi(token.lexeme.c_str());
	expect(NUM);
	inputAvailable++;
	if (peek().token_type == NUM) //Check and see for another inputs
	{
		parse_inputs();
	}
}

//Level 2
void Parser::parse_decl_section() {
		parse_decl();
}

//Level 2
void Parser::parse_main() {
	expect(MAIN);
	parse_procedure_body();
}

//Level 3 and 4
void Parser::parse_procedure_body() {
	parse_statement_list();
}

//Level 3
void Parser::parse_decl(){
	expect(PROC);
	parse_procedure_name();
	parse_procedure_body();
	expect(ENDPROC);
}

//Level 4
void Parser::parse_procedure_name() {
	if (token.token_type == ID)
	{
		expect(ID);
	}
	else if (token.token_type == NUM)
	{
		expect(NUM);
	}
}

//Level 5
void Parser::parse_statement_list() {
		parse_statement();
		Token lookahead = lexer.GetToken();
		lexer.UngetToken(lookahead);
		TokenType peekToken = peek().token_type;
		if ((peekToken== INPUT) || (peekToken == OUTPUT) || (peekToken== DO) || (peekToken == ID)) //Input, output, or do
		{
			parse_statement_list();
		}
		else if (lookahead.token_type == SEMICOLON)
		{
			parse_procedure_invocation();
		}
	}

//Level 6
void Parser::parse_statement() {
	if (token.token_type == INPUT)
	{
		parse_input_statement();
	}
	else if (token.token_type == OUTPUT)
	{
		parse_output_statement();
	}
	else if (token.token_type == DO)
	{
		parse_do_statement();
	}
	else if (token.token_type == ID)
	{
		if (peek().token_type == SEMICOLON)
		{
			parse_procedure_invocation();
		}
		else
		{
			parse_assign_statement();
		}
	}
	else if (token.token_type == NUM)
	{
		parse_procedure_invocation();
	}
	else
	{
		syntax_error();
	}
}

//Level 7
void Parser::parse_input_statement() {
	expect(INPUT);
	//Allocate memory for the input variable if it dosen't exist
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}

	struct stmt *temp = new stmt();
	*temp->stmt_type = INPUT;
	temp->LHS = -1;
	temp->operatorStatement = NOOP;
	temp->op1 = memory[token.lexeme]; //Location in memory of my variable
	temp->op2 = -1;
	temp->next = NULL;
	addToLinkedList(temp);

	expect(ID);
	expect(SEMICOLON);
	return;
}

void Parser::addToLinkedList(stmt* temp) {
	struct stmt* linkedListParser = head;
	if (head->stmt_type == NULL)
	{
		head = temp; //Head is empty
	}
	while (linkedListParser->next != NULL)
	{
		linkedListParser = linkedListParser->next; //Find the last spot in the LL to add the value
	}
	linkedListParser->next = temp; //Add it to the last spot
								   //We are done after this point?
}
//Level 7
void Parser::parse_output_statement() {
	expect(OUTPUT);
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}
	struct stmt *temp = new stmt();
	*temp->stmt_type = OUTPUT;
	temp->LHS = -1;
	temp->operatorStatement = NOOP;
	temp->op1 = memory[token.lexeme]; //Location in memory of my variable
	temp->op2 = -1;
	temp->next = NULL;
	addToLinkedList(temp);
	expect(ID);
	expect(SEMICOLON);
	return;
}

//Level 7
void Parser::parse_do_statement() {
	expect(DO);
	expect(ID);
	parse_procedure_invocation();
}

//Level 7
void Parser::parse_assign_statement() {
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}
	struct stmt *temp = new stmt();
	*temp->stmt_type = ASSIGN;
	temp->LHS = memory[token.lexeme];
	temp->next = NULL;
	expect(ID);
	expect(EQUAL);
	parse_expr(temp);
	expect(SEMICOLON);
}

//Level 8
void Parser::parse_procedure_invocation() {
	parse_procedure_name();
	expect(SEMICOLON);
}

//Level 8
void Parser::parse_expr(struct stmt* temp) {
	temp->op1 = parse_primary();
	if (peek().token_type == PLUS || MINUS || MULT || DIV)
	{
	temp->operatorStatement = parse_operator();
	temp->op2 =	parse_primary();
	addToLinkedList(temp);
	}
}

//Level 9
int Parser::parse_primary() {
	int memoryPass = 0;
	if (token.token_type == ID)
	{
		if (memory[token.lexeme] == NULL)
		{
			allocateMemory(token);
		}
		memoryPass = memory[token.lexeme];
		expect(ID);
	}
	else if (token.token_type == NUM)
	{
		memoryStack[nextAvailable] = atoi(token.lexeme.c_str()); //Adds constants to memory
		nextAvailable++;
		memoryPass = atoi(token.lexeme.c_str());
		expect(NUM);
	}
	else
	{
		syntax_error();
	}
	return memoryPass;
}

//Level 9
TokenType Parser::parse_operator() {
	TokenType returnedToken;
	if (token.token_type == PLUS)
	{
		returnedToken = PLUS;
		expect(PLUS);
	}
	else if (token.token_type == MINUS)
	{
		returnedToken = MINUS;
		expect(MINUS);
	}
	else if (token.token_type == MULT)
	{
		returnedToken = MULT;
		expect(MULT);
	}
	else if (token.token_type == DIV)
	{
		returnedToken = DIV;
		expect(DIV);
	}
	else
	{
		syntax_error();
	}
	return returnedToken;
}

void Parser::allocateMemory(Token token) {
	memory[token.lexeme] = nextAvailable;
	nextAvailable++;
}

int main()
{
		Parser p;
		token = lexer.GetToken();
		p.follow();
}

