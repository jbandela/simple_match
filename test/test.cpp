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

int main() {


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


}
