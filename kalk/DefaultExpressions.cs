using System;
using System.Linq;
using Math.Gmp.Native;
using Math.Mpfr.Native;
using Libs.Collections;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;

namespace kalk
{
    internal static class DefaultExpressions
    {
        #region Custom methods
        private static object StringLength(params object[] args)
        {
            return args.Length > 0 ? args[0].ToString().Length : 0;
        }

        private static object Random(params object[] args)
        {
            if(args.Length >= 2)
                return MPFR.URandom((MPFR)args[0], (MPFR)args[1], false);
            else if(args.Length >= 1)
                return MPFR.URandom((MPFR)args[0], false);
            else
                return MPFR.URandom(false);
        }

        private static object RandomInclusive(params object[] args)
        {
            if(args.Length >= 2)
                return MPFR.URandom((MPFR)args[0], (MPFR)args[1], true);
            else if(args.Length >= 1)
                return MPFR.URandom((MPFR)args[0], true);
            else
                return MPFR.URandom(true);
        }
        #endregion

        #region Operators
        private static readonly ExtendedDictionary<char, UnaryOperator> UnaryOperators = new ExtendedDictionary<char, UnaryOperator>((value) => value.Identifier)
        {
            ( '+', 1, AssociativityType.Right,  (value) => +(MPFR)value ),
            ( '-', 1, AssociativityType.Right,  (value) => -(MPFR)value ),

            ( '!', 1, AssociativityType.Right,  (value) => value is bool ? !(bool)value : !(MPFR)value )
        };

