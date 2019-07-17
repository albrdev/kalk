using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Linq;
using Math.Mpfr.Native;
using NDesk.Options;
using Libs.Collections;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;
using Libs.Text.Formatting;

namespace kalk
{
    class Program
    {
        #region Custom methods
        private static object Ans(params object[] args)
        {
            if(!ResultVariables.Any())
                throw new System.IndexOutOfRangeException($@"List contains no elements");

            if(!args.Any())
                return ResultVariables.Last();

            int index = System.Convert.ToInt32(args[0]);
            if(index < 0)
                index = ResultVariables.Count + index;

            if(index < 0 || index >= ResultVariables.Count)
                throw new System.IndexOutOfRangeException($@"Index out of range (Count: {ResultVariables.Count})");

            return ResultVariables[index];
        }

        private static object StringLength(params object[] args)
        {
            return args.Length > 0 ? args[0].ToString().Length : 0;
        }

        private static object Random(params object[] args)
        {
            if(args.Length >= 2)
                return MPFR_Value.URandom((MPFR_Value)args[0], (MPFR_Value)args[1], false);
            else if(args.Length >= 1)
                return MPFR_Value.URandom((MPFR_Value)args[0], false);
            else
                return MPFR_Value.URandom(false);
        }

        private static object RandomInclusive(params object[] args)
        {
            if(args.Length >= 2)
                return MPFR_Value.URandom((MPFR_Value)args[0], (MPFR_Value)args[1], true);
            else if(args.Length >= 1)
                return MPFR_Value.URandom((MPFR_Value)args[0], true);
            else
                return MPFR_Value.URandom(true);
        }

        private static ExtendedDictionary<string, BinaryOperator> ChemicalBinaryOperators { get; } = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
        {
            ( "+", 4, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs + (MPFR_Value)rhs ),
            ( "*", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs ),
        };

        private static ExtendedDictionary<string, Variable> ChemicalElementVariables { get; } = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
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

        private static BinaryOperator ChemicalAbbreviationOperator { get; } = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs);
        private static ExpressionParser ChemicalExpressionParser { get; } = new ExpressionParser(null, ChemicalBinaryOperators, ChemicalElementVariables, null, ChemicalAbbreviationOperator, null, null);

        private static object MolarMass(params object[] args)
        {
            return ChemicalExpressionParser.Evaluate(args[0].ToString());
        }

        private static object MakeChemicalCompoundString(params object[] args)
        {
            string tmp = args[0].ToString();
            char lastChar = '\0';
            StringBuilder result = new StringBuilder();
            foreach(var chr in tmp)
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

        private static UnaryOperator AssignmentOperator { get; } = ('=', 1, AssociativityType.Right, (value) => value);
        private static EscapeSequenceFormatter EscapeSequenceFormatter { get; } = new ExtendedNativeEscapeSequenceFormatter();
        private static List<object> ResultVariables { get; } = new List<object>();

        private static ExtendedDictionary<char, UnaryOperator> ArithmeticUnaryOperators { get; } = new ExtendedDictionary<char, UnaryOperator>((value) => value.Identifier)
        {
            ( '+', 1, AssociativityType.Right,  (value) => +(MPFR_Value)value ),
            ( '-', 1, AssociativityType.Right,  (value) => -(MPFR_Value)value )
        };

        private static ExtendedDictionary<string, BinaryOperator> ArithmeticBinaryOperators { get; } = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
        {
            ( "+", 4, AssociativityType.Left,   (lhs, rhs) => lhs is string || rhs is string? (object)$"{lhs}{rhs}" : (object)((MPFR_Value)lhs + (MPFR_Value)rhs) ),
            ( "-", 4, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs - (MPFR_Value)rhs ),
            ( "*", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs ),
            ( "/", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs / (MPFR_Value)rhs ),
            ( "%", 3, AssociativityType.Left,   (lhs, rhs) => (MPFR_Value)lhs % (MPFR_Value)rhs ),
            ( "^", 2, AssociativityType.Right,  (lhs, rhs) => MPFR_Value.Pow((MPFR_Value)lhs, (MPFR_Value)rhs) ),

            ( "//", 3, AssociativityType.Right, (lhs, rhs) => MPFR_Value.TruncatedDivision((MPFR_Value)lhs, (MPFR_Value)rhs) ),
            ( "**", 2, AssociativityType.Left,  (lhs, rhs) => MPFR_Value.Pow((MPFR_Value)lhs, (MPFR_Value)rhs) )
        };

