using System;
using Math.Gmp.Native;
using Math.Mpfr.Native;
using NDesk.Options;
using Libs.Text.Parsing;
using Libs.Extensions;

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
                        case "all": case "*":
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
                    Common.OutputBase = value;

                return null;
            }
            else
                return Common.OutputBase;
        }

        private static object Base(params string[] args)
        {
            if(args.Length >= 2)
            {
                InputBase(args[0]);
                OutputBase(args[1]);
                return null;
            }
            else if(args.Length == 1)
            {
                InputBase(args);
                OutputBase(args);
                return null;
            }
            else
                return $"{Common.InputBase}, {Common.OutputBase}";
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

        private static object Ans(params string[] args)
        {
            int start = Common.Resuls.Count - 1;
            int end = Common.Resuls.Count;

            ((int Value, bool IsSet) StartIndex, (int Value, bool IsSet) EndIndex, (int Value, bool IsSet) Count, bool Verbose) options = ((int.MinValue, false), (int.MinValue, false), (int.MinValue, false), false);

            var optionSet = new OptionSet()
            {
                { "s=", "Start index",  (int v) => {
                    options.StartIndex.Value = v;
                    options.StartIndex.IsSet = true;
                } },
                { "e=", "End index",    (int v) => {
                    options.EndIndex.Value = v;
                    options.EndIndex.IsSet = true;
                } },
                { "n=", "Count",        (int v) => {
                    options.Count.Value = v;
                    options.Count.IsSet = true;
                } },
                { "v",  "Verbose",      v => options.Verbose = true },
                { "<>",                 v => {
                    switch(v)
                    {
                        case "all": case "*":
                            start = 0;
                            end = Common.Resuls.Count;
                            break;

                        default:
                            options.EndIndex.Value = options.StartIndex.Value = int.Parse(v);
                            options.EndIndex.IsSet = options.StartIndex.IsSet = true;
                            break;
                    }
                } }
            };

            try
            {
                optionSet.Parse(args);
            }
            catch(OptionException e)
            {
                Console.WriteLine($@"{e.Message}");
                return null;
            }

            if(options.StartIndex.IsSet)
            {
                start = options.StartIndex.Value < 0 ? (Common.Resuls.Count + options.StartIndex.Value) : options.StartIndex.Value;
            }

            if(options.EndIndex.IsSet)
            {
                end = (options.EndIndex.Value < 0 ? (Common.Resuls.Count + options.EndIndex.Value) : options.EndIndex.Value) + 1;
            }

            if(options.Count.IsSet)
            {
                end = start + options.Count.Value;
            }

            if(end > Common.Resuls.Count)
                end = Common.Resuls.Count;

            if(start < 0)
                start = 0;
            else if(start > end)
                start = end;

            if(end - start == 1)
            {
                Console.WriteLine($@"{Common.Resuls[start].Result}{(options.Verbose ? $" ({Common.Resuls[start].Expression})" : null)}");
            }
            else
            {
                int padLength1 = (Common.Resuls.Count - 1).Length();
                int padLength2 = Common.Resuls.Count.Length() + 1;
                for(int i = start; i < end; i++)
                {
                    Console.WriteLine($@"[{i.ToString().PadLeft(padLength1)}, {(-(Common.Resuls.Count - i)).ToString().PadLeft(padLength2)}] = {Common.Resuls[i].Result}{(options.Verbose ? $" ({Common.Resuls[i].Expression})" : null)}");
                }
            }

            return null;
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

            { "ans", Ans },
            { "nans", (args) => Common.Resuls.Count },

            { "list", PrintInfo },
        };
    }
}
