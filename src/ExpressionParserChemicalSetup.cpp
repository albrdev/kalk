#include "ExpressionParserSetup.hpp"
#include <mpreal.h>

static KalkValueType* BinaryOperator_Addition(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(lhs->GetValue<KalkArithmeticType>() + rhs->GetValue<KalkArithmeticType>());
}

static KalkValueType* BinaryOperator_Multiplication(KalkValueType* lhs, KalkValueType* rhs)
{
  return new KalkValueType(lhs->GetValue<KalkArithmeticType>() * rhs->GetValue<KalkArithmeticType>());
}

void InitChemical(ExpressionParser<KalkArithmeticType>& instance)
{
  instance.AddBinaryOperator(BinaryOperator_Addition, "+", 1, Associativity::Left);
  instance.AddBinaryOperator(BinaryOperator_Multiplication, "*", 2, Associativity::Left);

  instance.SetJuxtapositionOperator(BinaryOperator_Multiplication, 2, Associativity::Right);

  instance.AddConstant(mpfr::mpreal("1.00794"), "H");
  instance.AddConstant(mpfr::mpreal("4.002602"), "He");

  instance.AddConstant(mpfr::mpreal("6.941"), "Li");
  instance.AddConstant(mpfr::mpreal("9.012182"), "Be");
  instance.AddConstant(mpfr::mpreal("10.811"), "B");
  instance.AddConstant(mpfr::mpreal("12.0107"), "C");
  instance.AddConstant(mpfr::mpreal("14.0067"), "N");
  instance.AddConstant(mpfr::mpreal("15.9994"), "O");
  instance.AddConstant(mpfr::mpreal("18.998403"), "F");
  instance.AddConstant(mpfr::mpreal("20.1797"), "Ne");

  instance.AddConstant(mpfr::mpreal("22.989769"), "Na");
  instance.AddConstant(mpfr::mpreal("24.305"), "Mg");
  instance.AddConstant(mpfr::mpreal("26.981539"), "Al");
  instance.AddConstant(mpfr::mpreal("28.0855"), "Si");
  instance.AddConstant(mpfr::mpreal("30.973762"), "P");
  instance.AddConstant(mpfr::mpreal("32.065"), "S");
  instance.AddConstant(mpfr::mpreal("35.453"), "Cl");
  instance.AddConstant(mpfr::mpreal("39.948"), "Ar");

  instance.AddConstant(mpfr::mpreal("39.0983"), "K");
  instance.AddConstant(mpfr::mpreal("40.078"), "Ca");
  instance.AddConstant(mpfr::mpreal("44.955912"), "Sc");
  instance.AddConstant(mpfr::mpreal("47.867"), "Ti");
  instance.AddConstant(mpfr::mpreal("50.9415"), "V");
  instance.AddConstant(mpfr::mpreal("51.9961"), "Cr");
  instance.AddConstant(mpfr::mpreal("54.938045"), "Mn");
  instance.AddConstant(mpfr::mpreal("55.845"), "Fe");
  instance.AddConstant(mpfr::mpreal("58.933195"), "Co");
  instance.AddConstant(mpfr::mpreal("58.6934"), "Ni");
  instance.AddConstant(mpfr::mpreal("63.546"), "Cu");
  instance.AddConstant(mpfr::mpreal("65.38"), "Zn");
  instance.AddConstant(mpfr::mpreal("69.723"), "Ga");
  instance.AddConstant(mpfr::mpreal("72.64"), "Ge");
  instance.AddConstant(mpfr::mpreal("74.9216"), "As");
  instance.AddConstant(mpfr::mpreal("78.96"), "Se");
  instance.AddConstant(mpfr::mpreal("79.904"), "Br");
  instance.AddConstant(mpfr::mpreal("83.798"), "Kr");

  instance.AddConstant(mpfr::mpreal("85.4678"), "Rb");
  instance.AddConstant(mpfr::mpreal("87.62"), "Sr");
  instance.AddConstant(mpfr::mpreal("88.90585"), "Y");
  instance.AddConstant(mpfr::mpreal("91.224"), "Zr");
  instance.AddConstant(mpfr::mpreal("92.90638"), "Nb");
  instance.AddConstant(mpfr::mpreal("95.94"), "Mo");
  instance.AddConstant(mpfr::mpreal("98"), "Tc");
  instance.AddConstant(mpfr::mpreal("101.07"), "Ru");
  instance.AddConstant(mpfr::mpreal("102.9055"), "Rh");
  instance.AddConstant(mpfr::mpreal("106.42"), "Pd");
  instance.AddConstant(mpfr::mpreal("107.8682"), "Ag");
  instance.AddConstant(mpfr::mpreal("112.411"), "Cd");
  instance.AddConstant(mpfr::mpreal("114.818"), "In");
  instance.AddConstant(mpfr::mpreal("118.71"), "Sn");
  instance.AddConstant(mpfr::mpreal("121.76"), "Sb");
  instance.AddConstant(mpfr::mpreal("127.6"), "Te");
  instance.AddConstant(mpfr::mpreal("126.90447"), "I");
  instance.AddConstant(mpfr::mpreal("131.293"), "Xe");

  instance.AddConstant(mpfr::mpreal("132.90545"), "Cs");
  instance.AddConstant(mpfr::mpreal("137.327"), "Ba");

  instance.AddConstant(mpfr::mpreal("138.90547"), "La");
  instance.AddConstant(mpfr::mpreal("140.116"), "Ce");
  instance.AddConstant(mpfr::mpreal("140.90765"), "Pr");
  instance.AddConstant(mpfr::mpreal("144.242"), "Nd");
  instance.AddConstant(mpfr::mpreal("145"), "Pm");
  instance.AddConstant(mpfr::mpreal("150.36"), "Sm");
  instance.AddConstant(mpfr::mpreal("151.964"), "Eu");
  instance.AddConstant(mpfr::mpreal("157.25"), "Gd");
  instance.AddConstant(mpfr::mpreal("158.92535"), "Tb");
  instance.AddConstant(mpfr::mpreal("162.5"), "Dy");
  instance.AddConstant(mpfr::mpreal("164.93032"), "Ho");
  instance.AddConstant(mpfr::mpreal("167.259"), "Er");
  instance.AddConstant(mpfr::mpreal("168.93421"), "Tm");
  instance.AddConstant(mpfr::mpreal("173.04"), "Yb");
  instance.AddConstant(mpfr::mpreal("174.967"), "Lu");

  instance.AddConstant(mpfr::mpreal("178.49"), "Hf");
  instance.AddConstant(mpfr::mpreal("180.94788"), "Ta");
  instance.AddConstant(mpfr::mpreal("183.84"), "W");
  instance.AddConstant(mpfr::mpreal("186.207"), "Re");
  instance.AddConstant(mpfr::mpreal("190.23"), "Os");
  instance.AddConstant(mpfr::mpreal("192.217"), "Ir");
  instance.AddConstant(mpfr::mpreal("195.084"), "Pt");
  instance.AddConstant(mpfr::mpreal("196.96657"), "Au");
  instance.AddConstant(mpfr::mpreal("200.59"), "Hg");
  instance.AddConstant(mpfr::mpreal("204.3833"), "Tl");
  instance.AddConstant(mpfr::mpreal("207.2"), "Pb");
  instance.AddConstant(mpfr::mpreal("208.9804"), "Bi");
  instance.AddConstant(mpfr::mpreal("209"), "Po");
  instance.AddConstant(mpfr::mpreal("210"), "At");
  instance.AddConstant(mpfr::mpreal("222"), "Rn");

  instance.AddConstant(mpfr::mpreal("223"), "Fr");
  instance.AddConstant(mpfr::mpreal("226"), "Ra");

  instance.AddConstant(mpfr::mpreal("227"), "Ac");
  instance.AddConstant(mpfr::mpreal("232.03806"), "Th");
  instance.AddConstant(mpfr::mpreal("231.03588"), "Pa");
  instance.AddConstant(mpfr::mpreal("238.02891"), "U");
  instance.AddConstant(mpfr::mpreal("237"), "Np");
  instance.AddConstant(mpfr::mpreal("244"), "Pu");
  instance.AddConstant(mpfr::mpreal("243"), "Am");
  instance.AddConstant(mpfr::mpreal("247"), "Cm");
  instance.AddConstant(mpfr::mpreal("247"), "Bk");
  instance.AddConstant(mpfr::mpreal("251"), "Cf");
  instance.AddConstant(mpfr::mpreal("252"), "Es");
  instance.AddConstant(mpfr::mpreal("257"), "Fm");
  instance.AddConstant(mpfr::mpreal("258"), "Md");
  instance.AddConstant(mpfr::mpreal("259"), "No");
  instance.AddConstant(mpfr::mpreal("262"), "Lr");

  instance.AddConstant(mpfr::mpreal("261"), "Rf");
  instance.AddConstant(mpfr::mpreal("262"), "Db");
  instance.AddConstant(mpfr::mpreal("266"), "Sg");
  instance.AddConstant(mpfr::mpreal("264"), "Bh");
  instance.AddConstant(mpfr::mpreal("277"), "Hs");
  instance.AddConstant(mpfr::mpreal("268"), "Mt");
  instance.AddConstant(mpfr::mpreal("281"), "Ds");
  instance.AddConstant(mpfr::mpreal("281"), "Uun"); //Ds
  instance.AddConstant(mpfr::mpreal("272"), "Rg");
  instance.AddConstant(mpfr::mpreal("272"), "Uuu"); //Rg
  instance.AddConstant(mpfr::mpreal("285"), "Cn");
  instance.AddConstant(mpfr::mpreal("285"), "Uub"); //Cn
  instance.AddConstant(mpfr::mpreal("284"), "Uut");
  instance.AddConstant(mpfr::mpreal("289"), "Fl");
  instance.AddConstant(mpfr::mpreal("289"), "Uuq"); //Fl
  instance.AddConstant(mpfr::mpreal("288"), "Uup");
  instance.AddConstant(mpfr::mpreal("292"), "Lv");
  instance.AddConstant(mpfr::mpreal("292"), "Uuh"); //Lv
  instance.AddConstant(mpfr::mpreal("294"), "Uus");
  instance.AddConstant(mpfr::mpreal("294"), "Uuo");
}