        private static ExtendedDictionary<string, Variable> ArithmeticVariables { get; } = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
        {
            ( "Y",          new MPFR_Value(80, "1000000000000000000000000") ),
            ( "Z",          new MPFR_Value(70, "1000000000000000000000") ),
            ( "E",          new MPFR_Value(60, "1000000000000000000") ),
            ( "P",          new MPFR_Value(50, "1000000000000000") ),
            ( "T",          new MPFR_Value(40, "1000000000000") ),
            ( "G",          new MPFR_Value(30, "1000000000") ),
            ( "M",          new MPFR_Value(20, "1000000") ),
            ( "k",          new MPFR_Value(10, "1000") ),
            ( "h",          new MPFR_Value(7, "100") ),
            ( "da",         new MPFR_Value(4, "10") ),
            ( "d",          new MPFR_Value(32, "0.1") ),
            ( "c",          new MPFR_Value(32, "0.01") ),
            ( "m",          new MPFR_Value(32, "0.001") ),
            ( "u",          new MPFR_Value(32, "0.000001") ),
            ( "n",          new MPFR_Value(64, "0.000000001") ),
            ( "p",          new MPFR_Value(64, "0.000000000001") ),
            ( "f",          new MPFR_Value(64, "0.000000000000001") ),
            ( "a",          new MPFR_Value(64, "0.000000000000000001") ),
            ( "z",          new MPFR_Value(128, "0.000000000000000000001") ),
            ( "y",          new MPFR_Value(128, "0.000000000000000000000001") ),

            ( "math.pi",    MPFR_Value.Pi ),
            ( "math.E",     MPFR_Value.Euler ),
            ( "math.G",     MPFR_Value.Catalan ),
            ( "math.ln2",   MPFR_Value.LN2 ),
            ( "math.e",     new MPFR_Value(128, "2.71828182846") ),

            ( "phys.NA",    new MPFR_Value(79, "602214085700000000000000") ),
            ( "phys.au",    new MPFR_Value(38, "149597870700") ),
            ( "phys.ly",    new MPFR_Value(54, "9460730472580800") ),
            ( "phys.pc",    new MPFR_Value(55, "30856775814913700") ),
            ( "phys.c",     new MPFR_Value(29, "299792458") ),
            ( "phys.G",     new MPFR_Value(64, "0.00000000006674") ),
            ( "phys.g",     new MPFR_Value(32, "9.80665") ),
            ( "phys.R",     new MPFR_Value(64, "8.3144626181532") ),

            ( "googol",     new MPFR_Value(333, "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"))
        };

