// Source: http://www.daniweb.com/software-development/cpp/code/427500/calculator-using-shunting-yard-algorithm#
// Source2: https://github.com/bamos/cpp-expression-parser
// Author: Jesse Brown
// Modifications: Brandon Amos, Jose Luis Quiroga
// Modifications: www.K3A.me (changed token class, long numbers, added support for boolean operators, added support for strings)
// Licence: MIT

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <math.h>
#include <fnmatch.h>
#include <stdio.h> // sprintf

#include "formu.h"
#include "solver.h"

std::string formu::val_t::toString(){
    return vl_str;
}

formu::formu(){
    // 1. Create the operator precedence map.
	int_map_t& pr = op_preced;
    pr["("] = -10;
	pr["||"]  = -3; pr["|"]  = -3;
    pr["&&"]  = -2; pr["&"]  = -2; 
    pr[">"]  = -1; pr["<"]  = -1; 
	pr["=>"]  = -1; pr["<="]  = -1; 
	pr["->"]  = -1; pr["<-"]  = -1;    
    pr[">>"] = 1; pr["<<"] = 1; 
	pr[">="]  = -1; 
    pr["=="]  = -1; pr["="]  = -1; 
	pr["!="]  = -1; 
    pr["+"]  = 5; pr["-"]  = 5;
    pr["!"] = 5;
}

long
formu::get_id_var(str_t vv){
	long_map_it_t it = var_ids.find(vv);
	if(it == var_ids.end()){
		curr_id_var++;
		var_ids[vv] = curr_id_var;
	}
	return var_ids[vv];
}

void
formu::print_rpn(std::ostream& os){
	val_fifo_t q2 = rpn_queue;
	while(! q2.empty()){
		os << q2.front() << ",";
		q2.pop();
	}
	os << "\n";
}

#define isvariablechar(c) (isalpha(c) || c == '_')
void 
formu::toRPN(const char* expr){
	curr_id_var = 0;
	var_ids = long_map_t();
	rpn_queue = val_fifo_t();
	op_stack = str_stack_t();
    
	str_stack_t& ostk = op_stack;
	val_fifo_t& rpn = rpn_queue;
    // In one pass, ignore whitespace and parse the expression into RPN
    // using Dijkstra's Shunting-yard algorithm.
    while (*expr && isspace(*expr)) ++expr;
    while (*expr)
    {
        if(isdigit(*expr)){
			/*
			char* nextChar = 0;
			num_t digit = strtod(expr, &nextChar);

			rpn.push(digit);
            expr = nextChar;
			*/
			
            std::stringstream ss;
            ss << *expr;
            ++expr;
            while(isdigit(*expr)){
                ss << *expr;
                ++expr;
            }
            
            std::string key = ss.str();
			rpn.push(key);
			rpn.back().vl_id_var = get_id_var(key);			
        }
        else if (isvariablechar(*expr )){
            std::stringstream ss;
            ss << *expr;
            ++expr;
            while(isvariablechar(*expr) || isdigit(*expr)){
                ss << *expr;
                ++expr;
            }
            
            std::string key = ss.str();
			bin_co bct = to_bin_const(key);
			if(bct != BAD_CO){
				if((bct != TRUE) && (bct != FALSE)){
					throw std::domain_error("Unrecognized constant '" + key + "'");					
				}
				rpn.push(val_t(bin_const_nm(bct), BCONST));
			} else {
				rpn.push(key);
				rpn.back().vl_id_var = get_id_var(key);
			}
        }
        else
        {
            // Otherwise, the variable is an operator or paranthesis.
            switch (*expr) {
                case '(':
                    ostk.push("(");
                    ++expr;
                    break;
                case ')':
                    while (ostk.top().compare("(")) {
                        rpn.push(val_t(ostk.top(), BOPER));
						rpn.back().vl_id_var = ++curr_id_var;
                        ostk.pop();
                    }
                    ostk.pop();
                    ++expr;
                    break;
                default:
                {
                    // The token is an operator.
                    //
                    // Let p(o) denote the precedence of an operator o.
                    //
                    // If the token is an operator, o1, then
                    //   While there is an operator token, o2, at the top
                    //       and p(o1) <= p(o2), then
                    //     pop o2 off the stack onto the output queue.
                    //   Push o1 on the stack.
					
                    std::string str;
                    std::string op1(expr, 1);
                    std::string op2 = "";
					
					if(*expr){
						op2 = std::string(expr, 2);
					}
					
					bin_op opk1 = to_bin_op(op1);
					bin_op opk2 = to_bin_op(op2);
					bool is_un = false;
					//MARK_USED(is_un);
					
					if(opk2 != BAD_OP){
						str = op2;
						is_un = is_unary(opk2);
						++expr; ++expr;
					} else if(opk1 != BAD_OP){
						str = op1;
						is_un = is_unary(opk1);
						++expr;
					} else {
						long nln = 0;
						if(op2 != "/*"){
							skip_line(expr, nln);
						} else {
							skip_c_comment(expr, nln);
						}
						break;
					}
					
                    while (!is_un && !ostk.empty() && op_preced[str] <= op_preced[ostk.top()]){
                        rpn.push(val_t(ostk.top(), BOPER));
						rpn.back().vl_id_var = ++curr_id_var;
                        ostk.pop();
                    }
                    ostk.push(str);
                }
            }
        }
        while (*expr && isspace(*expr )) ++expr;
    }
    while (!ostk.empty()) {
        rpn.push(val_t(ostk.top(), BOPER));
		rpn.back().vl_id_var = ++curr_id_var;
        ostk.pop();
    }
}

