using System;
using System.Linq;
using Math.Mpfr.Native;
using Libs.Collections;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;

namespace kalk
{
    internal static class ArithmeticExpressions
    {
        #region Custom methods
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
        #endregion

        #region Operators
        private static readonly ExtendedDictionary<char, UnaryOperator> UnaryOperators = new ExtendedDictionary<char, UnaryOperator>((value) => value.Identifier)
        {
            ( '+', 1, AssociativityType.Right,  (value) => +(MPFR_Value)value ),
            ( '-', 1, AssociativityType.Right,  (value) => -(MPFR_Value)value )
        };

        private static readonly ExtendedDictionary<string, BinaryOperator> BinaryOperators = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
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

        private static readonly BinaryOperator ShorthandOperator = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPFR_Value)lhs * (MPFR_Value)rhs);
        #endregion

        #region Variables/Functions
        private static readonly ExtendedDictionary<string, Variable> Variables = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
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

        private static readonly ExtendedDictionary<string, Function> Functions = new ExtendedDictionary<string, Function>((value) => value.Identifier)
        {
            ( "ans",        0, 1,   Common.Ans ),
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

            ( "chem.M",     1,      (args) => ChemicalExpressions.MolarMass(args) ),

            ( "strlen",     1,      (args) => (MPFR_Value)(int)StringLength(args) )
        };
        #endregion

        #region Info
        internal static (string Identifier, string Name, string Description)[] VariableInfo { get; } =
        {
            ( "Y",          @"Yotta",                           @"10^24" ),
            ( "Z",          @"Zetta",                           @"10^21" ),
            ( "E",          @"Exa",                             @"10^18" ),
            ( "P",          @"Peta",                            @"10^15" ),
            ( "T",          @"Tera",                            @"10^12" ),
            ( "G",          @"Giga",                            @"10^9" ),
            ( "M",          @"Mega",                            @"10^6" ),
            ( "k",          @"Kilo",                            @"10^3" ),
            ( "h",          @"Hecto",                           @"10^2" ),
            ( "da",         @"Deca",                            @"10^1" ),
            ( "d",          @"Deci",                            @"10^-1" ),
            ( "c",          @"Centi",                           @"10^-2" ),
            ( "m",          @"Milli",                           @"10^-3" ),
            ( "u",          @"Micro",                           @"10^-6" ),
            ( "n",          @"Nano",                            @"10^-9" ),
            ( "p",          @"Pico",                            @"10^-12" ),
            ( "f",          @"Femto",                           @"10^-15" ),
            ( "a",          @"Atto",                            @"10^-18" ),
            ( "z",          @"Zepto",                           @"10^-21" ),
            ( "y",          @"Yocto",                           @"10^-24" ),

            ( "math.pi",    @"Pi",                              @"~3.14159" ),
            ( "math.E",     @"Euler-Mascheroni constant",       @"~0.57721" ),
            ( "math.G",     @"Catalan's constant",              @"~0.91596" ),
            ( "math.ln2",   @"Logarithm of 2",                  @"~0.69314" ),
            ( "math.e",     @"Euler's number",                  @"Mathematical constant 'e' (~2.71828)" ),

            ( "phys.NA",    @"Avogadro constant",               @"6.02214076*10^23" ),
            ( "phys.au",    @"Astronomical_unit",               @"Defined in 'metres'" ),
            ( "phys.ly",    @"Light year",                      @"Defined in 'metres'" ),
            ( "phys.pc",    @"Parsec",                          @"Defined in 'metres'" ),
            ( "phys.c",     @"Speed of light",                  @"Defined in 'metres'" ),
            ( "phys.G",     @"Gravitational constant",          @"Defined in 'm^3*kg^-1*s^-2'" ),
            ( "phys.g",     @"Gravitational acceleration",      @"Defined in 'm/s^2'" ),
            ( "phys.R",     @"Gas constant",                    @"Defined in 'J*K^-1*mol^-1'" ),

            ( "googol",     @"Googol",                          @"10^100" )
        };

        internal static (string Identifier, string Name, string Description)[] FunctionInfo { get; } =
        {
            ( "ans",    @"Answer/Result",           @"Result of a previous calculation" ),

            ( "abs",    @"Absolute value",          @"+x" ),
            ( "neg",    @"Negative value",          @"-x" ),
            ( "pow",    @"Power",                   @"b^n" ),
            ( "root",   @"Root",                    @"Nth root of a value" ),
            ( "sqrt",   @"Square root",             @"Square root of a value" ),
            ( "cbrt",   @"Cubic root",              @"Cubic root of a value" ),
            ( "log",    @"Logarithm ( base 'e')",   @"Natural logarithm" ),
            ( "log2",   @"Logarithm (base 2)",      @"Binary logarithm" ),
            ( "log10",  @"Logarithm (base 10)",     @"Common logarithm" ),

            ( "sin",    @"Sine",                    @"Trigonometric function" ),
            ( "cos",    @"Cosine",                  @"Trigonometric function" ),
            ( "tan",    @"Tangent",                 @"Trigonometric function" ),
            ( "asin",   @"Arcsine",                 @"Inverse trigonometric function" ),
            ( "acos",   @"Arccosine",               @"Inverse trigonometric function" ),
            ( "atan",   @"Arctangent",              @"Inverse trigonometric function" ),
            ( "sinh",   @"Hyperbolic sine",         @"Hyperbolic trigonometric function" ),
            ( "cosh",   @"Hyperbolic cosine",       @"Hyperbolic trigonometric function" ),
            ( "tanh",   @"Hyperbolic tangent",      @"Hyperbolic trigonometric function" ),
            ( "asinh",  @"Hyperbolic arcsine",      @"Inverse hyperbolic trigonometric function" ),
            ( "acosh",  @"Hyperbolic arccosine",    @"Inverse hyperbolic trigonometric function" ),
            ( "atanh",  @"Hyperbolic arctangent",   @"Inverse hyperbolic trigonometric function" ),

            ( "min",    @"Minimum",                 @"Returns the smallest value in a set" ),
            ( "max",    @"Maximum",                 @"Returns the largest value in a set" ),

            ( "rnd",    @"Random",                  @"Uniform random value (0 <= x < 1)" ),
            ( "rndi",   @"Inclusive random",        @"Uniform random value (0 <= x <= 1)" ),

            ( "chem.M", @"Molar mass",              @"Return the molar mass of a chemical compound formed string" ),

            ( "strlen", @"String length",           @"Returns the length of a string" ),
        };
        #endregion

        internal static ExpressionParser Parser { get; } = new ExpressionParser(UnaryOperators, BinaryOperators, Variables, Functions, ShorthandOperator, Common.AssignmentOperator, Common.EscapeSequenceFormatter);

        static ArithmeticExpressions()
        {
            Parser.NumberConverter = Common.ParseMPFR;
        }
    }
}
