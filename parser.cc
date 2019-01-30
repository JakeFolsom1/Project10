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
#include <math.h>

using namespace std;
struct stmt* head;
Token token;
bool amIInProc = false;
LexicalAnalyzer lexer;
int nextAvailable = 1;
map<string, int> memory;
int memoryStack[100];
int inputStack[100];
int inputAvailable = 0;
map<string, stmt*> procedureMemory;

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
	for (int i = 0; i < sizeof(memoryStack) / sizeof(*memoryStack); i++)
	{
		memoryStack[i] = 0;
	}
	parse_input();
}

//Top layer (Level 0)
void Parser::parse_input(){
	if (token.token_type == NUM)
	{
		parse_inputs();
	}
	else
	{
		parse_program();
		if (token.token_type == NUM)
		{
			parse_inputs(); //WE got extra inputs at the end of the code
		}
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
	token = lexer.GetToken();
	inputAvailable++;
	if (token.token_type == NUM) //Check and see for another inputs
	{
		parse_inputs();
	}
}

//Level 2
void Parser::parse_decl_section() {
		amIInProc = true;
		parse_decl();
		amIInProc = false;
		if (token.token_type == PROC)
		{
			parse_decl_section();
		}
}

//Level 2
void Parser::parse_main() {
	expect(MAIN);
	head = parse_procedure_body();
}

//Level 3 and 4
stmt* Parser::parse_procedure_body() {
 	stmt* temp = parse_statement_list();
	return temp;
}

//Level 3
void Parser::parse_decl(){
	expect(PROC);
	string procedureName = parse_procedure_name();
	stmt* procedure = parse_procedure_body();
	procedureMemory[procedureName] = procedure;
	expect(ENDPROC);
}

//Level 4
string Parser::parse_procedure_name() {
	string procedureName = "";
	if (token.token_type == ID)
	{
		procedureName = token.lexeme;
		expect(ID);
	}
	else if (token.token_type == NUM)
	{
		procedureName = token.lexeme;
		expect(NUM);
	}
	return procedureName;
}

//Level 5
stmt* Parser::parse_statement_list() {
	stmt * st;
	stmt * stl;
	st = parse_statement();
	Token lookahead = lexer.GetToken();
	lexer.UngetToken(lookahead);
	TokenType peekToken = peek().token_type;
	if ((token.token_type== INPUT) || (token.token_type == OUTPUT) || (token.token_type == DO) || ((token.token_type == ID) && (lookahead.token_type == EQUAL) || (lookahead.token_type == SEMICOLON))) //Input, output, or do
		{
			stmt* stlParser = st;
			stl = parse_statement_list();
			while (stlParser->next != NULL)
			{
				stlParser = stlParser->next; //Find the last spot in the LL to add the value
			}
			stlParser->next = stl; //Add it to the last spot
		}
		return st;
	}

//Level 6
stmt* Parser::parse_statement() {
	struct stmt* returnStatement = new stmt();
	if (token.token_type == INPUT)
	{
		returnStatement = parse_input_statement();
	}
	else if (token.token_type == OUTPUT)
	{
		returnStatement = parse_output_statement();
	}
	else if (token.token_type == DO)
	{
		returnStatement = parse_do_statement();
	}
	else if (token.token_type == ID)
	{
		if (peek().token_type == SEMICOLON)
		{
			returnStatement = parse_procedure_invocation();
		}
		else
		{
			returnStatement = parse_assign_statement();
		}
	}
	else if (token.token_type == NUM)
	{
		returnStatement = parse_procedure_invocation();
	}
	else
	{
		syntax_error();
	}
	return returnStatement;
}

//Level 7
stmt* Parser::parse_input_statement() {
	expect(INPUT);
	//Allocate memory for the input variable if it dosen't exist
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}

	struct stmt *temp = new stmt();
	temp->stmt_type = INPUT;
	temp->LHS = -1;
	temp->operatorStatement = NOOP;
	temp->op1 = memory[token.lexeme]; //Location in memory of my variable
	temp->op2 = -1;
	temp->next = NULL;
	//addToLinkedList(temp);
	expect(ID);
	expect(SEMICOLON);
	return temp;
}

