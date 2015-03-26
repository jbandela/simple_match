// Copyright 2015 John R. Bandela
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
// Based on https://github.com/solodon4/Mach7/blob/master/code/test/unit/cppcon-matching.cpp from the mach7 library which has the following copyright
//
//  Mach7: Pattern Matching Library for C++
//
//  Copyright 2011-2013, Texas A&M University.
//  Copyright 2014 Yuriy Solodkyy.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//
//      * Neither the names of Mach7 project nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <memory>

#include "../include/simple_match/simple_match.hpp"
#include "../include/simple_match/some_none.hpp"


#include <string>

struct BoolExp { virtual ~BoolExp() {} };
struct VarExp : BoolExp { VarExp(std::string n) : name(std::move(n)) {} std::string name; };
struct ValExp : BoolExp { ValExp(bool b) : value(b) {} bool value; };
struct NotExp : BoolExp { NotExp(std::shared_ptr<BoolExp> e) : e(std::move(e)) {} std::shared_ptr<BoolExp> e; };
struct AndExp : BoolExp { AndExp(std::shared_ptr<BoolExp> e1, std::shared_ptr<BoolExp> e2) : e1(std::move(e1)), e2(std::move(e2)) {} std::shared_ptr<BoolExp> e1; std::shared_ptr<BoolExp> e2; };
struct OrExp : BoolExp { OrExp(std::shared_ptr<BoolExp> e1, std::shared_ptr<BoolExp> e2) : e1(std::move(e1)), e2(std::move(e2)) {} std::shared_ptr<BoolExp> e1; std::shared_ptr<BoolExp> e2; };


namespace simple_match {
	namespace customization {

		template<>
		struct tuple_adapter<VarExp> {
			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.name));
			}
		};

		template<>
		struct tuple_adapter<ValExp> {
			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.value));
			}
		};
		template<>
		struct tuple_adapter<NotExp> {
			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.e));
			}
		};
		template<>
		struct tuple_adapter<AndExp> {
			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.e1,t.e2));
			}
		};
		template<>
		struct tuple_adapter<OrExp> {
			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.e1,t.e2));
			}
		};
	}
}


#include <iostream>

using namespace simple_match;
using namespace simple_match::placeholders;
void print(const std::shared_ptr<BoolExp>& expression) {



	match(expression,

		some<VarExp>(tup(_x)), [](auto&& name) {std::cout << name; },
		some<ValExp>(tup(_x)), [](auto&& value) {std::cout << value; },
		some<NotExp>(tup(_x)), [](auto&& e1) {std::cout << '!'; print(e1); },
		some<AndExp>(tup(_x, _x)), [](auto&& e1, auto&& e2) {std::cout << '('; 
			print(e1); 
			std::cout << " & ";
			print(e2);
			std::cout << ')';
		},
		some<OrExp>(tup(_x, _x)), [](auto&& e1, auto&& e2) {std::cout << '(';
			print(e1);
			std::cout << " | ";
			print(e2);
			std::cout << ')';
		}

		);
}


std::shared_ptr<BoolExp> copy(const std::shared_ptr<BoolExp>& expression)
{
	return match(expression,

		some<VarExp>(tup(_x)), [](auto&& name) {return std::shared_ptr<BoolExp>(std::make_shared<VarExp>(name)); },
		some<ValExp>(tup(_x)), [](auto&& value) {return std::shared_ptr<BoolExp>(std::make_shared<ValExp>(value)); },
		some<NotExp>(tup(_x)), [](auto&& e1) {return std::shared_ptr<BoolExp>(std::make_shared<NotExp>(copy(e1))); },
		some<AndExp>(tup(_x, _x)), [](auto&& e1, auto&& e2) {return std::shared_ptr<BoolExp>(std::make_shared<AndExp>(copy(e1),copy(e2))); },
		some<OrExp>(tup(_x, _x)), [](auto&& e1, auto&& e2) {return std::shared_ptr<BoolExp>(std::make_shared<OrExp>(copy(e1), copy(e2))); }

	);
}

#include <map>

template <class K, class T, class C, class A>
std::ostream& operator<<(std::ostream& os, const std::map<K, T, C, A>& m)
{
	for (typename std::map<K, T, C, A>::const_iterator p = m.begin(); p != m.end(); ++p)
	{
		os << p->first << '=' << p->second << std::endl;
	}

	return os;
}

typedef std::map<std::string, bool> Context;

bool eval(Context& ctx, const std::shared_ptr<BoolExp>& expression)
{

	return match(expression,

		some<VarExp>(tup(_x)), [&](auto&& name) {return ctx[name]; },
		some<ValExp>(tup(_x)), [&](auto&& value) {return value; },
		some<NotExp>(tup(_x)), [&](auto&& e1) {return !eval(ctx,e1); },
		some<AndExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {return eval(ctx,e1) && eval(ctx,e2); },
		some<OrExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {return eval(ctx,e1) || eval(ctx,e2); }

	);

}

std::shared_ptr<BoolExp> replace(const std::shared_ptr<BoolExp>& where, const std::string& what, const std::shared_ptr<BoolExp>& with)
{
	return match(where,

		some<VarExp>(tup(what)), [&]() {return copy(with); },
		some<VarExp>(tup(_)), [&]() {return copy(where); },
		some<ValExp>(_), [&]() {return copy(where); },
		some<NotExp>(tup(_x)), [&](auto&& e1) {return std::shared_ptr<BoolExp>(std::make_shared<NotExp>(replace(e1,what,with))); },
		some<AndExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {return std::shared_ptr<BoolExp>(std::make_shared<AndExp>(replace(e1, what, with), replace(e2, what, with))); },
		some<OrExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {return std::shared_ptr<BoolExp>(std::make_shared<OrExp>(replace(e1, what, with), replace(e2, what, with))); }

	);
}

