//LambLexer
//by James 2017/04/20

#include "lex_config.h"
#include "test.h"
#include "token.h"
#include "symseg.h"
#include "lex.h"
#include "error.h"


/*
 * Simple Test Framework
 */
#define EXPECT_EQ_BASE(equality,expect,actual,format)	\
	do {	\
		test_count++;	\
		if (equality){	\
			test_pass++;\
		}else{	\
			main_ret = 1;	\
			fprintf(stderr,"%s:%d: expect: " format " actual: " format "\n",__FILE__,__LINE__,expect,actual);\
		}	\
	}while (0)
	
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_STRING(expect, actual) \
    EXPECT_EQ_BASE(actual.compare(expect) == 0, expect, actual.c_str(), "%s")
#define EXPECT_EQ_SIZE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%zu")


#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define GET_TOKEN_TYPE(idx) source_file.get_token_type(idx)
#define GET_TOKEN_NAME(idx) source_file.get_token_name(idx)

extern std::string cur_line;
extern line cur_line_info;
extern source source_file;
extern std::queue<_error> error_queue;
extern std::queue<_warning> warning_queue;

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;

void lexer::test_lexer() {
	
	/* Test operator */
	const std::vector<std::string> test_operator = {	
		" 			=!><+-*/%&|^>><<~&&||?:,				",
		"			()==!=>=<=+=-=*=/=%=&=|=				",
		"			^=>>=<<=[]{};++--->.::					",
	};
	
	for (std::string s : test_operator) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	int i = 0;
	for ( ; i < 46; i++) {
		EXPECT_EQ_INT(i, GET_TOKEN_TYPE(i));
	}
	
	
	/* Test number */
	const std::vector<std::string> test_number = {
		"			int a = 123;							",
		"			int b = 123.456;						",
		"			int c = 0xff00;							",
		"			int d = 0123;							",
	};
	
	for (std::string s : test_number) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	i += 3;  // index of "123"
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("123", GET_TOKEN_NAME(i));
	i += 5; // index of "123.456"
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("123.456", GET_TOKEN_NAME(i));
	i += 5; // index of "0xff00"
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("0xff00", GET_TOKEN_NAME(i));
	i += 5; // index of "0123"
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("0123", GET_TOKEN_NAME(i));
	
	/* Test identifier */
	const std::vector<std::string> test_identifier = {
		"			int identifier;							",
		"			int _identifier; 						",
		"			int _identifier_1_plus;					",
	};
	
	for (std::string s : test_identifier) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	i += 3; // index of "identifier"
	EXPECT_EQ_INT(C_NAME, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("identifier", GET_TOKEN_NAME(i));
	i += 3; // index of "_identifier"
	EXPECT_EQ_INT(C_NAME, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("_identifier", GET_TOKEN_NAME(i));
	i += 3; // index of "_identifier_1_plus"
	EXPECT_EQ_INT(C_NAME, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("_identifier_1_plus", GET_TOKEN_NAME(i));
	
	/* Test comment */
	const std::vector<std::string> test_comment = {
		"			//This is a single line comment			",
		"			/* This is a							",
		"			 * multi-line							",
		"			 * ע��									",
		"			 */										",
	};
	
	for (std::string s : test_comment) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	i += 2; // The size of token stream
	EXPECT_EQ_SIZE(static_cast<size_t>(i), source_file.get_token_size());
	
	/* Test literal */
	const std::vector<std::string> test_literal = {
		"			string s = \"hello, \\\"world\";		",
		"			char c = 'b';							",
	};
	
	for (std::string s : test_literal) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	i += 3; // index of "hello "world "
	EXPECT_EQ_INT(C_STRING, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("hello, \\\"world", GET_TOKEN_NAME(i));
	i += 5; // index of "b"
	EXPECT_EQ_INT(C_CHAR, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("b", GET_TOKEN_NAME(i));
	
	/* Test error */
	const std::vector<std::string> test_error = {
		"			int b = 123.456.789;					", /* too many decimal points in number */
		"			int c = 09;								", /* invalid digit in octal constant */
		"			string s = \"hello;						", /* missing terminating \" character */
		"			char cc = 'h;							", /* missing terminating \' character */
		"			��										", /* unknown type name */
	};
	
	for (size_t idx = 0; idx < test_error.size(); idx++) {
		cur_line = test_error[idx];
		cur_line_info++;
		lex();
		EXPECT_EQ_INT(static_cast<int>(idx), error_queue.back().get_error_id());
	}
	
	/* Test warning */
	const std::vector<std::string> test_warning = {
		"			cc = 'hello world';						", /* character constant too long for its type */
	};
	
	for (size_t idx = 0; idx < test_warning.size(); idx++) {
		cur_line = test_warning[idx];
		cur_line_info++;
		lex();
		EXPECT_EQ_INT(static_cast<int>(idx), warning_queue.back().get_warning_id());
	}
	
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}














