#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <regex>
#include <locale>
#include <exception>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/time_facet.hpp>
#include <mpreal.h>
#include "text/expression/ExpressionParser.hpp"
#include "text/CommandParser.hpp"
#include "KalkSetup.hpp"
#include "text/SyntaxException.hpp"

static const std::unordered_map<mpfr_rnd_t, std::string> rmodeNameMap = {
    {mpfr_rnd_t::MPFR_RNDN, "N"},
    {mpfr_rnd_t::MPFR_RNDZ, "Z"},
    {mpfr_rnd_t::MPFR_RNDU, "U"},
    {mpfr_rnd_t::MPFR_RNDD, "D"},
    {mpfr_rnd_t::MPFR_RNDA, "A"},
    {mpfr_rnd_t::MPFR_RNDF, "F"},
    {mpfr_rnd_t::MPFR_RNDNA, "NA"},
};

static const std::unordered_map<std::string, mpfr_rnd_t> rmodeNameMap2 = {
    {"N", mpfr_rnd_t::MPFR_RNDN},
    {"Z", mpfr_rnd_t::MPFR_RNDZ},
    {"U", mpfr_rnd_t::MPFR_RNDU},
    {"D", mpfr_rnd_t::MPFR_RNDD},
    {"A", mpfr_rnd_t::MPFR_RNDA},
    {"F", mpfr_rnd_t::MPFR_RNDF},
    {"NA", mpfr_rnd_t::MPFR_RNDNA},
};

static const std::unordered_map<Associativity, std::string> associativityNameMap = {
    {Associativity::Left, "Left"},
    {Associativity::Right, "Right"},
    {Associativity::Any, "Any"},
};

mpfr_rnd_t strToRmode(const std::string value)
{
  const auto iter = rmodeNameMap2.find(boost::to_upper_copy(value));
  if(iter != rmodeNameMap2.cend())
  {
    return iter->second;
  }
  else
  {
    throw std::domain_error("Invalid rounding mode");
  }
}

static std::istream& operator>>(std::istream& stream, mpfr_rnd_t& result)
{
  std::string value;
  stream >> value;
  result = strToRmode(value);
  return stream;
}

