#include <tuple>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include "../include/simple_match/simple_match.hpp"
#include <iostream>



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
			some( 11 <=_x <= 20 ), [](int x) {std::cout << x << " is on the range [11,20] \n"; },
			some(), [](int x) {std::cout << x << "\n"; },
			none(), []() {std::cout << "Nothing\n"; }
		);
	};

	m(nothing);
	m(five);
	m(ten);
	m(twelve);


	auto m2 = [](auto&& v) {
		match(v,
			some(ds(1,2)), []() {std::cout << "one,two\n"; },
			some(ds(_x,_y)), [](int x, int y) {std::cout << x << " " << y << "\n"; },
			none(), []() {std::cout << "Nothing\n"; }
		);
	};


	auto tup_12 = std::make_unique < std::tuple<int, int>>(1, 2);
	auto point_12 = std::make_unique <point>(point{ 1, 2 });
	auto point_34 = std::make_unique <point>(point{ 3, 4 });

	m2(tup_12);
	m2(point_12);
	m2(point_34);



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

#include <boost/variant.hpp>


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


#include <boost/any.hpp>
using boost::any;

struct add_tag {};
struct sub_tag {};
struct mul_tag {};
struct neg_tag {};



using add2 = simple_match::tagged_tuple<add_tag, any, any>;
using sub2 = simple_match::tagged_tuple<sub_tag, any, any>;
using mul2 = simple_match::tagged_tuple<mul_tag, any, any>;
using neg2 = simple_match::tagged_tuple<neg_tag, any>;

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







namespace simple_match {
	namespace customization {
		template<class... A>
		struct pointer_getter<boost::variant<A...>> {
			template<class To, class T>
			static auto get_pointer(T&& t) {
				return boost::get<To>(&t);
			}
		};
		template<>
		struct pointer_getter<boost::any> {
			template<class To, class T>
			static auto get_pointer(T&& t) {
				return boost::any_cast<To>(&t);
			}
		};
	}

}

int eval_variant(const math_variant_t& m) {
	using namespace simple_match;

	return simple_match::match(m,
		some<add>(), [](auto&& a) {return eval_variant(a.left) + eval_variant(a.right);},
		some<sub>(), [](auto&& a) {return eval_variant(a.left) - eval_variant(a.right);},
		some<neg>(), [](auto&& a) {return -eval_variant(a.value);},
		some<mul>(), [](auto&& a) {return eval_variant(a.left) * eval_variant(a.right);},
		some<int>(), [](auto a) {return a;}

		);



}

int eval_any(const any& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add2>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval_any(x) + eval_any(y);},
		some<sub2>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval_any(x) - eval_any(y);},
		some<neg2>(ds(_x)), [](auto&& x) {return -eval_any(x);},
		some<mul2>(ds(_x,_y)), [](auto&& x,auto&& y) {return eval_any(x) * eval_any(y);},
		some<int>(), [](auto x) {return x;}

	);



}


int eval_variant2(const math_variant2_t& m) {
	using namespace simple_match;
	using namespace simple_match::placeholders;

	return simple_match::match(m,
		some<add3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval_variant2(x) + eval_variant2(y);},
		some<sub3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval_variant2(x) - eval_variant2(y);},
		some<mul3>(ds(_x,_y)), [](auto&& x, auto&& y) {return eval_variant2(x) * eval_variant2(y);},
		some<neg3>(ds(_x)), [](auto&& x) {return -eval_variant2(x);},
		some<int>(), [](auto x) {return x;}
	);



}

int main() {
	math_variant_t var{ add{2,mul{3,neg{2}} } };
	std::cout << eval_variant(var) << "\n";

	any any_var{ add2{2,mul2{3,neg2{2}} } };
	std::cout << eval_any(any_var) << "\n";

	math_variant2_t var2{ add3{2,mul3{3,neg3{2}} } };
	std::cout << eval_variant2(var2) << "\n";





	test_string();
	test_some_none();

	using namespace simple_match;
	using namespace simple_match::placeholders;

	int x = 0;

	while (true) {
		std::cin >> x;
		match(x,
			1, []() {std::cout << "The answer is one\n"; },
			2, []() {std::cout << "The answer is two\n"; },
			_x < 10, [](auto&& a) {std::cout << "The answer " << a << " is less than 10\n"; },
			10 < _x < 20,	[](auto&& a) {std::cout << "The answer " << a  << " is between 10 and 20 exclusive\n"; },
			_, []() {std::cout << "Did not match\n"; }

		);


	}


}
