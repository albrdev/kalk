#ifndef __KALKSETUP_HPP__
#define __KALKSETUP_HPP__

#include "text/exception/SyntaxException.hpp"
#include "text/expression/ExpressionParser.hpp"
#include "text/parsing/CommandParser.hpp"

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <mpfr.h>
#include <mpreal.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using DefaultArithmeticType = mpfr::mpreal;
using DefaultValueType =
    Text::Expression::ValueToken<std::nullptr_t, DefaultArithmeticType, std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;
using DefaultVariableType =
    Text::Expression::VariableToken<std::nullptr_t, DefaultArithmeticType, std::string, boost::posix_time::ptime, boost::posix_time::time_duration>;

using ChemArithmeticType = mpfr::mpreal;
using ChemValueType      = Text::Expression::ValueToken<ChemArithmeticType>;
using ChemVariableType   = Text::Expression::VariableToken<ChemArithmeticType>;

using Text::Expression::IValueToken;
using Text::Expression::IVariableToken;
using Text::Expression::IUnaryOperatorToken;
using Text::Expression::IBinaryOperatorToken;
using Text::Expression::IFunctionToken;
using Text::Expression::UnaryOperatorToken;
using Text::Expression::BinaryOperatorToken;
using Text::Expression::FunctionToken;

using Text::Expression::Associativity;

using Text::Expression::ExpressionParser;
using Text::Parsing::CommandParser;
using Text::Exception::SyntaxException;

inline std::unordered_map<char, std::unique_ptr<UnaryOperatorToken>> defaultUnaryOperatorCache;
inline std::unordered_map<char, IUnaryOperatorToken*> defaultUnaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<BinaryOperatorToken>> defaultBinaryOperatorCache;
inline std::unordered_map<std::string, IBinaryOperatorToken*> defaultBinaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<FunctionToken>> defaultFunctionCache;
inline std::unordered_map<std::string, IFunctionToken*> defaultFunctions;

inline std::unordered_map<std::string, std::unique_ptr<DefaultVariableType>> defaultUninitializedVariableCache;
inline std::unordered_map<std::string, std::unique_ptr<DefaultVariableType>> defaultInitializedVariableCache;
inline std::unordered_map<std::string, IVariableToken*> defaultVariables;

inline std::vector<DefaultValueType> results;

inline std::vector<std::tuple<const IUnaryOperatorToken*, std::string, std::string>> unaryOperatorInfoMap;
inline std::vector<std::tuple<const IBinaryOperatorToken*, std::string, std::string>> binaryOperatorInfoMap;
inline std::vector<std::tuple<const IFunctionToken*, std::string, std::string>> functionInfoMap;
inline std::vector<std::tuple<const IVariableToken*, std::string, std::string>> variableInfoMap;

inline bool quit = false;

struct kalk_options
{
  mpfr_prec_t precision;
  mpfr_rnd_t roundingMode;
  int digits;
  int output_base;
  int input_base;
  int jpo_precedence;
  std::string date_ofmt;
  unsigned int seed;
  bool interactive;
};

const inline kalk_options defaultOptions {128, mpfr_rnd_t::MPFR_RNDN, 30, 10, 10, -1, "%Y-%m-%d %H:%M:%S", 0u, false};
inline kalk_options options {};

mpfr_rnd_t strToRmode(const std::string value);
void printValue(const DefaultValueType& value);
const DefaultValueType* ans(int index = -1);
void list(const std::string& searchPattern = ".*");

void InitDefaultExpressionParser(ExpressionParser& instance);
void InitChemicalExpressionParser(ExpressionParser& instance);
void InitCommandParser(CommandParser& instance);

#endif // __KALKSETUP_HPP__