std::ostream&
formu::val_t::print_val_fn(std::ostream& os, bool from_pt){
	os << "[" << toString() << "," << vl_id_var << "," << val_kind_nm() << "]";
	return os;
}

void
formu::parse_cnf(const char* expr, row<long>& cnf){
	
    toRPN(expr);
	val_fifo_t& rpn = rpn_queue;

	DBG_PRT(50, print_rpn(os));
	
	parse_stack = val_stack_t();
    val_stack_t& stk = parse_stack;
	
	cnf.clear();

	num_ccls = 0;
	num_vars = 0;
	
	while(! rpn.empty()){
		val_t tok = rpn.front();
		//std::cout << tok << " ";
		rpn.pop();

        if(tok.is_boper()){
            std::string str = tok.vl_str;
			bin_op bt = to_bin_op(str);

			val_t right = stk.top(); stk.pop();
			val_t left;
			
			bool is_un = is_unary(bt);
			if(! is_un){
				left = stk.top(); stk.pop();
			}

			//std::cout << "(" << left << " " << str << " " << right << ")";				
			
			switch(bt){
				case AND: add_and(tok, left, right, cnf); break;
				case OR: add_or(tok, left, right, cnf); break;
				case NOT: add_not(tok, left, right, cnf); break;
				case THEN: add_then(tok, left, right, cnf); break;
				case BAKTHEN: add_bakthen(tok, left, right, cnf); break;
				case EQUAL: add_equal(tok, left, right, cnf); break;
				case NOT_EQUAL: add_not_equal(tok, left, right, cnf); break;
				case SAME: add_same(tok, left, right, cnf); break;
				default:
					throw std::domain_error("Invalid binary operator (bin_op). Operator " + str+ " not valid.");
			}
		} else {
			stk.push(tok);
		}
	}

	if(! stk.empty()){
		val_t lft = stk.top(); stk.pop();
		val_t op; val_t rgt;
		prt_op(std::cout, op, lft, rgt);
		if(! lft.is_false() && ! lft.is_true()){
			add1cla(cnf, lft.pos());
		} else if(lft.is_false()){
			add_false(cnf);
		}
	}

	num_vars = curr_id_var;
	
	std::cout << "\n-------------------------------\n";
}

