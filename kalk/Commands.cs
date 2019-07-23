using System;
using Math.Gmp.Native;
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
                            Common.Resuls.Clear();
                            break;
                        case "variables":
                            Common.CustomVariables.Clear();
                            break;
                        case "all":
                            Common.CustomVariables.Clear();
                            Common.Resuls.Clear();
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

        private static object InputBase(params string[] args)
        {
            if(args.Length > 0)
            {
                int value = System.Convert.ToInt32(args[0]);
                if(value >= 2)
                    Common.InputBase = value;

                return null;
            }
            else
                return Common.InputBase;
        }

        private static object OutputBase(params string[] args)
        {
            if(args.Length > 0)
            {
                int value = System.Convert.ToInt32(args[0]);
                if(value >= 2)
                    MPZ.OutputBase = value;

                return null;
            }
            else
                return MPZ.OutputBase;
        }

        private static object Base(params string[] args)
        {
            if(args.Length > 0)
            {
                InputBase(args);
                OutputBase(args);
                return null;
            }
            else
                return $"{Common.InputBase}, {MPZ.OutputBase}";
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

        private static object SwitchMode(params string[] args)
        {
            Program.SwitchMode();

            string mode = string.Empty;
            if(Program.CurrentParser == DefaultExpressions.Parser)
                mode = "Default";
            else if(Program.CurrentParser == BinaryExpressions.Parser)
                mode = "Binary";

            if(args.Length > 0)
            {
                if(args.Length == 1)
                {
                    Base(args[0]);
                }
                else
                {
                    InputBase(args[0]);

                    if(args.Length > 1)
                        OutputBase(args[1]);
                }
            }

            Console.WriteLine($"Switched to '{mode}' mode");
            return null;
        }
        #endregion

        internal static CommandParser Parser { get; } = new CommandParser(Common.EscapeSequenceFormatter)
        {
            { "exit", Exit },
            { "clear", Clear },

            { "switch", SwitchMode },

            { "prec", Precision },
            { "rmode", RoundingMode },
            { "rmodes", (args) => Program.GetRoundingModesInfo() },
            { "oprec", OutputPrecision },
            { "obase", OutputBase },
            { "ibase", InputBase },
            { "base", Base },
            { "seed", Seed },
            { "seedstr", SeedString },

            { "ans", (args) => Common.Ans(args) },
            { "nans", (args) => Common.Resuls.Count },

            { "list", PrintInfo },
        };
    }
}