void Parser::addToLinkedList(stmt* temp) {
	struct stmt* linkedListParser = head;
	if (head == NULL)
	{
		head = temp; //Head is empty
	}
	else
	{
		while (linkedListParser->next != NULL)
		{
			linkedListParser = linkedListParser->next; //Find the last spot in the LL to add the value
		}
		linkedListParser->next = temp; //Add it to the last spot
	}							   //We are done after this point?
}
//Level 7
stmt* Parser::parse_output_statement() {
	expect(OUTPUT);
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}
	struct stmt *temp = new stmt();
	temp->stmt_type = OUTPUT;
	temp->LHS = -1;
	temp->operatorStatement = NOOP;
	temp->op1 = memory[token.lexeme]; //Location in memory of my variable
	temp->op2 = -1;
	temp->next = NULL;
	//addToLinkedList(temp);
	expect(ID);
	expect(SEMICOLON);
	return temp;
}

//Level 7
stmt* Parser::parse_do_statement() {
	int cheatingThing = 0;
	struct stmt* temp;
	expect(DO);
	cheatingThing = memory[token.lexeme];
	expect(ID);
	temp = parse_procedure_invocation();
	temp->stmt_type = DO;
	temp->op1 = cheatingThing;
	return temp;
}

//Level 7
stmt* Parser::parse_assign_statement() {
	if (memory[token.lexeme] == NULL)
	{
		allocateMemory(token);
	}
	struct stmt *temp = new stmt();
	temp->stmt_type = ASSIGN;
	temp->LHS = memory[token.lexeme];
	temp->next = NULL;
	expect(ID);
	expect(EQUAL);
	temp = parse_expr(temp);
	expect(SEMICOLON);
	return temp;
}

//Level 8
stmt* Parser::parse_procedure_invocation() {
	struct stmt* temp = new stmt();
	temp->operatorStatement = NOOP;
	temp->stmt_type = INVOKE;
	string procedureName = parse_procedure_name();
	temp->procedureName = procedureName;
	expect(SEMICOLON);
	return temp;
}

//Level 8
stmt* Parser::parse_expr(struct stmt* temp) {
	temp->op1 = parse_primary();
	if ((token.token_type == PLUS) || (token.token_type == MINUS) || (token.token_type == MULT) || (token.token_type == DIV)){
	temp->operatorStatement = parse_operator();
	temp->op2 =	parse_primary();
	//addToLinkedList(temp);
	}
	return temp;
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
		memoryPass = nextAvailable;
		nextAvailable++;
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

void execute_program(struct stmt* head)
{
	inputAvailable = 0;
	struct stmt * pc;
	pc = head;
	while (pc != NULL) {
		switch (pc->stmt_type) {
		case ASSIGN: 
			switch (pc->operatorStatement) {
			case PLUS: memoryStack[pc->LHS] = memoryStack[pc->op1] + memoryStack[pc->op2];
			break;
			case MINUS: memoryStack[pc->LHS] = memoryStack[pc->op1] - memoryStack[pc->op2];
			break;
			case MULT: memoryStack[pc->LHS] = memoryStack[pc->op1] * memoryStack[pc->op2];
			break;
			case DIV: memoryStack[pc->LHS] = floor(memoryStack[pc->op1] / memoryStack[pc->op2]);
			break;
			default: memoryStack[pc->LHS] = memoryStack[pc->op1];
			break;
		}
			break;
		case OUTPUT: cout << memoryStack[pc->op1] << " ";
			break;
		case INPUT:
			memoryStack[pc->op1] = inputStack[inputAvailable];
			inputAvailable++;
			break;
		case INVOKE:
		{
			stmt* procedure = procedureMemory[pc->procedureName];
			execute_program(procedure);
			break;
		}
		case DO: 
			int iterations = 0;
			iterations = memoryStack[pc->op1];
			for (iterations; iterations > 0; iterations--)
			{
				execute_program(procedureMemory[pc->procedureName]);
			}
			break;
			}
		pc = pc->next;
	}
}

int main()
{
		Parser p;
			token = lexer.GetToken();
		p.follow();
		execute_program(head);
}

