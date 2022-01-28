#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cerrno>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <regex>
#include <exception>
#include <unistd.h>
#include "readline/readline.h"
#include "readline/history.h"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <mpreal.h>
#include "text/expression/ExpressionParser.hpp"
#include "ExpressionParserSetup.hpp"

#define STRINGIFY(X) #X

static std::string EnumToString(mpfr_rnd_t value)
{
  //MPFR_RNDN = 0,  /* round to nearest, with ties to even */
  //MPFR_RNDZ,      /* round toward zero */
  //MPFR_RNDU,      /* round toward +Inf */
  //MPFR_RNDD,      /* round toward -Inf */
  //MPFR_RNDA,      /* round away from zero */
  //MPFR_RNDF,      /* faithful rounding */
  //MPFR_RNDNA = -1 /* round to nearest, with ties away from zero (mpfr_round) */

  switch(value)
  {
    case mpfr_rnd_t::MPFR_RNDN:
    {
      return STRINGIFY(MPFR_RNDN);
    }
    case mpfr_rnd_t::MPFR_RNDZ:
    {
      return STRINGIFY(MPFR_RNDZ);
    }
    case mpfr_rnd_t::MPFR_RNDU:
    {
      return STRINGIFY(MPFR_RNDU);
    }
    case mpfr_rnd_t::MPFR_RNDD:
    {
      return STRINGIFY(MPFR_RNDD);
    }
    case mpfr_rnd_t::MPFR_RNDA:
    {
      return STRINGIFY(MPFR_RNDA);
    }
    case mpfr_rnd_t::MPFR_RNDF:
    {
      return STRINGIFY(MPFR_RNDF);
    }
    case mpfr_rnd_t::MPFR_RNDNA:
    {
      return STRINGIFY(MPFR_RNDNA);
    }
    default:
    {
      throw std::domain_error("Invalid enum value");
    }
  }
}

static std::istream& operator>>(std::istream& stream, mpfr_rnd_t& result)
{
  std::string value;
  stream >> value;

  boost::to_upper(value);

  std::regex regex(R"(^(MPFR_)?(RND)?(.*)$)", std::regex_constants::icase);
  std::smatch matches;
  if(!std::regex_match(value, matches, regex))
  {
    throw std::domain_error("Invalid rounding mode");
  }
  std::string inputString = matches[matches.size() - 1u];

  std::string enumString = EnumToString(mpfr_rnd_t::MPFR_RNDN);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDN;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDZ);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDZ;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDU);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDU;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDD);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDD;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDA);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDA;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDF);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDF;
    return stream;
  }

  enumString = EnumToString(mpfr_rnd_t::MPFR_RNDNA);
  std::regex_match(enumString, matches, regex);
  enumString = matches[matches.size() - 1u];
  if(inputString == enumString)
  {
    result = mpfr_rnd_t::MPFR_RNDNA;
    return stream;
  }

  throw std::domain_error("Invalid rounding mode");
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

  if(value->GetType() == typeid(DefaultArithmeticType))
  {
    std::cout << value->GetValue<DefaultArithmeticType>().toString(options.digits, options.output_base, mpfr::mpreal::get_default_rnd()) << std::endl;
  }
  else
  {
    std::cout << value->ToString() << std::endl;
  }

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

static void printVersion() { std::cout << (boost::format("%1% v%2%") % PROJECT_NAME % PROJECT_VERSION).str() << std::endl; }

static void printUsage(const boost::program_options::options_description& desc)
{
  std::cout << (boost::format("%1% -[prdbBjzZiVh] expr...") % PROJECT_EXECUTABLE).str() << std::endl;
  std::cout << desc << std::endl;
}