        private static ExtendedDictionary<string, Function> ArithmeticFunctions { get; } = new ExtendedDictionary<string, Function>((value) => value.Identifier)
        {
            ( "ans",        0, 1,   Ans ),
            ( "abs",        1,      (args) => MPFR_Value.Abs((MPFR_Value)args[0]) ),
            ( "neg",        1,      (args) => MPFR_Value.Neg((MPFR_Value)args[0]) ),
            ( "pow",        2,      (args) => MPFR_Value.Pow((MPFR_Value)args[0], (MPFR_Value)args[1]) ),
            ( "root",       2,      (args) => MPFR_Value.Root((MPFR_Value)args[0], (MPFR_Value)args[1]) ),
            ( "sqrt",       1,      (args) => MPFR_Value.Sqrt((MPFR_Value)args[0]) ),
            ( "cbrt",       1,      (args) => MPFR_Value.Cbrt((MPFR_Value)args[0]) ),
            ( "log",        1,      (args) => MPFR_Value.Log((MPFR_Value)args[0]) ),
            ( "log2",       1,      (args) => MPFR_Value.Log2((MPFR_Value)args[0]) ),
            ( "log10",      1,      (args) => MPFR_Value.Log10((MPFR_Value)args[0]) ),
            ( "sin",        1,      (args) => MPFR_Value.Sin((MPFR_Value)args[0]) ),
            ( "cos",        1,      (args) => MPFR_Value.Cos((MPFR_Value)args[0]) ),
            ( "tan",        1,      (args) => MPFR_Value.Tan((MPFR_Value)args[0]) ),
            ( "asin",       1,      (args) => MPFR_Value.Asin((MPFR_Value)args[0]) ),
            ( "acos",       1,      (args) => MPFR_Value.Acos((MPFR_Value)args[0]) ),
            ( "atan",       1,      (args) => MPFR_Value.Atan((MPFR_Value)args[0]) ),
            ( "sinh",       1,      (args) => MPFR_Value.Sinh((MPFR_Value)args[0]) ),
            ( "cosh",       1,      (args) => MPFR_Value.Cosh((MPFR_Value)args[0]) ),
            ( "tanh",       1,      (args) => MPFR_Value.Tanh((MPFR_Value)args[0]) ),
            ( "asinh",      1,      (args) => MPFR_Value.Asinh((MPFR_Value)args[0]) ),
            ( "acosh",      1,      (args) => MPFR_Value.Acosh((MPFR_Value)args[0]) ),
            ( "atanh",      1,      (args) => MPFR_Value.Atanh((MPFR_Value)args[0]) ),
            ( "min",        1, -1,  (args) => MPFR_Value.Min(args.Select(e => (MPFR_Value)e).ToArray()) ),
            ( "max",        1, -1,  (args) => MPFR_Value.Max(args.Select(e => (MPFR_Value)e).ToArray()) ),

            ( "rnd",        0, 2,   Random ),
            ( "rndi",       0, 2,   RandomInclusive ),

            ( "chem.M",     1,      (args) => MolarMass(MakeChemicalCompoundString(args)) ),

            ( "strlen",     1,      (args) => (MPFR_Value)(int)StringLength(args) )
        };

        private static BinaryOperator ArithmeticAbbreviationOperator { get; } = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs);
        private static ExpressionParser ArithmeticExpressionParser { get; } = new ExpressionParser(ArithmeticUnaryOperators, ArithmeticBinaryOperators, ArithmeticVariables, ArithmeticFunctions, ArithmeticAbbreviationOperator, AssignmentOperator, EscapeSequenceFormatter);

        private static object ParseMPFR(string value) => new MPFR_Value(value);

        private static string ApplicationName
        {
            get => Path.GetFileNameWithoutExtension(Environment.GetCommandLineArgs()[0]);
        }

        private static void PrintUsage(OptionSet optionsSet)
        {
            Console.WriteLine($"Usage: {ApplicationName} [-p precision] [-r roundingmode] [-z seed] expression");
            Console.WriteLine("Command line calculator");
            Console.WriteLine();
            Console.WriteLine("Options:");
            optionsSet.WriteOptionDescriptions(Console.Out);
        }

        public static TEnum Parse<TEnum>(string value) where TEnum : struct, IConvertible
        {
            TEnum result = (TEnum)Enum.Parse(typeof(TEnum), value);
            if(!Enum.IsDefined(typeof(TEnum), value))
                throw new System.OverflowException();

            return result;
        }

        public static bool TryParse<TEnum>(string value, out TEnum result) where TEnum : struct, IConvertible
        {
            return Enum.TryParse(value, out result) ? Enum.IsDefined(typeof(TEnum), result) : false;
        }

        private static mpfr_rnd_t ParseRoundingMode(string value)
        {
            if(TryParse(value, out mpfr_rnd_t result))
                return result;

            return Parse<mpfr_rnd_t>("MPFR_" + value);
        }

        private static object CommandExit(params string[] args)
        {
            Environment.Exit(0);
            return null;
        }

        private static object CommandClear(params string[] args)
        {
            if(args.Length > 0)
            {
                foreach(var arg in args)
                {
                    switch(arg)
                    {
                        case "screen":
                            Console.Clear();
                            break;
                        case "history":
                            ReadLine.ClearHistory();
                            break;
                        case "results":
                            ResultVariables.Clear();
                            break;
                        case "variables":
                            ArithmeticExpressionParser.ClearAssignedVariables();
                            break;
                        case "all":
                            ArithmeticExpressionParser.ClearAssignedVariables();
                            ResultVariables.Clear();
                            ReadLine.ClearHistory();
                            Console.Clear();
                            break;
                    }
                }
            }
            else
            {
                Console.Clear();
            }

            GC.Collect();
            return null;
        }

        private static object CommandPrecision(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.DefaultPrecision = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.DefaultPrecision;
        }

