#include <iostream>
#include <memory>
#include <vector>

#include "simple_match/boost_support/lexical_cast.hpp"
#include "simple_match/boost_support/variant.hpp"
#include "simple_match/regex.hpp"
#include "simple_match/simple_match.hpp"

#if 0
void test_regex()
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  auto toll_free = make_matcher_predicate(
      [](boost::string_ref s)
      {
        const static std::vector<std::string> toll_free_nums {
            "800", "888", "877", "866", "855"};
        return std::find(toll_free_nums.begin(), toll_free_nums.end(), s)
            != toll_free_nums.end();
      });

  std::regex r_text {"([a-z]+)\\.txt"};
  auto m = [&](boost::string_ref s)
  {
    match(
        s,
        rex_search(r_text, _x),
        [](auto& x) { std::cout << x << "\n"; },
        rex_search("([0-9]{4})-([0-9]{2})-([0-9]{2})",
                   lexical_cast<int>(_x),
                   lexical_cast<int>(0 < _x <= 12),
                   lexical_cast<int>(0 < _x <= 31)),
        [](auto y, auto m, auto d)
        { std::cout << "Got date " << y << " " << m << " " << d << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", "979", _y, _z),
        [](auto& y, auto& z)
        { std::cout << "Got local phone " << y << "-" << z << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", toll_free, _y, _z),
        [](auto& x, auto& y, auto& z)
        { std::cout << "Got toll free " << x << "-" << y << "-" << z << "\n"; },
        rex_match("([0-9]{3})-([0-9]+)-([0-9]+)", _x, _y, _z),
        [](auto& x, auto& y, auto& z) {
          std::cout << "Got long distance " << x << "-" << y << "-" << z
                    << "\n";
        },
        _x,
        [](auto& x) { std::cout << x << " Did not match a regex\n"; }

    );
  };

  m(" foo.txt");
  m(" 2015-01-22");
  m("2015-13-22");
  m("2015-01-00");
  m("979-123-4567");
  m("877-123-4567");
  m("561-123-4567");
}
#endif

struct add;
struct sub;
struct neg;
struct mul;

using expression_t = boost::variant<boost::recursive_wrapper<add>,
                                    boost::recursive_wrapper<sub>,
                                    boost::recursive_wrapper<neg>,
                                    boost::recursive_wrapper<mul>,
                                    int>;

struct add : std::tuple<expression_t, expression_t>
{
  using tuple::tuple;
};
struct sub : std::tuple<expression_t, expression_t>
{
  using tuple::tuple;
};
struct mul : std::tuple<expression_t, expression_t>
{
  using tuple::tuple;
};
struct neg : std::tuple<expression_t>
{
  using tuple::tuple;
};

auto eval(const expression_t& e) -> int
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  return simple_match::match(
      e,
      some<add>(ds(some<int>(_x), some<mul>(ds(some<int>(_y), some<int>(_z))))),
      [](int x, int y, int z)
      {
        std::cout << "Fused add-mul\n";
        return x + y * z;
      },
      some<add>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) + eval(y); },
      some<sub>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) - eval(y); },
      some<mul>(ds(_x, _y)),
      [](auto&& x, auto&& y) { return eval(x) * eval(y); },
      some<neg>(ds(_x)),
      [](auto&& x) { return -eval(x); },
      some<int>(),
      [](auto x) { return x; });
}

struct animal
{
  virtual ~animal() {}
};
struct mammal : animal
{
};
struct domesticated_mammal : mammal
{
};
struct wild_mammal : mammal
{
};
struct cat : domesticated_mammal
{
};
struct dog : domesticated_mammal
{
};
struct bear : wild_mammal
{
};

struct platypus : wild_mammal
{
};

struct bird : animal
{
};
struct eagle : bird
{
};

void do_animal_stuff(const animal* a)
{
  using namespace simple_match;
  using namespace simple_match::placeholders;

  // Reproduce
  match(
      a,
      some<platypus>(),
      [](auto&&) { std::cout << "Lay eggs even though a mammal\n"; },
      some<mammal>(),
      [](auto&&) { std::cout << "Give live birth\n"; },
      some<bird>(),
      [](auto&&) { std::cout << "Lay eggs in nest\n"; },
      _,
      []() { std::cout << "Reproduce somehow\n"; });

  // make sound
  match(
      a,
      some<cat>(),
      [](auto&) { std::cout << "Meow\n"; },
      some<dog>(),
      [](auto&) { std::cout << "Woof\n"; },
      some<bear>(),
      [](auto&) { std::cout << "Growl\n"; },
      some<eagle>(),
      [](auto&) { std::cout << "Screech\n"; },
      _,
      []() { std::cout << "Unknown animal sound\n"; });

  // Check if can be pet
  match(
      a,
      some<domesticated_mammal>(),
      [](auto&&) { std::cout << "This will make a good pet\n"; },
      some<wild_mammal>(),
      [](auto&&) { std::cout << "This will not make a good pet\n"; },
      _,
      []() { std::cout << "Don't know what kind of pet this will make\n"; });
}

void test_animal()
{
  std::vector<std::unique_ptr<animal>> animals;
  animals.emplace_back(new cat);
  animals.emplace_back(new dog);
  animals.emplace_back(new platypus);
  animals.emplace_back(new bear);
  animals.emplace_back(new eagle);

  for (auto& a : animals) {
    const auto& expr = *a.get();
    std::cout << typeid(expr).name() << "\n";
    do_animal_stuff(a.get());
  }
}

auto main() -> int
{
#if 0
  test_regex();
#endif

  expression_t e = sub {10, add {2, {mul {2, 3}}}};
  eval(e);

  test_animal();

  return 0;
}
