using System;
using System.Collections.Generic;
using System.Linq;
using Math.Mpfr.Native;
using Libs.Text.Parsing;
using static Libs.Text.Parsing.Operator;
using Libs.Text.Formatting;

namespace kalk
{
    internal static class Common
    {
        #region Custom methods
        internal static object Ans(params object[] args)
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
        #endregion

        internal static object ParseMPFR(string value) => new MPFR_Value(value);

        internal static UnaryOperator AssignmentOperator { get; } = ('=', 1, AssociativityType.Right, (value) => value);
        internal static EscapeSequenceFormatter EscapeSequenceFormatter { get; } = new ExtendedNativeEscapeSequenceFormatter();

        internal static List<object> ResultVariables { get; } = new List<object>();
    }
}
