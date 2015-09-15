#include <tuple>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include "../include/simple_match/simple_match.hpp"
#include <iostream>

#include "../include/simple_match/boost/any.hpp"
#include "../include/simple_match/boost/variant.hpp"
#include "../include/simple_match/utility.hpp"


struct point {
	int x;
	int y;
	point(int x_, int y_) :x(x_), y(y_) {}
};


auto simple_match_get_tuple(const point& p) {
	return std::tie(p.x, p.y);
}




void test_some_none() {
	std::unique_ptr<int> nothing;
	using namespace simple_match;
	using namespace simple_match::placeholders;

	auto five = std::make_unique<int>(5);
	auto ten = std::make_unique<int>(10);
	auto twelve = std::make_unique<int>(12);

	auto m = [](auto&& v) {
		match(v,
			some(5), []() {std::cout << "five\n"; },
			some(11 <= _x <= 20), [](int x) {std::cout << x << " is on the range [11,20] \n"; },
			some(), [](int x) {std::cout << x << "\n"; },
			none(), []() {std::cout << "Nothing\n"; }
		);
	};

	m(nothing);
	m(five);
	m(ten);
	m(twelve);

}

void test_ds(){

	using namespace simple_match;
	using namespace simple_match::placeholders;

	auto m = [](auto&& v) {
		match(v,
			some(ds(1,2)), []() {std::cout << "one,two\n"; },
			some(ds(_x,_y)), [](int x, int y) {std::cout << x << " " << y << "\n"; },
			none(), []() {std::cout << "Nothing\n"; }
		);
	};


	auto tup_12 = std::make_unique < std::tuple<int, int>>(1, 2);
	auto point_12 = std::make_unique <point>(point{ 1, 2 });
	auto point_34 = std::make_unique <point>(point{ 3, 4 });

	m(tup_12);
	m(point_12);
	m(point_34);



}


void test_string() {

	std::string s = "Test";

	using namespace simple_match;
	using namespace simple_match::placeholders;
	match(s,
		
		"One", []() {std::cout << "one\n"; },
		"Test", []() {std::cout << "two \n"; },
		otherwise, []() {std::cout << "did not match \n"; }
	);

}



struct add;
struct sub;
struct neg;
struct mul;

using math_variant_t = boost::variant<boost::recursive_wrapper<add>, boost::recursive_wrapper<sub>, boost::recursive_wrapper<neg>, boost::recursive_wrapper<mul>,int >;

struct add {
	math_variant_t left;
	math_variant_t right;
};
struct sub {
	math_variant_t left;
	math_variant_t right;
};
struct neg {
	math_variant_t value;
};
struct mul {
	math_variant_t left;
	math_variant_t right;
};


struct eval_t {};
using eval_any = simple_match::tagged_any<eval_t>;

struct add_tag {};
struct sub_tag {};
struct mul_tag {};
struct neg_tag {};



using add2 = simple_match::tagged_tuple<add_tag, eval_any, eval_any>;
using sub2 = simple_match::tagged_tuple<sub_tag, eval_any, eval_any>;
using mul2 = simple_match::tagged_tuple<mul_tag, eval_any, eval_any>;
using neg2 = simple_match::tagged_tuple<neg_tag, eval_any>;

struct add3;
struct sub3;
struct neg3;
struct mul3;

using math_variant2_t = boost::variant<boost::recursive_wrapper<add3>, boost::recursive_wrapper<sub3>, boost::recursive_wrapper<neg3>, boost::recursive_wrapper<mul3>,int >;

struct add3 :std::tuple<math_variant2_t, math_variant2_t> {
	template<class... T>
	add3(T&&...t) :tuple(std::forward<T>(t)...) {}
};
struct sub3 :std::tuple<math_variant2_t, math_variant2_t> {
	template<class... T>
	sub3(T&&...t) :tuple(std::forward<T>(t)...) {}
};
struct mul3 :std::tuple<math_variant2_t, math_variant2_t> {
	template<class... T>
	mul3(T&&...t) :tuple(std::forward<T>(t)...) {}
};
struct neg3 :std::tuple<math_variant2_t> {
	template<class... T>
	neg3(T&&...t) :tuple(std::forward<T>(t)...) {}
};


int eval(const math_variant_t& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add>(), [](auto&& a) {return eval(a.left) + eval(a.right);},
		some<sub>(), [](auto&& a) {return eval(a.left) - eval(a.right);},
		some<neg>(), [](auto&& a) {return -eval(a.value);},
		some<mul>(), [](auto&& a) {return eval(a.left) * eval(a.right);},
		some<int>(), [](auto a) {return a;}

		);



}
namespace simple_match {
	namespace customization {

