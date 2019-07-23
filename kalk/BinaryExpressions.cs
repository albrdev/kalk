using System;
using Math.Gmp.Native;
using Math.Mpfr.Native;
using Libs.Collections;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;

namespace kalk
{
    internal static class BinaryExpressions
    {
        #region Custom methods
        private static object Random(params object[] args)
        {
            if(args.Length >= 2)
                return MPZ.URandom((MPZ)args[0], (MPZ)args[1], false);
            else if(args.Length >= 1)
                return MPZ.URandom((MPZ)args[0], false);
            else
                throw new System.ArgumentException();
        }

        private static object RandomInclusive(params object[] args)
        {
            if(args.Length >= 2)
                return MPZ.URandom((MPZ)args[0], (MPZ)args[1], true);
            else if(args.Length >= 1)
                return MPZ.URandom((MPZ)args[0], true);
            else
                throw new System.ArgumentException();
        }
        #endregion

        #region Operators
        private static readonly ExtendedDictionary<char, UnaryOperator> UnaryOperators = new ExtendedDictionary<char, UnaryOperator>((value) => value.Identifier)
        {
            ( '-', 1, AssociativityType.Right, (value) => -(MPZ)value ),

            ( '~', 1, AssociativityType.Right, (value) => ~(MPZ)value ),

            ( '!', 1, AssociativityType.Right,  (value) => value is bool ? !(bool)value : !(MPZ)value )
        };

        private static readonly ExtendedDictionary<string, BinaryOperator> BinaryOperators = new ExtendedDictionary<string, BinaryOperator>((value) => value.Identifier)
        {
            ( "+", 5, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs | (MPZ)rhs ),
            ( "*", 3, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs & (MPZ)rhs ),

            ( "|", 5, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs | (MPZ)rhs ),
            ( "&", 3, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs & (MPZ)rhs ),
            ( "^", 4, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs ^ (MPZ)rhs ),

            ( "<<", 2, AssociativityType.Right, (lhs, rhs) => (MPZ)lhs << (MPZ)rhs ),
            ( ">>", 2, AssociativityType.Left, (lhs, rhs) => (MPZ)lhs >> (MPZ)rhs ),

            ( "==", 5,  AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs == (MPZ)rhs ),
            ( "!=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs != (MPZ)rhs ),
            ( "<", 5,   AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs < (MPZ)rhs ),
            ( ">", 5,   AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs > (MPZ)rhs ),
            ( "<=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs <= (MPZ)rhs ),
            ( ">=", 5,  AssociativityType.Left,     (lhs, rhs) => (MPZ)lhs >= (MPZ)rhs ),

            ( "||", 6,  AssociativityType.Left,     (lhs, rhs) => System.Convert.ToBoolean(lhs) || System.Convert.ToBoolean(rhs) ),
            ( "&&", 6,  AssociativityType.Left,     (lhs, rhs) => System.Convert.ToBoolean(lhs) && System.Convert.ToBoolean(rhs) )
        };

        private static readonly BinaryOperator ShorthandOperator = ("*", 3, AssociativityType.Right, (lhs, rhs) => (MPZ)lhs & (MPZ)rhs);
        #endregion

        #region Variables/Functions
        private static readonly ExtendedDictionary<string, Variable> Variables = new ExtendedDictionary<string, Variable>((value) => value.Identifier)
        {
            ( "false",  0 ),
            ( "true",   1 ),

            ( "BM",     (MPZ)(uint)byte.MaxValue ),
            ( "bm",     (MPZ)sbyte.MinValue ),
            ( "bM",     (MPZ)sbyte.MaxValue ),

            ( "sm",     (MPZ)short.MinValue ),
            ( "sM",     (MPZ)short.MaxValue ),
            ( "SM",     (MPZ)(uint)ushort.MaxValue ),

            ( "im",     (MPZ)int.MinValue ),
            ( "iM",     (MPZ)int.MaxValue ),
            ( "IM",     (MPZ)uint.MaxValue ),

            ( "lm",     (MPZ)long.MinValue ),
            ( "lM",     (MPZ)long.MaxValue ),
            ( "LM",     (MPZ)ulong.MaxValue )
        };

        private static readonly ExtendedDictionary<string, Function> Functions = new ExtendedDictionary<string, Function>((value) => value.Identifier)
        {
            ( "ans",    0, 1,   Common.Ans ),

            ( "rnd",    1, 2,   Random ),
            ( "rndi",   1, 2,   RandomInclusive ),
            ( "brnd",   1,      (args) => MPZ.BRandom((MPZ)args[0]) )
        };
        #endregion

        #region Info
        internal static (string Identifier, string Name, string Description)[] VariableInfo { get; } =
        {
            ( "false",  @"False",               @"0" ),
            ( "true",   @"True",                @"1" ),

            ( "BM",     @"Unsigned byte max.",  @"10^18" ),
            ( "bm",     @"Signed byte min.",    @"10^15" ),
            ( "bM",     @"Singed byte max.",    @"10^12" ),

            ( "sm",     @"Signed short min.",   @"10^9" ),
            ( "sM",     @"Signed short max.",   @"10^6" ),
            ( "SM",     @"Unsigned short max.", @"10^3" ),

            ( "im",     @"Signed int min.",     @"10^2" ),
            ( "iM",     @"Signed int max.",     @"10^1" ),
            ( "IM",     @"Unsigned int max.",   @"10^-1" ),

            ( "lm",     @"Signed long min.",    @"10^-2" ),
            ( "lM",     @"Signed long max.",    @"10^-3" ),
            ( "LM",     @"Unsigned long max.",  @"10^-6" )
        };

        internal static (string Identifier, string Name, string Description)[] FunctionInfo { get; } =
        {
            ( "ans",    @"Answer/Result",       @"Result of a previous calculation" ),

            ( "rnd",    @"Random",              @"Uniform random value (min <= x < max)" ),
            ( "rndi",   @"Inclusive random",    @"Uniform random value (min <= x <= max)" ),
            ( "brnd",   @"Random (bit count)",  @"Uniform random value (0 <= x <= 2^max - 1)" )
        };
        #endregion

        internal static object ArgumentHandler(object value)
        {
            if(value is MPFR tmp)
                return (MPZ)tmp;

            return value;
        }

        internal static ExpressionParser Parser { get; } = new ExpressionParser(UnaryOperators, BinaryOperators, Variables, Functions, Common.CustomVariables, Common.AssignmentOperator)
        {
            ShorthandOperator = ShorthandOperator,
            EscapeSequenceFormatter = Common.EscapeSequenceFormatter
        };

        static BinaryExpressions()
        {
            Parser.NumberConverter = Common.ParseMPZ;
            Parser.ArgumentHandler = ArgumentHandler;
        }
    }
}