        private static readonly ExtendedDictionary<string, BinaryOperator> BinaryOperators = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
        {
            ( "+", 4,   AssociativityType.Left,     (lhs, rhs) => lhs is string || rhs is string? (object)$"{lhs}{rhs}" : (object)((MPFR)lhs + (MPFR)rhs) ),
            ( "-", 4,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs - (MPFR)rhs ),
            ( "*", 3,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs * (MPFR)rhs ),
            ( "/", 3,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs / (MPFR)rhs ),
            ( "%", 3,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs % (MPFR)rhs ),
            ( "^", 2,   AssociativityType.Right,    (lhs, rhs) => MPFR.Pow((MPFR)lhs, (MPFR)rhs) ),

            ( "//", 3,  AssociativityType.Right,    (lhs, rhs) => MPFR.TruncatedDivision((MPFR)lhs, (MPFR)rhs) ),
            ( "**", 2,  AssociativityType.Right,    (lhs, rhs) => MPFR.Pow((MPFR)lhs, (MPFR)rhs) ),

            ( "==", 5,  AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs == (MPFR)rhs ),
            ( "!=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs != (MPFR)rhs ),
            ( "<", 5,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs < (MPFR)rhs ),
            ( ">", 5,   AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs > (MPFR)rhs ),
            ( "<=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs <= (MPFR)rhs ),
            ( ">=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPFR)lhs >= (MPFR)rhs ),

            ( "||", 6,  AssociativityType.Left,     (lhs, rhs) => System.Convert.ToBoolean(lhs) || System.Convert.ToBoolean(rhs) ),
            ( "&&", 6,  AssociativityType.Left,     (lhs, rhs) => System.Convert.ToBoolean(lhs) && System.Convert.ToBoolean(rhs) )
        };

        private static readonly BinaryOperator ShorthandOperator = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR)lhs * (MPFR)rhs);
        #endregion

        #region Variables/Functions
        private static readonly ExtendedDictionary<string, Variable> Variables = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
        {
            ( "nan",        MPFR.NaN ),
            ( "inf",        MPFR.PositiveInfinity ),
            ( "ninf",       MPFR.NegativeInfinity ),

            ( "Y",          new MPFR(80, "1000000000000000000000000") ),
            ( "Z",          new MPFR(70, "1000000000000000000000") ),
            ( "E",          new MPFR(60, "1000000000000000000") ),
            ( "P",          new MPFR(50, "1000000000000000") ),
            ( "T",          new MPFR(40, "1000000000000") ),
            ( "G",          new MPFR(30, "1000000000") ),
            ( "M",          new MPFR(20, "1000000") ),
            ( "k",          new MPFR(10, "1000") ),
            ( "h",          new MPFR(7, "100") ),
            ( "da",         new MPFR(4, "10") ),
            ( "d",          new MPFR(32, "0.1") ),
            ( "c",          new MPFR(32, "0.01") ),
            ( "m",          new MPFR(32, "0.001") ),
            ( "u",          new MPFR(32, "0.000001") ),
            ( "n",          new MPFR(64, "0.000000001") ),
            ( "p",          new MPFR(64, "0.000000000001") ),
            ( "f",          new MPFR(64, "0.000000000000001") ),
            ( "a",          new MPFR(64, "0.000000000000000001") ),
            ( "z",          new MPFR(128, "0.000000000000000000001") ),
            ( "y",          new MPFR(128, "0.000000000000000000000001") ),

            ( "pc",         new MPFR(32, "0.01") ),
            ( "pm",         new MPFR(32, "0.001") ),
            ( "ptt",        new MPFR(32, "0.0001") ),
            ( "ppm",        new MPFR(32, "0.000001") ),
            ( "ppb",        new MPFR(64, "0.000000001") ),
            ( "ppt",        new MPFR(64, "0.000000000001") ),
            ( "ppq",        new MPFR(64, "0.000000000000001") ),

            ( "math.pi",    MPFR.Pi ),
            ( "math.E",     MPFR.Euler ),
            ( "math.G",     MPFR.Catalan ),
            ( "math.ln2",   MPFR.LN2 ),
            ( "math.e",     new MPFR(128, "2.71828182846") ),

            ( "phys.NA",    new MPFR(79, "602214085700000000000000") ),
            ( "phys.au",    new MPFR(38, "149597870700") ),
            ( "phys.D",     new MPFR(17, "86400") ),
            ( "phys.M",     new MPFR(101, "1988920000000000000000000000000") ),
            ( "phys.ly",    new MPFR(54, "9460730472580800") ),
            ( "phys.pc",    new MPFR(55, "30856775814913700") ),
            ( "phys.c",     new MPFR(29, "299792458") ),
            ( "phys.G",     new MPFR(64, "0.00000000006674") ),
            ( "phys.g",     new MPFR(32, "9.80665") ),
            ( "phys.R",     new MPFR(64, "8.3144626181532") ),

            ( "googol",     new MPFR(333, "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"))
        };

        private static readonly ExtendedDictionary<string, Function> Functions = new ExtendedDictionary<string, Function>((value) => value.Identifier)
        {
            ( "ans",    0, 1,   Common.Ans ),

            ( "trunc",  1,      (args) => MPFR.Trunc((MPFR)args[0]) ),
            ( "abs",    1,      (args) => MPFR.Abs((MPFR)args[0]) ),
            ( "neg",    1,      (args) => MPFR.Neg((MPFR)args[0]) ),
            ( "neg2",   1,      (args) => MPFR.Neg2((MPFR)args[0]) ),
            ( "pow",    2,      (args) => MPFR.Pow((MPFR)args[0], (MPFR)args[1]) ),
            ( "sqr",    1,      (args) => MPFR.Sqr((MPFR)args[0]) ),
            ( "cb",     1,      (args) => MPFR.Cb((MPFR)args[0]) ),
            ( "exp",    1,      (args) => MPFR.Exp((MPFR)args[0]) ),
            ( "exp2",   1,      (args) => MPFR.Exp2((MPFR)args[0]) ),
            ( "exp10",  1,      (args) => MPFR.Exp10((MPFR)args[0]) ),
            ( "root",   2,      (args) => MPFR.Root((MPFR)args[0], (MPFR)args[1]) ),
            ( "sqrt",   1,      (args) => MPFR.Sqrt((MPFR)args[0]) ),
            ( "cbrt",   1,      (args) => MPFR.Cbrt((MPFR)args[0]) ),
            ( "logn",   2,      (args) => MPFR.LogN((MPFR)args[0], (MPFR)args[1]) ),
            ( "log",    1,      (args) => MPFR.Log((MPFR)args[0]) ),
            ( "log2",   1,      (args) => MPFR.Log2((MPFR)args[0]) ),
            ( "log10",  1,      (args) => MPFR.Log10((MPFR)args[0]) ),
            ( "sin",    1,      (args) => MPFR.Sin((MPFR)args[0]) ),
            ( "cos",    1,      (args) => MPFR.Cos((MPFR)args[0]) ),
            ( "tan",    1,      (args) => MPFR.Tan((MPFR)args[0]) ),
            ( "sec",    1,      (args) => MPFR.Sec((MPFR)args[0]) ),
            ( "asin",   1,      (args) => MPFR.Asin((MPFR)args[0]) ),
            ( "acos",   1,      (args) => MPFR.Acos((MPFR)args[0]) ),
            ( "atan",   1,      (args) => MPFR.Atan((MPFR)args[0]) ),
            ( "atan2",  2,      (args) => MPFR.Atan2((MPFR)args[0], (MPFR)args[1]) ),
            ( "asec",   1,      (args) => MPFR.Asec((MPFR)args[0]) ),
            ( "sinh",   1,      (args) => MPFR.Sinh((MPFR)args[0]) ),
            ( "cosh",   1,      (args) => MPFR.Cosh((MPFR)args[0]) ),
            ( "tanh",   1,      (args) => MPFR.Tanh((MPFR)args[0]) ),
            ( "coth",   1,      (args) => MPFR.Coth((MPFR)args[0]) ),
            ( "sech",   1,      (args) => MPFR.Sech((MPFR)args[0]) ),
            ( "csch",   1,      (args) => MPFR.Csch((MPFR)args[0]) ),
            ( "asinh",  1,      (args) => MPFR.Asinh((MPFR)args[0]) ),
            ( "acosh",  1,      (args) => MPFR.Acosh((MPFR)args[0]) ),
            ( "atanh",  1,      (args) => MPFR.Atanh((MPFR)args[0]) ),

            ( "min",    1, -1,  (args) => MPFR.Min(args.Select(e => (MPFR)e).ToArray()) ),
            ( "max",    1, -1,  (args) => MPFR.Max(args.Select(e => (MPFR)e).ToArray()) ),

            ( "mean",   1, -1,  (args) => MPFR.Mean(args.Select(e => (MPFR)e).ToArray()) ),

            ( "rnd",    0, 2,   Random ),
            ( "rndi",   0, 2,   RandomInclusive ),

            ( "chem.M", 1,      (args) => ChemicalExpressions.MolarMass(args) ),

            ( "strlen", 1,      (args) => (MPFR)(int)StringLength(args) )
        };
        #endregion

        #region Info
        internal static (string Identifier, string Name, string Description)[] VariableInfo { get; } =
        {
            ( "Y",          @"Yotta",                       @"10^24" ),
            ( "Z",          @"Zetta",                       @"10^21" ),
            ( "E",          @"Exa",                         @"10^18" ),
            ( "P",          @"Peta",                        @"10^15" ),
            ( "T",          @"Tera",                        @"10^12" ),
            ( "G",          @"Giga",                        @"10^9" ),
            ( "M",          @"Mega",                        @"10^6" ),
            ( "k",          @"Kilo",                        @"10^3" ),
            ( "h",          @"Hecto",                       @"10^2" ),
            ( "da",         @"Deca",                        @"10^1" ),
            ( "d",          @"Deci",                        @"10^-1" ),
            ( "c",          @"Centi",                       @"10^-2" ),
            ( "m",          @"Milli",                       @"10^-3" ),
            ( "u",          @"Micro",                       @"10^-6" ),
            ( "n",          @"Nano",                        @"10^-9" ),
            ( "p",          @"Pico",                        @"10^-12" ),
            ( "f",          @"Femto",                       @"10^-15" ),
            ( "a",          @"Atto",                        @"10^-18" ),
            ( "z",          @"Zepto",                       @"10^-21" ),
            ( "y",          @"Yocto",                       @"10^-24" ),

            ( "pc",         @"Percent",                     @"10^-2" ),
            ( "pm",         @"Permille",                    @"10^-3" ),
            ( "ptt",        @"Parts per ten thousand",      @"10^-4" ),
            ( "ppm",        @"Parts per million",           @"10^-6" ),
            ( "ppb",        @"Parts per billion",           @"10^-9" ),
            ( "ppt",        @"Parts per trillion",          @"10^-12" ),
            ( "ppq",        @"Parts per quadrillion",       @"10^-15" ),

            ( "math.pi",    @"Pi",                          @"~3.14159" ),
            ( "math.E",     @"Euler-Mascheroni constant",   @"~0.57721" ),
            ( "math.G",     @"Catalan's constant",          @"~0.91596" ),
            ( "math.ln2",   @"Logarithm of 2",              @"~0.69314" ),
            ( "math.e",     @"Euler's number",              @"Mathematical constant 'e' (~2.71828)" ),

            ( "phys.NA",    @"Avogadro constant",           @"6.02214076*10^23" ),
            ( "phys.au",    @"Astronomical unit of length", @"Defined in 'metres'" ),
            ( "phys.au",    @"Astronomical unit of time",   @"Defined in 'seconds'" ),
            ( "phys.au",    @"Astronomical unit of mass",   @"Defined in 'kilograms'" ),
            ( "phys.ly",    @"Light year",                  @"Defined in 'metres'" ),
            ( "phys.pc",    @"Parsec",                      @"Defined in 'metres'" ),
            ( "phys.c",     @"Speed of light",              @"Defined in 'm/s'" ),
            ( "phys.G",     @"Gravitational constant",      @"Defined in 'm^3*kg^-1*s^-2'" ),
            ( "phys.g",     @"Gravitational acceleration",  @"Defined in 'm/s^2'" ),
            ( "phys.R",     @"Gas constant",                @"Defined in 'J*K^-1*mol^-1'" ),

            ( "googol",     @"Googol",                      @"10^100" )
        };

        internal static (string Identifier, string Name, string Description)[] FunctionInfo { get; } =
        {
            ( "ans",    @"Answer/Result",               @"Result of a previous calculation" ),

            ( "trunc",  @"Truncation",                  @"Truncates the fractional part" ),
            ( "abs",    @"Absolute value",              @"+x" ),
            ( "neg",    @"Negative value",              @"-x" ),
            ( "neg2",   @"Negative value (explicit)",   @"-x" ),
            ( "pow",    @"Power",                       @"b^n" ),
            ( "sqr",    @"Square",                      @"Square of a value" ),
            ( "cb",     @"Cube",                        @"Cube of a value" ),
            ( "exp",    @"Natural exponent",            @"e^n" ),
            ( "exp2",   @"Binary exponent",             @"2^n" ),
            ( "exp10",  @"Decimal exponent",            @"10^n" ),
            ( "root",   @"Root",                        @"Nth root" ),
            ( "sqrt",   @"Square root",                 @"Square root" ),
            ( "cbrt",   @"Cubic root",                  @"Cubic root" ),
            ( "logn",   @"Logarithm (base 'n')",        @"Nth logarithm" ),
            ( "log",    @"Logarithm (base 'e')",        @"Natural logarithm" ),
            ( "log2",   @"Logarithm (base 2)",          @"Binary logarithm" ),
            ( "log10",  @"Logarithm (base 10)",         @"Decimal logarithm" ),

            ( "sin",    @"Sine",                        @"Trigonometric function" ),
            ( "cos",    @"Cosine",                      @"Trigonometric function" ),
            ( "tan",    @"Tangent",                     @"Trigonometric function" ),
            ( "cot",    @"Cotangent",                   @"Trigonometric function" ),
            ( "sec",    @"Secant",                      @"Trigonometric function" ),
            ( "csc",    @"Cosecant",                    @"Trigonometric function" ),
            ( "asin",   @"Arcsine",                     @"Inverse trigonometric function" ),
            ( "acos",   @"Arccosine",                   @"Inverse trigonometric function" ),
            ( "atan",   @"Arctangent",                  @"Inverse trigonometric function" ),
            ( "atan2",  @"2-argument arctangent",       @"Inverse trigonometric function" ),
            ( "asec",   @"Arcsecant",                   @"Inverse trigonometric function" ),
            ( "sinh",   @"Hyperbolic sine",             @"Hyperbolic trigonometric function" ),
            ( "cosh",   @"Hyperbolic cosine",           @"Hyperbolic trigonometric function" ),
            ( "tanh",   @"Hyperbolic tangent",          @"Hyperbolic trigonometric function" ),
            ( "coth",   @"Hyperbolic cotangent",        @"Hyperbolic trigonometric function" ),
            ( "sech",   @"Hyperbolic secant",           @"Hyperbolic trigonometric function" ),
            ( "csch",   @"Hyperbolic cosecant",         @"Hyperbolic trigonometric function" ),
            ( "asinh",  @"Hyperbolic arcsine",          @"Inverse hyperbolic trigonometric function" ),
            ( "acosh",  @"Hyperbolic arccosine",        @"Inverse hyperbolic trigonometric function" ),
            ( "atanh",  @"Hyperbolic arctangent",       @"Inverse hyperbolic trigonometric function" ),

            ( "min",    @"Minimum",                     @"Returns the smallest value in a set" ),
            ( "max",    @"Maximum",                     @"Returns the largest value in a set" ),

            ( "mean",   @"Mean",                        @"Returns the mean value in a set" ),

            ( "rnd",    @"Random",                      @"Uniform random value (min <= x < max)" ),
            ( "rndi",   @"Inclusive random",            @"Uniform random value (min <= x <= max)" ),

            ( "chem.M", @"Molar mass",                  @"Return the molar mass of a chemical compound formed string" ),

            ( "strlen", @"String length",               @"Returns the length of a string" )
        };
        #endregion

        private static object ArgumentHandler(object value)
        {
            if(value is MPZ tmp)
                return (MPFR)tmp;

            return value;
        }

        internal static ExpressionParser Parser { get; } = new ExpressionParser(UnaryOperators, BinaryOperators, Variables, Functions, Common.CustomVariables, Common.AssignmentOperator)
        {
            ShorthandOperator = ShorthandOperator,
            EscapeSequenceFormatter = Common.EscapeSequenceFormatter
        };

        static DefaultExpressions()
        {
            Parser.NumberConverter = Common.ParseMPFR;
            Parser.ArgumentHandler = ArgumentHandler;
        }
    }
}