		template<>
		struct exhaustiveness_checker<eval_any> {
			using type = some_exhaustiveness<add2,sub2,mul2,neg2,int>;
		};
	}
}

int eval(const eval_any& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add2>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval(x) + eval(y);},
		some<sub2>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval(x) - eval(y);},
		some<neg2>(ds(_x)), [](auto&& x) {return -eval(x);},
		some<mul2>(ds(_x,_y)), [](auto&& x,auto&& y) {return eval(x) * eval(y);},
		some<int>(), [](auto x) {return x;}

	);



}


int eval(const math_variant2_t& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval(x) + eval(y);},
		some<sub3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval(x) - eval(y);},
		some<mul3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval(x) * eval(y);},
		some<neg3>(ds(_x)), [](auto&& x) {return -eval(x);},
		some<int>(), [](auto x) {return x;}
	);



}


struct eval_base_t { virtual ~eval_base_t() {} };
struct int_holder_tag {};


using add4 = simple_match::inheriting_tagged_tuple<eval_base_t,add_tag, std::unique_ptr<eval_base_t>, std::unique_ptr<eval_base_t>>;
using sub4 = simple_match::inheriting_tagged_tuple<eval_base_t,sub_tag, std::unique_ptr<eval_base_t>, std::unique_ptr<eval_base_t>>;
using mul4 = simple_match::inheriting_tagged_tuple<eval_base_t,mul_tag, std::unique_ptr<eval_base_t>, std::unique_ptr<eval_base_t>>;
using neg4 = simple_match::inheriting_tagged_tuple<eval_base_t,neg_tag, std::unique_ptr<eval_base_t>>;
using int_holder = simple_match::inheriting_tagged_tuple<eval_base_t,int_holder_tag, int>;


namespace simple_match {
	namespace customization {

		template<>
		struct exhaustiveness_checker<std::unique_ptr<eval_base_t>> {
			using type = some_exhaustiveness<add4, sub4, mul4, neg4, int_holder>;
		};
	}
}



int eval(const std::unique_ptr<eval_base_t>& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add4>(ds(_x, _y)), [](auto&& x, auto&& y) {return eval(x) + eval(y);},
		some<sub4>(ds(_x, _y)), [](auto&& x, auto&& y) {return eval(x) - eval(y);},
		some<mul4>(ds(_x, _y)), [](auto&& x, auto&& y) {return eval(x) * eval(y);},
		some<neg4>(ds(_x)), [](auto&& x) {return -eval(x);},
		some<int_holder>(ds(_x)), [](auto x) {return x;}
	);



}

void FizzBuzz() {

	using namespace simple_match;
	using namespace simple_match::placeholders;
	for (int i = 1; i <= 100; ++i) {
		match(std::make_tuple(i%3 == 0,i%5 == 0),
			ds(true, true ), []() {std::cout << "FizzBuzz\n";},
			ds(true, _), []() {std::cout << "Fizz\n";},
			ds(_, true), []() {std::cout << "Buzz\n";},
			_, [i]() {std::cout << i << "\n";}
		);
	}
}



// Adapted from https://github.com/solodon4/Mach7/blob/master/code/test/unit/cppcon-matching.cpp

struct BoolExp { virtual ~BoolExp() {} };

struct VarExpTag {};
struct ValExpTag {};
struct NotExpTag {};
struct AndExpTag {};
struct OrExpTag {};

using VarExp = simple_match::inheriting_tagged_tuple<BoolExp, VarExpTag, std::string>; 
using ValExp = simple_match::inheriting_tagged_tuple<BoolExp, ValExpTag, bool>; 
using NotExp = simple_match::inheriting_tagged_tuple<BoolExp, NotExpTag, std::unique_ptr<BoolExp>>; 
using AndExp = simple_match::inheriting_tagged_tuple<BoolExp, AndExpTag, std::unique_ptr<BoolExp>,std::unique_ptr<BoolExp>>; 
using OrExp = simple_match::inheriting_tagged_tuple<BoolExp, OrExpTag, std::unique_ptr<BoolExp>,std::unique_ptr<BoolExp>>; 

namespace simple_match {
	namespace customization {

		template<class D>
		struct exhaustiveness_checker<std::unique_ptr<BoolExp,D>> {
			using type = some_exhaustiveness<VarExp, ValExp, NotExp, AndExp, OrExp>;
		};
	}
}

