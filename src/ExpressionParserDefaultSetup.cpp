#include "Setup.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <gmpxx.h>
#include <boost/date_time/time_duration.hpp>
#include <boost/format.hpp>

static IValueToken* numberConverter(const std::string& value)
{
  return new DefaultValueType(mpfr::mpreal(value, mpfr::mpreal::get_default_prec(), options.input_base, mpfr::mpreal::get_default_rnd()));
}

static IValueToken* stringConverter(const std::string& value) { return new DefaultValueType(value); }

struct GreaterComparer
{
  bool operator()(IValueToken* a, IValueToken* b) const
  {
    return a->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() < b->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
  }
};

static std::string operator*(const std::string& lhs, std::size_t rhs)
{
  std::string result;
  result.reserve(lhs.length() * rhs);
  for(std::size_t i = 0u; i < rhs; i++)
  {
    result += lhs;
  }

  return result;
}

int compare(const IValueToken* a, const IValueToken* b)
{
  auto aValue = a->As<const DefaultValueType*>();
  auto bValue = b->As<const DefaultValueType*>();
  if(a->GetType() == typeid(std::string) && b->GetType() == typeid(std::string))
  {
    return aValue->GetValue<std::string>().compare(bValue->GetValue<std::string>());
  }
  else if(a->GetType() == typeid(boost::posix_time::ptime) && b->GetType() == typeid(boost::posix_time::ptime))
  {
    return aValue->GetValue<boost::posix_time::ptime>() < bValue->GetValue<boost::posix_time::ptime>() ?
             -1 :
             (aValue->GetValue<boost::posix_time::ptime>() > bValue->GetValue<boost::posix_time::ptime>() ? 1 : 0);
  }
  else if(a->GetType() == typeid(boost::posix_time::time_duration) && bValue->GetType() == typeid(boost::posix_time::time_duration))
  {
    return aValue->GetValue<boost::posix_time::time_duration>() < bValue->GetValue<boost::posix_time::time_duration>() ?
             -1 :
             (aValue->GetValue<boost::posix_time::time_duration>() > bValue->GetValue<boost::posix_time::time_duration>() ? 1 : 0);
  }
  else if(a->GetType() == typeid(std::nullptr_t) && b->GetType() == typeid(std::nullptr_t))
  {
    return 0;
  }
  else
  {
    return aValue->GetValue<DefaultArithmeticType>() < bValue->GetValue<DefaultArithmeticType>() ?
             -1 :
             (aValue->GetValue<DefaultArithmeticType>() > bValue->GetValue<DefaultArithmeticType>() ? 1 : 0);
  }
}

void printValue(const DefaultValueType& value)
{
  if(value.GetType() == typeid(DefaultArithmeticType))
  {
    std::cout << value.GetValue<DefaultArithmeticType>().toString(options.digits, options.output_base, mpfr::mpreal::get_default_rnd()) << std::endl;
  }
  else if(value.GetType() == typeid(boost::posix_time::ptime))
  {
    std::cout << value.GetValue<boost::posix_time::ptime>() << std::endl;
  }
  else if(value.GetType() == typeid(boost::posix_time::time_duration))
  {
    std::cout << value.GetValue<boost::posix_time::time_duration>() << std::endl;
  }
  else
  {
    std::cout << value.ToString() << std::endl;
  }
}

const DefaultValueType* ans(int index)
{
  if(results.empty())
  {
    throw std::runtime_error("No results available");
  }

  if(index < 0)
  {
    index = static_cast<int>(results.size()) + index;
  }

  if(index < 0 || static_cast<std::size_t>(index) >= results.size())
  {
    throw SyntaxError((boost::format("Results index out of range: %1%/%2%") % index % results.size()).str());
  }

  return &results.at(static_cast<std::size_t>(index));
}

static std::string makeCompoundString(std::string text)
{
  if(text.empty() || std::isdigit(text.front()) != 0)
  {
    throw SyntaxError("Invalid chemical compound string");
  }

  char lastChar = '\0';
  std::string result;
  for(const auto& i : text)
  {
    if(std::isalnum(i) == 0 && (i != '(' && i != ')'))
    {
      throw SyntaxError("Invalid characters in chemical compound string");
    }

    if(lastChar != '\0' && lastChar != '(')
    {
      if(std::isdigit(i) != 0)
      {
        if(std::isdigit(lastChar) == 0)
        {
          result += '*';
        }
      }
      else if(std::isupper(i) != 0 || i == '(')
      {
        result += '+';
      }
    }

    result += i;
    lastChar = i;
  }

  return result;
}

