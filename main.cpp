#include <ctime>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <ratio>
#include <limits>
#include <regex>
#include <exception>
#include "readline/readline.h"
#include "readline/history.h"
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/preprocessor.hpp>
#include <gmpxx.h>
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

static struct
{
  mpfr_prec_t precision;
  mpfr_rnd_t roundingMode;
  int digits;
  int output_base;
  int input_base;
  unsigned int seed;
  bool interactive;
  bool printVersion;
  bool printUsage;
} options {};

static DefaultValueType* numberConverter(const std::string& value)
{
  return new DefaultValueType(mpfr::mpreal(value, mpfr::mpreal::get_default_prec(), options.input_base, mpfr::mpreal::get_default_rnd()));
}

static void printResult(const DefaultValueType& value)
{
  if(value.GetType() == typeid(DefaultArithmeticType))
  {
    std::cout << value.GetValue<DefaultArithmeticType>().toString(options.digits, options.output_base, mpfr::mpreal::get_default_rnd()) << std::endl;
  }
  else
  {
    std::cout << value.ToString() << std::endl;
  }
}

static void printVersion() { std::cout << (boost::format("%1% v%2%") % PROJECT_NAME % PROJECT_VERSION).str() << std::endl; }

static void printUsage(const boost::program_options::options_description& desc)
{
  std::cout << (boost::format("%1% -[pdrbBzZiVh] expr...") % PROJECT_EXECUTABLE).str() << std::endl;
  std::cout << desc << std::endl;
}

int main(int argc, char* argv[])
{
  boost::program_options::options_description desc_named("Options");
  desc_named.add_options()("expr,x", boost::program_options::value<std::vector<std::string>>(), "Add an expression");
  desc_named.add_options()("prec,p", boost::program_options::value<mpfr_prec_t>(&options.precision)->default_value(128), "Set precision");
  desc_named.add_options()("digits,d", boost::program_options::value<int>(&options.digits)->default_value(30), "Set output precision (Number of digits)");
  desc_named.add_options()("rmode,r",
                           boost::program_options::value<mpfr_rnd_t>(&options.roundingMode)->default_value(mpfr_rnd_t::MPFR_RNDN),
                           "Set rounding mode (N, Z, U, D, A, F, NA)");
  desc_named.add_options()("obase,b", boost::program_options::value<int>(&options.output_base), "Set output base");
  desc_named.add_options()("ibase,B", boost::program_options::value<int>(&options.input_base), "Set input base");
  desc_named.add_options()(
      "base",
      boost::program_options::value<int>()->default_value(10)->notifier([](int value) { options.input_base = options.output_base = value; }),
      "Set output and input base");
  desc_named.add_options()("seed,z", boost::program_options::value<unsigned int>(&options.seed), "Set random seed (number)");
  desc_named.add_options()("seedstr,Z",
                           boost::program_options::value<std::string>()->notifier([](std::string value) {
                             std::hash<std::string> hasher;
                             options.seed = value.empty() ? 0 : static_cast<unsigned int>(hasher(value));
                           }),
                           "Set random seed (string)");
  desc_named.add_options()("interactive,i", boost::program_options::bool_switch(&options.interactive)->default_value(false), "Enable interactive mode");
  desc_named.add_options()("version,V", boost::program_options::bool_switch(&options.printVersion)->default_value(false), "Print version");
  desc_named.add_options()("help,h", boost::program_options::bool_switch(&options.printUsage)->default_value(false), "Print usage");

  boost::program_options::positional_options_description desc_pos;
  desc_pos.add("expr", -1);

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(desc_named).positional(desc_pos).run(), vm);
  boost::program_options::notify(vm);

  if(options.printUsage)
  {
    printUsage(desc_named);
    return 0;
  }

  if(options.printVersion)
  {
    printVersion();
    return 0;
  }

  if(vm.count("expr") == 0u && !options.interactive)
  {
    std::cerr << "*** Error: No expression specified" << std::endl;
    return 1;
  }

  mpfr::random(options.seed == 0u ? static_cast<unsigned int>(std::time(nullptr)) : options.seed);

  mpfr::mpreal::set_default_prec(options.precision);
  mpfr::mpreal::set_default_rnd(options.roundingMode);

  ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration> expressionParser(numberConverter);
  InitDefault(expressionParser);

  if(vm.count("expr") > 0u)
  {
    for(auto& i : vm["expr"].as<std::vector<std::string>>())
    {
      auto result = expressionParser.Evaluate(i);
      results.push_back(result);
      printResult(result);
    }
  }

  if(options.interactive)
  {
    results.clear();

    char* line;
    while((line = readline("> ")) != nullptr)
    {
      auto tmpPtr = std::unique_ptr<char, decltype(std::free)*>(line, std::free);

      auto result = expressionParser.Evaluate(line);
      results.push_back(result);
      printResult(result);

      if(*line != '\0')
      {
        add_history(line);
      }
    }
  }

  return 0;
}
