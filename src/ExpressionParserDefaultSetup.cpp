#include "ExpressionParserSetup.hpp"
#include <string>
#include <unordered_map>
#include <sstream>
#include <memory>
#include <boost/format.hpp>
#include <gmpxx.h>
#include <mpreal.h>

static ChemValueType* numberConverter(const std::string& value)
{
  std::istringstream iss(value);
  ChemArithmeticType result;
  iss >> result;
  return new ChemValueType(result);
}

struct GreaterComparer
{
  bool operator()(DefaultValueType* a, DefaultValueType* b) const { return a->GetValue<DefaultArithmeticType>() < b->GetValue<DefaultArithmeticType>(); }
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

int compare(const DefaultValueType* a, const DefaultValueType* b)
{
  if(a->GetType() == typeid(std::string) && b->GetType() == typeid(std::string))
  {
    return a->GetValue<std::string>().compare(b->GetValue<std::string>());
  }
  if(a->GetType() == typeid(boost::posix_time::ptime) && b->GetType() == typeid(boost::posix_time::ptime))
  {
    return a->GetValue<boost::posix_time::ptime>() < b->GetValue<boost::posix_time::ptime>() ?
             -1 :
             (a->GetValue<boost::posix_time::ptime>() > b->GetValue<boost::posix_time::ptime>() ? 1 : 0);
  }
  if(a->GetType() == typeid(boost::posix_time::time_duration) && b->GetType() == typeid(boost::posix_time::time_duration))
  {
    return a->GetValue<boost::posix_time::time_duration>() < b->GetValue<boost::posix_time::time_duration>() ?
             -1 :
             (a->GetValue<boost::posix_time::time_duration>() > b->GetValue<boost::posix_time::time_duration>() ? 1 : 0);
  }
  else if(a->GetType() == typeid(std::nullptr_t) && b->GetType() == typeid(std::nullptr_t))
  {
    return 0;
  }
  else
  {
    return a->GetValue<DefaultArithmeticType>() < b->GetValue<DefaultArithmeticType>() ?
             -1 :
             (a->GetValue<DefaultArithmeticType>() > b->GetValue<DefaultArithmeticType>() ? 1 : 0);
  }
}

static std::string makeCompoundString(std::string text)
{
  if(text.empty() || std::isdigit(text.front()) != 0)
  {
    throw SyntaxException("Invalid chemical compound string");
  }

  char lastChar = '\0';
  std::string result;
  for(const auto& i : text)
  {
    if(std::isalnum(i) == 0 && (i != '(' && i != ')'))
    {
      throw SyntaxException("Invalid characters in chemical compound string");
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

static void addUnaryOperator(const DefaultUnaryOperatorType::CallbackType& callback, char identifier, int precedence, Associativity associativity)
{
  auto tmpNew                           = std::make_unique<DefaultUnaryOperatorType>(callback, identifier, precedence, associativity);
  auto tmp                              = tmpNew.get();
  defaultUnaryOperatorCache[identifier] = std::move(tmpNew);
  defaultUnaryOperators[identifier]     = tmp;
}

static void
addBinaryOperator(const DefaultBinaryOperatorType::CallbackType& callback, const std::string& identifier, int precedence, Associativity associativity)
{
  auto tmpNew                            = std::make_unique<DefaultBinaryOperatorType>(callback, identifier, precedence, associativity);
  auto tmp                               = tmpNew.get();
  defaultBinaryOperatorCache[identifier] = std::move(tmpNew);
  defaultBinaryOperators[identifier]     = tmp;
}

static void addFunction(const DefaultFunctionType::CallbackType& callback,
                        const std::string& identifier,
                        std::size_t minArgs = 0u,
                        std::size_t maxArgs = DefaultFunctionType::GetArgumentCountMaxLimit())
{
  auto tmpNew                      = std::make_unique<DefaultFunctionType>(callback, identifier, minArgs, maxArgs);
  auto tmp                         = tmpNew.get();
  defaultFunctionCache[identifier] = std::move(tmpNew);
  defaultFunctions[identifier]     = tmp;
}

template<class T>
static void addVariable(const T& value, const std::string& identifier)
{
  auto tmpNew                                 = std::make_unique<DefaultVariableType>(identifier, value);
  auto tmp                                    = tmpNew.get();
  defaultInitializedVariableCache[identifier] = std::move(tmpNew);
  defaultVariables[identifier]                = tmp;
}

static void removeVariable(const std::string& identifier)
{
  defaultVariables.erase(identifier);
  if(defaultInitializedVariableCache.erase(identifier) == 0u)
  {
    defaultUninitializedVariableCache.erase(identifier);
  }
}

DefaultValueType* addNewVariable(const std::string& identifier)
{
  auto tmpNew                                   = std::make_unique<DefaultVariableType>(identifier);
  auto result                                   = tmpNew.get();
  defaultUninitializedVariableCache[identifier] = std::move(tmpNew);
  defaultVariables[identifier]                  = result;
  return result;
}

#ifndef __REGION__UNOPS
#ifndef __REGION__UNOPS__COMMON
static DefaultValueType* UnaryOperator_Plus(DefaultValueType* rhs) { return new DefaultValueType(mpfr::abs(rhs->GetValue<DefaultArithmeticType>())); }

static DefaultValueType* UnaryOperator_Minus(DefaultValueType* rhs) { return new DefaultValueType(-rhs->GetValue<DefaultArithmeticType>()); }
#endif // __REGION__UNOPS__COMMON

#ifndef __REGION__UNOPS__BITWISE
static DefaultValueType* UnaryOperator_Not(DefaultValueType* rhs) { return new DefaultValueType(!rhs->GetValue<DefaultArithmeticType>()); }

static DefaultValueType* UnaryOperator_OnesComplement(DefaultValueType* rhs)
{
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = ~tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}
#endif // __REGION__UNOPS__BITWISE
#endif // __REGION__UNOPS

#ifndef __REGION__BINOPS
#ifndef __REGION__BINOPS__COMPARISON
static DefaultValueType* BinaryOperator_Equals(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) == 0));
}

static DefaultValueType* BinaryOperator_NotEquals(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) != 0));
}

