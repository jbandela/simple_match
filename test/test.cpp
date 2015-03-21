#include <tuple>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include "../include/simple_match/simple_match.hpp"
#include <iostream>


int main() {

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