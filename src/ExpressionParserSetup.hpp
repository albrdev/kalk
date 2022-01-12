#include <vector>
#include <mpreal.h>
#include "expression/ExpressionParser.hpp"

using KalkArithmeticType = mpfr::mpreal;
using KalkValueType      = ExpressionParser<KalkArithmeticType>::ValueType;
using KalkVariableType   = ExpressionParser<KalkArithmeticType>::VariableType;
using KalkFunctionType   = ExpressionParser<KalkArithmeticType>::FunctionType;

inline std::vector<KalkValueType> results;

void InitDefault(ExpressionParser<KalkArithmeticType>& instance);
void InitChemical(ExpressionParser<KalkArithmeticType>& instance);