static void addUnaryOperator(const UnaryOperatorToken::CallbackType& callback,
                             char identifier,
                             int precedence,
                             Associativity associativity,
                             const std::string& title       = "",
                             const std::string& description = "")
{
  auto tmpNew                           = std::make_unique<UnaryOperatorToken>(identifier, callback, precedence, associativity);
  auto tmp                              = tmpNew.get();
  defaultUnaryOperatorCache[identifier] = std::move(tmpNew);
  defaultUnaryOperators[identifier]     = tmp;

  unaryOperatorInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static void addBinaryOperator(const BinaryOperatorToken::CallbackType& callback,
                              const std::string& identifier,
                              int precedence,
                              Associativity associativity,
                              const std::string& title       = "",
                              const std::string& description = "")
{
  auto tmpNew                            = std::make_unique<BinaryOperatorToken>(identifier, callback, precedence, associativity);
  auto tmp                               = tmpNew.get();
  defaultBinaryOperatorCache[identifier] = std::move(tmpNew);
  defaultBinaryOperators[identifier]     = tmp;

  binaryOperatorInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static void addFunction(const FunctionToken::CallbackType& callback,
                        const std::string& identifier,
                        std::size_t minArgs            = 0u,
                        std::size_t maxArgs            = FunctionToken::GetArgumentCountMaxLimit(),
                        const std::string& title       = "",
                        const std::string& description = "")
{
  auto tmpNew                      = std::make_unique<FunctionToken>(identifier, callback, minArgs, maxArgs);
  auto tmp                         = tmpNew.get();
  defaultFunctionCache[identifier] = std::move(tmpNew);
  defaultFunctions[identifier]     = tmp;

  functionInfoMap.push_back(std::make_tuple(tmp, title, description));
}

template<class T>
static void addVariable(const T& value, const std::string& identifier, const std::string& title = "", const std::string& description = "")
{
  auto tmpNew                                 = std::make_unique<DefaultVariableType>(identifier, value);
  auto tmp                                    = tmpNew.get();
  defaultInitializedVariableCache[identifier] = std::move(tmpNew);
  defaultVariables[identifier]                = tmp;

  variableInfoMap.push_back(std::make_tuple(tmp, title, description));
}

static void removeVariable(const std::string& identifier)
{
  defaultVariables.erase(identifier);
  if(defaultInitializedVariableCache.erase(identifier) == 0u)
  {
    defaultUninitializedVariableCache.erase(identifier);
  }
}

static DefaultValueType* addNewVariable(const std::string& identifier)
{
  auto tmpNew                                   = std::make_unique<DefaultVariableType>(identifier);
  auto result                                   = tmpNew.get();
  defaultUninitializedVariableCache[identifier] = std::move(tmpNew);
  defaultVariables[identifier]                  = result;
  return result;
}

#ifndef __REGION__UNOPS
#ifndef __REGION__UNOPS__COMMON
static IValueToken* UnaryOperator_Plus(IValueToken* rhs)
{
  if(rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    const auto& tmpValue = rhs->As<DefaultValueType*>()->GetValue<boost::posix_time::time_duration>();
    const boost::posix_time::time_duration zero;
    return new DefaultValueType(tmpValue < zero ? -tmpValue : tmpValue);
  }
  else
  {
    return new DefaultValueType(mpfr::abs(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
  }
}

static IValueToken* UnaryOperator_Minus(IValueToken* rhs)
{
  if(rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(-rhs->As<DefaultValueType*>()->GetValue<boost::posix_time::time_duration>());
  }
  else
  {
    return new DefaultValueType(-rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* UnaryOperator_Factorial(IValueToken* rhs)
{
  if(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() < 0)
  {
    throw std::range_error("Value cannot be negative");
  }

  return new DefaultValueType(mpfr::fac_ui(static_cast<unsigned long>(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>())));
}
#endif // __REGION__UNOPS__COMMON

#ifndef __REGION__UNOPS__BITWISE
static IValueToken* UnaryOperator_Not(IValueToken* rhs) { return new DefaultValueType(!rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()); }

static IValueToken* UnaryOperator_BitwiseOnesComplement(IValueToken* rhs)
{
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = ~tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}
#endif // __REGION__UNOPS__BITWISE
#endif // __REGION__UNOPS

#ifndef __REGION__BINOPS
#ifndef __REGION__BINOPS__COMPARISON
static IValueToken* BinaryOperator_Equals(IValueToken* lhs, IValueToken* rhs) { return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) == 0)); }

static IValueToken* BinaryOperator_NotEquals(IValueToken* lhs, IValueToken* rhs) { return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) != 0)); }

static IValueToken* BinaryOperator_Lesser(IValueToken* lhs, IValueToken* rhs) { return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) < 0)); }

static IValueToken* BinaryOperator_Greater(IValueToken* lhs, IValueToken* rhs) { return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) > 0)); }

static IValueToken* BinaryOperator_LesserOrEquals(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) <= 0));
}

static IValueToken* BinaryOperator_GreaterOrEquals(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) >= 0));
}

static IValueToken* BinaryOperator_LogicalOr(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() ||
                              rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
}

static IValueToken* BinaryOperator_LogicalAnd(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() &&
                              rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
}
#endif // __REGION__BINOPS__COMPARISON

