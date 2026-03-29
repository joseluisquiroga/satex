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

num_t formu::val_t::toNumber()
{
    if (isToken()) return 0;
    if (isNumber()) return number;
    
    number = str_to_num(string.c_str());
    //type |= NUMBER;
    
    return number;
}

std::string formu::val_t::toString()
{
    if (isToken()) return string;
    if (isString()) return string;
    
    char str[16];
    sprintf(str, num_format, number);
    string = str;
    //type |= STRING;
    
    return string;
}

formu::formu()
{
    // 1. Create the operator precedence map.
    opPrecedence["("] = -10;
    opPrecedence["&&"]  = -2; opPrecedence["||"]  = -3; opPrecedence["&"]  = -2; opPrecedence["|"]  = -3;
    opPrecedence[">"]  = -1; opPrecedence["<"]  = -1; 
	opPrecedence["=>"]  = -1; opPrecedence["<="]  = -1; 
	opPrecedence["->"]  = -1; opPrecedence["<-"]  = -1;    
    opPrecedence[">>"] = 1; opPrecedence["<<"] = 1; 
	opPrecedence[">="]  = -1; 
    opPrecedence["=="]  = -1; opPrecedence["="]  = -1; 
	opPrecedence["!="]  = -1; 
    opPrecedence["+"]  = 2; opPrecedence["-"]  = 2;
    opPrecedence["*"]  = 3; opPrecedence["/"]  = 3;
    opPrecedence["^"] = 4;
    opPrecedence["!"] = 5;
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
    //val_fifo_t rpnQueue; 
	//std::stack<std::string> operatorStack;
	curr_id_var = 0;
	var_ids = long_map_t();
	rpnQueue = val_fifo_t();
	operatorStack = str_stack_t();
    bool lastTokenWasOp = true;
    
    // In one pass, ignore whitespace and parse the expression into RPN
    // using Dijkstra's Shunting-yard algorithm.
    while (*expr && isspace(*expr)) ++expr;
    while (*expr)
    {
        if(isdigit(*expr )){
			/*
			char* nextChar = 0;
			num_t digit = strtod(expr, &nextChar);

			rpnQueue.push(digit);
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
			rpnQueue.push(key);
			rpnQueue.back().id_var = get_id_var(key);
			
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
			bin_tok bct = to_bin_const(key);
			if(bct != BAD_BIN){
				str_t ktok = "INVALID_CONSTANT";
				if(bct == TRUE){
					ktok = "true";
				} else if(bct == FALSE){
					ktok = "false";
				} else {
					throw std::domain_error("Unrecognized constant '" + key + "'");					
				}
				rpnQueue.push(val_t(ktok, TOKEN));
			} else {
				rpnQueue.push(key);
				rpnQueue.back().id_var = get_id_var(key);
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
            
            rpnQueue.push(ss.str());
            lastTokenWasOp = false;
        }*/
        else
        {
            // Otherwise, the variable is an operator or paranthesis.
            switch (*expr) {
                case '(':
                    operatorStack.push("(");
                    ++expr;
                    break;
                case ')':
                    while (operatorStack.top().compare("(")) {
                        rpnQueue.push(val_t(operatorStack.top(), TOKEN));
						rpnQueue.back().id_var = ++curr_id_var;
                        operatorStack.pop();
                    }
                    operatorStack.pop();
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
						std::cerr << "UNARY " << str << " detected\n";
                        if (!str.compare("-") || !str.compare("+") || !str.compare("!"))
                            rpnQueue.push(val_t("UNARY_FILL", TOKEN));
                        else
                            throw std::domain_error("Unrecognized unary operator: '" + str + "'");
                        
                    }
                    
                    while (!operatorStack.empty() && opPrecedence[str] <= opPrecedence[operatorStack.top()])
                    {
                        rpnQueue.push(val_t(operatorStack.top(), TOKEN));
						rpnQueue.back().id_var = ++curr_id_var;
                        operatorStack.pop();
                    }
                    operatorStack.push(str);
                    lastTokenWasOp = true;
                }
            }
        }
        while (*expr && isspace(*expr )) ++expr;
    }
    while (!operatorStack.empty()) {
        rpnQueue.push(val_t(operatorStack.top(), TOKEN));
		rpnQueue.back().id_var = ++curr_id_var;
        operatorStack.pop();
    }
    //return rpnQueue;
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
	val_fifo_t& rpn = rpnQueue;
    
    // Evaluate the expression in RPN form.
    val_stack_t evaluation;
    
    while (!rpn.empty()) {
        val_t tok = rpn.front();
        rpn.pop();
        
        if(tok.isToken())
        {
            std::string str = tok.string;
            if (evaluation.size() < 2) {
                throw std::domain_error("Invalid equation.");
            }
            val_t right = evaluation.top(); evaluation.pop();
            val_t left  = evaluation.top(); evaluation.pop();
            if (!str.compare("+") && left.isNumber()){
				std::cout << "ADDITION\n";
                evaluation.push(left.number + right.toNumber());
			}
            else if (!str.compare("+") && left.isString())
                evaluation.push(left.string + right.toString());
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
                else if (left.isString() && right.isString())
                    evaluation.push(left.string == right.string);
                else if (left.isString())
                    evaluation.push(left.string == right.toString());
                else
                    evaluation.push(left.toNumber() == right.toNumber());
            }
            else if (!str.compare("!="))
            {
                if (left.isNumber() && right.isNumber())
                    evaluation.push(left.number != right.number);
                else if (left.isString() && right.isString())
                    evaluation.push(left.string != right.string);
                else if (left.isString())
                    evaluation.push(left.string != right.toString());
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
        else if (tok.isNumber() || tok.isString())
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
	os << "[" << toString() << "," << id_var << "," << val_kind_nm() << "]";
	return os;
}

void
formu::parse_cnf(const char* expr, row<long>& cnf){
    toRPN(expr);
	val_fifo_t& rpn = rpnQueue;

    val_stack_t stk;
	
	cnf.clear();
	
	while(! rpn.empty()){
		val_t tok = rpn.front();
		//std::cout << tok << " ";
		rpn.pop();

        if(tok.isToken()){
            std::string str = tok.string;
			bin_tok bt = to_bin_tok(str);

			bool is_op = (bt != FILL) && (bt != TRUE) && (bt != FALSE);
			if(is_op){
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
						throw std::domain_error("Invalid bin_tok '" + str+ "'.");
				}
			}
		}

		stk.push(tok);
	}
	
	std::cout << "\n-------------------------------\n";
}

