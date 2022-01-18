#include <vector>
#include <mpreal.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "text/expression/ExpressionParser.hpp"

using DefaultArithmeticType = mpfr::mpreal;
using DefaultValueType      = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::ValueType;
using DefaultVariableType   = ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>::VariableType;
using DefaultFunctionType   = ExpressionParser<DefaultArithmeticType>::FunctionType;

using ChemArithmeticType = mpfr::mpreal;
using ChemValueType      = ExpressionParser<DefaultArithmeticType>::ValueType;
using ChemVariableType   = ExpressionParser<DefaultArithmeticType>::VariableType;
using ChemFunctionType   = ExpressionParser<DefaultArithmeticType>::FunctionType;

inline std::vector<DefaultValueType> results;

void InitDefault(ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>& instance);
void InitChemical(ExpressionParser<DefaultArithmeticType>& instance);