std::shared_ptr<BoolExp> inplace(std::shared_ptr<BoolExp>& where, const std::string& what, const std::shared_ptr<BoolExp>& with)
{

	return match(where,

		some<VarExp>(tup(what)), [&]() {return copy(with); },
		some<VarExp>(tup(_)), [&]() {return where; },
		some<ValExp>(_), [&]() {return where; },
		some<NotExp>(tup(_x)), [&](auto&& e1) {e1 = inplace(e1, what, with); return where; },
		some<AndExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {e1 = inplace(e1, what, with); e2 = inplace(e2, what, with); return where; },
		some<OrExp>(tup(_x, _x)), [&](auto&& e1, auto&& e2) {e1 = inplace(e1, what, with); e2 = inplace(e2, what, with); return where; }

	);

}



bool equal(const std::shared_ptr<BoolExp>& x1, const std::shared_ptr<BoolExp>& x2)
{

	return match(std::tie(x1, x2),
		tup(some<VarExp>(tup(_x)), some<VarExp>(tup(_x))), [](auto&& v1, auto&& v2) {return v1 == v2; },
		tup(some<ValExp>(tup(_x)), some<ValExp>(tup(_x))), [](auto&& v1, auto&& v2) {return v1 == v2; },
		tup(some<NotExp>(tup(_x)), some<NotExp>(tup(_x))), [](auto&& v1, auto&& v2) {return equal(v1, v2); },
		tup(some<AndExp>(tup(_x,_x)), some<AndExp>(tup(_x,_x))), [](auto&& v1, auto&& v2, auto&& w1, auto&& w2) {return equal(v1, w1) && equal(v2,w2); },
		tup(some<OrExp>(tup(_x,_x)), some<OrExp>(tup(_x,_x))), [](auto&& v1, auto&& v2, auto&& w1, auto&& w2) {return equal(v1, w1) && equal(v2,w2); },
		tup(none(), none()), []() {return true; },
		otherwise, []() {return false; }


	);

}

typedef std::map<std::string, std::shared_ptr<BoolExp>> Assignments;

// Other example: unify
bool bool_match(const std::shared_ptr<BoolExp>& p, const std::shared_ptr<BoolExp>& x, Assignments& ctx)
{

	return match(std::tie(p, x),
		tup(some<VarExp>(tup(_x)), _), [&](auto&& name) {
			if (ctx[name] == nullptr) {
				ctx[name] = copy(x);
				return true;
			}
			else {
				return equal(ctx[name], x);
			}
		},
		tup(some<ValExp>(tup(_x)), some<ValExp>(tup(_x))), [](auto&& v1, auto&& v2) {return v1 == v2; },
		tup(some<NotExp>(tup(_x)), some<NotExp>(tup(_x))), [&](auto&& p1, auto&& e1) {return bool_match(p1, e1, ctx); },
		tup(some<AndExp>(tup(_x, _x)), some<AndExp>(tup(_x, _x))), [&](auto&& p1, auto&& p2, auto&& e1, auto&& e2) {return bool_match(p1, e1,ctx) && bool_match(p2, e2, ctx); },
		tup(some<OrExp>(tup(_x, _x)), some<OrExp>(tup(_x, _x))), [&](auto&& p1, auto&& p2, auto&& e1, auto&& e2) {return bool_match(p1, e1,ctx) && bool_match(p2, e2, ctx); },
		tup(none(), none()), []() {return true; },
		otherwise, []() {return false; }


	);

}

int main() {
	std::shared_ptr<BoolExp> exp1=  std::make_shared<AndExp>(std::make_shared<OrExp>(std::make_shared<VarExp>("X"), std::make_shared<VarExp>("Y")), std::make_shared<NotExp>(std::make_shared<VarExp>("Z")));

	std::cout << "exp1 = "; print(exp1); std::cout << std::endl;



	auto exp2 = copy(exp1);

	std::cout << "exp2 = "; print(exp2); std::cout << std::endl;

	auto exp3 = replace(exp1, "Z", exp2);

	std::cout << "exp3 = "; print(exp3); std::cout << std::endl;

	auto exp4 = inplace(exp1, "Z", exp2);

	std::cout << "exp4 = "; print(exp4); std::cout << std::endl;
	std::cout << "exp1 = "; print(exp1); std::cout << " updated! " << std::endl;

	std::cout << (equal(exp1, exp4) ? "exp1 == exp4" : "exp1 <> exp4") << std::endl;
	std::cout << (equal(exp1, exp2) ? "exp1 == exp2" : "exp1 <> exp2") << std::endl;

	Context ctx;
	ctx["Y"] = true;
	std::cout << eval(ctx, exp1) << std::endl;
	std::cout << eval(ctx, exp2) << std::endl;
	std::cout << eval(ctx, exp3) << std::endl;

	std::cout << ctx << std::endl;



	Assignments ctx2;

	if (bool_match(exp2, exp3, ctx2))
	{
		std::cout << "exp2 matches exp3 with assignments: " << std::endl;

		for (Assignments::const_iterator p = ctx2.begin(); p != ctx2.end(); ++p)
		{
			std::cout << p->first << '='; print(p->second); std::cout << std::endl;
		}
	}
}