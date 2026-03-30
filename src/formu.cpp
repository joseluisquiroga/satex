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

/*
num_t formu::val_t::toNumber()
{
    if (is_boper()) return 0;
    if (isNumber()) return number;
    
    number = str_to_num(vl_str.c_str());
    //vl_kind |= NUMBER;
    
    return number;
}
*/

std::string formu::val_t::toString()
{
	/*
    if (is_boper()) return vl_str;
    if (is_bvar()) return vl_str;
    
    char str[16];
    sprintf(str, num_format, number);
    vl_str = str;
    //vl_kind |= BVAR;
    */
    return vl_str;
}

formu::formu(){
    // 1. Create the operator precedence map.
	int_map_t& pr = op_preced;
    pr["("] = -10;
    pr["&&"]  = -2; pr["||"]  = -3; pr["&"]  = -2; pr["|"]  = -3;
    pr[">"]  = -1; pr["<"]  = -1; 
	pr["=>"]  = -1; pr["<="]  = -1; 
	pr["->"]  = -1; pr["<-"]  = -1;    
    pr[">>"] = 1; pr["<<"] = 1; 
	pr[">="]  = -1; 
    pr["=="]  = -1; pr["="]  = -1; 
	pr["!="]  = -1; 
    pr["+"]  = 2; pr["-"]  = 2;
    pr["*"]  = 3; pr["/"]  = 3;
    pr["^"] = 4;
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

#define isvariablechar(c) (isalpha(c) || c == '_')
void 
formu::toRPN(const char* expr){
	curr_id_var = 0;
	var_ids = long_map_t();
	rpn_queue = val_fifo_t();
	op_stack = str_stack_t();
    bool lastTokenWasOp = true;
    
	str_stack_t& ostk = op_stack;
	val_fifo_t& rpn = rpn_queue;
    // In one pass, ignore whitespace and parse the expression into RPN
    // using Dijkstra's Shunting-yard algorithm.
    while (*expr && isspace(*expr)) ++expr;
    while (*expr)
    {
        if(isdigit(*expr )){
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
			
            lastTokenWasOp = false;
        }
        else if (isvariablechar(*expr )){
            std::stringstream ss;
            ss << *expr;
            ++expr;
            while(isvariablechar(*expr)){
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

			lastTokenWasOp = false;
        }
        /*else if (*expr == '\'' || *expr == '"')
        {
            // It's a string val_t
            
            char startChr = *expr;
            
            std::stringstream ss;
            ++expr;
            while (*expr && *expr != startChr)
            {
                ss << *expr;
                ++expr;
            }
            if (*expr) expr++;
            
            rpn.push(ss.str());
            lastTokenWasOp = false;
        }*/
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
                    std::stringstream ss;
                    ss << *expr;
                    ++expr;
                    while (*expr && !isspace(*expr ) && !isdigit(*expr )
                           && !isvariablechar(*expr) && *expr != '(' && *expr != ')') {
                        ss << *expr;
                        ++expr;
                    }
                    ss.clear();
                    std::string str;
                    ss >> str;
                    
                    if (lastTokenWasOp) {
                        // Convert unary operators to binary in the RPN.
						//std::cerr << "UNARY " << str << " detected\n";
                        if (!str.compare("-") || !str.compare("+") || !str.compare("!"))
                            rpn.push(val_t("UNARY_FILL", BCONST));
                        else
                            throw std::domain_error("Unrecognized unary operator: '" + str + "'");
                        
                    }
                    
                    while (!ostk.empty() && op_preced[str] <= op_preced[ostk.top()])
                    {
                        rpn.push(val_t(ostk.top(), BOPER));
						rpn.back().vl_id_var = ++curr_id_var;
                        ostk.pop();
                    }
                    ostk.push(str);
                    lastTokenWasOp = true;
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

/*
static bool widcardCompare(std::string str, std::string wc)
{
    return !fnmatch(wc.c_str(), str.c_str(), FNM_NOESCAPE | FNM_PERIOD);
}

formu::val_t 
formu::eval(const char* expr) {
    
    // Convert to RPN with Dijkstra's Shunting-yard algorithm.
    toRPN(expr);
	val_fifo_t& rpn = rpn_queue;
    
    // Evaluate the expression in RPN form.
    val_stack_t evaluation;
    
    while (!rpn.empty()) {
        val_t tok = rpn.front();
        rpn.pop();
        
        if(tok.is_boper())
        {
            std::string str = tok.vl_str;
            if (evaluation.size() < 2) {
                throw std::domain_error("Invalid equation.");
            }
            val_t right = evaluation.top(); evaluation.pop();
            val_t left  = evaluation.top(); evaluation.pop();
            if (!str.compare("+") && left.isNumber()){
				std::cout << "ADDITION\n";
                evaluation.push(left.number + right.toNumber());
			}
            else if (!str.compare("+") && left.is_bvar())
                evaluation.push(left.vl_str + right.toString());
            else if (!str.compare("*"))
                evaluation.push(left.toNumber() * right.toNumber());
            else if (!str.compare("-"))
                evaluation.push(left.toNumber() - right.toNumber());
            else if (!str.compare("/"))
            {
                num_t r = right.toNumber();
                if (r == 0)
                    evaluation.push(0);
                else
                    evaluation.push(left.toNumber() / r);
            }
            else if (!str.compare("<<"))
                evaluation.push((int)left.toNumber() << (int)right.toNumber());
            else if (!str.compare("^"))
                evaluation.push(pow(left.toNumber(), right.toNumber()));
            else if (!str.compare(">>"))
                evaluation.push((int)left.toNumber() >> (int)right.toNumber());
            else if (!str.compare(">"))
                evaluation.push(left.toNumber() > right.toNumber());
            else if (!str.compare(">="))
                evaluation.push(left.toNumber() >= right.toNumber());
            else if (!str.compare("<"))
                evaluation.push(left.toNumber() < right.toNumber());
            else if (!str.compare("<="))
                evaluation.push(left.toNumber() <= right.toNumber());
            else if (!str.compare("&&"))
                evaluation.push(left.toNumber() && right.toNumber());
            else if (!str.compare("||"))
                evaluation.push(left.toNumber() || right.toNumber());
            else if (!str.compare("=="))
            {
                if (left.isNumber() && right.isNumber())
                    evaluation.push(left.number == right.number);
                else if (left.is_bvar() && right.is_bvar())
                    evaluation.push(left.vl_str == right.vl_str);
                else if (left.is_bvar())
                    evaluation.push(left.vl_str == right.toString());
                else
                    evaluation.push(left.toNumber() == right.toNumber());
            }
            else if (!str.compare("!="))
            {
                if (left.isNumber() && right.isNumber())
                    evaluation.push(left.number != right.number);
                else if (left.is_bvar() && right.is_bvar())
                    evaluation.push(left.vl_str != right.vl_str);
                else if (left.is_bvar())
                    evaluation.push(left.vl_str != right.toString());
                else
                    evaluation.push(left.toNumber() != right.toNumber());
            }
            else if (!str.compare("=~"))
                evaluation.push( widcardCompare(left.toString(), right.toString()) );
            else if (!str.compare("!~"))
                evaluation.push( !widcardCompare(left.toString(), right.toString()) );
            else if (!str.compare("!"))
                evaluation.push(!right.toNumber());
            else
                throw std::domain_error("Unknown operator: " + left.toString() + " " + str + " " + right.toString() + ".");
        }
        else if (tok.isNumber() || tok.is_bvar())
        {
            evaluation.push(tok);
        }
        else
        {
            throw std::domain_error("Invalid token '" + tok.toString() + "'.");
        }
        
        //delete tok;
    }
    return evaluation.top();
}
*/

std::ostream&
formu::val_t::print_val_fn(std::ostream& os, bool from_pt){
	os << "[" << toString() << "," << vl_id_var << "," << val_kind_nm() << "]";
	return os;
}

void
formu::parse_cnf(const char* expr, row<long>& cnf){
    toRPN(expr);
	val_fifo_t& rpn = rpn_queue;

	parse_stack = val_stack_t();
    val_stack_t& stk = parse_stack;
	
	cnf.clear();
	
	while(! rpn.empty()){
		val_t tok = rpn.front();
		//std::cout << tok << " ";
		rpn.pop();

        if(tok.is_boper()){
            std::string str = tok.vl_str;
			bin_op bt = to_bin_op(str);

			val_t right = stk.top(); stk.pop();
			val_t left  = stk.top(); stk.pop();

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
	} else if(tok == "UNARY_FILL"){ tt = FILL;
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
	} else if(tok == ">"){ tt = THEN;
	} else if(tok == "<"){ tt = BAKTHEN;
	} else if(tok == "=>"){ tt = THEN;
	} else if(tok == "<="){ tt = BAKTHEN;
	} else if(tok == "->"){ tt = THEN;
	} else if(tok == "<-"){ tt = BAKTHEN;
	} else if(tok == ">>"){ tt = THEN;
	} else if(tok == "<<"){ tt = BAKTHEN;
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
	} else {
		os << "[" << lft.vl_id_var << "]";
	}
	
	os << " [" << op.vl_id_var << "." << to_nm(op.vl_str) << "] ";
	if(rgt.is_false() || rgt.is_true()){
		os << rgt.vl_str;
	} else if(rgt.vl_kind == BVAR){
		os << "[" << rgt.vl_id_var << "." << rgt.vl_str << "]";
	} else {
		os << "[" << rgt.vl_id_var << "]";
	}
	os << ")";
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
	stk.push(op);
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
	stk.push(op);
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
	stk.push(op);
}

void
formu::add_then(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(op);
}

void
formu::add_bakthen(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(op);
}

void
formu::add_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(op);
}

void
formu::add_not_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(op);
}

void
formu::add_same(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
    val_stack_t& stk = parse_stack;
	prt_op(std::cout, op, lft, rgt);
	stk.push(op);
}