bin_co
to_bin_const(str_t tok){
	bin_co tt = BAD_CO;
	if(tok == "T"){ tt = TRUE;
	} else if(tok == "F"){ tt = FALSE;
	} else if(tok == "true"){ tt = TRUE;
	} else if(tok == "false"){ tt = FALSE;
	} else if(tok == "True"){ tt = TRUE;
	} else if(tok == "False"){ tt = FALSE;
	} else if(tok == "TRUE"){ tt = TRUE;
	} else if(tok == "FALSE"){ tt = FALSE;
	}
	return tt;
}

bin_op
to_bin_op(str_t tok){
	bin_op tt = BAD_OP;
	if(tok == "&&"){ tt = AND;
	} else if(tok == "||"){ tt = OR;
	} else if(tok == "&"){ tt = AND;
	} else if(tok == "|"){ tt = OR;
	} else if(tok == "=>"){ tt = THEN;
	} else if(tok == "<="){ tt = BAKTHEN;
	} else if(tok == "->"){ tt = THEN;
	} else if(tok == "<-"){ tt = BAKTHEN;
	} else if(tok == ">>"){ tt = THEN;
	} else if(tok == "<<"){ tt = BAKTHEN;
	} else if(tok == ">"){ tt = THEN;
	} else if(tok == "<"){ tt = BAKTHEN;
	} else if(tok == "=="){ tt = EQUAL;
	} else if(tok == "="){ tt = EQUAL;
	} else if(tok == "!="){ tt = NOT_EQUAL;
	} else if(tok == "+"){ tt = SAME;
	} else if(tok == "-"){ tt = NOT;
	} else if(tok == "!"){ tt = NOT;
	}
	return tt;
}

void
formu::prt_op(std::ostream& os, val_t& op, val_t& lft, val_t& rgt){
	os << "(";
	if(lft.is_false() || lft.is_true()){
		os << lft.vl_str;
	} else if(lft.vl_kind == BVAR){
		os << "[" << lft.vl_id_var << "." << lft.vl_str << "]";
	} else if(lft.vl_kind != BUNDEFINED){
		os << "[" << lft.vl_id_var << "]";
	}
	if(op.vl_kind != BUNDEFINED){
		os << " [" << op.vl_id_var << "." << to_nm(op.vl_str) << "] ";
	}
	if(rgt.is_false() || rgt.is_true()){
		os << rgt.vl_str;
	} else if(rgt.vl_kind == BVAR){
		os << "[" << rgt.vl_id_var << "." << rgt.vl_str << "]";
	} else if(rgt.vl_kind != BUNDEFINED){
		os << "[" << rgt.vl_id_var << "]";
	}
	os << ")\n";
}

void 
formu::add_false(row<long>& cnf){
	cnf.push(1);
	cnf.push(0);
	cnf.push(-1);
	cnf.push(0);
	num_ccls += 2;
}

void 
formu::add1cla(row<long>& cnf, long v1){
	FORMU_CK(v1 != 0);
	cnf.push(v1);
	cnf.push(0);
	num_ccls++;
}

void 
formu::add2cla(row<long>& cnf, long v1, long v2){
	FORMU_CK(v1 != 0);
	FORMU_CK(v2 != 0);
	cnf.push(v1);
	cnf.push(v2);
	cnf.push(0);
	num_ccls++;
}

void 
formu::add3cla(row<long>& cnf, long v1, long v2, long v3){
	FORMU_CK(v1 != 0);
	FORMU_CK(v2 != 0);
	FORMU_CK(v3 != 0);
	cnf.push(v1);
	cnf.push(v2);
	cnf.push(v3);
	cnf.push(0);
	num_ccls++;
}

void
formu::add_and(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_false() || rgt.is_false()){
		stk.push(false_val());
		return;
	}
	if(lft.is_true()){
		stk.push(rgt);
		return;
	}
	if(rgt.is_true()){
		stk.push(lft);
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);
	
	add3cla(cnf, lft.neg(), rgt.neg(), op.pos());
	add2cla(cnf, lft.pos(), op.neg());
	add2cla(cnf, rgt.pos(), op.neg());
}