int main(int argc, char* argv[])
{
  std::vector<std::string> envs;
  resolveEnvironmentVariables(envs);

  boost::program_options::options_description env_desc_named;
  env_desc_named.add_options()("KALK_PREC", boost::program_options::value<mpfr_prec_t>(&options.precision)->default_value(defaultOptions.precision));
  env_desc_named.add_options()("KALK_RMODE", boost::program_options::value<mpfr_rnd_t>(&options.roundingMode)->default_value(defaultOptions.roundingMode));
  env_desc_named.add_options()("KALK_DIGITS", boost::program_options::value<int>(&options.digits)->default_value(defaultOptions.digits));
  env_desc_named.add_options()("KALK_OBASE", boost::program_options::value<int>(&options.output_base));
  env_desc_named.add_options()("KALK_IBASE", boost::program_options::value<int>(&options.input_base));
  env_desc_named.add_options()("KALK_BASE", boost::program_options::value<int>()->default_value(defaultOptions.output_base)->notifier([](int value) {
    options.output_base = value;
    options.input_base  = value;
  }));
  env_desc_named.add_options()("KALK_JUXTA", boost::program_options::value<int>()->default_value(defaultOptions.jpo_precedence)->notifier([](int value) {
    options.jpo_precedence = sgn(value);
  }));
  env_desc_named.add_options()("KALK_INTERACTIVE", boost::program_options::value<bool>(&options.interactive)->default_value(defaultOptions.interactive));

  boost::program_options::variables_map env_vm;
  boost::program_options::store(boost::program_options::command_line_parser(envs)
                                    .options(env_desc_named)
                                    .extra_parser([](const std::string& value) { return std::make_pair(value, std::string()); })
                                    .run(),
                                env_vm);
  boost::program_options::notify(env_vm);

  boost::program_options::options_description arg_desc_named("Options");
  arg_desc_named.add_options()("expr,x", boost::program_options::value<std::vector<std::string>>(), "Add an expression");
  arg_desc_named.add_options()("prec,p", boost::program_options::value<mpfr_prec_t>(&options.precision), "Set precision");
  arg_desc_named.add_options()("rmode,r", boost::program_options::value<mpfr_rnd_t>(&options.roundingMode), "Set rounding mode (N, Z, U, D, A, F, NA)");
  arg_desc_named.add_options()("digits,d", boost::program_options::value<int>(&options.digits), "Set output precision (Number of digits)");
  arg_desc_named.add_options()("obase,b", boost::program_options::value<int>(&options.output_base), "Set output base");
  arg_desc_named.add_options()("ibase,B", boost::program_options::value<int>(&options.input_base), "Set input base");
  arg_desc_named.add_options()("base",
                               boost::program_options::value<int>()->notifier([](int value) { options.input_base = options.output_base = value; }),
                               "Set output and input base");
  arg_desc_named.add_options()("juxta,j",
                               boost::program_options::value<int>()->notifier([](int value) { options.jpo_precedence = sgn(value); }),
                               "Set juxtaposition operator precedence (-1, 0, 1)");
  arg_desc_named.add_options()("seed,z", boost::program_options::value<unsigned int>(&options.seed), "Set random seed (number)");
  arg_desc_named.add_options()("seedstr,Z",
                               boost::program_options::value<std::string>()->notifier([](std::string value) {
                                 std::hash<std::string> hasher;
                                 options.seed = value.empty() ? defaultOptions.seed : static_cast<unsigned int>(hasher(value));
                               }),
                               "Set random seed (string)");
  arg_desc_named.add_options()("interactive,i", boost::program_options::value<bool>(&options.interactive)->implicit_value(true), "Enable interactive mode");
  arg_desc_named.add_options()("version,V", "Print version");
  arg_desc_named.add_options()("help,h", "Print usage");

  boost::program_options::positional_options_description desc_pos;
  desc_pos.add("expr", -1);

  boost::program_options::variables_map arg_vm;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(arg_desc_named).positional(desc_pos).run(), arg_vm);
  boost::program_options::notify(arg_vm);

  if(arg_vm.count("help") > 0u)
  {
    printUsage(arg_desc_named);
    return 0;
  }

  if(arg_vm.count("version") > 0u)
  {
    printVersion();
    return 0;
  }

  mpfr::random(options.seed == 0u ? static_cast<unsigned int>(std::time(nullptr)) : options.seed);

  mpfr::mpreal::set_default_prec(options.precision);
  mpfr::mpreal::set_default_rnd(options.roundingMode);

  ExpressionParser expressionParser;
  InitDefault(expressionParser);

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

  if(arg_vm.count("expr") == 0u && !options.interactive && !hasPipedData)
  {
    std::cerr << "*** Error: No expression specified" << std::endl;
    return 1;
  }

  if(arg_vm.count("expr") > 0u)
  {
    results.clear();

    const auto& exprs = arg_vm["expr"].as<const std::vector<std::string>&>();
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
    while((tmpInput = readline("> ")) != nullptr)
    {
      auto tmpPtr       = std::unique_ptr<char, decltype(&std::free)>(tmpInput, &std::free);
      std::string input = tmpInput;

      if(input.find_first_not_of(kWhitespaceCharacters) != std::string::npos)
      {
        try
        {
          auto result = expressionParser.Evaluate(input);
          handleResult(result->AsPointer<DefaultValueType>());
        }
        catch(const SyntaxException& e)
        {
          std::cerr << "*** Error: " << e.what() << std::endl;
        }
        catch(const std::runtime_error& e)
        {
          std::cerr << "*** Error: " << e.what() << std::endl;
        }

        add_history(tmpInput);
      }
    }
  }

  std::exit(0);
}
