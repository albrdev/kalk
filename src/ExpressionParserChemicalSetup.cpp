#include "ExpressionParserSetup.hpp"
#include <mpreal.h>

inline std::unordered_map<std::string, std::unique_ptr<ChemBinaryOperatorType>> chemBinaryOperatorCache;
inline std::unordered_map<std::string, ChemBinaryOperatorType*> chemBinaryOperators;

inline std::unordered_map<std::string, std::unique_ptr<ChemVariableType>> chemInitializedVariableCache;
inline std::unordered_map<std::string, ChemVariableType*> chemVariables;

static void addBinaryOperator(const ChemBinaryOperatorType::CallbackType& callback, const std::string& identifier, int precedence, Associativity associativity)
{
  auto tmpNew                         = std::make_unique<ChemBinaryOperatorType>(callback, identifier, precedence, associativity);
  auto tmp                            = tmpNew.get();
  chemBinaryOperatorCache[identifier] = std::move(tmpNew);
  chemBinaryOperators[identifier]     = tmp;
}

template<class T>
static void addVariable(const T& value, const std::string& identifier)
{
  auto tmpNew                              = std::make_unique<ChemVariableType>(identifier, value);
  auto tmp                                 = tmpNew.get();
  chemInitializedVariableCache[identifier] = std::move(tmpNew);
  chemVariables[identifier]                = tmp;
}

static ChemValueType* BinaryOperator_Addition(ChemValueType* lhs, ChemValueType* rhs)
{
  return new ChemValueType(lhs->GetValue<ChemArithmeticType>() + rhs->GetValue<ChemArithmeticType>());
}

static ChemValueType* BinaryOperator_Multiplication(ChemValueType* lhs, ChemValueType* rhs)
{
  return new ChemValueType(lhs->GetValue<ChemArithmeticType>() * rhs->GetValue<ChemArithmeticType>());
}

