#include <iostream>

class counter {
  public:
  int i;

  counter();

  int inspect() const;
  void increment();
};

counter sigma_inspect;

counter::counter()
{
  i = 0;
}

int counter::inspect() const
{
  sigma_inspect.increment();
  return i;
}

void counter::increment()
{
  ++ i;
  return;
}

int main(void)
{
  counter a;

  std::cout << a.inspect() << std::endl;

  std::cout << sigma_inspect.inspect() << std::endl;
  std::cout << sigma_inspect.inspect() << std::endl;
  return 0;
}
