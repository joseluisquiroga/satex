// Source: http://www.daniweb.com/software-development/cpp/code/427500/calculator-using-shunting-yard-algorithm#
// Source2: https://github.com/bamos/cpp-expression-parser
// Author: Jesse Brown
// Modifications: Brandon Amos, Jose Luis Quiroga
// Modifications: www.K3A.me (changed token class, long numbers, added support for boolean operators, added support for strings)

#ifndef _FORMU_H
#define _FORMU_H

#include <map>
#include <string>
#include <queue>
#include <stack>

#include "print_stm.h"
#include "tools.h"

typedef long num_t;
#define str_to_num 	atol
#define num_format 	"%ld"

/*
typedef float num_t;
#define str_to_num 	atof
#define num_format 	"%f"
*/

#define INVALID_ID_VAR -1

typedef std::string str_t;


class formu
{
public:
    enum val_kind
    {
        UNDEFINED = 0,
        TOKEN =  1,
        NUMBER = 2,
        STRING = 4,
        
    };
	
    enum bin_tok
    {
        BAD_BIN = 0,
        AND,
        OR,
        NOT,
        THEN,
        BAKTHEN,
        EQUAL,
        NOT_EQUAL,
        TRUE,
		FALSE,
        FILL,
		SAME,
    };	

	str_t bin_tok_nm(bin_tok btk){
		str_t nm = "INVALID_BIN_TOK";
		switch(btk){
			case AND: nm = "&"; break;
			case OR: nm = "|"; break;
			case NOT: nm = "-"; break;
			case THEN: nm = ">"; break;
			case BAKTHEN: nm = "<"; break;
			case EQUAL: nm = "="; break;
			case NOT_EQUAL: nm = "!="; break;
			case TRUE: nm = "T"; break;
			case FALSE: nm = "F"; break;
			case FILL: nm = "?"; break;
			case SAME: nm = "+"; break;
			default: break;
		}
		return nm;
	}

	str_t to_nm(str_t tok){
		return bin_tok_nm(to_bin_tok(tok));
	}
	
    bin_tok to_bin_const(str_t tok);
    bin_tok to_bin_tok(str_t tok);
	long	get_id_var(str_t vv);
	
    struct val_t
    {
        //unsigned type = UNDEFINED;
		val_kind type = UNDEFINED;
        num_t number = 0;
        std::string string = "";
		long  id_var = INVALID_ID_VAR;
        
        val_t():type(UNDEFINED){};
        val_t(std::string str, val_kind t):type(t),string(str){};
        val_t(std::string str):type(STRING),string(str){};
        val_t(num_t n):type(NUMBER),number(n){};
        //val_t(val_t& vv):type(vv.type),number(vv.number),string(vv.string),id_var(vv.id_var){};
        val_t& operator=(std::string str) {
            type = STRING;
            string = str;
            return *this;
        }
        val_t& operator=(num_t n) {
            type = NUMBER;
            number = n;
			id_var = INVALID_ID_VAR;
            return *this;
        }
        val_t& operator=(val_t vv) {
            type = vv.type;
            number = vv.number;
            string = vv.string;
            id_var = vv.id_var;
            return *this;
        }
        
        bool isToken()const{ return type == TOKEN; };
        bool isString()const{ return !isToken() && (type & STRING); };
        bool isNumber()const{ return !isToken() && (type & NUMBER); };

		str_t val_kind_nm(){
			str_t nm = "INVALID_KIND";
			switch(type){
				case TOKEN: nm = "TOK"; break;
				case STRING: nm = "STR"; break;
				case NUMBER: nm = "NUM"; break;
				default: break;
			}
			return nm;
		}
		
        num_t toNumber();
        std::string toString();
		
		std::ostream&	print_val_fn(std::ostream& os, bool from_pt = false);		
	};
	
    //typedef std::map<std::string, val_t*> val_map_t;
	typedef std::stack<std::string> str_stack_t;
    typedef std::queue<val_t> val_fifo_t;
    typedef std::stack<val_t> val_stack_t;
    typedef std::map<std::string, int> int_map_t;
    typedef std::map<std::string, long> long_map_t;
	typedef long_map_t::iterator long_map_it_t;
	    
public:
    formu();	
    val_t eval(const char* expr);
    void parse_cnf(const char* expr, row<long>& cnf);
	    
private:
    void toRPN(const char* expr);
	
	void prt_op(std::ostream& os, val_t& op, val_t& lft, val_t& rgt);

	void add_and(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_or(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_not(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_then(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_bakthen(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_not_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_same(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	
    int_map_t opPrecedence;
	
	long		curr_id_var;
	long_map_t	var_ids;
	str_stack_t operatorStack;
    val_fifo_t rpnQueue;
};

DECLARE_PRINT_FUNCS(formu::val_t)

DEFINE_PRINT_FUNCS_WITH(formu::val_t, print_val_fn)

#endif // _FORMU_H