formu::bin_tok
formu::to_bin_const(str_t tok){
	bin_tok tt = BAD_BIN;
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

formu::bin_tok
formu::to_bin_tok(str_t tok){
	bin_tok tt = BAD_BIN;
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
	} else if(tok == "UNARY_FILL"){ tt = FILL;
	} else {
		tt = to_bin_const(tok);
	}
	return tt;
}

void
formu::prt_op(std::ostream& os, val_t& op, val_t& lft, val_t& rgt){
	os << "(";
	if((lft.string == "false") || (lft.string == "true")){
		os << lft.string;
	} else if(lft.type == STRING){
		os << "[" << lft.id_var << "." << lft.string << "]";
	} else {
		os << "[" << lft.id_var << "]";
	}
	
	os << " [" << op.id_var << "." << to_nm(op.string) << "] ";
	if((rgt.string == "false") || (rgt.string == "true")){
		os << rgt.string;
	} else if(rgt.type == STRING){
		os << "[" << rgt.id_var << "." << rgt.string << "]";
	} else {
		os << "[" << rgt.id_var << "]";
	}
	os << ")";
}

void
formu::add_and(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_or(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_not(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_then(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_bakthen(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_not_equal(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}

void
formu::add_same(val_t& op, val_t& lft, val_t& rgt, row<long>& cnf){
	prt_op(std::cout, op, lft, rgt);
}


