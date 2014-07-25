#include <fstream>

#include <grakopp/grakopp.hpp>
#include <grakopp/ast-io.hpp>

int main(int argc, char *argv[])
{
  std::ifstream file1;
  std::ifstream file2;

  file1.open(argv[1]);
  file2.open(argv[2]);

  AstPtr ast1 = std::make_shared<Ast>();
  AstPtr ast2 = std::make_shared<Ast>();

  try
    {
      file1 >> std::noskipws >> std::ws >> ast1;
      file2 >> std::noskipws >> std::ws >> ast2;
    }
  catch (const std::invalid_argument& exc)
    {
      std::cout << "ERROR: " << exc.what() << "\n";
    }

  return ast1 == ast2 ? 0 : 1;
}
