using System;
using System.Text;
using Math.Mpfr.Native;
using Libs.Collections;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;

namespace kalk
{
    internal static class ChemicalExpressions
    {
        #region Custom methods
        internal static object MolarMass(params object[] args)
        {
            return MolarMass_Internal(MakeCompoundString_Internal(args[0].ToString()));
        }

        private static object MolarMass_Internal(string text)
        {
            return Parser.Evaluate(text);
        }

        private static string MakeCompoundString_Internal(string text)
        {
            char lastChar = '\0';
            StringBuilder result = new StringBuilder();
            foreach(var chr in text)
            {
                if(!char.IsLetterOrDigit(chr) && (chr != '(' && chr != ')'))
                    throw new System.FormatException("Invalid character in checmical compound string");

                if(lastChar != '\0' && lastChar != '(')
                {
                    if(char.IsDigit(chr))
                    {
                        if(!char.IsDigit(lastChar))
                            result.Append('*');
                    }
                    else if(char.IsUpper(chr) || chr == '(')
                    {
                        result.Append('+');
                    }
                }

                result.Append(chr);
                lastChar = chr;
            }

            return result.ToString();
        }
        #endregion

        #region Operators
        private static readonly ExtendedDictionary<string, BinaryOperator> BinaryOperators = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
        {
            ( "+", 4, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs + (MPFR_Value)rhs ),
            ( "*", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs ),
        };

        private static readonly BinaryOperator ShorthandOperator = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs);
        #endregion