        private static object CommandDigitCount(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.DigitOutputCount = System.Convert.ToInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.DigitOutputCount;
        }

        private static object CommandRoundingMode(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RoundingMode = ParseRoundingMode(args[0].ToString());
                return null;
            }
            else
                return MPFR_Value.RoundingMode;
        }

        private static object CommandSeed(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RandomState = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.RandomState;
        }

        private static object CommandSeedString(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RandomState = args[0];
                return null;
            }
            else
                return MPFR_Value.RandomState;
        }

        private static CommandParser CommandParser { get; } = new CommandParser(EscapeSequenceFormatter)
        {
            { "exit", CommandExit },
            { "clear", CommandClear },

            { "prec", CommandPrecision },
            { "rmode", CommandRoundingMode },
            { "dgtcnt", CommandDigitCount },
            { "seed", CommandSeed },
            { "seedstr", CommandSeedString },

            { "ans", (args) => Ans(args) },
            { "nans", (args) => ResultVariables.Count }
        };

        static int Main(string[] args)
        {
            (List<string> Expressions, mpfr_prec_t Precision, mpfr_rnd_t RoundingMode, int DigitOuputCount, string Seed, string SeedString, bool InteractiveMode, bool PrintUsage) options = (new List<string>(), 1024, default, 128, null, null, false, false);

            var optionSet = new OptionSet()
            {
                { "x|expr=", "Expression", v => options.Expressions.Add(v) },
                { "p|prec=", "Precision", (uint v) => options.Precision = v },
                { "r|rmode=", "Rounding mode", v => options.RoundingMode = ParseRoundingMode(v) },
                { "n|dgtcnt=", "Digit output count", (int v) => options.DigitOuputCount = v },
                { "z|seed=", "Random seed", v => options.Seed = v },
                { "Z|seedstring=", "Random seed string", v => options.SeedString = v },
                { "i|interactive", "Interactive mode", v => options.InteractiveMode = v != null },
                { "h|help",  "Prints usage", v => options.PrintUsage = v != null },
                { "<>", v => options.Expressions.Add(v) }
            };

            try
            {
                optionSet.Parse(args);
            }
            catch(OptionException e)
            {
                Console.WriteLine($"{ApplicationName}: {e.Message}");
                Console.WriteLine($@"Try '{ApplicationName} --help' for more information.");
                return 1;
            }

            if(options.PrintUsage)
            {
                PrintUsage(optionSet);
                return 0;
            }

            if(options.SeedString != null)
            {
                MPFR_Value.RandomState = options.SeedString;
            }
            else if(options.Seed != null)
            {
                MPFR_Value.RandomState = uint.Parse(options.Seed);
            }

            MPFR_Value.DefaultPrecision = options.Precision;
            MPFR_Value.RoundingMode = options.RoundingMode;
            MPFR_Value.DigitOutputCount = options.DigitOuputCount;
            ArithmeticExpressionParser.NumberConverter = ParseMPFR;
            ChemicalExpressionParser.NumberConverter = ParseMPFR;

            foreach(var expr in options.Expressions)
            {
                var result = ArithmeticExpressionParser.Evaluate(expr);
                if(!options.InteractiveMode)
                    ResultVariables.Add(result);

                Console.WriteLine($"{result}");
            }

            if(options.InteractiveMode)
            {
                ReadLine.HistoryEnabled = false;
                while(true)
                {
                    string inputRaw = null;
                    inputRaw = ReadLine.Read("> ");

                    if(string.IsNullOrWhiteSpace(inputRaw))
                        continue;

                    string input = inputRaw.Trim();
                    object result;
                    if(input[0] == '!')
                    {
                        try
                        {
                            result = CommandParser.Execute(input.Remove(0, 1));
                        }
                        catch(Exception e)
                        {
                            result = $"*** Error: {e.Message}";
                        }
                    }
                    else
                    {
                        try
                        {
                            result = ArithmeticExpressionParser.Evaluate(input);
                            ResultVariables.Add(result);
                        }
                        catch(Exception e)
                        {
                            result = $"*** Error: {e.Message}";
                        }
                    }

                    if(result != null)
                    {
                        Console.WriteLine($"{result}");
                    }

                    ReadLine.AddHistory(inputRaw);
                }
            }

            return 0;
        }
    }
}
