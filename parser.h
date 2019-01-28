/*
 * Copyright (C) Rida Bazzi, 2019
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

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
	void parse_procedure_body();
	void parse_decl_section();
	void parse_decl();
	void parse_procedure_name();
	void parse_statement_list();
	void parse_statement();
	void parse_input_statement();
	void parse_output_statement();
	void parse_do_statement();
	void parse_procedure_invocation();
	void parse_assign_statement();
	void parse_expr(struct stmt* temp);
	int parse_primary();
	TokenType parse_operator();
	void allocateMemory(Token token);
	void addToLinkedList(stmt* stmt);
};

struct stmt {
	TokenType *stmt_type;
	int LHS;
	TokenType operatorStatement;
	int op1;
	int op2;
	struct stmt * next;
};

#endif