        #region Variables/Functions
        private static readonly ExtendedDictionary<string, Variable> Variables = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
        {
            ( "H",      new MPFR_Value(128, "1.00794")),
            ( "He",     new MPFR_Value(128, "4.002602")),

            ( "Li",     new MPFR_Value(128, "6.941")),
            ( "Be",     new MPFR_Value(128, "9.012182")),
            ( "B",      new MPFR_Value(128, "10.811")),
            ( "C",      new MPFR_Value(128, "12.0107")),
            ( "N",      new MPFR_Value(128, "14.0067")),
            ( "O",      new MPFR_Value(128, "15.9994")),
            ( "F",      new MPFR_Value(128, "18.998403")),
            ( "Ne",     new MPFR_Value(128, "20.1797")),

            ( "Na",     new MPFR_Value(128, "22.989769")),
            ( "Mg",     new MPFR_Value(128, "24.305")),
            ( "Al",     new MPFR_Value(128, "26.981539")),
            ( "Si",     new MPFR_Value(128, "28.0855")),
            ( "P",      new MPFR_Value(128, "30.973762")),
            ( "S",      new MPFR_Value(128, "32.065")),
            ( "Cl",     new MPFR_Value(128, "35.453")),
            ( "Ar",     new MPFR_Value(128, "39.948")),

            ( "K",      new MPFR_Value(128, "39.0983")),
            ( "Ca",     new MPFR_Value(128, "40.078")),
            ( "Sc",     new MPFR_Value(128, "44.955912")),
            ( "Ti",     new MPFR_Value(128, "47.867")),
            ( "V",      new MPFR_Value(128, "50.9415")),
            ( "Cr",     new MPFR_Value(128, "51.9961")),
            ( "Mn",     new MPFR_Value(128, "54.938045")),
            ( "Fe",     new MPFR_Value(128, "55.845")),
            ( "Co",     new MPFR_Value(128, "58.933195")),
            ( "Ni",     new MPFR_Value(128, "58.6934")),
            ( "Cu",     new MPFR_Value(128, "63.546")),
            ( "Zn",     new MPFR_Value(128, "65.38")),
            ( "Ga",     new MPFR_Value(128, "69.723")),
            ( "Ge",     new MPFR_Value(128, "72.64")),
            ( "As",     new MPFR_Value(128, "74.9216")),
            ( "Se",     new MPFR_Value(128, "78.96")),
            ( "Br",     new MPFR_Value(128, "79.904")),
            ( "Kr",     new MPFR_Value(128, "83.798")),

            ( "Rb",     new MPFR_Value(128, "85.4678")),
            ( "Sr",     new MPFR_Value(128, "87.62")),
            ( "Y",      new MPFR_Value(128, "88.90585")),
            ( "Zr",     new MPFR_Value(128, "91.224")),
            ( "Nb",     new MPFR_Value(128, "92.90638")),
            ( "Mo",     new MPFR_Value(128, "95.94")),
            ( "Tc",     new MPFR_Value(128, "98")),
            ( "Ru",     new MPFR_Value(128, "101.07")),
            ( "Rh",     new MPFR_Value(128, "102.9055")),
            ( "Pd",     new MPFR_Value(128, "106.42")),
            ( "Ag",     new MPFR_Value(128, "107.8682")),
            ( "Cd",     new MPFR_Value(128, "112.411")),
            ( "In",     new MPFR_Value(128, "114.818")),
            ( "Sn",     new MPFR_Value(128, "118.71")),
            ( "Sb",     new MPFR_Value(128, "121.76")),
            ( "Te",     new MPFR_Value(128, "127.6")),
            ( "I",      new MPFR_Value(128, "126.90447")),
            ( "Xe",     new MPFR_Value(128, "131.293")),

            ( "Cs",     new MPFR_Value(128, "132.90545")),
            ( "Ba",     new MPFR_Value(128, "137.327")),

            ( "La",     new MPFR_Value(128, "138.90547")),
            ( "Ce",     new MPFR_Value(128, "140.116")),
            ( "Pr",     new MPFR_Value(128, "140.90765")),
            ( "Nd",     new MPFR_Value(128, "144.242")),
            ( "Pm",     new MPFR_Value(128, "145")),
            ( "Sm",     new MPFR_Value(128, "150.36")),
            ( "Eu",     new MPFR_Value(128, "151.964")),
            ( "Gd",     new MPFR_Value(128, "157.25")),
            ( "Tb",     new MPFR_Value(128, "158.92535")),
            ( "Dy",     new MPFR_Value(128, "162.5")),
            ( "Ho",     new MPFR_Value(128, "164.93032")),
            ( "Er",     new MPFR_Value(128, "167.259")),
            ( "Tm",     new MPFR_Value(128, "168.93421")),
            ( "Yb",     new MPFR_Value(128, "173.04")),
            ( "Lu",     new MPFR_Value(128, "174.967")),

            ( "Hf",     new MPFR_Value(128, "178.49")),
            ( "Ta",     new MPFR_Value(128, "180.94788")),
            ( "W",      new MPFR_Value(128, "183.84")),
            ( "Re",     new MPFR_Value(128, "186.207")),
            ( "Os",     new MPFR_Value(128, "190.23")),
            ( "Ir",     new MPFR_Value(128, "192.217")),
            ( "Pt",     new MPFR_Value(128, "195.084")),
            ( "Au",     new MPFR_Value(128, "196.96657")),
            ( "Hg",     new MPFR_Value(128, "200.59")),
            ( "Tl",     new MPFR_Value(128, "204.3833")),
            ( "Pb",     new MPFR_Value(128, "207.2")),
            ( "Bi",     new MPFR_Value(128, "208.9804")),
            ( "Po",     new MPFR_Value(128, "209")),
            ( "At",     new MPFR_Value(128, "210")),
            ( "Rn",     new MPFR_Value(128, "222")),

            ( "Fr",     new MPFR_Value(128, "223")),
            ( "Ra",     new MPFR_Value(128, "226")),

            ( "Ac",     new MPFR_Value(128, "227")),
            ( "Th",     new MPFR_Value(128, "232.03806")),
            ( "Pa",     new MPFR_Value(128, "231.03588")),
            ( "U",      new MPFR_Value(128, "238.02891")),
            ( "Np",     new MPFR_Value(128, "237")),
            ( "Pu",     new MPFR_Value(128, "244")),
            ( "Am",     new MPFR_Value(128, "243")),
            ( "Cm",     new MPFR_Value(128, "247")),
            ( "Bk",     new MPFR_Value(128, "247")),
            ( "Cf",     new MPFR_Value(128, "251")),
            ( "Es",     new MPFR_Value(128, "252")),
            ( "Fm",     new MPFR_Value(128, "257")),
            ( "Md",     new MPFR_Value(128, "258")),
            ( "No",     new MPFR_Value(128, "259")),
            ( "Lr",     new MPFR_Value(128, "262")),

            ( "Rf",     new MPFR_Value(128, "261")),
            ( "Db",     new MPFR_Value(128, "262")),
            ( "Sg",     new MPFR_Value(128, "266")),
            ( "Bh",     new MPFR_Value(128, "264")),
            ( "Hs",     new MPFR_Value(128, "277")),
            ( "Mt",     new MPFR_Value(128, "268")),
            ( "Ds",     new MPFR_Value(128, "281")),
            ( "Uun",    new MPFR_Value(128, "281")),//Ds
            ( "Rg",     new MPFR_Value(128, "272")),
            ( "Uuu",    new MPFR_Value(128, "272")),//Rg
            ( "Cn",     new MPFR_Value(128, "285")),
            ( "UUb",    new MPFR_Value(128, "285")),//Cn
            ( "Uut",    new MPFR_Value(128, "284")),
            ( "Fl",     new MPFR_Value(128, "289")),
            ( "Uuq",    new MPFR_Value(128, "289")),//Fl
            ( "Uup",    new MPFR_Value(128, "288")),
            ( "Lv",     new MPFR_Value(128, "292")),
            ( "Uuh",    new MPFR_Value(128, "292")),//Lv
            ( "Uus",    new MPFR_Value(128, "294")),
            ( "Uuo",    new MPFR_Value(128, "294"))
        };
        #endregion

        private static readonly ExpressionParser Parser = new ExpressionParser(null, BinaryOperators, Variables, null, ShorthandOperator, null, null);

        static ChemicalExpressions()
        {
            Parser.NumberConverter = Common.ParseMPFR;
        }
    }
}
