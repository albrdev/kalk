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
        #region Operators
        private static readonly ExtendedDictionary<char, UnaryOperator> UnaryOperators = new ExtendedDictionary<char, UnaryOperator>((value) => value.Identifier)
        {
            ( '-', 1, AssociativityType.Right, (value) => -(MPZ)value ),

            ( '!', 1, AssociativityType.Right, (value) => (MPZ)value != 0 ? 0 : 1 ),
            ( '~', 1, AssociativityType.Right, (value) => ~(MPZ)value )
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
            ( ">>>", 2, AssociativityType.Left, (lhs, rhs) => MPZ.RightShift2((MPZ)lhs, (MPZ)rhs) )
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
            ( "ans",    0, 1,   Common.Ans )
        };
        #endregion

        internal static object ArgumentHandler(object value)
        {
            if(value is MPFR tmp)
                return (MPZ)tmp;

            return value;
        }

        internal static ExpressionParser Parser { get; } = new ExpressionParser(UnaryOperators, BinaryOperators, Variables, Functions, ShorthandOperator, Common.AssignmentOperator, Common.EscapeSequenceFormatter);

        static BinaryExpressions()
        {
            Parser.NumberConverter = Common.ParseMPZ;
            Parser.ArgumentHandler = ArgumentHandler;
        }
    }
}
