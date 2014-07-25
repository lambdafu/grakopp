#include <grakopp/grakopp.hpp>
#include <grakopp/ast-io.hpp>


int main()
{
  AstPtr ast = std::make_shared<Ast>();

  try
    {
      std::cin >> std::noskipws >> std::ws >> ast;
      std::cout << *ast << "\n";
    }
  catch (const std::invalid_argument& exc)
    {
      std::cout << "ERROR: " << exc.what() << "\n";
    }

  return 0;
}