void resolveEnvironmentVariables(std::vector<std::string>& result)
{
  const char* pTmp;

  if((pTmp = std::getenv("KALK_PREC")) != nullptr)
  {
    result.push_back("KALK_PREC");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_RMODE")) != nullptr)
  {
    result.push_back("KALK_RMODE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_DIGITS")) != nullptr)
  {
    result.push_back("KALK_DIGITS");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_OBASE")) != nullptr)
  {
    result.push_back("KALK_OBASE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_IBASE")) != nullptr)
  {
    result.push_back("KALK_IBASE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_BASE")) != nullptr)
  {
    result.push_back("KALK_BASE");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_JUXTA")) != nullptr)
  {
    result.push_back("KALK_JUXTA");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_DATE_OFMT")) != nullptr)
  {
    result.push_back("KALK_DATE_OFMT");
    result.push_back(pTmp);
  }

  if((pTmp = std::getenv("KALK_INTERACTIVE")) != nullptr)
  {
    result.push_back("KALK_INTERACTIVE");
    result.push_back(pTmp);
  }
}

template<class T>
int sgn(T value)
{
  return (value > static_cast<T>(0)) - (value < static_cast<T>(0));
}

void handleResult(const DefaultValueType* value)
{
  results.push_back(*value);

  printValue(*value);

  if(!defaultUninitializedVariableCache.empty())
  {
    std::cout << "*** Warning: Uninitialized variable(s)" << std::endl;
    while(!defaultUninitializedVariableCache.empty())
    {
      auto iter = defaultUninitializedVariableCache.begin();
      defaultVariables.erase(iter->first);
      defaultUninitializedVariableCache.erase(iter);
    }
  }
}

void list(const std::string& searchPattern)
{
  const std::regex regex(searchPattern);
  bool isPrevEmptyLine = true;

  std::cerr << "Unary operators" << std::endl;
  for(const auto& i : unaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto identifier    = std::string(1u, entry->GetIdentifier());
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-5|%|2$-5|%|3$-9|%|4$-20|%|5$|") % identifier % precedence % associativity % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Binary operators" << std::endl;
  for(const auto& i : binaryOperatorInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier   = entry->GetIdentifier();
    const auto precedence    = entry->GetPrecedence();
    const auto associativity = associativityNameMap.at(entry->GetAssociativity());
    const auto& title        = std::get<1u>(i);
    const auto& description  = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-6|%|2$-5|%|3$-9|%|4$-25|%|5$|") % identifier % precedence % associativity % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Functions" << std::endl;
  for(const auto& i : functionInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier = entry->GetIdentifier();
    const auto argMinCount = ((entry->GetMinArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMinArgumentCount()) : "-");
    const auto argMaxCount = ((entry->GetMaxArgumentCount() != FunctionToken::GetArgumentCountMaxLimit()) ? std::to_string(entry->GetMaxArgumentCount()) : "-");
    const auto& title      = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-15|%|2$-5|%|3$-5|%|4$-27|%|5$|") % identifier % argMinCount % argMaxCount % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
  std::cerr << std::endl;

  isPrevEmptyLine = true;
  std::cerr << "Variables" << std::endl;
  for(const auto& i : variableInfoMap)
  {
    const auto& entry = std::get<0u>(i);
    if(entry == nullptr)
    {
      if(!isPrevEmptyLine)
      {
        std::cerr << std::endl;
        isPrevEmptyLine = true;
      }

      continue;
    }

    const auto& identifier  = entry->GetIdentifier();
    const auto& title       = std::get<1u>(i);
    const auto& description = std::get<2u>(i);
    if(std::regex_match(identifier.begin(), identifier.end(), regex) || std::regex_match(title.begin(), title.end(), regex) ||
       std::regex_match(description.begin(), description.end(), regex))
    {
      std::cout << (boost::format("  %|1$-18|%|2$-31|%|3$|") % identifier % title % description).str() << std::endl;
      isPrevEmptyLine = false;
    }
  }
}

static void printOptions()
{
  std::cerr << "Options" << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Precision" % options.precision).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Rounding mode" % rmodeNameMap.at(options.roundingMode)).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Output precision" % options.digits).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Output base" % options.output_base).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Input base" % options.input_base).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Juxtaposition precedence" % options.jpo_precedence).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Date output format" % options.date_ofmt).str() << std::endl;
  std::cerr << (boost::format("  %|1$-26|%|2$|") % "Seed" % options.seed).str() << std::endl;
  std::cerr << std::endl;
}

static void printVersion() { std::cout << (boost::format("%1% v%2%") % PROJECT_NAME % PROJECT_VERSION).str() << std::endl; }

static void printUsage(const boost::program_options::options_description& desc)
{
  std::cerr << (boost::format("%1% -[prnbBjdzZilvVh] expr...") % PROJECT_EXECUTABLE).str() << std::endl;
  std::cerr << desc << std::endl;
}

int main(int argc, char* argv[])
{
  std::vector<std::string> envs;
  resolveEnvironmentVariables(envs);

  boost::program_options::options_description namedEnvDescs;
  namedEnvDescs.add_options()("KALK_PREC", boost::program_options::value<mpfr_prec_t>(&options.precision)->default_value(defaultOptions.precision));
  namedEnvDescs.add_options()("KALK_RMODE", boost::program_options::value<mpfr_rnd_t>(&options.roundingMode)->default_value(defaultOptions.roundingMode));
  namedEnvDescs.add_options()("KALK_DIGITS", boost::program_options::value<int>(&options.digits)->default_value(defaultOptions.digits));
  namedEnvDescs.add_options()("KALK_OBASE", boost::program_options::value<int>(&options.output_base));
  namedEnvDescs.add_options()("KALK_IBASE", boost::program_options::value<int>(&options.input_base));
  namedEnvDescs.add_options()("KALK_BASE", boost::program_options::value<int>()->default_value(defaultOptions.output_base)->notifier([](int value) {
    options.output_base = value;
    options.input_base  = value;
  }));
  namedEnvDescs.add_options()("KALK_JUXTA", boost::program_options::value<int>()->default_value(defaultOptions.jpo_precedence)->notifier([](int value) {
    options.jpo_precedence = sgn(value);
  }));
  namedEnvDescs.add_options()("KALK_DATE_OFMT", boost::program_options::value<std::string>(&options.date_ofmt)->default_value(defaultOptions.date_ofmt));
  namedEnvDescs.add_options()("KALK_INTERACTIVE", boost::program_options::value<bool>(&options.interactive)->default_value(defaultOptions.interactive));

  boost::program_options::variables_map envVariableMap;
  boost::program_options::store(boost::program_options::command_line_parser(envs)
                                    .options(namedEnvDescs)
                                    .extra_parser([](const std::string& value) { return std::make_pair(value, std::string()); })
                                    .run(),
                                envVariableMap);
  boost::program_options::notify(envVariableMap);

  boost::program_options::options_description namedArgDescs("Options");
  namedArgDescs.add_options()("expr,x", boost::program_options::value<std::vector<std::string>>(), "Add an expression");
  namedArgDescs.add_options()("prec,p", boost::program_options::value<mpfr_prec_t>(&options.precision), "Set precision");
  namedArgDescs.add_options()("rmode,r", boost::program_options::value<mpfr_rnd_t>(&options.roundingMode), "Set rounding mode (N, Z, U, D, A, F, NA)");
  namedArgDescs.add_options()("digits,n", boost::program_options::value<int>(&options.digits), "Set output precision (Number of digits)");
  namedArgDescs.add_options()("obase,b", boost::program_options::value<int>(&options.output_base), "Set output base");
  namedArgDescs.add_options()("ibase,B", boost::program_options::value<int>(&options.input_base), "Set input base");
  namedArgDescs.add_options()("base",
                              boost::program_options::value<int>()->notifier([](int value) { options.input_base = options.output_base = value; }),
                              "Set output and input base");
  namedArgDescs.add_options()("juxta,j",
                              boost::program_options::value<int>()->notifier([](int value) { options.jpo_precedence = sgn(value); }),
                              "Set juxtaposition operator precedence (-1, 0, 1)");
  namedArgDescs.add_options()("date_ofmt,d", boost::program_options::value<std::string>(&options.date_ofmt), "Set date output format");
  namedArgDescs.add_options()("seed,z", boost::program_options::value<unsigned int>(&options.seed), "Set random seed (number)");
  namedArgDescs.add_options()("seedstr,Z",
                              boost::program_options::value<std::string>()->notifier([](std::string value) {
                                std::hash<std::string> hasher;
                                options.seed = value.empty() ? defaultOptions.seed : static_cast<unsigned int>(hasher(value));
                              }),
                              "Set random seed (string)");
  namedArgDescs.add_options()("interactive,i", boost::program_options::value<bool>(&options.interactive)->implicit_value(true), "Enable interactive mode");
  namedArgDescs.add_options()("list,l", boost::program_options::value<std::string>()->implicit_value(".*"), "List available operators/functions/variables");
  namedArgDescs.add_options()("verbose,v", "Enable verbose mode");
  namedArgDescs.add_options()("version,V", "Print version");
  namedArgDescs.add_options()("help,h", "Print usage");

  boost::program_options::positional_options_description positionalArgDescs;
  positionalArgDescs.add("expr", -1);

  boost::program_options::variables_map argVariableMap;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(namedArgDescs).positional(positionalArgDescs).run(),
                                argVariableMap);
  boost::program_options::notify(argVariableMap);

  if(argVariableMap.count("help") > 0u)
  {
    printUsage(namedArgDescs);
    return 0;
  }

  if(argVariableMap.count("version") > 0u)
  {
    printVersion();
    return 0;
  }

  if(options.seed == 0u)
  {
    options.seed = static_cast<unsigned int>(std::time(nullptr));
  }

  mpfr::random(options.seed);

  mpfr::mpreal::set_default_prec(options.precision);
  mpfr::mpreal::set_default_rnd(options.roundingMode);

  auto dateFacet = new boost::posix_time::time_facet(options.date_ofmt.c_str());
  std::cout.imbue(std::locale(std::cout.getloc(), dateFacet));

  if(argVariableMap.count("verbose") > 0u)
  {
    printOptions();
  }

  ExpressionParser expressionParser;
  InitDefaultExpressionParser(expressionParser);

  CommandParser commandParser;
  InitCommandParser(commandParser);

  if(argVariableMap.count("list") > 0u)
  {
    list(argVariableMap["list"].as<const std::string&>());
    return 0;
  }

  constexpr char kWhitespaceCharacters[] = " \t\v\n\r\f";
  std::unique_ptr<FILE, decltype(&std::fclose)> file_stdin(nullptr, &std::fclose);
  bool hasPipedData = std::cin.rdbuf()->in_avail() != -1 && isatty(fileno(stdin)) == 0;
  if(hasPipedData)
  {
    std::string input;
    while(std::getline(std::cin, input))
    {
      if(input.find_first_not_of(kWhitespaceCharacters) != std::string::npos)
      {
        auto result = expressionParser.Evaluate(input);
        handleResult(result->AsPointer<DefaultValueType>());
      }
    }

    if(options.interactive)
    {
      const char* ttyFileName = ttyname(fileno(stdout));
      if(ttyFileName == nullptr)
      {
        std::cerr << "*** Error: " << std::strerror(errno) << std::endl;
        return 1;
      }

      FILE* tmpFile;
      if((tmpFile = freopen(ttyFileName, "r", stdin)) == nullptr)
      {
        std::cerr << "*** Error: " << std::strerror(errno) << std::endl;
        return 1;
      }

      file_stdin.reset(tmpFile);
    }
  }

  if(argVariableMap.count("expr") == 0u && !options.interactive && !hasPipedData)
  {
    std::cerr << "*** Error: No expression specified" << std::endl;
    return 1;
  }

  if(argVariableMap.count("expr") > 0u)
  {
    results.clear();

    const auto& exprs = argVariableMap["expr"].as<const std::vector<std::string>&>();
    for(auto& expr : exprs)
    {
      if(expr.find_first_not_of(kWhitespaceCharacters) != std::string::npos)
      {
        const auto result = expressionParser.Evaluate(expr);
        handleResult(result->AsPointer<const DefaultValueType>());
      }
    }
  }

  if(options.interactive)
  {
    results.clear();

    char* tmpInput;
    while(!quit && (tmpInput = readline("> ")) != nullptr)
    {
      auto tmpPtr       = std::unique_ptr<char, decltype(&std::free)>(tmpInput, &std::free);
      std::string input = tmpInput;

      if(input.find_first_not_of(kWhitespaceCharacters) != std::string::npos)
      {
        boost::trim(input);
        if(input.front() == '/')
        {
          try
          {
            commandParser.Execute(input.erase(0, 1));
          }
          catch(const SyntaxException& e)
          {
            std::cerr << "*** Command error: " << e.what() << std::endl;
          }
          catch(const std::runtime_error& e)
          {
            std::cerr << "*** Command error: " << e.what() << std::endl;
          }
          catch(const std::domain_error& e)
          {
            std::cerr << "*** Command error: " << e.what() << std::endl;
          }
        }
        else
        {
          try
          {
            auto result = expressionParser.Evaluate(input);
            handleResult(result->AsPointer<DefaultValueType>());
          }
          catch(const SyntaxException& e)
          {
            std::cerr << "*** Expression error: " << e.what() << std::endl;
          }
          catch(const std::runtime_error& e)
          {
            std::cerr << "*** Expression error: " << e.what() << std::endl;
          }
          catch(const std::domain_error& e)
          {
            std::cerr << "*** Expression error: " << e.what() << std::endl;
          }

          add_history(tmpInput);
        }
      }
    }
  }

  std::exit(0);
}
