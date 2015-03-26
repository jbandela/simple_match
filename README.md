#Simple, Extensible C++ Pattern Matching Library

I have recently been looking at Haskell and Rust. One of the things I wanted in C++ from those languages is pattern matching.

Here is an example from the Rustlang Book (http://static.rust-lang.org/doc/master/book/match.html)

```
match x {
    1 => println!("one"),
    2 => println!("two"),
    3 => println!("three"),
    4 => println!("four"),
    5 => println!("five"),
    _ => println!("something else"),
}
```
There is currently a C++ Library Mach7 that does pattern matching (https://github.com/solodon4/Mach7), however it is big, complicated, and uses a lot of macros. I wanted to see if I could use C++14 to write a simple implementation without macros. 

This library is the result of that effort. If you are familiar with C++14 especially variadic templates, forwarding, and tuples, this library and implementation should be easy for you to understand and extend.

## Usage
You will need a C++14 compiler. I have used the latest Visual C++ 2015 CTP, GCC 4.9.2, and Clang 3.5 to test this library.

The library consists of 2 headers. `simple_match.hpp` which is the core of the library, and `some_none.hpp` which contains code that lets you match on raw pointers, and unique_ptr, and shared_ptr.

Here is a simple excerpt. Assume you have included simple_match.hpp

```
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


```

## Example Files
There are 2 files under the test directory: `test.cpp` and `cppcon-matching.cpp`. `test.cpp` contains just some simple tests of matching. `cppcon-matching.cpp`contains the example from Mach7 that was presented at cppcon.

## Extending
There are 2 points of customization provided in namespace `simple_matcher::customization`. They are 
```
		template<class T, class U>
		struct matcher;
```

and 

```
		template<class Type>
		struct tuple_adapter;
```
## License
Licensed under the Boost Software License.



> Written with [StackEdit](https://stackedit.io/).