template<class T, class... A>
std::unique_ptr<BoolExp> make_bool_exp(A&&... a) {
	return std::make_unique<T>(std::forward<A>(a)...);
}




void print(const std::unique_ptr<BoolExp>& exp)
{
	using namespace simple_match;
	using namespace simple_match::placeholders;

	match(exp,
		some<VarExp>(ds(_x)), [](auto& x) {std::cout << x;},
		some<ValExp>(ds(_x)), [](auto& x) {std::cout << x;},
		some<NotExp>(ds(_x)), [](auto& x) {std::cout << '!'; print(x);},
		some<AndExp>(ds(_x, _y)), [](auto& x, auto& y) {std::cout << '('; print(x);std::cout << " & "; print(y); std::cout << ')';},
		some<OrExp>(ds(_x, _y)), [](auto& x, auto& y) {std::cout << '('; print(x);std::cout << " | "; print(y); std::cout << ')';}
	);


}


std::unique_ptr<BoolExp> copy(const std::unique_ptr<BoolExp>& exp)
{	
	using namespace simple_match;
	using namespace simple_match::placeholders;


	
	return match(exp,
		some<VarExp>(ds(_x)), [](auto& x) {return make_bool_exp<VarExp>(x);},
		some<ValExp>(ds(_x)), [](auto& x) {return make_bool_exp<ValExp>(x);},
		some<NotExp>(ds(_x)), [](auto& x) {return make_bool_exp<NotExp>(copy(x));},
		some<AndExp>(ds(_x,_y)), [](auto& x, auto& y) {return make_bool_exp<AndExp>(copy(x),copy(y));},
		some<OrExp>(ds(_x,_y)), [](auto& x, auto& y) {return make_bool_exp<OrExp>(copy(x),copy(y));}
	);

}

#include <map>

template <class K, class T, class C, class A>
std::ostream& operator<<(std::ostream& os, const std::map<K, T, C, A>& m)
{
	for (auto&& p:m)
	{
		os << p.first << '=' << p.second << std::endl;
	}

	return os;
}
typedef std::map<std::string, bool> Context;

bool eval(Context& ctx, const std::unique_ptr<BoolExp>& exp)
{
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return match(exp,
		some<VarExp>(ds(_x)), [&](auto& x)mutable {return ctx[x];},
		some<ValExp>(ds(_x)), [](auto& x) {return x;},
		some<NotExp>(ds(_x)), [&](auto& x)mutable {return !eval(ctx,x);},
		some<AndExp>(ds(_x, _y)), [&](auto& x, auto& y)mutable {return eval(ctx,x) && eval(ctx,y);},
		some<OrExp>(ds(_x, _y)), [&](auto& x, auto& y)mutable {return eval(ctx,x) || eval(ctx,y);}
	);

}

std::unique_ptr<BoolExp> replace(const std::unique_ptr<BoolExp>& where, const std::string& what, const std::unique_ptr<BoolExp>& with)
{
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return match(where,
		some<VarExp>(ds(_x)), [&](auto& x){
			if (what == x) {
				return copy(with);
			}
			else {
				return make_bool_exp<VarExp>(x);
			}
		},
		some<ValExp>(), [&](auto& x) {return copy(where);},
		some<NotExp>(ds(_x)), [&](auto& x) {return make_bool_exp<NotExp>(replace(x,what,with));},
		some<AndExp>(ds(_x,_y)), [&](auto& x, auto& y) {return make_bool_exp<AndExp>(replace(x,what,with), replace(y,what,with));},
		some<OrExp>(ds(_x,_y)), [&](auto& x, auto& y) {return make_bool_exp<OrExp>(replace(x,what,with), replace(y,what,with));}
	);

}
bool equal(const std::unique_ptr<BoolExp>& x1, const std::unique_ptr<BoolExp>& x2)
{
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return match(std::tie(x1, x2),
		ds(some<VarExp>(ds(_x)), some<VarExp>(ds(_y))), [](auto& x, auto& y) {return x == y;},
		ds(some<ValExp>(ds(_x)), some<ValExp>(ds(_y))), [](auto& x, auto& y) {return x == y;},
		ds(some<NotExp>(ds(_x)), some<NotExp>(ds(_y))), [](auto& x, auto& y) {return equal(x,y);},
		ds(some<AndExp>(ds(_w,_x)), some<AndExp>(ds(_y,_z))), [](auto& w, auto& x, auto& y, auto& z) {return equal(w,y) && equal(x,z);},
		ds(some<OrExp>(ds(_w,_x)), some<OrExp>(ds(_y,_z))), [](auto& w, auto& x, auto& y, auto& z) {return equal(w,y) && equal(x,z);},
		_, []() {return false;}



		);

	
}

