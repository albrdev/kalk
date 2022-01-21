#include <vector>
#include <mpreal.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "text/expression/ExpressionParser.hpp"

using DefaultArithmeticType     = mpfr::mpreal;
using DefaultValueType          = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::ValueType;
using DefaultVariableType       = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::VariableType;
using DefaultUnaryOperatorType  = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::UnaryOperatorType;
using DefaultBinaryOperatorType = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::BinaryOperatorType;
using DefaultFunctionType       = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::FunctionType;

using ChemArithmeticType     = mpfr::mpreal;
using ChemValueType          = ExpressionParser<ChemArithmeticType>::ValueType;
using ChemVariableType       = ExpressionParser<ChemArithmeticType>::VariableType;
using ChemUnaryOperatorType  = ExpressionParser<ChemArithmeticType>::UnaryOperatorType;
using ChemBinaryOperatorType = ExpressionParser<ChemArithmeticType>::BinaryOperatorType;
using ChemFunctionType       = ExpressionParser<ChemArithmeticType>::FunctionType;

inline std::unordered_map<char, std::unique_ptr<DefaultUnaryOperatorType>> defaultUnaryOperatorCache;
inline std::unordered_map<char, DefaultUnaryOperatorType*> defaultUnaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<DefaultBinaryOperatorType>> defaultBinaryOperatorCache;
inline std::unordered_map<std::string, DefaultBinaryOperatorType*> defaultBinaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<DefaultFunctionType>> defaultFunctionCache;
inline std::unordered_map<std::string, DefaultFunctionType*> defaultFunctions;

inline std::unordered_map<std::string, std::unique_ptr<DefaultVariableType>> defaultUninitializedVariableCache;
inline std::unordered_map<std::string, std::unique_ptr<DefaultVariableType>> defaultInitializedVariableCache;
inline std::unordered_map<std::string, DefaultVariableType*> defaultVariables;

inline std::vector<DefaultValueType> results;

void InitDefault(ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>& instance);
void InitChemical(ExpressionParser<ChemArithmeticType>& instance);
