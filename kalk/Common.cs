using System;
using System.Collections.Generic;
using System.Linq;
using Math.Gmp.Native;
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
            if(!Resuls.Any())
                throw new System.IndexOutOfRangeException($@"List contains no elements");

            if(!args.Any())
                return Resuls.Last().Result;

            int index = System.Convert.ToInt32(args[0]);
            if(index < 0)
                index = Resuls.Count + index;

            if(index < 0 || index >= Resuls.Count)
                throw new System.IndexOutOfRangeException($@"Index out of range (Count: {Resuls.Count})");

            return Resuls[index].Result;
        }
        #endregion

        internal static object ParseMPFR(string value) => new MPFR(value, InputBase);
        internal static object ParseMPZ(string value) => new MPZ(value, InputBase);
        internal static int InputBase { get; set; } = 10;
        internal static int OutputBase
        {
            get { return MPZ.OutputBase; /*return MPFR.OutputBase;*/ }
            set { MPFR.OutputBase = MPZ.OutputBase = value; }
        }

        internal static BinaryOperator AssignmentOperator { get; } = ("=", 1, AssociativityType.Right, (lhs, rhs) => (((Variable)lhs).Value = rhs));
        internal static EscapeSequenceFormatter EscapeSequenceFormatter { get; } = new ExtendedNativeEscapeSequenceFormatter();

        internal static Dictionary<string, Variable> CustomVariables { get; } = new Dictionary<string, Variable>();
        internal static List<(string Expression, object Result)> Resuls { get; } = new List<(string Expression, object Result)>();
    }
}