typedef std::map<std::string, std::unique_ptr<BoolExp>> Assignments;

// Other example: unify
bool exp_match(const std::unique_ptr<BoolExp>& p, const std::unique_ptr<BoolExp>& x, Assignments& ctx)
{
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return match(std::tie(p, x),
		ds(some<VarExp>(ds(_x)), _), [&](auto& name)mutable {
			if (!ctx[name]) { ctx[name] = copy(x);return true; }
			else { return equal(ctx[name], x); }
		},
		ds(some<ValExp>(ds(_x)), some<ValExp>(ds(_y))), [&](auto& x, auto& y)mutable {return x == y;},
		ds(some<NotExp>(ds(_x)), some<NotExp>(ds(_y))), [&](auto& x, auto& y)mutable {return exp_match(x, y,ctx);},
		ds(some<AndExp>(ds(_w, _x)), some<AndExp>(ds(_y, _z))), [&](auto& w, auto& x, auto& y, auto& z)mutable {return exp_match(w, y,ctx) && exp_match(x, z,ctx);},
		ds(some<OrExp>(ds(_w, _x)), some<OrExp>(ds(_y, _z))), [&](auto& w, auto& x, auto& y, auto& z)mutable {return exp_match(w, y,ctx )&& exp_match(x, z,ctx);},
		_, []() {return false;}



	);

}
void TestBoolExp() {

	std::unique_ptr<BoolExp> exp1 = std::make_unique<AndExp>( 
		std::make_unique<OrExp>(std::make_unique<VarExp>("X"), std::make_unique<VarExp>("Y")), 
		std::make_unique<NotExp>(std::make_unique<VarExp>("Z")));


	std::cout << "exp1 = "; print(exp1); std::cout << std::endl;

	auto exp2 = copy(exp1);

	std::cout << "exp2 = "; print(exp2); std::cout << std::endl;

	auto exp3 = replace(exp1, "Z", exp2);

	std::cout << "exp3 = "; print(exp3); std::cout << std::endl;

	std::cout << (equal(exp1, exp2) ? "exp1 == exp2" : "exp1 <> exp2") << std::endl;

	Context ctx;
	ctx["Y"] = true;
	std::cout << eval(ctx, exp1) << std::endl;
	std::cout << eval(ctx, exp2) << std::endl;
	std::cout << eval(ctx, exp3) << std::endl;

	std::cout << ctx << std::endl;

	Assignments ctx2;

	if (exp_match(exp2, exp3, ctx2))
	{
		std::cout << "exp2 matches exp3 with assignments: " << std::endl;

		for (Assignments::const_iterator p = ctx2.begin(); p != ctx2.end(); ++p)
		{
			std::cout << p->first << '='; print(p->second); std::cout << std::endl;
		}
	}
}




int main() {


	FizzBuzz();


	math_variant_t var{ add{2,mul{3,neg{2}} } };
	std::cout << eval(var) << "\n";

	eval_any any_var{ add2{2,mul2{3,neg2{2}} } };
	std::cout << eval(any_var) << "\n";

	math_variant2_t var2{ add3{2,mul3{3,neg3{2}} } };
	std::cout << eval(var2) << "\n";

	std::unique_ptr<eval_base_t> eval_base = std::make_unique<add4>(
		std::make_unique<int_holder>(2),
		std::make_unique<mul4>(
			std::make_unique<int_holder>(3),
			std::make_unique<neg4>(std::make_unique<int_holder>(2))));



	std::cout << eval(eval_base) << "\n";


	test_string();
	test_some_none();
	test_ds();

	using namespace simple_match;
	using namespace simple_match::placeholders;

	int x = 0;

	int xs[] = { 1,2,4,15,20,21 };

	for (auto x:xs) {
		match(x,
			1, []() {std::cout << "The answer is one\n"; },
			2, []() {std::cout << "The answer is two\n"; },
			_x < 10, [](auto&& a) {std::cout << "The answer " << a << " is less than 10\n"; },
			10 < _x < 20,	[](auto&& a) {std::cout << "The answer " << a  << " is between 10 and 20 exclusive\n"; },
			_, []() {std::cout << "Did not match\n"; }

		);


	}


	TestBoolExp();

}