void InitChemical(ExpressionParser<ChemArithmeticType>& instance)
{
  instance.SetBinaryOperators(&chemBinaryOperators);
  instance.SetVariables(&chemVariables);
  instance.SetJuxtapositionOperator(BinaryOperator_Multiplication, 2, Associativity::Right);

  addBinaryOperator(BinaryOperator_Addition, "+", 1, Associativity::Left);
  addBinaryOperator(BinaryOperator_Multiplication, "*", 2, Associativity::Left);

  addVariable(mpfr::mpreal("1.00794"), "H");
  addVariable(mpfr::mpreal("4.002602"), "He");

  addVariable(mpfr::mpreal("6.941"), "Li");
  addVariable(mpfr::mpreal("9.012182"), "Be");
  addVariable(mpfr::mpreal("10.811"), "B");
  addVariable(mpfr::mpreal("12.0107"), "C");
  addVariable(mpfr::mpreal("14.0067"), "N");
  addVariable(mpfr::mpreal("15.9994"), "O");
  addVariable(mpfr::mpreal("18.998403"), "F");
  addVariable(mpfr::mpreal("20.1797"), "Ne");

  addVariable(mpfr::mpreal("22.989769"), "Na");
  addVariable(mpfr::mpreal("24.305"), "Mg");
  addVariable(mpfr::mpreal("26.981539"), "Al");
  addVariable(mpfr::mpreal("28.0855"), "Si");
  addVariable(mpfr::mpreal("30.973762"), "P");
  addVariable(mpfr::mpreal("32.065"), "S");
  addVariable(mpfr::mpreal("35.453"), "Cl");
  addVariable(mpfr::mpreal("39.948"), "Ar");

  addVariable(mpfr::mpreal("39.0983"), "K");
  addVariable(mpfr::mpreal("40.078"), "Ca");
  addVariable(mpfr::mpreal("44.955912"), "Sc");
  addVariable(mpfr::mpreal("47.867"), "Ti");
  addVariable(mpfr::mpreal("50.9415"), "V");
  addVariable(mpfr::mpreal("51.9961"), "Cr");
  addVariable(mpfr::mpreal("54.938045"), "Mn");
  addVariable(mpfr::mpreal("55.845"), "Fe");
  addVariable(mpfr::mpreal("58.933195"), "Co");
  addVariable(mpfr::mpreal("58.6934"), "Ni");
  addVariable(mpfr::mpreal("63.546"), "Cu");
  addVariable(mpfr::mpreal("65.38"), "Zn");
  addVariable(mpfr::mpreal("69.723"), "Ga");
  addVariable(mpfr::mpreal("72.64"), "Ge");
  addVariable(mpfr::mpreal("74.9216"), "As");
  addVariable(mpfr::mpreal("78.96"), "Se");
  addVariable(mpfr::mpreal("79.904"), "Br");
  addVariable(mpfr::mpreal("83.798"), "Kr");

  addVariable(mpfr::mpreal("85.4678"), "Rb");
  addVariable(mpfr::mpreal("87.62"), "Sr");
  addVariable(mpfr::mpreal("88.90585"), "Y");
  addVariable(mpfr::mpreal("91.224"), "Zr");
  addVariable(mpfr::mpreal("92.90638"), "Nb");
  addVariable(mpfr::mpreal("95.94"), "Mo");
  addVariable(mpfr::mpreal("98"), "Tc");
  addVariable(mpfr::mpreal("101.07"), "Ru");
  addVariable(mpfr::mpreal("102.9055"), "Rh");
  addVariable(mpfr::mpreal("106.42"), "Pd");
  addVariable(mpfr::mpreal("107.8682"), "Ag");
  addVariable(mpfr::mpreal("112.411"), "Cd");
  addVariable(mpfr::mpreal("114.818"), "In");
  addVariable(mpfr::mpreal("118.71"), "Sn");
  addVariable(mpfr::mpreal("121.76"), "Sb");
  addVariable(mpfr::mpreal("127.6"), "Te");
  addVariable(mpfr::mpreal("126.90447"), "I");
  addVariable(mpfr::mpreal("131.293"), "Xe");

  addVariable(mpfr::mpreal("132.90545"), "Cs");
  addVariable(mpfr::mpreal("137.327"), "Ba");

  addVariable(mpfr::mpreal("138.90547"), "La");
  addVariable(mpfr::mpreal("140.116"), "Ce");
  addVariable(mpfr::mpreal("140.90765"), "Pr");
  addVariable(mpfr::mpreal("144.242"), "Nd");
  addVariable(mpfr::mpreal("145"), "Pm");
  addVariable(mpfr::mpreal("150.36"), "Sm");
  addVariable(mpfr::mpreal("151.964"), "Eu");
  addVariable(mpfr::mpreal("157.25"), "Gd");
  addVariable(mpfr::mpreal("158.92535"), "Tb");
  addVariable(mpfr::mpreal("162.5"), "Dy");
  addVariable(mpfr::mpreal("164.93032"), "Ho");
  addVariable(mpfr::mpreal("167.259"), "Er");
  addVariable(mpfr::mpreal("168.93421"), "Tm");
  addVariable(mpfr::mpreal("173.04"), "Yb");
  addVariable(mpfr::mpreal("174.967"), "Lu");

  addVariable(mpfr::mpreal("178.49"), "Hf");
  addVariable(mpfr::mpreal("180.94788"), "Ta");
  addVariable(mpfr::mpreal("183.84"), "W");
  addVariable(mpfr::mpreal("186.207"), "Re");
  addVariable(mpfr::mpreal("190.23"), "Os");
  addVariable(mpfr::mpreal("192.217"), "Ir");
  addVariable(mpfr::mpreal("195.084"), "Pt");
  addVariable(mpfr::mpreal("196.96657"), "Au");
  addVariable(mpfr::mpreal("200.59"), "Hg");
  addVariable(mpfr::mpreal("204.3833"), "Tl");
  addVariable(mpfr::mpreal("207.2"), "Pb");
  addVariable(mpfr::mpreal("208.9804"), "Bi");
  addVariable(mpfr::mpreal("209"), "Po");
  addVariable(mpfr::mpreal("210"), "At");
  addVariable(mpfr::mpreal("222"), "Rn");

  addVariable(mpfr::mpreal("223"), "Fr");
  addVariable(mpfr::mpreal("226"), "Ra");

  addVariable(mpfr::mpreal("227"), "Ac");
  addVariable(mpfr::mpreal("232.03806"), "Th");
  addVariable(mpfr::mpreal("231.03588"), "Pa");
  addVariable(mpfr::mpreal("238.02891"), "U");
  addVariable(mpfr::mpreal("237"), "Np");
  addVariable(mpfr::mpreal("244"), "Pu");
  addVariable(mpfr::mpreal("243"), "Am");
  addVariable(mpfr::mpreal("247"), "Cm");
  addVariable(mpfr::mpreal("247"), "Bk");
  addVariable(mpfr::mpreal("251"), "Cf");
  addVariable(mpfr::mpreal("252"), "Es");
  addVariable(mpfr::mpreal("257"), "Fm");
  addVariable(mpfr::mpreal("258"), "Md");
  addVariable(mpfr::mpreal("259"), "No");
  addVariable(mpfr::mpreal("262"), "Lr");

  addVariable(mpfr::mpreal("261"), "Rf");
  addVariable(mpfr::mpreal("262"), "Db");
  addVariable(mpfr::mpreal("266"), "Sg");
  addVariable(mpfr::mpreal("264"), "Bh");
  addVariable(mpfr::mpreal("277"), "Hs");
  addVariable(mpfr::mpreal("268"), "Mt");
  addVariable(mpfr::mpreal("281"), "Ds");
  addVariable(mpfr::mpreal("281"), "Uun"); //Ds
  addVariable(mpfr::mpreal("272"), "Rg");
  addVariable(mpfr::mpreal("272"), "Uuu"); //Rg
  addVariable(mpfr::mpreal("285"), "Cn");
  addVariable(mpfr::mpreal("285"), "Uub"); //Cn
  addVariable(mpfr::mpreal("284"), "Uut");
  addVariable(mpfr::mpreal("289"), "Fl");
  addVariable(mpfr::mpreal("289"), "Uuq"); //Fl
  addVariable(mpfr::mpreal("288"), "Uup");
  addVariable(mpfr::mpreal("292"), "Lv");
  addVariable(mpfr::mpreal("292"), "Uuh"); //Lv
  addVariable(mpfr::mpreal("294"), "Uus");
  addVariable(mpfr::mpreal("294"), "Uuo");
}
