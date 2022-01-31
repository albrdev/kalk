#include "KalkSetup.hpp"
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <readline/history.h>
#include <mpfr.h>
#include "text/CommandParser.hpp"

int Command_Prec(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.precision << std::endl;
  }
  else
  {
    options.precision = static_cast<mpfr_prec_t>(std::stol(args[0]));
  }

  return 0;
}

int Command_RMode(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.roundingMode << std::endl;
  }
  else
  {
    options.roundingMode = strToRnd(args[0]);
  }

  return 0;
}

int Command_Digits(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.digits << std::endl;
  }
  else
  {
    options.digits = std::stoi(args[0]);
  }

  return 0;
}

int Command_OBase(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.output_base << std::endl;
  }
  else
  {
    options.output_base = std::stoi(args[0]);
  }

  return 0;
}

int Command_IBase(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.input_base << std::endl;
  }
  else
  {
    options.input_base = std::stoi(args[0]);
  }

  return 0;
}

int Command_Base(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.output_base << ", " << options.input_base << std::endl;
  }
  else if(args.size() == 1u)
  {
    options.input_base = options.output_base = std::stoi(args[0]);
  }
  else if(args.size() == 2u)
  {
    options.output_base = std::stoi(args[0]);
    options.input_base  = std::stoi(args[1]);
  }
  else
  {
    return 1;
  }

  return 0;
}

int Command_Jpo(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.jpo_precedence << std::endl;
  }
  else
  {
    options.jpo_precedence = std::stoi(args[0]);
  }

  return 0;
}

int Command_Date_Ofmt(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.date_ofmt << std::endl;
  }
  else
  {
    options.date_ofmt = args[0];
  }

  return 0;
}

int Command_Seed(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.seed << std::endl;
  }
  else
  {
    options.seed = static_cast<unsigned int>(std::stoul(args[0]));
    mpfr::random(options.seed == 0u ? static_cast<unsigned int>(std::time(nullptr)) : options.seed);
  }

  return 0;
}

int Command_SeedStr(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    std::cout << options.seed << std::endl;
  }
  else
  {
    std::hash<std::string> hasher;
    options.seed = args[0].empty() ? defaultOptions.seed : static_cast<unsigned int>(hasher(args[0]));
    mpfr::random(options.seed == 0u ? static_cast<unsigned int>(std::time(nullptr)) : options.seed);
  }

  return 0;
}

int Command_Ans(const std::vector<std::string>& args)
{
  if(args.size() == 0u)
  {
    auto value = ans();
    printValue(*value);
  }
  else
  {
    if(args[0] == "*")
    {
      for(auto iter = results.rbegin(); iter != results.rend(); iter++)
      {
        printValue(*iter);
      }
    }
    else if(args[0] == "#")
    {
      std::cout << results.size() << std::endl;
    }
    else
    {
      int index  = std::stoi(args[0]);
      auto value = ans(index);
      printValue(*value);
    }
  }

  return 0;
}

int Command_Clear(const std::vector<std::string>& args)
{
  constexpr char kValidArgs[] = "chrv";

  std::string arg = (args.size() == 0u) ? "c" : ((args[0] == "all") ? kValidArgs : args[0]);

  if(arg.find_first_not_of(kValidArgs) != std::string::npos)
  {
    return 1;
  }

  if(arg.find('c') != std::string::npos)
  {
    std::system("clear");
  }

  if(arg.find('h') != std::string::npos)
  {
    clear_history();
  }

  if(arg.find('r') != std::string::npos)
  {
    results.clear();
  }

  if(arg.find('v') != std::string::npos)
  {
    defaultVariables.clear();
    defaultInitializedVariableCache.clear();
    defaultUninitializedVariableCache.clear();
  }

  return 0;
}

int Command_Exit(const std::vector<std::string>& args)
{
  static_cast<void>(args);

  quit = true;
  return 0;
}

static CommandParser::CallbackCollection callbacks;

void InitCommandParser(CommandParser& instance)
{
  instance.SetCallbacks(&callbacks);

  callbacks["prec"]      = Command_Prec;
  callbacks["rmode"]     = Command_RMode;
  callbacks["digits"]    = Command_Digits;
  callbacks["obase"]     = Command_OBase;
  callbacks["ibase"]     = Command_IBase;
  callbacks["base"]      = Command_Base;
  callbacks["jpo"]       = Command_Jpo;
  callbacks["date_ofmt"] = Command_Date_Ofmt;
  callbacks["seed"]      = Command_Seed;
  callbacks["seedstr"]   = Command_SeedStr;
  callbacks["ans"]       = Command_Ans;
  callbacks["clear"]     = Command_Clear;
  callbacks["exit"]      = Command_Exit;
}
