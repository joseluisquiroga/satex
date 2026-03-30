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

#define INVALID_ID_VAR 0

typedef std::string str_t;

//=================================================================================================
// defines

#define FORMU_DBG(prm) 		DBG(prm)

#define FORMU_CK(prm) 		DBG_CK(prm)

//=================================================================================================
// enums


enum val_kind{
	BUNDEFINED = 0,
	BOPER,
	BVAR,
	BCONST,
};

enum bin_op{
	BAD_OP = 0,
	AND,
	OR,
	NOT,
	THEN,
	BAKTHEN,
	EQUAL,
	NOT_EQUAL,
	SAME,
};	

enum bin_co{
	BAD_CO = 0,
	TRUE,
	FALSE,
};	

bin_co 
to_bin_const(str_t tok);
	
bin_op 
to_bin_op(str_t tok);	

inline
bool
is_unary(bin_op btk){
	bool is_un = false;
	switch(btk){
		case NOT: 
		case SAME:
			is_un = true; break;
		default: break;
	}
	return is_un;
}

inline
str_t 
bin_op_nm(bin_op btk){
	str_t nm = "INVALID_BIN_OP";
	switch(btk){
		case AND: nm = "&"; break;
		case OR: nm = "|"; break;
		case NOT: nm = "-"; break;
		case THEN: nm = ">"; break;
		case BAKTHEN: nm = "<"; break;
		case EQUAL: nm = "="; break;
		case NOT_EQUAL: nm = "!="; break;
		case SAME: nm = "+"; break;
		default: break;
	}
	return nm;
}

inline
str_t 
bin_const_nm(bin_co btk){
	str_t nm = "INVALID_BIN_CO";
	switch(btk){
		case TRUE: nm = "T"; break;
		case FALSE: nm = "F"; break;
		default: break;
	}
	return nm;
}

inline
str_t 
to_nm(str_t tok){
	bin_op op = to_bin_op(tok);
	if(op != BAD_OP){
		return bin_op_nm(op);
	}
	bin_co kk = to_bin_const(tok);
	return bin_const_nm(kk);
}


class formu
{
public:

	long
	get_id_var(str_t vv);
	
    struct val_t
    {
		val_kind 			vl_kind = BUNDEFINED;
        std::string 		vl_str = "";
		long  				vl_id_var = INVALID_ID_VAR;
        
        val_t():vl_kind(BUNDEFINED){};
        val_t(std::string str, val_kind t):vl_kind(t),vl_str(str){};
        val_t(std::string str):vl_kind(BVAR),vl_str(str){};
        val_t& operator=(std::string str) {
            vl_kind = BVAR;
            vl_str = str;
            return *this;
        }
        val_t& operator=(val_t vv) {
            vl_kind = vv.vl_kind;
            vl_str = vv.vl_str;
            vl_id_var = vv.vl_id_var;
            return *this;
        }
        
        bool is_boper()const{ return vl_kind == BOPER; };
        bool is_bconst()const{ return vl_kind == BCONST; };
        bool is_false()const{ return (to_bin_const(vl_str) == FALSE); };
        bool is_true()const{ return (to_bin_const(vl_str) == TRUE); };
		
		long pos(){ return vl_id_var; }
		long neg(){ return -vl_id_var; }
		
		str_t 
		val_kind_nm(){
			str_t nm = "INVALID_KIND";
			switch(vl_kind){
				case BOPER: nm = "BOPER"; break;
				case BVAR: nm = "BVAR"; break;
				case BCONST: nm = "BCONST"; break;
				case BUNDEFINED: nm = "BUNDEFINED"; break;
				default: break;
			}
			return nm;
		}
		
        std::string toString();
		
		std::ostream&	print_val_fn(std::ostream& os, bool from_pt = false);		
	};
	
	typedef std::stack<std::string> 	str_stack_t;
    typedef std::queue<val_t> 			val_fifo_t;
    typedef std::stack<val_t> 			val_stack_t;
    typedef std::map<std::string, int> 	int_map_t;
    typedef std::map<std::string, long> long_map_t;
	typedef long_map_t::iterator 		long_map_it_t;
	    
public:
    formu();	
    val_t eval(const char* expr);
    void parse_cnf(const char* expr, row<long>& cnf);
	    
private:
    int_map_t 		op_preced;
	
	long			curr_id_var;
	long_map_t		var_ids;
	str_stack_t 	op_stack;
    val_fifo_t 		rpn_queue;
	val_stack_t		parse_stack;
	
    void toRPN(const char* expr);
	
	void prt_op(std::ostream& os, val_t& op, val_t& lft, val_t& rgt);

	val_t false_val(){ return val_t(bin_const_nm(FALSE), BCONST); }
	val_t true_val(){ return val_t(bin_const_nm(TRUE), BCONST); }
	
	val_t neg_val(val_t& vv){ 
		val_t nn;
		nn = vv; 
		FORMU_CK(vv.vl_id_var != INVALID_ID_VAR);
		nn.vl_id_var = -vv.vl_id_var; 
		return nn;
	}
	
	void add_and(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_or(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_not(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_then(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_bakthen(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_not_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	void add_same(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf);
	
	void add_false(row<long>& cnf);
	void add1cla(row<long>& cnf, long v1);
	void add2cla(row<long>& cnf, long v1, long v2);
	void add3cla(row<long>& cnf, long v1, long v2, long v3);
};

DECLARE_PRINT_FUNCS(formu::val_t)

DEFINE_PRINT_FUNCS_WITH(formu::val_t, print_val_fn)

#endif // _FORMU_H