void
formu::add_or(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_true() || rgt.is_true()){
		stk.push(true_val());
		return;
	}
	if(lft.is_false()){
		stk.push(rgt);
		return;
	}
	if(rgt.is_false()){
		stk.push(lft);
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);

	add3cla(cnf, lft.pos(), rgt.pos(), op.neg());
	add2cla(cnf, lft.neg(), op.pos());
	add2cla(cnf, rgt.neg(), op.pos());
}

void
formu::add_then(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_true()){
		stk.push(rgt);
		return;
	}
	if(rgt.is_false()){
		stk.push(neg_val(lft));
		return;
	}
	if(lft.is_false() || rgt.is_true()){
		stk.push(true_val());
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);

	// -lft | rgt
	add3cla(cnf, lft.neg(), rgt.pos(), op.neg());
	add2cla(cnf, lft.pos(), op.pos());
	add2cla(cnf, rgt.neg(), op.pos());
}

void
formu::add_bakthen(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_true() || rgt.is_false()){
		stk.push(true_val());
		return;
	}
	if(rgt.is_true()){
		stk.push(lft);
		return;
	}
	if(lft.is_false()){
		stk.push(neg_val(rgt));
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);

	// lft | -rgt
	add3cla(cnf, lft.pos(), rgt.neg(), op.neg());
	add2cla(cnf, lft.neg(), op.pos());
	add2cla(cnf, rgt.pos(), op.pos());
}

void
formu::add_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_true()){
		stk.push(rgt);
		return;
	}
	if(rgt.is_true()){
		stk.push(lft);
		return;
	}
	if(lft.is_false()){
		stk.push(neg_val(rgt));
		return;
	}
	if(rgt.is_false()){
		stk.push(neg_val(lft));
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);

	// XNOR <=> EQ
	add3cla(cnf, lft.neg(), rgt.neg(), op.pos());
	add3cla(cnf, lft.pos(), rgt.pos(), op.pos());
	add3cla(cnf, lft.pos(), rgt.neg(), op.neg());
	add3cla(cnf, lft.neg(), rgt.pos(), op.neg());
}

void
formu::add_not_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(lft.is_true()){
		stk.push(neg_val(rgt));
		return;
	}
	if(lft.is_false()){
		stk.push(rgt);
		return;
	}
	if(rgt.is_true()){
		stk.push(neg_val(lft));
		return;
	}
	if(rgt.is_false()){
		stk.push(lft);
		return;
	}
	FORMU_CK(! lft.is_bconst() && ! rgt.is_bconst());
	stk.push(op);

	// XOR <=> NOT_EQ
	add3cla(cnf, lft.neg(), rgt.neg(), op.neg());
	add3cla(cnf, lft.pos(), rgt.pos(), op.neg());
	add3cla(cnf, lft.pos(), rgt.neg(), op.pos());
	add3cla(cnf, lft.neg(), rgt.pos(), op.pos());
}

void
formu::add_not(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	if(rgt.is_false()){
		stk.push(true_val());
		return;
	}
	if(rgt.is_true()){
		stk.push(false_val());
		return;
	}
	FORMU_CK(! rgt.is_bconst());
	stk.push(neg_val(rgt));

	add2cla(cnf, rgt.neg(), op.neg());
	add2cla(cnf, rgt.pos(), op.pos());
}

void
formu::add_same(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(rgt);
}

void
formu::parse_file(std::string& f_nam, row<long>& inst_ccls)
{
	read_file(f_nam, frm_content);
	file_sha = sha_txt_of_arr((uchar_t*)frm_content.get_data(), frm_content.get_data_sz());
	
	frm_content.push(END_OF_FRM); // it already has room for it

	frm_cursor = frm_content.get_c_array();
	
	parse_cnf(frm_cursor, inst_ccls);

	DBG_PRT(45, os << inst_ccls);
}


