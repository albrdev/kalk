using System;
using Math.Mpfr.Native;
using Libs.Text.Parsing;

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
                MPFR.DefaultPrecision = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR.DefaultPrecision;
        }

        private static object OutputPrecision(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR.OutputPrecision = System.Convert.ToInt32(args[0]);
                return null;
            }
            else
                return MPFR.OutputPrecision;
        }

        private static object RoundingMode(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR.RoundingMode = Program.ParseRoundingMode(args[0].ToString());
                return null;
            }
            else
                return $"{MPFR.RoundingMode} ({(int)MPFR.RoundingMode})";
        }

        private static object Seed(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR.RandomState = System.Convert.ToUInt32(args[0]);
                return null;
            }
            else
                return MPFR.RandomState;
        }

        private static object SeedString(params string[] args)
        {
            if(args.Length > 0)
            {
                MPFR.RandomState = args[0];
                return null;
            }
            else
                return MPFR.RandomState;
        }

        private static object PrintInfo(params string[] args)
        {
            Program.PrintInfo(args.Length > 0 ? args[0] : null);
            return null;
        }
        #endregion

        internal static CommandParser Parser { get; } = new CommandParser(Common.EscapeSequenceFormatter)
        {
            { "exit", Exit },
            { "clear", Clear },

            { "prec", Precision },
            { "rmode", RoundingMode },
            { "rmodes", (args) => Program.GetRoundingModesInfo() },
            { "oprec", OutputPrecision },
            { "seed", Seed },
            { "seedstr", SeedString },

            { "ans", (args) => Common.Ans(args) },
            { "nans", (args) => Common.ResultVariables.Count },

            { "list", PrintInfo },
        };
    }
}
