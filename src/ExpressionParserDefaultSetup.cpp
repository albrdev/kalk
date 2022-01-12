#include "ExpressionParserSetup.hpp"
#include <string>
#include <boost/format.hpp>
#include <gmpxx.h>
#include <mpreal.h>

struct GreaterComparer
{
  bool operator()(KalkValueType* a, KalkValueType* b) const { return a->GetValue<KalkArithmeticType>() < b->GetValue<KalkArithmeticType>(); }
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

static KalkValueType* UnaryOperator_Not(KalkValueType* rhs) { return new KalkValueType(!rhs->GetValue<KalkArithmeticType>()); }
static KalkValueType* UnaryOperator_Plus(KalkValueType* rhs) { return new KalkValueType(mpfr::abs(rhs->GetValue<KalkArithmeticType>())); }
static KalkValueType* UnaryOperator_Minus(KalkValueType* rhs) { return new KalkValueType(-rhs->GetValue<KalkArithmeticType>()); }
static KalkValueType* UnaryOperator_OnesComplement(KalkValueType* rhs)
{
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(rhs->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = ~tmpRhs;
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}

static KalkValueType* BinaryOperator_Addition(KalkValueType* lhs, KalkValueType* rhs)
{
  if(lhs->GetType() == typeid(KalkArithmeticType) && rhs->GetType() == typeid(KalkArithmeticType))
  {
    return new KalkValueType(lhs->GetValue<KalkArithmeticType>() + rhs->GetValue<KalkArithmeticType>());
  }
  else
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

    return new KalkValueType(tmpString);
  }
}
static KalkValueType* BinaryOperator_Subtraction(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(lhs->GetValue<KalkArithmeticType>() - rhs->GetValue<KalkArithmeticType>());
}
static KalkValueType* BinaryOperator_Multiplication(KalkValueType* lhs, KalkValueType* rhs)
{
  if(lhs->GetType() == typeid(std::string) && rhs->GetType() == typeid(KalkArithmeticType))
  {
    return new KalkValueType(lhs->GetValue<std::string>() * static_cast<std::size_t>(rhs->GetValue<KalkArithmeticType>()));
  }
  else
  {
    return new KalkValueType(lhs->GetValue<KalkArithmeticType>() * rhs->GetValue<KalkArithmeticType>());
  }
}
static KalkValueType* BinaryOperator_Division(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(lhs->GetValue<KalkArithmeticType>() / rhs->GetValue<KalkArithmeticType>());
}
static KalkValueType* BinaryOperator_TruncatedDivision(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(mpfr::trunc(lhs->GetValue<KalkArithmeticType>() / rhs->GetValue<KalkArithmeticType>()));
}
static KalkValueType* BinaryOperator_Modulo(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(mpfr::fmod(lhs->GetValue<KalkArithmeticType>(), rhs->GetValue<KalkArithmeticType>()));
}
static KalkValueType* BinaryOperator_Exponentiation(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(mpfr::pow(lhs->GetValue<KalkArithmeticType>(), rhs->GetValue<KalkArithmeticType>()));
}
static KalkValueType* BinaryOperator_Exponentiation2(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(mpfr::pow(lhs->GetValue<KalkArithmeticType>(), rhs->GetValue<KalkArithmeticType>()));
}

static KalkValueType* BinaryOperator_VariableAssignment(KalkValueType* lhs, KalkValueType* rhs)
{
  KalkVariableType* variable = dynamic_cast<KalkVariableType*>(lhs);
  if(variable == nullptr)
  {
    throw SyntaxException((boost::format("Assignment of non-variable type: %1% (%2%)") % lhs->ToString() % lhs->GetType().name()).str());
  }

  if(rhs->GetType() == typeid(KalkArithmeticType))
  {
    (*variable) = rhs->GetValue<KalkArithmeticType>();
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

  return variable;
}

static KalkValueType* Function_ToStr(const std::vector<KalkValueType*>& args) { return new KalkValueType(args[0]->ToString()); }

static KalkValueType* Function_StrLen(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(static_cast<KalkArithmeticType>(args[0]->GetValue<std::string>().length()));
}

static KalkValueType* Function_Random(const std::vector<KalkValueType*>& args)
{
  static_cast<void>(args);
  return new KalkValueType(static_cast<KalkArithmeticType>(std::rand()));
}

static KalkValueType* Function_Trunc(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_Sgn(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType((args[0]->GetValue<KalkArithmeticType>() > 0) - (args[0]->GetValue<KalkArithmeticType>() < 0));
}
static KalkValueType* Function_Abs(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::abs(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Neg(const std::vector<KalkValueType*>& args) { return new KalkValueType(-args[0]->GetValue<KalkArithmeticType>()); }
static KalkValueType* Function_Neg2(const std::vector<KalkValueType*>& args) { return new KalkValueType(-mpfr::abs(args[0]->GetValue<KalkArithmeticType>())); }

static KalkValueType* Function_Pow(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::pow(args[0]->GetValue<KalkArithmeticType>(), args[1]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_Sqr(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::pow(args[0]->GetValue<KalkArithmeticType>(), 2)); }
static KalkValueType* Function_Cb(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::pow(args[0]->GetValue<KalkArithmeticType>(), 3)); }
static KalkValueType* Function_Root(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::pow(args[0]->GetValue<KalkArithmeticType>(), 1 / args[1]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_Sqrt(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::sqrt(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Cbrt(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::cbrt(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Exp(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::exp(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Exp2(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::exp2(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Exp10(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::pow(10, args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_Log(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::log(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Log2(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::log2(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Log10(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::log10(args[0]->GetValue<KalkArithmeticType>()));
}

static KalkValueType* Function_Sin(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::sin(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Cos(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::cos(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Tan(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::tan(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Cot(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::cot(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Sec(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::sec(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_Csc(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::csc(args[0]->GetValue<KalkArithmeticType>())); }

static KalkValueType* Function_ASin(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::asin(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_ACos(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::acos(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_ATan(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::atan(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_ATan2(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::atan2(args[0]->GetValue<KalkArithmeticType>(), args[1]->GetValue<KalkArithmeticType>()));
} //*
static KalkValueType* Function_ACot(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::acot(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_ASec(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::asec(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_ACsc(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::acsc(args[0]->GetValue<KalkArithmeticType>())); }

static KalkValueType* Function_SinH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::sinh(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_CosH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::cosh(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_TanH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::tanh(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_CotH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::coth(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_SecH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::sech(args[0]->GetValue<KalkArithmeticType>())); }
static KalkValueType* Function_CscH(const std::vector<KalkValueType*>& args) { return new KalkValueType(mpfr::csch(args[0]->GetValue<KalkArithmeticType>())); }

static KalkValueType* Function_ASinH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::asinh(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_ACosH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::acosh(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_ATanH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::atanh(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_ACotH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::acoth(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_ASecH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::asech(args[0]->GetValue<KalkArithmeticType>()));
}
static KalkValueType* Function_ACscH(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::acsch(args[0]->GetValue<KalkArithmeticType>()));
}

static KalkValueType* Function_Or(const std::vector<KalkValueType*>& args)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(args[1]->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs | tmpRhs;
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}

static KalkValueType* Function_And(const std::vector<KalkValueType*>& args)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(args[1]->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs & tmpRhs;
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}
static KalkValueType* Function_Xor(const std::vector<KalkValueType*>& args)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()).toString(), 10);
  mpz_class tmpRhs;
  tmpRhs.set_str(mpfr::trunc(args[1]->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs ^ tmpRhs;
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}
static KalkValueType* Function_LShift(const std::vector<KalkValueType*>& args)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs << args[1]->GetValue<KalkArithmeticType>().toULLong();
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}
static KalkValueType* Function_RShift(const std::vector<KalkValueType*>& args)
{
  mpz_class tmpLhs;
  tmpLhs.set_str(mpfr::trunc(args[0]->GetValue<KalkArithmeticType>()).toString(), 10);

  mpz_class tmpResult = tmpLhs >> args[1]->GetValue<KalkArithmeticType>().toULLong();
  return new KalkValueType(mpfr::mpreal(tmpResult.get_str()));
}

static KalkValueType* Function_Min(const std::vector<KalkValueType*>& args)
{
  auto result = std::numeric_limits<KalkArithmeticType>::max();
  for(const auto& i : args)
  {
    auto tmpValue = i->GetValue<KalkArithmeticType>();
    if(tmpValue < result)
    {
      result = tmpValue;
    }
  }

  return new KalkValueType(result);
}
static KalkValueType* Function_Max(const std::vector<KalkValueType*>& args)
{
  auto result = std::numeric_limits<KalkArithmeticType>::min();
  for(const auto& i : args)
  {
    auto tmpValue = i->GetValue<KalkArithmeticType>();
    if(tmpValue > result)
    {
      result = tmpValue;
    }
  }

  return new KalkValueType(result);
}

static KalkValueType* Function_Mean(const std::vector<KalkValueType*>& args)
{
  KalkArithmeticType result = 0;
  for(const auto& i : args)
  {
    result += i->GetValue<KalkArithmeticType>();
  }

  return new KalkValueType(result / static_cast<KalkArithmeticType>(args.size()));
}

static KalkValueType* Function_Median(const std::vector<KalkValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 2u;
  if(tmpArgs.size() % 2 == 0)
  {
    return new KalkValueType((tmpArgs[middle - 1u]->GetValue<KalkArithmeticType>() + tmpArgs[middle]->GetValue<KalkArithmeticType>()) / 2);
  }
  else
  {
    return new KalkValueType(tmpArgs[middle]->GetValue<KalkArithmeticType>());
  }
}

static KalkValueType* Function_Quartile_Lower(const std::vector<KalkValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle = tmpArgs.size() / 4u;
  if(middle % 2 == 0)
  {
    return new KalkValueType((tmpArgs[middle - 1u]->GetValue<KalkArithmeticType>() + tmpArgs[middle]->GetValue<KalkArithmeticType>()) / 2);
  }
  else
  {
    return new KalkValueType(tmpArgs[middle]->GetValue<KalkArithmeticType>());
  }
}

static KalkValueType* Function_Quartile_Upper(const std::vector<KalkValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());
  std::size_t middle   = tmpArgs.size() / 2u;
  std::size_t q        = middle / 2u;
  std::size_t tmpIndex = (middle + (tmpArgs.size() % 2 == 0 ? 0 : 1)) + q;
  if(middle % 2 == 0)
  {
    return new KalkValueType((tmpArgs[tmpIndex - 1u]->GetValue<KalkArithmeticType>() + tmpArgs[tmpIndex]->GetValue<KalkArithmeticType>()) / 2);
  }
  else
  {
    return new KalkValueType(tmpArgs[tmpIndex]->GetValue<KalkArithmeticType>());
  }
}

static KalkValueType* Function_Mode(const std::vector<KalkValueType*>& args)
{
  auto tmpArgs = args;
  std::sort(tmpArgs.begin(), tmpArgs.end(), GreaterComparer());

  auto current             = tmpArgs.front();
  std::size_t currentCount = 1u;
  auto mode                = current;
  std::size_t modeCount    = currentCount;

  for(std::size_t i = 1u; i < tmpArgs.size(); i++)
  {
    if(tmpArgs[i]->GetValue<KalkArithmeticType>() == current->GetValue<KalkArithmeticType>())
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

  return new KalkValueType(mode->GetValue<KalkArithmeticType>());
}

static KalkValueType* Function_Ans(const std::vector<KalkValueType*>& args)
{
  if(results.empty())
  {
    throw std::runtime_error("Accessing empty results container");
  }

  if(args.empty())
  {
    return new KalkValueType(results.back());
  }

  int index = static_cast<int>(args[0]->GetValue<KalkArithmeticType>());
  if(index < 0)
  {
    index = static_cast<int>(results.size()) + index;
  }

  if(index < 0 || static_cast<std::size_t>(index) >= results.size())
  {
    throw SyntaxException((boost::format("Results index out of range: %1%/%2%") % index % results.size()).str());
  }

  return new KalkValueType(results.at(static_cast<std::size_t>(index)));
}

static KalkValueType* Function_BConv(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(mpfr::mpreal(args[0]->GetValue<std::string>(),
                                        mpfr::mpreal::get_default_prec(),
                                        static_cast<int>(mpfr::trunc(args[1]->GetValue<KalkArithmeticType>()).toLong())));
}

static ExpressionParser<KalkArithmeticType> chemicalExpressionParser;

static KalkValueType* Function_MolarMass(const std::vector<KalkValueType*>& args)
{
  return new KalkValueType(chemicalExpressionParser.Evaluate(makeCompoundString(args[0]->GetValue<std::string>())));
}

void InitDefault(ExpressionParser<KalkArithmeticType>& instance)
{
  instance.AddUnaryOperator(UnaryOperator_Not, '!', 4, Associativity::Right);
  instance.AddUnaryOperator(UnaryOperator_Plus, '+', 4, Associativity::Right);
  instance.AddUnaryOperator(UnaryOperator_Minus, '-', 4, Associativity::Right);
  instance.AddUnaryOperator(UnaryOperator_OnesComplement, '~', 4, Associativity::Right);

  instance.AddBinaryOperator(BinaryOperator_Addition, "+", 1, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Subtraction, "-", 1, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Multiplication, "*", 2, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Division, "/", 2, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_TruncatedDivision, "//", 2, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Modulo, "%", 2, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Exponentiation, "^", 3, Associativity::Right);
  instance.AddBinaryOperator(BinaryOperator_Exponentiation2, "**", 3, Associativity::Right);
  instance.AddBinaryOperator(BinaryOperator_VariableAssignment, "=", 4, Associativity::Right);

  instance.SetJuxtapositionOperator(BinaryOperator_Multiplication, 2, Associativity::Right);

  instance.AddFunction(Function_Ans, "ans", 0, 1);
  instance.AddFunction(Function_BConv, "bconv", 2u, 2u);

  instance.AddFunction(Function_Random, "random", 0, 0);

  instance.AddFunction(Function_Trunc, "trunc", 1u, 1u);
  instance.AddFunction(Function_Sgn, "sgn", 1u, 1u);
  instance.AddFunction(Function_Abs, "abs", 1u, 1u);
  instance.AddFunction(Function_Neg, "neg", 1u, 1u);
  instance.AddFunction(Function_Neg2, "neg2", 1u, 1u);

  instance.AddFunction(Function_Pow, "math.pow", 2u, 2u);
  instance.AddFunction(Function_Sqr, "math.sqr", 1u, 1u);
  instance.AddFunction(Function_Cb, "math.cb", 1u, 1u);
  instance.AddFunction(Function_Root, "math.root", 2u, 2u);
  instance.AddFunction(Function_Sqrt, "math.sqrt", 1u, 1u);
  instance.AddFunction(Function_Cbrt, "math.cbrt", 1u, 1u);

  instance.AddFunction(Function_Exp, "math.exp", 1u, 1u);
  instance.AddFunction(Function_Exp2, "math.exp2", 1u, 1u);
  instance.AddFunction(Function_Exp10, "math.exp10", 1u, 1u);
  instance.AddFunction(Function_Log, "math.log", 1u, 1u);
  instance.AddFunction(Function_Log2, "math.log2", 1u, 1u);
  instance.AddFunction(Function_Log10, "math.log10", 1u, 1u);

  instance.AddFunction(Function_Sin, "math.sin", 1u, 1u);
  instance.AddFunction(Function_Cos, "math.cos", 1u, 1u);
  instance.AddFunction(Function_Tan, "math.tan", 1u, 1u);
  instance.AddFunction(Function_Cot, "math.cot", 1u, 1u);
  instance.AddFunction(Function_Sec, "math.sec", 1u, 1u);
  instance.AddFunction(Function_Csc, "math.csc", 1u, 1u);

  instance.AddFunction(Function_ASin, "math.asin", 1u, 1u);
  instance.AddFunction(Function_ACos, "math.acos", 1u, 1u);
  instance.AddFunction(Function_ATan, "math.atan", 1u, 1u);
  instance.AddFunction(Function_ATan2, "math.atan2", 2u, 2u);
  instance.AddFunction(Function_ACot, "math.acot", 1u, 1u);
  instance.AddFunction(Function_ASec, "math.asec", 1u, 1u);
  instance.AddFunction(Function_ACsc, "math.acsc", 1u, 1u);

  instance.AddFunction(Function_SinH, "math.sinh", 1u, 1u);
  instance.AddFunction(Function_CosH, "math.cosh", 1u, 1u);
  instance.AddFunction(Function_TanH, "math.tanh", 1u, 1u);
  instance.AddFunction(Function_CotH, "math.coth", 1u, 1u);
  instance.AddFunction(Function_SecH, "math.sech", 1u, 1u);
  instance.AddFunction(Function_CscH, "math.csch", 1u, 1u);

  instance.AddFunction(Function_ASinH, "math.asinh", 1u, 1u);
  instance.AddFunction(Function_ACosH, "math.acosh", 1u, 1u);
  instance.AddFunction(Function_ATanH, "math.atanh", 1u, 1u);
  instance.AddFunction(Function_ACotH, "math.acoth", 1u, 1u);
  instance.AddFunction(Function_ASecH, "math.asech", 1u, 1u);
  instance.AddFunction(Function_ACscH, "math.acsch", 1u, 1u);

  instance.AddFunction(Function_Or, "or", 2u, 2u);
  instance.AddFunction(Function_And, "and", 2u, 2u);
  instance.AddFunction(Function_Xor, "xor", 2u, 2u);
  instance.AddFunction(Function_LShift, "lshift", 2u, 2u);
  instance.AddFunction(Function_RShift, "rshift", 2u, 2u);

  instance.AddFunction(Function_Min, "min", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Max, "max", 1u, KalkFunctionType::GetArgumentCountMaxLimit());

  instance.AddFunction(Function_Mean, "math.mean", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Median, "math.median", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Mode, "math.mode", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Quartile_Lower, "math.q1", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Median, "math.q2", 1u, KalkFunctionType::GetArgumentCountMaxLimit());
  instance.AddFunction(Function_Quartile_Upper, "math.q3", 1u, KalkFunctionType::GetArgumentCountMaxLimit());

  instance.AddFunction(Function_ToStr, "tostr", 1u, 1u);
  instance.AddFunction(Function_StrLen, "strlen", 1u, 1u);

  InitChemical(chemicalExpressionParser);
  instance.AddFunction(Function_MolarMass, "chem.M", 1u, 1u);

  instance.AddConstant(nullptr, "null");

  instance.AddConstant(mpfr::const_infinity(), "inf");
  mpfr::mpreal tmpValue;
  instance.AddConstant(tmpValue.setNan(), "nan");

  instance.AddConstant(mpfr::mpreal("1000000000000000000000000"), "Y");
  instance.AddConstant(mpfr::mpreal("1000000000000000000000"), "Z");
  instance.AddConstant(mpfr::mpreal("1000000000000000000"), "E");
  instance.AddConstant(mpfr::mpreal("1000000000000000"), "P");
  instance.AddConstant(mpfr::mpreal("1000000000000"), "T");
  instance.AddConstant(mpfr::mpreal("1000000000"), "G");
  instance.AddConstant(mpfr::mpreal("1000000"), "M");
  instance.AddConstant(mpfr::mpreal("1000"), "k");
  instance.AddConstant(mpfr::mpreal("100"), "h");
  instance.AddConstant(mpfr::mpreal("10"), "da");
  instance.AddConstant(mpfr::mpreal("0.1"), "d");
  instance.AddConstant(mpfr::mpreal("0.01"), "c");
  instance.AddConstant(mpfr::mpreal("0.001"), "m");
  instance.AddConstant(mpfr::mpreal("0.000001"), "u");
  instance.AddConstant(mpfr::mpreal("0.000000001"), "n");
  instance.AddConstant(mpfr::mpreal("0.000000000001"), "p");
  instance.AddConstant(mpfr::mpreal("0.000000000000001"), "f");
  instance.AddConstant(mpfr::mpreal("0.000000000000000001"), "a");
  instance.AddConstant(mpfr::mpreal("0.000000000000000000001"), "z");
  instance.AddConstant(mpfr::mpreal("0.000000000000000000000001"), "y");

  instance.AddConstant(mpfr::mpreal("0.01"), "pc");
  instance.AddConstant(mpfr::mpreal("0.001"), "pm");
  instance.AddConstant(mpfr::mpreal("0.0001"), "ptt");
  instance.AddConstant(mpfr::mpreal("0.000001"), "ppm");
  instance.AddConstant(mpfr::mpreal("0.000000001"), "ppb");
  instance.AddConstant(mpfr::mpreal("0.000000000001"), "ppt");
  instance.AddConstant(mpfr::mpreal("0.000000000000001"), "ppq");

  instance.AddConstant(mpfr::const_pi(), "math.pi");
  instance.AddConstant(mpfr::const_euler(), "math.E");
  instance.AddConstant(mpfr::const_catalan(), "math.catalan");

  instance.AddConstant(mpfr::mpreal("2.71828182846"), "math.e");

  instance.AddConstant(mpfr::mpreal("602214085700000000000000"), "phys.N");
  instance.AddConstant(mpfr::mpreal("299792458"), "phys.c");
  instance.AddConstant(mpfr::mpreal("149597870700"), "phys.au");
  instance.AddConstant(mpfr::mpreal("86400"), "phys.D");
  instance.AddConstant(mpfr::mpreal("1988920000000000000000000000000"), "phys.M");
  instance.AddConstant(mpfr::mpreal("9460730472580800"), "phys.ly");
  instance.AddConstant(mpfr::mpreal("30856775814913700"), "phys.pc");
  instance.AddConstant(mpfr::mpreal("0.00000000006674"), "phys.G");
  instance.AddConstant(mpfr::mpreal("9.80665"), "phys.g");
  instance.AddConstant(mpfr::mpreal("8.3144626181532"), "phys.R");

  instance.AddConstant(mpfr::mpreal("10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"), "googol");
}