static DefaultValueType* BinaryOperator_Lesser(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) < 0));
}

static DefaultValueType* BinaryOperator_Greater(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) > 0));
}

static DefaultValueType* BinaryOperator_LesserOrEquals(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) <= 0));
}

static DefaultValueType* BinaryOperator_GreaterOrEquals(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(DefaultArithmeticType(compare(lhs, rhs) >= 0));
}

static DefaultValueType* BinaryOperator_LogicalOr(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() || rhs->GetValue<DefaultArithmeticType>());
}

static DefaultValueType* BinaryOperator_LogicalAnd(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() && rhs->GetValue<DefaultArithmeticType>());
}
#endif // __REGION__BINOPS__COMPARISON
#ifndef __REGION__BINOPS__COMMON
static DefaultValueType* BinaryOperator_Addition(DefaultValueType* lhs, DefaultValueType* rhs)
{
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
  else if(lhs->GetType() == typeid(boost::posix_time::time_duration) || rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    if(lhs->GetType() == typeid(boost::posix_time::ptime))
    {
      return new DefaultValueType(lhs->GetValue<boost::posix_time::ptime>() + rhs->GetValue<boost::posix_time::time_duration>());
    }
    else
    {
      return new DefaultValueType(lhs->GetValue<boost::posix_time::time_duration>() + rhs->GetValue<boost::posix_time::time_duration>());
    }
  }
  else
  {
    return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() + rhs->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* BinaryOperator_Subtraction(DefaultValueType* lhs, DefaultValueType* rhs)
{
  if(lhs->GetType() == typeid(boost::posix_time::time_duration) || rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    if(lhs->GetType() == typeid(boost::posix_time::ptime))
    {
      return new DefaultValueType(lhs->GetValue<boost::posix_time::ptime>() - rhs->GetValue<boost::posix_time::time_duration>());
    }
    else
    {
      return new DefaultValueType(lhs->GetValue<boost::posix_time::time_duration>() - rhs->GetValue<boost::posix_time::time_duration>());
    }
  }
  else
  {
    return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() - rhs->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* BinaryOperator_Multiplication(DefaultValueType* lhs, DefaultValueType* rhs)
{
  if(lhs->GetType() == typeid(std::string) && rhs->GetType() == typeid(DefaultArithmeticType))
  {
    return new DefaultValueType(lhs->GetValue<std::string>() * static_cast<std::size_t>(rhs->GetValue<DefaultArithmeticType>()));
  }
  else if(lhs->GetType() == typeid(boost::posix_time::time_duration) || rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    using nanoseconds = boost::date_time::subsecond_duration<boost::posix_time::time_duration, 1000000000l>;
    if(lhs->GetType() == typeid(DefaultArithmeticType))
    {
      auto ticks = nanoseconds(static_cast<long>(static_cast<double>(rhs->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) *
                                                 lhs->GetValue<DefaultArithmeticType>().toDouble()));
      return new DefaultValueType(boost::posix_time::time_duration(ticks));
    }
    else
    {
      auto ticks = nanoseconds(static_cast<long>(static_cast<double>(lhs->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) *
                                                 rhs->GetValue<DefaultArithmeticType>().toDouble()));
      return new DefaultValueType(boost::posix_time::time_duration(ticks));
    }
  }
  else
  {
    return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() * rhs->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* BinaryOperator_Division(DefaultValueType* lhs, DefaultValueType* rhs)
{
  if(lhs->GetType() == typeid(boost::posix_time::time_duration) || rhs->GetType() == typeid(DefaultArithmeticType))
  {
    using nanoseconds = boost::date_time::subsecond_duration<boost::posix_time::time_duration, 1000000000l>;
    auto ticks        = nanoseconds(static_cast<long>(static_cast<double>(lhs->GetValue<boost::posix_time::time_duration>().total_nanoseconds()) /
                                               rhs->GetValue<DefaultArithmeticType>().toDouble()));
    return new DefaultValueType(boost::posix_time::time_duration(ticks));
  }
  else
  {
    return new DefaultValueType(lhs->GetValue<DefaultArithmeticType>() * rhs->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* BinaryOperator_TruncatedDivision(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>() / rhs->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* BinaryOperator_Modulo(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(mpfr::fmod(lhs->GetValue<DefaultArithmeticType>(), rhs->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* BinaryOperator_Exponentiation(DefaultValueType* lhs, DefaultValueType* rhs)
{
  return new DefaultValueType(mpfr::pow(lhs->GetValue<DefaultArithmeticType>(), rhs->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__BINOPS__COMMON

#ifndef __REGION__BINOPS__BITWISE
static DefaultValueType* BinaryOperator_Or(DefaultValueType* lhs, DefaultValueType* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs | tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static DefaultValueType* BinaryOperator_And(DefaultValueType* lhs, DefaultValueType* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs & tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static DefaultValueType* BinaryOperator_Xor(DefaultValueType* lhs, DefaultValueType* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs ^ tmpRhs;
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static DefaultValueType* BinaryOperator_LeftShift(DefaultValueType* lhs, DefaultValueType* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs << rhs->GetValue<DefaultArithmeticType>().toULLong();
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}

static DefaultValueType* BinaryOperator_RightShift(DefaultValueType* lhs, DefaultValueType* rhs)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(lhs->GetValue<DefaultArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs >> rhs->GetValue<DefaultArithmeticType>().toULLong();
  return new DefaultValueType(DefaultArithmeticType(tmpResult.get_str()));
}
#endif // __REGION__BINOPS__BITWISE

#ifndef __REGION__BINOPS__SPECIAL
static DefaultValueType* BinaryOperator_VariableAssignment(DefaultValueType* lhs, DefaultValueType* rhs)
{
  DefaultVariableType* variable = lhs->AsPointer<DefaultVariableType>();
  if(variable == nullptr)
  {
    throw SyntaxException((boost::format("Assignment of non-variable type: %1% (%2%)") % lhs->ToString() % lhs->GetType().name()).str());
  }

  bool isInitialAssignment = !variable->IsInitialized();

  if(rhs->GetType() == typeid(DefaultArithmeticType))
  {
    (*variable) = rhs->GetValue<DefaultArithmeticType>();
  }
  else if(rhs->GetType() == typeid(boost::posix_time::ptime))
  {
    (*variable) = rhs->GetValue<boost::posix_time::ptime>();
  }
  else if(rhs->GetType() == typeid(boost::posix_time::time_duration))
  {
    (*variable) = rhs->GetValue<boost::posix_time::time_duration>();
  }
  else if(rhs->GetType() == typeid(std::string))
  {
    (*variable) = rhs->GetValue<std::string>();
  }
  else if(rhs->GetType() == typeid(std::nullptr_t))
  {
    (*variable) = rhs->GetValue<std::nullptr_t>();
  }
  else
  {
    throw SyntaxException((boost::format("Assignment from unsupported type: %1% (%2%)") % rhs->ToString() % rhs->GetType().name()).str());
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
static DefaultValueType* Function_Sgn(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType((args[0]->GetValue<DefaultArithmeticType>() > 0) - (args[0]->GetValue<DefaultArithmeticType>() < 0));
}

static DefaultValueType* Function_Abs(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::abs(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Neg(const std::vector<DefaultValueType*>& args) { return new DefaultValueType(-args[0]->GetValue<DefaultArithmeticType>()); }

static DefaultValueType* Function_Neg2(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(-mpfr::abs(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Mod(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::fmod(args[0]->GetValue<DefaultArithmeticType>(), args[1]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Rem(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::remainder(args[0]->GetValue<DefaultArithmeticType>(), args[1]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Pow(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->GetValue<DefaultArithmeticType>(), args[1]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Sqr(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->GetValue<DefaultArithmeticType>(), 2));
}

static DefaultValueType* Function_Cb(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->GetValue<DefaultArithmeticType>(), 3));
}

static DefaultValueType* Function_Root(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::pow(args[0]->GetValue<DefaultArithmeticType>(), 1 / args[1]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Sqrt(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::sqrt(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Cbrt(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::cbrt(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Exp(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::exp(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Exp2(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::exp2(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Exp10(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::pow(10, args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Log(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::log(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Log2(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::log2(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Log10(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::log10(args[0]->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__FUNCTIONS__COMMON

#ifndef __REGION__FUNCTIONS__TRIGONOMETRY
static DefaultValueType* Function_Sin(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::sin(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Cos(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::cos(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Tan(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::tan(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Cot(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::cot(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Sec(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::sec(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_Csc(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::csc(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ASin(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::asin(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACos(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acos(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ATan(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::atan(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ATan2(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::atan2(args[0]->GetValue<DefaultArithmeticType>(), args[1]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACot(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acot(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ASec(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::asec(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACsc(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acsc(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_SinH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::sinh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_CosH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::cosh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_TanH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::tanh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_CotH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::coth(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_SecH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::sech(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_CscH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::csch(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ASinH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::asinh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACosH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acosh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ATanH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::atanh(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACotH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acoth(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ASecH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::asech(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_ACscH(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::acsch(args[0]->GetValue<DefaultArithmeticType>()));
}
#endif // __REGION__FUNCTIONS__TRIGONOMETRY

#ifndef __REGION__FUNCTIONS__AGGREGATES
static DefaultValueType* Function_Min(const std::vector<DefaultValueType*>& args)
{
  auto result = std::numeric_limits<DefaultArithmeticType>::max();
  for(const auto& i : args)
  {
    auto tmpValue = i->GetValue<DefaultArithmeticType>();
    if(tmpValue < result)
    {
      result = tmpValue;
    }
  }

  return new DefaultValueType(result);
}
static DefaultValueType* Function_Max(const std::vector<DefaultValueType*>& args)
{
  auto result = std::numeric_limits<DefaultArithmeticType>::min();
  for(const auto& i : args)
  {
    auto tmpValue = i->GetValue<DefaultArithmeticType>();
    if(tmpValue > result)
    {
      result = tmpValue;
    }
  }

  return new DefaultValueType(result);
}

static DefaultValueType* Function_Mean(const std::vector<DefaultValueType*>& args)
{
  DefaultArithmeticType result = 0;
  for(const auto& i : args)
  {
    result += i->GetValue<DefaultArithmeticType>();
  }

  return new DefaultValueType(result / static_cast<DefaultArithmeticType>(args.size()));
}

static DefaultValueType* Function_Median(const std::vector<DefaultValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 2u;
  if(tmpArgs.size() % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[middle - 1u]->GetValue<DefaultArithmeticType>() + tmpArgs[middle]->GetValue<DefaultArithmeticType>()) / 2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[middle]->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* Function_Quartile_Lower(const std::vector<DefaultValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 4u;
  if(middle % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[middle - 1u]->GetValue<DefaultArithmeticType>() + tmpArgs[middle]->GetValue<DefaultArithmeticType>()) / 2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[middle]->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* Function_Quartile_Upper(const std::vector<DefaultValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle   = tmpArgs.size() / 2u;
  std::size_t q        = middle / 2u;
  std::size_t tmpIndex = (middle + (tmpArgs.size() % 2 == 0 ? 0 : 1)) + q;
  if(middle % 2 == 0)
  {
    return new DefaultValueType((tmpArgs[tmpIndex - 1u]->GetValue<DefaultArithmeticType>() + tmpArgs[tmpIndex]->GetValue<DefaultArithmeticType>()) / 2);
  }
  else
  {
    return new DefaultValueType(tmpArgs[tmpIndex]->GetValue<DefaultArithmeticType>());
  }
}

static DefaultValueType* Function_Mode(const std::vector<DefaultValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());

  auto current             = tmpArgs.front();
  std::size_t currentCount = 1u;
  auto mode                = current;
  std::size_t modeCount    = currentCount;

  for(std::size_t i = 1u; i < tmpArgs.size(); i++)
  {
    if(tmpArgs[i]->GetValue<DefaultArithmeticType>() == current->GetValue<DefaultArithmeticType>())
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

  return new DefaultValueType(mode->GetValue<DefaultArithmeticType>());
}
#endif // __REGION__FUNCTIONS__AGGREGATES

#ifndef __REGION__FUNCTIONS__STRING
static DefaultValueType* Function_Str(const std::vector<DefaultValueType*>& args) { return new DefaultValueType(args[0]->ToString()); }

static DefaultValueType* Function_StrLen(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(static_cast<DefaultArithmeticType>(args[0]->GetValue<std::string>().length()));
}
#endif // #ifndef __REGION__FUNCTIONS__STRING

#ifndef __REGION__FUNCTIONS__DATE_TIME
static DefaultValueType* Function_Date(const std::vector<DefaultValueType*>& args)
{
  if(args.size() == 0u)
  {
    return new DefaultValueType(boost::posix_time::second_clock::local_time());
  }
  else
  {
    return new DefaultValueType(boost::posix_time::time_from_string(args[0]->GetValue<std::string>()));
  }
}

static DefaultValueType* Function_Dur(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(boost::posix_time::duration_from_string(args[0]->GetValue<std::string>()));
}
#endif // __REGION__FUNCTIONS__DATE_TIME

#ifndef __REGION__FUNCTIONS__MISC
static DefaultValueType* Function_Random(const std::vector<DefaultValueType*>& args)
{
  if(args.size() == 0u)
  {
    return new DefaultValueType(mpfr::random());
  }
  else if(args.size() == 1u)
  {
    return new DefaultValueType(mpfr::random() * args[0]->GetValue<DefaultArithmeticType>());
  }
  else
  {
    const auto diff = args[1]->GetValue<DefaultArithmeticType>() - args[0]->GetValue<DefaultArithmeticType>();
    return new DefaultValueType(args[0]->GetValue<DefaultArithmeticType>() + (mpfr::random() * diff));
  }
}

static DefaultValueType* Function_Trunc(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::trunc(args[0]->GetValue<DefaultArithmeticType>()));
}

static DefaultValueType* Function_BConv(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(mpfr::mpreal(args[0]->GetValue<std::string>(),
                                           mpfr::mpreal::get_default_prec(),
                                           static_cast<int>(mpfr::trunc(args[1]->GetValue<DefaultArithmeticType>()).toLong())));
}
#endif // __REGION__FUNCTIONS__MISC

#ifndef __REGION__FUNCTIONS__SPECIAL
static DefaultValueType* Function_Ans(const std::vector<DefaultValueType*>& args)
{
  if(results.empty())
  {
    throw std::runtime_error("No results available");
  }

  if(args.empty())
  {
    return new DefaultValueType(results.back());
  }

  int index = static_cast<int>(args[0]->GetValue<DefaultArithmeticType>());
  if(index < 0)
  {
    index = static_cast<int>(results.size()) + index;
  }

  if(index < 0 || static_cast<std::size_t>(index) >= results.size())
  {
    throw SyntaxException((boost::format("Results index out of range: %1%/%2%") % index % results.size()).str());
  }

  return new DefaultValueType(results.at(static_cast<std::size_t>(index)));
}

static DefaultValueType* Function_Del(const std::vector<DefaultValueType*>& args)
{
  const std::string identifier = args[0]->GetValue<std::string>();
  auto iter                    = defaultInitializedVariableCache.find(identifier);
  if(iter == defaultInitializedVariableCache.end())
  {
    iter = defaultUninitializedVariableCache.find(identifier);
    if(iter == defaultUninitializedVariableCache.end())
    {
      throw SyntaxException("Deletion of nonexistent variable: " + identifier);
    }
  }

  auto result = new DefaultValueType(*dynamic_cast<DefaultValueType*>(iter->second.get()));
  removeVariable(identifier);
  return result;
}
#endif // __REGION__FUNCTIONS__SPECIAL

static ExpressionParser<ChemArithmeticType> chemicalExpressionParser(numberConverter);

static DefaultValueType* Function_MolarMass(const std::vector<DefaultValueType*>& args)
{
  return new DefaultValueType(chemicalExpressionParser.Evaluate(makeCompoundString(args[0]->GetValue<std::string>())).GetValue<DefaultArithmeticType>());
}
#endif // __REGION__FUNCTIONS

void InitDefault(ExpressionParser<DefaultArithmeticType, boost::posix_time::ptime, boost::posix_time::time_duration>& instance)
{
  instance.SetUnaryOperators(&defaultUnaryOperators);
  instance.SetBinaryOperators(&defaultBinaryOperators);
  instance.SetFunctions(&defaultFunctions);
  instance.SetVariables(&defaultVariables);
  instance.SetOnUnknownIdentifierCallback(addNewVariable);
  instance.SetJuxtapositionOperator(BinaryOperator_Multiplication, 2, Associativity::Right);

  addUnaryOperator(UnaryOperator_Not, '!', 7, Associativity::Right);
  addUnaryOperator(UnaryOperator_Plus, '+', 7, Associativity::Right);
  addUnaryOperator(UnaryOperator_Minus, '-', 7, Associativity::Right);
  addUnaryOperator(UnaryOperator_OnesComplement, '~', 7, Associativity::Right);

  addBinaryOperator(BinaryOperator_Equals, "==", 3, Associativity::Left);
  addBinaryOperator(BinaryOperator_NotEquals, "!=", 3, Associativity::Left);
  addBinaryOperator(BinaryOperator_Lesser, "<", 3, Associativity::Left);
  addBinaryOperator(BinaryOperator_Greater, ">", 3, Associativity::Left);
  addBinaryOperator(BinaryOperator_LesserOrEquals, "<=", 3, Associativity::Left);
  addBinaryOperator(BinaryOperator_GreaterOrEquals, ">=", 3, Associativity::Left);

  addBinaryOperator(BinaryOperator_LogicalOr, "||", 1, Associativity::Left);
  addBinaryOperator(BinaryOperator_LogicalAnd, "&&", 1, Associativity::Left);

  addBinaryOperator(BinaryOperator_Addition, "+", 4, Associativity::Left);
  addBinaryOperator(BinaryOperator_Subtraction, "-", 4, Associativity::Left);
  addBinaryOperator(BinaryOperator_Multiplication, "*", 5, Associativity::Left);
  addBinaryOperator(BinaryOperator_Division, "/", 5, Associativity::Left);
  addBinaryOperator(BinaryOperator_TruncatedDivision, "//", 5, Associativity::Left);
  addBinaryOperator(BinaryOperator_Modulo, "%", 5, Associativity::Left);
  addBinaryOperator(BinaryOperator_Exponentiation, "**", 6, Associativity::Right);

  addBinaryOperator(BinaryOperator_Or, "|", 2, Associativity::Left);
  addBinaryOperator(BinaryOperator_And, "&", 2, Associativity::Left);
  addBinaryOperator(BinaryOperator_Xor, "^", 2, Associativity::Left);
  addBinaryOperator(BinaryOperator_LeftShift, "<<", 2, Associativity::Left);
  addBinaryOperator(BinaryOperator_RightShift, ">>", 2, Associativity::Left);

  addBinaryOperator(BinaryOperator_VariableAssignment, "=", 7, Associativity::Right);

  addFunction(Function_Ans, "ans", 0u, 1u);
  addFunction(Function_Del, "del", 1u, 1u);
  addFunction(Function_BConv, "bconv", 2u, 2u);

  addFunction(Function_Random, "random", 0, 2);

  addFunction(Function_Trunc, "trunc", 1u, 1u);
  addFunction(Function_Sgn, "sgn", 1u, 1u);
  addFunction(Function_Abs, "abs", 1u, 1u);
  addFunction(Function_Neg, "neg", 1u, 1u);
  addFunction(Function_Neg2, "neg2", 1u, 1u);

  addFunction(Function_Mod, "math.mod", 2u, 2u);
  addFunction(Function_Rem, "math.rem", 2u, 2u);
  addFunction(Function_Pow, "math.pow", 2u, 2u);
  addFunction(Function_Sqr, "math.sqr", 1u, 1u);
  addFunction(Function_Cb, "math.cb", 1u, 1u);
  addFunction(Function_Root, "math.root", 2u, 2u);
  addFunction(Function_Sqrt, "math.sqrt", 1u, 1u);
  addFunction(Function_Cbrt, "math.cbrt", 1u, 1u);

  addFunction(Function_Exp, "math.exp", 1u, 1u);
  addFunction(Function_Exp2, "math.exp2", 1u, 1u);
  addFunction(Function_Exp10, "math.exp10", 1u, 1u);
  addFunction(Function_Log, "math.log", 1u, 1u);
  addFunction(Function_Log2, "math.log2", 1u, 1u);
  addFunction(Function_Log10, "math.log10", 1u, 1u);

  addFunction(Function_Sin, "math.sin", 1u, 1u);
  addFunction(Function_Cos, "math.cos", 1u, 1u);
  addFunction(Function_Tan, "math.tan", 1u, 1u);
  addFunction(Function_Cot, "math.cot", 1u, 1u);
  addFunction(Function_Sec, "math.sec", 1u, 1u);
  addFunction(Function_Csc, "math.csc", 1u, 1u);

  addFunction(Function_ASin, "math.asin", 1u, 1u);
  addFunction(Function_ACos, "math.acos", 1u, 1u);
  addFunction(Function_ATan, "math.atan", 1u, 1u);
  addFunction(Function_ATan2, "math.atan2", 2u, 2u);
  addFunction(Function_ACot, "math.acot", 1u, 1u);
  addFunction(Function_ASec, "math.asec", 1u, 1u);
  addFunction(Function_ACsc, "math.acsc", 1u, 1u);

  addFunction(Function_SinH, "math.sinh", 1u, 1u);
  addFunction(Function_CosH, "math.cosh", 1u, 1u);
  addFunction(Function_TanH, "math.tanh", 1u, 1u);
  addFunction(Function_CotH, "math.coth", 1u, 1u);
  addFunction(Function_SecH, "math.sech", 1u, 1u);
  addFunction(Function_CscH, "math.csch", 1u, 1u);

  addFunction(Function_ASinH, "math.asinh", 1u, 1u);
  addFunction(Function_ACosH, "math.acosh", 1u, 1u);
  addFunction(Function_ATanH, "math.atanh", 1u, 1u);
  addFunction(Function_ACotH, "math.acoth", 1u, 1u);
  addFunction(Function_ASecH, "math.asech", 1u, 1u);
  addFunction(Function_ACscH, "math.acsch", 1u, 1u);

  addFunction(Function_Min, "min", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Max, "max", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());

  addFunction(Function_Mean, "math.mean", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Median, "math.median", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Mode, "math.mode", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Quartile_Lower, "math.q1", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Median, "math.q2", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());
  addFunction(Function_Quartile_Upper, "math.q3", 1u, DefaultFunctionType::GetArgumentCountMaxLimit());

  addFunction(Function_Str, "str", 1u, 1u);
  addFunction(Function_StrLen, "strlen", 1u, 1u);

  addFunction(Function_Date, "date", 0u, 1u);
  addFunction(Function_Dur, "dur", 1u, 1u);

  InitChemical(chemicalExpressionParser);
  addFunction(Function_MolarMass, "chem.M", 1u, 1u);

  addVariable(nullptr, "null");

  addVariable(mpfr::const_infinity(), "inf");
  mpfr::mpreal tmpValue;
  addVariable(tmpValue.setNan(), "nan");

  addVariable(mpfr::mpreal("1000000000000000000000000"), "Y");
  addVariable(mpfr::mpreal("1000000000000000000000"), "Z");
  addVariable(mpfr::mpreal("1000000000000000000"), "E");
  addVariable(mpfr::mpreal("1000000000000000"), "P");
  addVariable(mpfr::mpreal("1000000000000"), "T");
  addVariable(mpfr::mpreal("1000000000"), "G");
  addVariable(mpfr::mpreal("1000000"), "M");
  addVariable(mpfr::mpreal("1000"), "k");
  addVariable(mpfr::mpreal("100"), "h");
  addVariable(mpfr::mpreal("10"), "da");
  addVariable(mpfr::mpreal("0.1"), "d");
  addVariable(mpfr::mpreal("0.01"), "c");
  addVariable(mpfr::mpreal("0.001"), "m");
  addVariable(mpfr::mpreal("0.000001"), "u");
  addVariable(mpfr::mpreal("0.000000001"), "n");
  addVariable(mpfr::mpreal("0.000000000001"), "p");
  addVariable(mpfr::mpreal("0.000000000000001"), "f");
  addVariable(mpfr::mpreal("0.000000000000000001"), "a");
  addVariable(mpfr::mpreal("0.000000000000000000001"), "z");
  addVariable(mpfr::mpreal("0.000000000000000000000001"), "y");

  addVariable(mpfr::mpreal("0.01"), "pc");
  addVariable(mpfr::mpreal("0.001"), "pm");
  addVariable(mpfr::mpreal("0.0001"), "ptt");
  addVariable(mpfr::mpreal("0.000001"), "ppm");
  addVariable(mpfr::mpreal("0.000000001"), "ppb");
  addVariable(mpfr::mpreal("0.000000000001"), "ppt");
  addVariable(mpfr::mpreal("0.000000000000001"), "ppq");

  addVariable(mpfr::const_pi(), "math.pi");
  addVariable(mpfr::const_euler(), "math.E");
  addVariable(mpfr::const_catalan(), "math.catalan");

  addVariable(mpfr::mpreal("2.71828182846"), "math.e");

  addVariable(mpfr::mpreal("602214085700000000000000"), "phys.N");
  addVariable(mpfr::mpreal("299792458"), "phys.c");
  addVariable(mpfr::mpreal("149597870700"), "phys.au");
  addVariable(mpfr::mpreal("86400"), "phys.D");
  addVariable(mpfr::mpreal("1988920000000000000000000000000"), "phys.M");
  addVariable(mpfr::mpreal("9460730472580800"), "phys.ly");
  addVariable(mpfr::mpreal("30856775814913700"), "phys.pc");
  addVariable(mpfr::mpreal("0.00000000006674"), "phys.G");
  addVariable(mpfr::mpreal("9.80665"), "phys.g");
  addVariable(mpfr::mpreal("8.3144626181532"), "phys.R");

  addVariable(mpfr::mpreal("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"), "googol");
}
