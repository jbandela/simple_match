// Comparison to https://github.com/solodon4/Mach7/blob/master/code/test/unit/cppcon-matching.cpp from the mach7 library

#include <memory>

#include "../include/simple_match/simple_match.hpp"
#include "../include/simple_match/some_none.hpp"


#include <string>

struct BoolExp { virtual ~BoolExp() {} };
struct VarExp : BoolExp { VarExp(std::string n) : name(std::move(n)) {} std::string name; };
struct ValExp : BoolExp { ValExp(bool b) : value(b) {} bool value; };
struct NotExp : BoolExp { NotExp(std::unique_ptr<BoolExp> e) : e(std::move(e)) {} std::unique_ptr<BoolExp> e; };
struct AndExp : BoolExp { AndExp(std::unique_ptr<BoolExp> e1, std::unique_ptr<BoolExp> e2) : e1(std::move(e1)), e2(std::move(e2)) {} std::unique_ptr<BoolExp> e1; std::unique_ptr<BoolExp> e2; };
struct OrExp : BoolExp { OrExp(std::unique_ptr<BoolExp> e1, std::unique_ptr<BoolExp> e2) : e1(std::move(e1)), e2(std::move(e2)) {} std::unique_ptr<BoolExp> e1; std::unique_ptr<BoolExp> e2; };


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

void print(const std::unique_ptr<BoolExp>& expression) {

	using namespace simple_match;
	using namespace simple_match::placeholders;

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

int main() {
	std::unique_ptr<BoolExp> exp1=  std::make_unique<AndExp>(std::make_unique<OrExp>(std::make_unique<VarExp>("X"), std::make_unique<VarExp>("Y")), std::make_unique<NotExp>(std::make_unique<VarExp>("Z")));

	std::cout << "exp1 = "; print(exp1); std::cout << std::endl;
}