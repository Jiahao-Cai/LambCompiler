#include "lex_config.h"
#include "test.h"
#include "token.h"
#include "symseg.h"
#include "lex.h"


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
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual) \
    EXPECT_EQ_BASE(actual.compare(expect) == 0, expect, actual.c_str(), "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#define GET_TOKEN_TYPE(idx) source_file.get_token_type(idx)
#define GET_TOKEN_NAME(idx) source_file.get_token_name(idx)

extern std::string cur_line;
extern line cur_line_info;
extern source source_file;

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0;

void test_lexer() {
	std::vector<std::string> test_operator = {	
		" 			=!><+-*/%&|^>><<~&&||?:,				",
		"			()==!=>=<=+=-=*=/=%=&=|=				",
		"			^=>>=<<=[]{};++--->.::					",
	};
	
	std::vector<std::string> test_number = {
		"			int a = 123;							",
		"			int b = 123.456;						",
		"			int c = 0xff00;							",
		"			int d = 0123;							",
	};
	
	std::vector<std::string> test_identifier = {
		"			int a;									",
		"			int _b;			 						",
		"			int _c_d_1;								",
	};
	
	std::vector<std::string> test_comment = {
		"			//This is a single line comment			",
		"			/* This is a							",
		"			 * multi-line							",
		"			 * ע��									",
		"			 */										",
	};
	
	std::vector<std::string> test_literal = {
		"			string s = \"hello, \\\"world\";		",
		"			char c = 'b';							",
	};
	
	std::vector<std::string> test_warning_and_error = {
		"			int 3a;									", /* invalid identifier */
		"			����									", /* unknown type name */
		"			int b = 123.456.789;					", /* too many decimal points in number */
		"			int c = 09;								", /* too many decimal points in number */
		"			string s = \"hello;						", /* missing terminating \" character */
		"			char cc = 'h;							", /* missing terminating \' character */
		"			cc = 'hello world';						", /* character constant too long for its type */
	};
	
	/* Test operator */
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
	for (std::string s : test_number) {
		cur_line = s;
		cur_line_info++;
		lex();
	}
	i += 3;
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("123", GET_TOKEN_NAME(i));
	i += 5;
	EXPECT_EQ_INT(C_NUMBER, GET_TOKEN_TYPE(i));
	EXPECT_EQ_STRING("123.456", GET_TOKEN_NAME(i));
	
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}