#ifndef __REGION__BINOPS__COMMON
static IValueToken* BinaryOperator_Addition(IValueToken* lhs, IValueToken* rhs)
{
  auto lhsValue = lhs->As<DefaultValueType*>();
  auto rhsValue = rhs->As<DefaultValueType*>();
  if(lhs->GetType() == typeid(std::string) || rhs->GetType() == typeid(std::string))
  {
    std::string tmpString;
    if(lhs->GetType() != typeid(std::nullptr_t))
    {
      tmpString += lhs->ToString();
    }

    if(rhs->GetType() != typeid(std::nullptr_t))
    {
      tmpString += rhs->ToString();
    }

    return new DefaultValueType(tmpString);
  }
  else if(lhs->GetType() == typeid(boost::posix_time::time_duration) && rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(lhsValue->GetValue<boost::posix_time::time_duration>() + rhsValue->GetValue<boost::posix_time::time_duration>());
  }
  else if(lhs->GetType() == typeid(boost::posix_time::ptime) && rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(lhsValue->GetValue<boost::posix_time::ptime>() + rhsValue->GetValue<boost::posix_time::time_duration>());
  }
  else
  {
    return new DefaultValueType(lhsValue->GetValue<DefaultArithmeticType>() + rhsValue->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* BinaryOperator_Subtraction(IValueToken* lhs, IValueToken* rhs)
{
  auto lhsValue = lhs->As<DefaultValueType*>();
  auto rhsValue = rhs->As<DefaultValueType*>();
  if(lhs->GetType() == typeid(boost::posix_time::ptime) && rhs->GetType() == typeid(boost::posix_time::ptime))
  {
    return new DefaultValueType(lhsValue->GetValue<boost::posix_time::ptime>() - rhsValue->GetValue<boost::posix_time::ptime>());
  }
  else if(lhs->GetType() == typeid(boost::posix_time::time_duration) && rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(lhsValue->GetValue<boost::posix_time::time_duration>() - rhsValue->GetValue<boost::posix_time::time_duration>());
  }
  else if(lhs->GetType() == typeid(boost::posix_time::ptime) && rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(lhsValue->GetValue<boost::posix_time::ptime>() - rhsValue->GetValue<boost::posix_time::time_duration>());
  }
  else
  {
    return new DefaultValueType(lhsValue->GetValue<DefaultArithmeticType>() - rhsValue->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* BinaryOperator_Multiplication(IValueToken* lhs, IValueToken* rhs)
{
  auto lhsValue = lhs->As<DefaultValueType*>();
  auto rhsValue = rhs->As<DefaultValueType*>();
  if(lhs->GetType() == typeid(std::string) && rhs->GetType() == typeid(DefaultArithmeticType))
  {
    return new DefaultValueType(lhsValue->GetValue<std::string>() * static_cast<std::size_t>(rhsValue->GetValue<DefaultArithmeticType>()));
  }
  else if(lhs->GetType() == typeid(boost::posix_time::time_duration) || rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    if(lhs->GetType() == typeid(DefaultArithmeticType))
    {
      auto ticks =
          boost::posix_time::nanoseconds(static_cast<long>(static_cast<double>(rhsValue->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) *
                                                           lhsValue->GetValue<DefaultArithmeticType>().toDouble()));
      return new DefaultValueType(boost::posix_time::time_duration(ticks));
    }
    else
    {
      auto ticks =
          boost::posix_time::nanoseconds(static_cast<long>(static_cast<double>(lhsValue->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) *
                                                           rhsValue->GetValue<DefaultArithmeticType>().toDouble()));
      return new DefaultValueType(boost::posix_time::time_duration(ticks));
    }
  }
  else
  {
    return new DefaultValueType(lhsValue->GetValue<DefaultArithmeticType>() * rhsValue->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* BinaryOperator_Division(IValueToken* lhs, IValueToken* rhs)
{
  auto lhsValue = lhs->As<DefaultValueType*>();
  auto rhsValue = rhs->As<DefaultValueType*>();
  if(lhs->GetType() == typeid(boost::posix_time::time_duration) && rhs->GetType() == typeid(DefaultArithmeticType))
  {
    auto ticks =
        boost::posix_time::nanoseconds(static_cast<long>(static_cast<double>(lhsValue->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) /
                                                         rhsValue->GetValue<DefaultArithmeticType>().toDouble()));
    return new DefaultValueType(boost::posix_time::time_duration(ticks));
  }
  else
  {
    return new DefaultValueType(lhsValue->GetValue<DefaultArithmeticType>() / rhsValue->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* BinaryOperator_TruncatedDivision(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(
      mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() / rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_Fmod(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(
      mpfr::fmod(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_Remainder(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(
      mpfr::remainder(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* BinaryOperator_Exponentiation(IValueToken* lhs, IValueToken* rhs)
{
  return new DefaultValueType(
      mpfr::pow(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__BINOPS__COMMON

#ifndef __REGION__BINOPS__BITWISE
static IValueToken* BinaryOperator_BitwiseOr(IValueToken* lhs, IValueToken* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs | tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static IValueToken* BinaryOperator_BitwiseAnd(IValueToken* lhs, IValueToken* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs & tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static IValueToken* BinaryOperator_BitwiseXor(IValueToken* lhs, IValueToken* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs ^ tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static IValueToken* BinaryOperator_BitwiseLeftShift(IValueToken* lhs, IValueToken* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs << static_cast<mp_bitcnt_t>(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>().toULong());
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static IValueToken* BinaryOperator_BitwiseRightShift(IValueToken* lhs, IValueToken* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs >> static_cast<mp_bitcnt_t>(rhs->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>().toULong());
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}
#endif // __REGION__BINOPS__BITWISE

#ifndef __REGION__BINOPS__SPECIAL
static IValueToken* BinaryOperator_VariableAssignment(IValueToken* lhs, IValueToken* rhs)
{
  DefaultVariableType* variable = lhs->As<DefaultVariableType*>();
  if(variable == nullptr)
  {
    throw SyntaxError((boost::format("Assignment of non-variable type: %1% (%2%)") % lhs->ToString() % lhs->GetTypeInfo().name()).str());
  }

  bool isInitialAssignment = !variable->IsInitialized();

  auto rhsValue = rhs->As<DefaultValueType*>();
  if(rhs->GetType() == typeid(DefaultArithmeticType))
  {
    (*variable) = rhsValue->GetValue<DefaultArithmeticType>();
  }
  else if(rhs->GetType() == typeid(boost::posix_time::ptime))
  {
    (*variable) = rhsValue->GetValue<boost::posix_time::ptime>();
  }
  else if(rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    (*variable) = rhsValue->GetValue<boost::posix_time::time_duration>();
  }
  else if(rhs->GetType() == typeid(std::string))
  {
    (*variable) = rhsValue->GetValue<std::string>();
  }
  else if(rhs->GetType() == typeid(std::nullptr_t))
  {
    (*variable) = rhsValue->GetValue<std::nullptr_t>();
  }
  else
  {
    throw SyntaxError((boost::format("Assignment from unsupported type: %1% (%2%)") % rhs->ToString() % rhs->GetType().name()).str());
  }

  if(isInitialAssignment)
  {
    auto variableIterator = defaultUninitializedVariableCache.extract(variable->GetIdentifier());
    defaultInitializedVariableCache.insert(std::move(variableIterator));
  }

  return variable;
}
#endif // __REGION__BINOPS__SPECIAL
#endif // __REGION__BINOPS

#ifndef __REGION__FUNCTIONS
#ifndef __REGION__FUNCTIONS__COMMON
static IValueToken* Function_Sgn(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType((args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() > 0) -
                              (args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() < 0));
}

static IValueToken* Function_Abs(const std::vector<IValueToken*>& args)
{
  if(args[0]->GetType() == typeid(boost::posix_time::time_duration))
  {
    const auto& tmpValue = args[0]->As<DefaultValueType*>()->GetValue<boost::posix_time::time_duration>();
    const boost::posix_time::time_duration zero;
    return new DefaultValueType(tmpValue < zero ? -tmpValue : tmpValue);
  }
  else
  {
    return new DefaultValueType(mpfr::abs(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
  }
}

static IValueToken* Function_Neg(const std::vector<IValueToken*>& args)
{
  if(args[0]->GetType() == typeid(boost::posix_time::time_duration))
  {
    return new DefaultValueType(-args[0]->As<DefaultValueType*>()->GetValue<boost::posix_time::time_duration>());
  }
  else
  {
    return new DefaultValueType(-args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* Function_NegAbs(const std::vector<IValueToken*>& args)
{
  if(args[0]->GetType() == typeid(boost::posix_time::time_duration))
  {
    const auto& tmpValue = args[0]->As<DefaultValueType*>()->GetValue<boost::posix_time::time_duration>();
    const boost::posix_time::time_duration zero;
    return new DefaultValueType(tmpValue > zero ? -tmpValue : tmpValue);
  }
  else
  {
    return new DefaultValueType(-mpfr::abs(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
  }
}

static IValueToken* Function_Round(const std::vector<IValueToken*>& args)
{
  const mpfr_rnd_t tmpRndMode = (args.size() > 0u) ? strToRmode(args[1]->As<DefaultValueType*>()->GetValue<std::string>()) : mpfr::mpreal::get_default_rnd();
  return new DefaultValueType(mpfr::rint(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), tmpRndMode));
}

static IValueToken* Function_RoundE(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::rint(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), mpfr_rnd_t::MPFR_RNDN));
}

static IValueToken* Function_RoundA(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::round(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Ceil(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::ceil(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Floor(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::floor(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Trunc(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::trunc(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Fmod(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      mpfr::fmod(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Rem(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::remainder(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(),
                                              args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Mod(const std::vector<IValueToken*>& args)
{
  const auto& a = args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
  const auto& b = args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
  return new DefaultValueType(a - (mpfr::floor(a / b) * b));
}

static IValueToken* Function_Pow(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      mpfr::pow(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Sqr(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), 2));
}

static IValueToken* Function_Cb(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), 3));
}

static IValueToken* Function_Root(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      mpfr::pow(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), 1 / args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Sqrt(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::sqrt(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Cbrt(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::cbrt(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Exp(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::exp(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Exp2(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::exp2(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Exp10(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::exp10(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_LogN(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::log(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()) /
                              mpfr::log(args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Log(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::log(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Log2(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::log2(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Log10(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::log10(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__FUNCTIONS__COMMON

#ifndef __REGION__FUNCTIONS__TRIGONOMETRY
static IValueToken* Function_Sin(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::sin(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Cos(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::cos(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Tan(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::tan(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Cot(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::cot(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Sec(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::sec(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_Csc(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::csc(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ASin(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::asin(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACos(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acos(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ATan(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::atan(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ATan2(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(
      mpfr::atan2(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>(), args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACot(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acot(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ASec(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::asec(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACsc(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acsc(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_SinH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::sinh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_CosH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::cosh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_TanH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::tanh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_CotH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::coth(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_SecH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::sech(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_CscH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::csch(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ASinH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::asinh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACosH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acosh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ATanH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::atanh(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACotH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acoth(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ASecH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::asech(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}

static IValueToken* Function_ACscH(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::acsch(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__FUNCTIONS__TRIGONOMETRY

#ifndef __REGION__FUNCTIONS__AGGREGATES
static IValueToken* Function_Min(const std::vector<IValueToken*>& args)
{
  auto result = std::numeric_limits<DefaultArithmeticType>::max();
  for(const auto& i : args)
  {
    auto tmpValue = i->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
    if(tmpValue < result)
    {
      result = tmpValue;
    }
  }

  return new DefaultValueType(result);
}
static IValueToken* Function_Max(const std::vector<IValueToken*>& args)
{
  auto result = std::numeric_limits<DefaultArithmeticType>::min();
  for(const auto& i : args)
  {
    auto tmpValue = i->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
    if(tmpValue > result)
    {
      result = tmpValue;
    }
  }

  return new DefaultValueType(result);
}

static IValueToken* Function_Mean(const std::vector<IValueToken*>& args)
{
  DefaultArithmeticType result = 0;
  for(const auto& i : args)
  {
    result += i->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
  }

  return new DefaultValueType(result / static_cast<DefaultArithmeticType>(args.size()));
}

static IValueToken* Function_Median(const std::vector<IValueToken*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 2u;
  if(tmpArgs.size() % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[middle - 1u]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() +
                                 tmpArgs[middle]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()) /
                                2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[middle]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* Function_Quartile_Lower(const std::vector<IValueToken*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 4u;
  if(middle % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[middle - 1u]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() +
                                 tmpArgs[middle]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()) /
                                2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[middle]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* Function_Quartile_Upper(const std::vector<IValueToken*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle   = tmpArgs.size() / 2u;
  std::size_t q        = middle / 2u;
  std::size_t tmpIndex = (middle + (tmpArgs.size() % 2 == 0 ? 0 : 1)) + q;
  if(middle % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[tmpIndex - 1u]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() +
                                 tmpArgs[tmpIndex]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()) /
                                2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[tmpIndex]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
}

static IValueToken* Function_Mode(const std::vector<IValueToken*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());

  auto current             = tmpArgs.front();
  std::size_t currentCount = 1u;
  auto mode                = current;
  std::size_t modeCount    = currentCount;

  for(std::size_t i = 1u; i < tmpArgs.size(); i++)
  {
    if(tmpArgs[i]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() == current->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>())
    {
      currentCount++;
    }
    else
    {
      if(currentCount > modeCount)
      {
        modeCount = currentCount;
        mode      = current;
      }

      currentCount = 1u;
      current      = tmpArgs[i];
    }
  }

  return new DefaultValueType(mode->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
}
#endif // __REGION__FUNCTIONS__AGGREGATES

#ifndef __REGION__FUNCTIONS__STRING
static IValueToken* Function_Str(const std::vector<IValueToken*>& args) { return new DefaultValueType(args[0]->ToString()); }

static IValueToken* Function_StrLen(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(static_cast<DefaultArithmeticType>(args[0]->As<DefaultValueType*>()->GetValue<std::string>().length()));
}
#endif // #ifndef __REGION__FUNCTIONS__STRING

#ifndef __REGION__FUNCTIONS__DATE_TIME
static IValueToken* Function_Date(const std::vector<IValueToken*>& args)
{
  if(args.size() == 0u)
  {
    return new DefaultValueType(boost::posix_time::second_clock::local_time());
  }
  else
  {
    return new DefaultValueType(boost::posix_time::time_from_string(args[0]->As<DefaultValueType*>()->GetValue<std::string>()));
  }
}

static IValueToken* Function_Dur(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(boost::posix_time::duration_from_string(args[0]->As<DefaultValueType*>()->GetValue<std::string>()));
}
#endif // __REGION__FUNCTIONS__DATE_TIME

#ifndef __REGION__FUNCTIONS__MISC
static IValueToken* Function_Random(const std::vector<IValueToken*>& args)
{
  if(args.size() == 0u)
  {
    return new DefaultValueType(mpfr::random());
  }
  else if(args.size() == 1u)
  {
    return new DefaultValueType(mpfr::random() * args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  }
  else
  {
    const auto diff = args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() - args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>();
    return new DefaultValueType(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>() + (mpfr::random() * diff));
  }
}

static IValueToken* Function_BConv(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(mpfr::mpreal(args[0]->As<DefaultValueType*>()->GetValue<std::string>(),
                                           mpfr::mpreal::get_default_prec(),
                                           static_cast<int>(mpfr::trunc(args[1]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>()).toLong())));
}
#endif // __REGION__FUNCTIONS__MISC

#ifndef __REGION__FUNCTIONS__SPECIAL
static IValueToken* Function_Ans(const std::vector<IValueToken*>& args)
{
  if(args.empty())
  {
    return new DefaultValueType(*ans());
  }

  int index = static_cast<int>(args[0]->As<DefaultValueType*>()->GetValue<DefaultArithmeticType>());
  return new DefaultValueType(*ans(index));
}

static IValueToken* Function_Del(const std::vector<IValueToken*>& args)
{
  const std::string identifier = args[0]->As<DefaultValueType*>()->GetValue<std::string>();
  auto iter                    = defaultInitializedVariableCache.find(identifier);
  if(iter == defaultInitializedVariableCache.end())
  {
    iter = defaultUninitializedVariableCache.find(identifier);
    if(iter == defaultUninitializedVariableCache.end())
    {
      throw SyntaxError("Deletion of nonexistent variable: " + identifier);
    }
  }

  auto result = new DefaultValueType(*dynamic_cast<DefaultValueType*>(iter->second.get()));
  removeVariable(identifier);
  return result;
}
#endif // __REGION__FUNCTIONS__SPECIAL

static ExpressionParser chemicalExpressionParser;

static IValueToken* Function_MolarMass(const std::vector<IValueToken*>& args)
{
  return new DefaultValueType(chemicalExpressionParser.Evaluate(makeCompoundString(args[0]->As<DefaultValueType*>()->GetValue<std::string>()))
                                  ->As<ChemValueType*>()
                                  ->GetValue<ChemArithmeticType>());
}
#endif // __REGION__FUNCTIONS

static std::unique_ptr<BinaryOperatorToken> juxtapositionOperator;

void InitDefaultExpressionParser(ExpressionParser& instance)
{
  if(options.jpo_precedence != 0)
  {
    juxtapositionOperator = std::make_unique<BinaryOperatorToken>("*", BinaryOperator_Multiplication, 6 + options.jpo_precedence, Associativity::Left);
  }

  instance.SetOnParseNumberCallback(numberConverter);
  instance.SetOnParseStringCallback(stringConverter);
  instance.SetOnUnknownIdentifierCallback(addNewVariable);
  instance.SetJuxtapositionOperator(juxtapositionOperator.get());

  instance.SetUnaryOperators(&defaultUnaryOperators);
  instance.SetBinaryOperators(&defaultBinaryOperators);
  instance.SetFunctions(&defaultFunctions);
  instance.SetVariables(&defaultVariables);

  addUnaryOperator(UnaryOperator_Not, '!', 9, Associativity::Right, "Not", "!x");
  unaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));
  addUnaryOperator(UnaryOperator_Plus, '+', 9, Associativity::Right, "Unary plus", "+x");
  addUnaryOperator(UnaryOperator_Minus, '-', 9, Associativity::Right, "Unary minus", "-x");
  addUnaryOperator(UnaryOperator_Factorial, ':', 9, Associativity::Right, "Factorial", ":x = x!");
  addUnaryOperator(UnaryOperator_BitwiseOnesComplement, '~', 9, Associativity::Right, "One\'s complement", "Invert bits");

  addBinaryOperator(BinaryOperator_Equals, "==", 3, Associativity::Left, "Equals", "x == y");
  addBinaryOperator(BinaryOperator_NotEquals, "!=", 3, Associativity::Left, "Not equals", "x != y");
  addBinaryOperator(BinaryOperator_Lesser, "<", 3, Associativity::Left, "Lesser", "x < y");
  addBinaryOperator(BinaryOperator_Greater, ">", 3, Associativity::Left, "Greater", "x > y");
  addBinaryOperator(BinaryOperator_LesserOrEquals, "<=", 3, Associativity::Left, "Lesser or equal", "x <= y");
  addBinaryOperator(BinaryOperator_GreaterOrEquals, ">=", 3, Associativity::Left, "Greater or equal", "x >= y");
  binaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addBinaryOperator(BinaryOperator_LogicalOr, "||", 1, Associativity::Left, "Logical OR", "x || y");
  addBinaryOperator(BinaryOperator_LogicalAnd, "&&", 1, Associativity::Left, "Logical AND", "x && y");
  binaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addBinaryOperator(BinaryOperator_Addition, "+", 4, Associativity::Left, "Addition", "x + y");
  addBinaryOperator(BinaryOperator_Subtraction, "-", 4, Associativity::Left, "Subtraction", "x - y");
  addBinaryOperator(BinaryOperator_Multiplication, "*", 6, Associativity::Left, "Multiplication", "x * y");
  addBinaryOperator(BinaryOperator_Division, "/", 6, Associativity::Left, "Division", "x / y");
  addBinaryOperator(BinaryOperator_TruncatedDivision,
                    "//",
                    6,
                    Associativity::Left,
                    "Truncated division",
                    "Division with the quotient\'s fractional part truncated");
  addBinaryOperator(BinaryOperator_Fmod, "%", 6, Associativity::Left, "Floating point modulo", "Returns the remainder of x / y (Using truncation)");
  addBinaryOperator(BinaryOperator_Remainder, "%%", 6, Associativity::Left, "Remainder", "Returns the remainder of x / y (Using round to nearest)");
  addBinaryOperator(BinaryOperator_Exponentiation, "**", 8, Associativity::Right, "Power", "Returns x to the power of y");
  binaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addBinaryOperator(BinaryOperator_BitwiseOr, "|", 2, Associativity::Left, "Bitwise OR", "x | y");
  addBinaryOperator(BinaryOperator_BitwiseAnd, "&", 2, Associativity::Left, "Bitwise AND", "x & y");
  addBinaryOperator(BinaryOperator_BitwiseXor, "^", 2, Associativity::Left, "Bitwise XOR", "x ^ y");
  addBinaryOperator(BinaryOperator_BitwiseLeftShift, "<<", 2, Associativity::Left, "Bitwise left shift", "Shift bits n steps to the left");
  addBinaryOperator(BinaryOperator_BitwiseRightShift, ">>", 2, Associativity::Left, "Bitwise right shift", "Shift bits n steps to the right");
  binaryOperatorInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addBinaryOperator(BinaryOperator_VariableAssignment, "=", 10, Associativity::Right, "Assignment", "Assigns variable");

  addFunction(Function_Ans, "ans", 0u, 1u, "Answer", "Returns the result at the index specified by argument");
  addFunction(Function_Del, "del", 1u, 1u, "Delete", "Delete and return the variable that matches the argument");
  addFunction(Function_BConv, "bconv", 2u, 2u, "Base conversion", "Convert value specified by argument x(str) from the base specified by y(num)");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Sgn, "sgn", 1u, 1u, "Sign", "Returns the sign (-1, 0, 1)");
  addFunction(Function_Abs, "abs", 1u, 1u, "Absolute value", "Returns the absolute value");
  addFunction(Function_Neg, "neg", 1u, 1u, "Negate", "Returns the negated value");
  addFunction(Function_NegAbs, "negabs", 1u, 1u, "Negate absolute value", "Returns the negated absolute value");
  addFunction(Function_Round, "round", 1u, 2u, "Round", "Round a value according to second argument or the default rounding mode");
  addFunction(Function_RoundE, "rounde", 1u, 1u, "Round even", "Round a value with halfway cases to nearest even number");
  addFunction(Function_RoundA, "rounda", 1u, 1u, "Round away", "Round a value with halfway cases away from zero");
  addFunction(Function_Ceil, "ceil", 1u, 1u, "Ceil", "Round a value towards higher or equal number");
  addFunction(Function_Floor, "floor", 1u, 1u, "Floor", "Round a value towards lower or equal number");
  addFunction(Function_Trunc, "trunc", 1u, 1u, "Truncation", "Truncates the fractional part (Round towards zero)");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Fmod, "math.fmod", 2u, 2u, "Floating point modulo", "Returns the remainder of x / y (Using truncation)");
  addFunction(Function_Rem, "math.rem", 2u, 2u, "Remainder", "Returns the remainder of x / y (Using round to nearest)");
  addFunction(Function_Mod, "math.mod", 2u, 2u, "Modulo", "Returns modulo of x / y");
  addFunction(Function_Pow, "math.pow", 2u, 2u, "Power", "Returns x to the power of y");
  addFunction(Function_Sqr, "math.sqr", 1u, 1u, "Square", "Returns x to the power of 2");
  addFunction(Function_Cb, "math.cb", 1u, 1u, "Cube", "Returns x to the power of 3");
  addFunction(Function_Root, "math.root", 2u, 2u, "Root", "Returns nth root of x");
  addFunction(Function_Sqrt, "math.sqrt", 1u, 1u, "Square root", "Returns square root of x");
  addFunction(Function_Cbrt, "math.cbrt", 1u, 1u, "Cubic root", "Returns cubic root of x");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Exp, "math.exp", 1u, 1u, "Natural exponent", "Returns e to the power of x");
  addFunction(Function_Exp2, "math.exp2", 1u, 1u, "Binary exponent", "Returns 2 to the power of x");
  addFunction(Function_Exp10, "math.exp10", 1u, 1u, "Decimal exponent", "Returns 10 to the power of x");
  addFunction(Function_LogN, "math.logn", 1u, 1u, "Logarithm", "Returns nth logarithm of x");
  addFunction(Function_Log, "math.log", 1u, 1u, "Natural logarithm", "Returns nth logarithm of e");
  addFunction(Function_Log2, "math.log2", 1u, 1u, "Binary logarithm", "Returns nth logarithm of 2");
  addFunction(Function_Log10, "math.log10", 1u, 1u, "Decimal logarithm", "Returns nth logarithm of 10");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Sin, "math.sin", 1u, 1u, "Sine", "Trigonometric function");
  addFunction(Function_Cos, "math.cos", 1u, 1u, "Cosine", "Trigonometric function");
  addFunction(Function_Tan, "math.tan", 1u, 1u, "tangent", "Trigonometric function");
  addFunction(Function_Cot, "math.cot", 1u, 1u, "cotangent", "Trigonometric function");
  addFunction(Function_Sec, "math.sec", 1u, 1u, "secant", "Trigonometric function");
  addFunction(Function_Csc, "math.csc", 1u, 1u, "cosecant", "Trigonometric function");

  addFunction(Function_ASin, "math.asin", 1u, 1u, "Arcsine", "Trigonometric function");
  addFunction(Function_ACos, "math.acos", 1u, 1u, "Arccosine", "Trigonometric function");
  addFunction(Function_ATan, "math.atan", 1u, 1u, "Arctangent", "Trigonometric function");
  addFunction(Function_ATan2, "math.atan2", 2u, 2u, "Arctangent 2", "Trigonometric function (2 argument version)");
  addFunction(Function_ACot, "math.acot", 1u, 1u, "Arccotangent", "Trigonometric function");
  addFunction(Function_ASec, "math.asec", 1u, 1u, "Arcsecant", "Trigonometric function");
  addFunction(Function_ACsc, "math.acsc", 1u, 1u, "Arccosecant", "Trigonometric function");

  addFunction(Function_SinH, "math.sinh", 1u, 1u, "Hyperbolic sine", "Trigonometric function");
  addFunction(Function_CosH, "math.cosh", 1u, 1u, "Hyperbolic cosine", "Trigonometric function");
  addFunction(Function_TanH, "math.tanh", 1u, 1u, "Hyperbolic tangent", "Trigonometric function");
  addFunction(Function_CotH, "math.coth", 1u, 1u, "Hyperbolic cotangent", "Trigonometric function");
  addFunction(Function_SecH, "math.sech", 1u, 1u, "Hyperbolic secant", "Trigonometric function");
  addFunction(Function_CscH, "math.csch", 1u, 1u, "Hyperbolic cosecant", "Trigonometric function");

  addFunction(Function_ASinH, "math.asinh", 1u, 1u, "Hyperbolic arcsine", "Trigonometric function");
  addFunction(Function_ACosH, "math.acosh", 1u, 1u, "Hyperbolic arccosine", "Trigonometric function");
  addFunction(Function_ATanH, "math.atanh", 1u, 1u, "Hyperbolic arctangent", "Trigonometric function");
  addFunction(Function_ACotH, "math.acoth", 1u, 1u, "Hyperbolic arccotangent", "Trigonometric function");
  addFunction(Function_ASecH, "math.asech", 1u, 1u, "Hyperbolic arcsecant", "Trigonometric function");
  addFunction(Function_ACscH, "math.acsch", 1u, 1u, "Hyperbolic arccosecant", "Trigonometric function");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Min, "min", 1u, FunctionToken::GetArgumentCountMaxLimit(), "Min", "Returns the minimum of specified arguments");
  addFunction(Function_Max, "max", 1u, FunctionToken::GetArgumentCountMaxLimit(), "Max", "Returns the maximum of specified arguments");

  addFunction(Function_Mean, "math.mean", 1u, FunctionToken::GetArgumentCountMaxLimit(), "Mean", "Returns the mean of specified arguments");
  addFunction(Function_Median, "math.median", 1u, FunctionToken::GetArgumentCountMaxLimit(), "Median", "Returns the median of specified arguments");
  addFunction(Function_Mode, "math.mode", 1u, FunctionToken::GetArgumentCountMaxLimit(), "Mode", "Returns the mode of specified arguments");
  addFunction(Function_Quartile_Lower,
              "math.q1",
              1u,
              FunctionToken::GetArgumentCountMaxLimit(),
              "First quartile",
              "Returns the first quartile of specified arguments");
  addFunction(Function_Median,
              "math.q2",
              1u,
              FunctionToken::GetArgumentCountMaxLimit(),
              "Second quartile",
              "Returns the second quartile of specified arguments");
  addFunction(Function_Quartile_Upper,
              "math.q3",
              1u,
              FunctionToken::GetArgumentCountMaxLimit(),
              "Third quartile",
              "Returns the third quartile of specified arguments");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Str, "str", 1u, 1u, "Stringify", "Returns string representation of argument");
  addFunction(Function_StrLen, "strlen", 1u, 1u, "String length", "Returns length of string argument");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Date, "date", 0u, 1u, "Date", "Returns a date/time value that respresents the argument or the current date/time if empty");
  addFunction(Function_Dur, "dur", 1u, 1u, "Duration", "Returns a value representing a time duration specified by argument");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addFunction(Function_Random, "random", 0, 2, "Random", "Returns a random number between (0 and 1), (0 and x) or (x and y) depending of arguments specified");
  functionInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  InitChemicalExpressionParser(chemicalExpressionParser);
  addFunction(Function_MolarMass, "chem.M", 1u, 1u, "Molar mass", "Returns molar mass calculated from chemical compound string");

  addVariable(nullptr, "null", "Null", "Represents an undefined value type");
  addVariable(nullptr, "nil", "Nil", "Represents an undefined value type");
  addVariable(nullptr, "none", "None", "Represents an undefined value type");
  mpfr::mpreal tmpValue;
  addVariable(tmpValue.setNan(), "nan", "Not a number", "Represents an undefined numeric value");
  addVariable(mpfr::const_infinity(), "inf", "Infinity", "Represents infinity");
  addVariable(1, "true", "True", "Boolean value");
  addVariable(0, "false", "False", "Boolean value");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(mpfr::const_pi(), "math.pi", "Pi", "Mathematical constant");
  addVariable(mpfr::const_euler(), "math.E", "Euler-Mascheroni constant", "Mathematical constant");
  addVariable(mpfr::const_catalan(), "math.catalan", "Catalan's constant", "Mathematical constant");
  addVariable(mpfr::const_log2(), "math.ln2", "Logarithm of 2", "Mathematical constant");
  addVariable(mpfr::mpreal("2.71828182846"), "math.e", "Euler's number", "Mathematical constant");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(mpfr::exp10(mpfr::mpreal(24)), "Y", "Yotta", "Metric prefix (10^24)");
  addVariable(mpfr::exp10(mpfr::mpreal(21)), "Z", "Zetta", "Metric prefix (10^21)");
  addVariable(mpfr::exp10(mpfr::mpreal(18)), "E", "Exa", "Metric prefix (10^18)");
  addVariable(mpfr::exp10(mpfr::mpreal(15)), "P", "Peta", "Metric prefix (10^15)");
  addVariable(mpfr::exp10(mpfr::mpreal(12)), "T", "Tera", "Metric prefix (10^12)");
  addVariable(mpfr::exp10(mpfr::mpreal(9)), "G", "Giga", "Metric prefix (10^9)");
  addVariable(mpfr::exp10(mpfr::mpreal(6)), "M", "Mega", "Metric prefix (10^6)");
  addVariable(mpfr::exp10(mpfr::mpreal(3)), "k", "Kilo", "Metric prefix (10^3)");
  addVariable(mpfr::exp10(mpfr::mpreal(2)), "h", "Hecto", "Metric prefix (10^2)");
  addVariable(mpfr::exp10(mpfr::mpreal(1)), "da", "Deca", "Metric prefix (10^1)");
  addVariable(mpfr::exp10(mpfr::mpreal(-1)), "d", "Deci", "Metric prefix (10^-1)");
  addVariable(mpfr::exp10(mpfr::mpreal(-2)), "c", "Centi", "Metric prefix (10^-2)");
  addVariable(mpfr::exp10(mpfr::mpreal(-3)), "m", "Milli", "Metric prefix (10^-3)");
  addVariable(mpfr::exp10(mpfr::mpreal(-6)), "u", "Micro", "Metric prefix (10^-6)");
  addVariable(mpfr::exp10(mpfr::mpreal(-9)), "n", "Nano", "Metric prefix (10^-9)");
  addVariable(mpfr::exp10(mpfr::mpreal(-12)), "p", "Pico", "Metric prefix (10^-12)");
  addVariable(mpfr::exp10(mpfr::mpreal(-15)), "f", "Femto", "Metric prefix (10^-15)");
  addVariable(mpfr::exp10(mpfr::mpreal(-18)), "a", "Atto", "Metric prefix (10^-18)");
  addVariable(mpfr::exp10(mpfr::mpreal(-21)), "z", "Zepto", "Metric prefix (10^-21)");
  addVariable(mpfr::exp10(mpfr::mpreal(-24)), "y", "Yocto", "Metric prefix (10^-24)");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(mpfr::exp2(mpfr::mpreal(10)), "Ki", "Kibi", "Binary prefix (2^10)");
  addVariable(mpfr::exp2(mpfr::mpreal(20)), "Mi", "Mebi", "Binary prefix (2^20)");
  addVariable(mpfr::exp2(mpfr::mpreal(30)), "Gi", "Gibi", "Binary prefix (2^30)");
  addVariable(mpfr::exp2(mpfr::mpreal(40)), "Ti", "Tebi", "Binary prefix (2^40)");
  addVariable(mpfr::exp2(mpfr::mpreal(50)), "Pi", "Pebi", "Binary prefix (2^50)");
  addVariable(mpfr::exp2(mpfr::mpreal(60)), "Ei", "Exbi", "Binary prefix (2^60)");
  addVariable(mpfr::exp2(mpfr::mpreal(60)), "Zi", "Zebi", "Binary prefix (2^70)");
  addVariable(mpfr::exp2(mpfr::mpreal(60)), "Yi", "Yobi", "Binary prefix (2^80)");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(mpfr::exp10(mpfr::mpreal(-2)), "pc", "Percent", "Parts-per notation (10^-2)");
  addVariable(mpfr::exp10(mpfr::mpreal(-3)), "pm", "Permille", "Parts-per notation (10^-3)");
  addVariable(mpfr::exp10(mpfr::mpreal(-4)), "ptt", "Parts per ten thousand", "Parts-per notation (10^-4)");
  addVariable(mpfr::exp10(mpfr::mpreal(-6)), "ppm", "Parts per million", "Parts-per notation (10^-6)");
  addVariable(mpfr::exp10(mpfr::mpreal(-9)), "ppb", "Parts per billion", "Parts-per notation (10^-9)");
  addVariable(mpfr::exp10(mpfr::mpreal(-12)), "ppt", "Parts per trillion", "Parts-per notation (10^-12)");
  addVariable(mpfr::exp10(mpfr::mpreal(-15)), "ppq", "Parts per quadrillion", "Parts-per notation (10^-15)");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(boost::posix_time::hours(1l), "time.h", "Hour", "60 * 60 seconds");
  addVariable(boost::posix_time::minutes(1l), "time.m", "Minute", "60 seconds");
  addVariable(boost::posix_time::seconds(1l), "time.s", "Second", "1 second");
  addVariable(boost::posix_time::milliseconds(1l), "time.ms", "Millisecond", "10^-3 of a second");
  addVariable(boost::posix_time::microseconds(1l), "time.us", "Microsecond", "10^-6 of a second");
  addVariable(boost::posix_time::nanoseconds(1l), "time.ns", "Nanosecond", "10^-9 of a second");
  addVariable(boost::posix_time::hours(24l), "time.d", "Day", "24 hours");
  variableInfoMap.push_back(std::make_tuple(nullptr, "", ""));

  addVariable(mpfr::mpreal(CHAR_BIT), "bpB", "Bits per byte", "Common value for number of bits per byte");

  addVariable(mpfr::mpreal("-1000", mpfr::mpreal::get_default_prec(), 2), "i4.min", "Signed nibble min", "4 bit signed integer min. limit");
  addVariable(mpfr::mpreal("0111", mpfr::mpreal::get_default_prec(), 2), "i4.max", "Signed nibble max", "4 bit signed integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int8_t>::min()), "i8.min", "Signed byte min", "8 bit signed integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int8_t>::max()), "i8.max", "Signed byte max", "8 bit signed integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int16_t>::min()), "i16.min", "Signed short min", "16 bit signed integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int16_t>::max()), "i16.max", "Signed short max", "16 bit signed integer max. limit");
  addVariable(mpfr::mpreal("-100000000000000000000000", mpfr::mpreal::get_default_prec(), 2),
              "i24.min",
              "Signed 24-bit min",
              "24 bit signed integer min. limit");
  addVariable(mpfr::mpreal("011111111111111111111111", mpfr::mpreal::get_default_prec(), 2),
              "i24.max",
              "Signed 24-bit max",
              "24 bit signed integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int32_t>::min()), "i32.min", "Signed int min", "32 bit signed integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int32_t>::max()), "i32.max", "Signed int max", "32 bit signed integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int64_t>::min()), "i64.min", "Signed long min", "64 bit signed integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::int64_t>::max()), "i64.max", "Signed long max", "64 bit signed integer max. limit");
  addVariable(mpfr::mpreal("-10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
                           mpfr::mpreal::get_default_prec(),
                           2),
              "i128.min",
              "Signed long long min",
              "128 bit signed integer min. limit");
  addVariable(mpfr::mpreal("01111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
                           mpfr::mpreal::get_default_prec(),
                           2),
              "i128.max",
              "Signed long long max",
              "128 bit signed integer max. limit");

  addVariable(mpfr::mpreal("0000", mpfr::mpreal::get_default_prec(), 2), "u4.min", "Unsigned nibble min", "4 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal("1111", mpfr::mpreal::get_default_prec(), 2), "u4.max", "Unsigned nibble max", "4 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint8_t>::min()), "u8.min", "Unsigned byte min", "8 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint8_t>::max()), "u8.max", "Unsigned byte max", "8 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint16_t>::min()), "u16.min", "Unsigned short min", "16 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint16_t>::max()), "u16.max", "Unsigned short max", "16 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal("000000000000000000000000", mpfr::mpreal::get_default_prec(), 2),
              "u24.min",
              "Unsigned 24-bit min",
              "24 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal("111111111111111111111111", mpfr::mpreal::get_default_prec(), 2),
              "u24.max",
              "Unsigned 24-bit max",
              "24 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint32_t>::min()), "u32.min", "Unsigned int min", "32 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint32_t>::max()), "u32.max", "Unsigned int max", "32 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint64_t>::min()), "u64.min", "Unsigned long min", "64 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<std::uint64_t>::max()), "u64.max", "Unsigned long max", "64 bit unsigned integer max. limit");
  addVariable(mpfr::mpreal("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000",
                           mpfr::mpreal::get_default_prec(),
                           2),
              "u128.min",
              "Unsigned long long min",
              "128 bit unsigned integer min. limit");
  addVariable(mpfr::mpreal("11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
                           mpfr::mpreal::get_default_prec(),
                           2),
              "u128.max",
              "Unsigned long long max",
              "128 bit unsigned integer max. limit");

  addVariable(mpfr::mpreal(std::numeric_limits<float>::min()), "f32.min", "Float min", "32 bit floating point min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<float>::max()), "f32.max", "Float max", "32 bit floating point max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<float>::epsilon()), "f32.epsilon", "Float epsilon", "32 bit floating point epsilon");
  addVariable(mpfr::mpreal(std::numeric_limits<double>::min()), "f64.min", "Double min", "64 bit floating point min. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<double>::max()), "f64.max", "Double max", "64 bit floating point max. limit");
  addVariable(mpfr::mpreal(std::numeric_limits<double>::epsilon()), "f64.epsilon", "Double epsilon", "64 bit floating point epsilon");
}
