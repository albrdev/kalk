#include <vector>
#include <mpreal.h>
#include <boost/date_time/gregorian/gregorian.hpp>
#include "text/expression/ExpressionParser.hpp"

using KalkArithmeticType = mpfr::mpreal;
using KalkValueType      = ExpressionParser<KalkArithmeticType, boost::gregorian::date, boost::gregorian::date_duration>::ValueType;
using KalkVariableType   = ExpressionParser<KalkArithmeticType, boost::gregorian::date, boost::gregorian::date_duration>::VariableType;
using KalkFunctionType   = ExpressionParser<KalkArithmeticType>::FunctionType;

using KalkValueType2    = ExpressionParser<KalkArithmeticType>::ValueType;
using KalkVariableType2 = ExpressionParser<KalkArithmeticType>::VariableType;
using KalkFunctionType2 = ExpressionParser<KalkArithmeticType>::FunctionType;

inline std::vector<KalkValueType> results;

void InitDefault(ExpressionParser<KalkArithmeticType, boost::gregorian::date, boost::gregorian::date_duration>& instance);
void InitChemical(ExpressionParser<KalkArithmeticType>& instance);
