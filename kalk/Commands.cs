using System;
using Math.Mpfr.Native;
using Libs.Text.Parsing;
using Libs.Text.Formatting;

namespace kalk
{
    internal static class Commands
    {
        #region Custom methods
        private static object Exit(params string[] args)
        {
            Environment.Exit(0);
            return null;
        }

        private static object Clear(params string[] args)
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
                            Common.ResultVariables.Clear();
                            break;
                        case "variables":
                            ArithmeticExpressions.Parser.ClearAssignedVariables();
                            break;
                        case "all":
                            ArithmeticExpressions.Parser.ClearAssignedVariables();
                            Common.ResultVariables.Clear();
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

        private static object Precision(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.DefaultPrecision = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.DefaultPrecision;
        }

        private static object DigitCount(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.DigitOutputCount = System.Convert.ToInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.DigitOutputCount;
        }

        private static object RoundingMode(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RoundingMode = Program.ParseRoundingMode(args[0].ToString());
                return null;
            }
            else
                return $"{MPFR_Value.RoundingMode} ({(int)MPFR_Value.RoundingMode})";
        }

        private static object Seed(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RandomState = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR_Value.RandomState;
        }

        private static object SeedString(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR_Value.RandomState = args[0];
                return null;
            }
            else
                return MPFR_Value.RandomState;
        }
        #endregion

        internal static CommandParser Parser { get; } = new CommandParser(Common.EscapeSequenceFormatter)
        {
            { "exit", Exit },
            { "clear", Clear },

            { "prec", Precision },
            { "rmode", RoundingMode },
            { "dgtcnt", DigitCount },
            { "seed", Seed },
            { "seedstr", SeedString },

            { "ans", (args) => Common.Ans(args) },
            { "nans", (args) => Common.ResultVariables.Count }
        };
    }
}
