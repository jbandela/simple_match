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
			some(tup(1,2)), []() {std::cout << "one,two\n"; },
			some(tup(_x,_x)), [](int x, int y) {std::cout << x << " " << y << "\n"; },
			none(), []() {std::cout << "Nothing\n"; }
		);
	};


	auto tup_12 = std::make_unique < std::tuple<int, int>>(1, 2);
	auto point_12 = std::make_unique <point>(point{ 1, 2 });

	m2(tup_12);
	m2(point_12);



}


int main() {

	test_some_none();

	using namespace simple_match;
	using namespace simple_match::placeholders;

	std::tuple<int, int> t{1, 2};
	int x = 0;
	int y = 0;

	using std::tie;
	using std::forward_as_tuple;

	while (true) {
		std::cin >> x;
		std::cin >> y;
		match(tie(x,y),
			tup(1,1),						[]() {std::cout << "The answer is one\n"; },
			tup(2,2),						[]() {std::cout << "The answer is two\n"; },
			tup(_x > 10, _x < 10),				[](auto&& a, auto && b) {std::cout << "The answer " << a << " " << b  << " is less than 10\n"; },
			tup(10 < _x < 20,10 < _x < 20),			[](auto&& a, auto&& b) {std::cout << "The answer " << a << " " << b << " is between 10 and 20 exclusive\n"; },
			tup(_ ,100 <= _x && _x <= 200), [](auto&& a) {std::cout << "The answer " << a << " is between 100 and 200 inclusive\n"; },
			otherwise,				[]() {std::cout << "Did not match\n"; }

		);


	}


}