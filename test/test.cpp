#include <tuple>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include "../include/simple_match/simple_match.hpp"
#include <iostream>




#include "../include/simple_match/some_none.hpp"

struct point {
	int x;
	int y;
	point(int x_, int y_) :x(x_), y(y_) {}
};

namespace simple_match {
	namespace customization {
		template<>
		struct tuple_adapter<point>{

			enum { tuple_len = 2 };

			template<size_t I, class T>
			static decltype(auto) get(T&& t) {
				return std::get<I>(std::tie(t.x,t.y));
			}
		};
	}
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




namespace simple_match {
	namespace customization {
		template<class... T>
		struct pointer_getter<boost::variant<T...>> {
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
		some<add2>(ds(_x,_y)), [](auto&& a, auto&& b) {return eval_any(a) + eval_any(b);},
		some<sub2>(ds(_x,_y)), [](auto&& a, auto&& b) {return eval_any(a) - eval_any(b);},
		some<neg2>(ds(_x)), [](auto&& a) {return -eval_any(a);},
		some<mul2>(ds(_x,_y)), [](auto&& a,auto&& b) {return eval_any(a) * eval_any(b);},
		some<int>(), [](auto a) {return a;}

	);



}


int main() {
	math_variant_t var{ add{2,mul{3,neg{2}} } };
	std::cout << eval_variant(var) << "\n";

	any any_var{ add2{2,mul2{3,neg2{2}} } };
	std::cout << eval_any(any_var) << "\n";


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
