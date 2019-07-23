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
            ( "+", 4, AssociativityType.Left,   (lhs, rhs) => (MPFR)lhs + (MPFR)rhs ),
            ( "*", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR)lhs * (MPFR)rhs ),
        };

        private static readonly BinaryOperator ShorthandOperator = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR)lhs * (MPFR)rhs);
        #endregion

        #region Variables/Functions
        private static readonly ExtendedDictionary<string, Variable> Variables = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
        {
            ( "H",      new MPFR(128, "1.00794")),
            ( "He",     new MPFR(128, "4.002602")),

            ( "Li",     new MPFR(128, "6.941")),
            ( "Be",     new MPFR(128, "9.012182")),
            ( "B",      new MPFR(128, "10.811")),
            ( "C",      new MPFR(128, "12.0107")),
            ( "N",      new MPFR(128, "14.0067")),
            ( "O",      new MPFR(128, "15.9994")),
            ( "F",      new MPFR(128, "18.998403")),
            ( "Ne",     new MPFR(128, "20.1797")),

            ( "Na",     new MPFR(128, "22.989769")),
            ( "Mg",     new MPFR(128, "24.305")),
            ( "Al",     new MPFR(128, "26.981539")),
            ( "Si",     new MPFR(128, "28.0855")),
            ( "P",      new MPFR(128, "30.973762")),
            ( "S",      new MPFR(128, "32.065")),
            ( "Cl",     new MPFR(128, "35.453")),
            ( "Ar",     new MPFR(128, "39.948")),

            ( "K",      new MPFR(128, "39.0983")),
            ( "Ca",     new MPFR(128, "40.078")),
            ( "Sc",     new MPFR(128, "44.955912")),
            ( "Ti",     new MPFR(128, "47.867")),
            ( "V",      new MPFR(128, "50.9415")),
            ( "Cr",     new MPFR(128, "51.9961")),
            ( "Mn",     new MPFR(128, "54.938045")),
            ( "Fe",     new MPFR(128, "55.845")),
            ( "Co",     new MPFR(128, "58.933195")),
            ( "Ni",     new MPFR(128, "58.6934")),
            ( "Cu",     new MPFR(128, "63.546")),
            ( "Zn",     new MPFR(128, "65.38")),
            ( "Ga",     new MPFR(128, "69.723")),
            ( "Ge",     new MPFR(128, "72.64")),
            ( "As",     new MPFR(128, "74.9216")),
            ( "Se",     new MPFR(128, "78.96")),
            ( "Br",     new MPFR(128, "79.904")),
            ( "Kr",     new MPFR(128, "83.798")),

            ( "Rb",     new MPFR(128, "85.4678")),
            ( "Sr",     new MPFR(128, "87.62")),
            ( "Y",      new MPFR(128, "88.90585")),
            ( "Zr",     new MPFR(128, "91.224")),
            ( "Nb",     new MPFR(128, "92.90638")),
            ( "Mo",     new MPFR(128, "95.94")),
            ( "Tc",     new MPFR(128, "98")),
            ( "Ru",     new MPFR(128, "101.07")),
            ( "Rh",     new MPFR(128, "102.9055")),
            ( "Pd",     new MPFR(128, "106.42")),
            ( "Ag",     new MPFR(128, "107.8682")),
            ( "Cd",     new MPFR(128, "112.411")),
            ( "In",     new MPFR(128, "114.818")),
            ( "Sn",     new MPFR(128, "118.71")),
            ( "Sb",     new MPFR(128, "121.76")),
            ( "Te",     new MPFR(128, "127.6")),
            ( "I",      new MPFR(128, "126.90447")),
            ( "Xe",     new MPFR(128, "131.293")),

            ( "Cs",     new MPFR(128, "132.90545")),
            ( "Ba",     new MPFR(128, "137.327")),

            ( "La",     new MPFR(128, "138.90547")),
            ( "Ce",     new MPFR(128, "140.116")),
            ( "Pr",     new MPFR(128, "140.90765")),
            ( "Nd",     new MPFR(128, "144.242")),
            ( "Pm",     new MPFR(128, "145")),
            ( "Sm",     new MPFR(128, "150.36")),
            ( "Eu",     new MPFR(128, "151.964")),
            ( "Gd",     new MPFR(128, "157.25")),
            ( "Tb",     new MPFR(128, "158.92535")),
            ( "Dy",     new MPFR(128, "162.5")),
            ( "Ho",     new MPFR(128, "164.93032")),
            ( "Er",     new MPFR(128, "167.259")),
            ( "Tm",     new MPFR(128, "168.93421")),
            ( "Yb",     new MPFR(128, "173.04")),
            ( "Lu",     new MPFR(128, "174.967")),

            ( "Hf",     new MPFR(128, "178.49")),
            ( "Ta",     new MPFR(128, "180.94788")),
            ( "W",      new MPFR(128, "183.84")),
            ( "Re",     new MPFR(128, "186.207")),
            ( "Os",     new MPFR(128, "190.23")),
            ( "Ir",     new MPFR(128, "192.217")),
            ( "Pt",     new MPFR(128, "195.084")),
            ( "Au",     new MPFR(128, "196.96657")),
            ( "Hg",     new MPFR(128, "200.59")),
            ( "Tl",     new MPFR(128, "204.3833")),
            ( "Pb",     new MPFR(128, "207.2")),
            ( "Bi",     new MPFR(128, "208.9804")),
            ( "Po",     new MPFR(128, "209")),
            ( "At",     new MPFR(128, "210")),
            ( "Rn",     new MPFR(128, "222")),

            ( "Fr",     new MPFR(128, "223")),
            ( "Ra",     new MPFR(128, "226")),

            ( "Ac",     new MPFR(128, "227")),
            ( "Th",     new MPFR(128, "232.03806")),
            ( "Pa",     new MPFR(128, "231.03588")),
            ( "U",      new MPFR(128, "238.02891")),
            ( "Np",     new MPFR(128, "237")),
            ( "Pu",     new MPFR(128, "244")),
            ( "Am",     new MPFR(128, "243")),
            ( "Cm",     new MPFR(128, "247")),
            ( "Bk",     new MPFR(128, "247")),
            ( "Cf",     new MPFR(128, "251")),
            ( "Es",     new MPFR(128, "252")),
            ( "Fm",     new MPFR(128, "257")),
            ( "Md",     new MPFR(128, "258")),
            ( "No",     new MPFR(128, "259")),
            ( "Lr",     new MPFR(128, "262")),

            ( "Rf",     new MPFR(128, "261")),
            ( "Db",     new MPFR(128, "262")),
            ( "Sg",     new MPFR(128, "266")),
            ( "Bh",     new MPFR(128, "264")),
            ( "Hs",     new MPFR(128, "277")),
            ( "Mt",     new MPFR(128, "268")),
            ( "Ds",     new MPFR(128, "281")),
            ( "Uun",    new MPFR(128, "281")),//Ds
            ( "Rg",     new MPFR(128, "272")),
            ( "Uuu",    new MPFR(128, "272")),//Rg
            ( "Cn",     new MPFR(128, "285")),
            ( "UUb",    new MPFR(128, "285")),//Cn
            ( "Uut",    new MPFR(128, "284")),
            ( "Fl",     new MPFR(128, "289")),
            ( "Uuq",    new MPFR(128, "289")),//Fl
            ( "Uup",    new MPFR(128, "288")),
            ( "Lv",     new MPFR(128, "292")),
            ( "Uuh",    new MPFR(128, "292")),//Lv
            ( "Uus",    new MPFR(128, "294")),
            ( "Uuo",    new MPFR(128, "294"))
        };
        #endregion

        private static readonly ExpressionParser Parser = new ExpressionParser(null, BinaryOperators, Variables, null, null, null)
        {
            ShorthandOperator = ShorthandOperator,
        };

        static ChemicalExpressions()
        {
            Parser.NumberConverter = Common.ParseMPFR;
        }
    }
}
