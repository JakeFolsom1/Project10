/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"
using namespace std;

class Parser {
public:
	void follow();
private:
	LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();
	void parse_input();
	void parse_inputs();
	void parse_program();
	void parse_main();
	struct stmt* parse_procedure_body();
	void parse_decl_section();
	void parse_decl();
	string parse_procedure_name();
	struct stmt* parse_statement_list();
	struct stmt* parse_statement();
	struct stmt* parse_input_statement();
	struct stmt* parse_output_statement();
	struct stmt* parse_do_statement();
	struct stmt* parse_procedure_invocation();
	struct stmt* parse_assign_statement();
	struct stmt* parse_expr(struct stmt* temp);
	int parse_primary();
	TokenType parse_operator();
	void allocateMemory(Token token);
	void addToLinkedList(stmt* stmt);
};

struct stmt {
	TokenType stmt_type;
	int LHS;
	TokenType operatorStatement;
	int op1;
	int op2;
	struct stmt * procCode;
	struct stmt * next;
	string procedureName;
};

#endif

