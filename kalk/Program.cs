using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using Math.Gmp.Native;
using Math.Mpfr.Native;
using NDesk.Options;
using Libs.Utilities;
using Libs.Text.Parsing;
using Libs.Settings;

namespace kalk
{
    class Program
    {
        internal static VersionData Version { get; } = new VersionData(0, 1, 0, VersionData.RevisionType.a);

        internal static mpfr_rnd_t ParseRoundingMode(string value)
        {
            if(EnumUtilities.TryParse(value, out mpfr_rnd_t result))
                return result;

            return EnumUtilities.Parse<mpfr_rnd_t>("MPFR_" + value);
        }

        internal static string GetRoundingModesInfo()
        {
            StringBuilder result = new StringBuilder();

            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDN}\t- {"Round to nearest, with ties to even"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDZ}\t- {"Round toward zero"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDU}\t- {"Round toward +Infinity"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDD}\t- {"Round toward -Infinity"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDA}\t- {"Round away from zero"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDF}\t- {"Faithful rounding"}");
            result.AppendLine($"{mpfr_rnd_t.MPFR_RNDNA}\t- {"Round to nearest, with ties away from zero"}");

            return result.ToString();
        }

        internal static void PrintInfo(string pattern = null)
        {
            Regex regex = new Regex($@"{(string.IsNullOrWhiteSpace(pattern) ? "." : Regex.Escape(pattern))}+");

            Console.WriteLine("Variables:");
            foreach(var (Identifier, Name, Description) in ArithmeticExpressions.VariableInfo)
            {
                if(regex.IsMatch(Identifier) || regex.IsMatch(Name) || regex.IsMatch(Description))
                {
                    Console.WriteLine($"{Identifier}");
                    Console.WriteLine($"  {Name} - {Description}\n");
                }
            }

            Console.WriteLine("Functions:");
            foreach(var (Identifier, Name, Description) in ArithmeticExpressions.FunctionInfo)
            {
                if(regex.IsMatch(Identifier) || regex.IsMatch(Name) || regex.IsMatch(Description))
                {
                    Console.WriteLine($"{Identifier}");
                    Console.WriteLine($"  {Name} - {Description}\n");
                }
            }
        }

        private static string ApplicationName
        {
            get => Path.GetFileNameWithoutExtension(Environment.GetCommandLineArgs()[0]);
        }

        private static void PrintUsage(OptionSet optionsSet)
        {
            Console.WriteLine($"Usage: {ApplicationName} [-p precision] [-r roundingmode] [-z seed] expression");
            Console.WriteLine("Command line calculator");
            Console.WriteLine();
            Console.WriteLine("Options:");
            optionsSet.WriteOptionDescriptions(Console.Out);
        }

        internal static ExpressionParser CurrentParser { get; private set; } = ArithmeticExpressions.Parser;

        internal static void SwitchMode() => Program.CurrentParser = Program.CurrentParser == ArithmeticExpressions.Parser ? BinaryExpressions.Parser : ArithmeticExpressions.Parser;

        static int Main(string[] args)
        {
            (List<string> Expressions, mpfr_prec_t Precision, mpfr_rnd_t RoundingMode, int OutputPrecision, int OutputBase, int InputBase, string Seed, string SeedString, bool BinaryMode, bool InteractiveMode, (bool Flag, string Pattern) PrintInfo, bool PrintUsage, bool PrintVersion) options = (new List<string>(), 1024, default, 128, 10, 10, null, null, false, false, (false, null), false, false);

            var optionSet = new OptionSet()
            {
                { "x|expr=",        "Expression",                                       v => options.Expressions.Add(v) },
                { "p|prec=",        "Precision",                                        (uint v) => options.Precision = v },
                { "r|rmode=",       "Rounding mode\n" + GetRoundingModesInfo(),         v => options.RoundingMode = ParseRoundingMode(v) },
                { "n|oprec=",       "Ouput precision (decimal count)",                  (int v) => options.OutputPrecision = v },
                { "b|obase=",       "Set ouput base",                                   (int v) => options.OutputBase = v },
                { "B|ibase=",       "Set input base",                                   (int v) => options.InputBase = v },
                { "base=",          "Set input/output base",                            (int v) => options.InputBase = options.OutputBase = v },
                { "z|seed=",        "Random seed",                                      v => options.Seed = v },
                { "Z|seedstring=",  "Random seed string",                               v => options.SeedString = v },
                { "binary",         "Binary mode",                                      v => options.BinaryMode = true },
                { "i|interactive",  "Interactive mode",                                 v => options.InteractiveMode = true },
                { "l|list:",        "Prints info about available variables/functions",  v => options.PrintInfo = (true, v) },
                { "h|help",         "Prints usage",                                     v => options.PrintUsage = true },
                { "version",        "Prints version information",                       v => options.PrintVersion = true },
                { "<>",                                                                 v => options.Expressions.Add(v) }
            };

            try
            {
                optionSet.Parse(args);
            }
            catch(OptionException e)
            {
                Console.WriteLine($"{ApplicationName}: {e.Message}");
                Console.WriteLine($@"Try '{ApplicationName} --help' for more information.");
                return 1;
            }

            if(options.PrintUsage)
            {
                PrintUsage(optionSet);
                return 0;
            }
            else if(options.PrintVersion)
            {
                Console.WriteLine($"{ApplicationName} v{Program.Version}");
                return 0;
            }
            else if(options.PrintInfo.Flag)
            {
                PrintInfo(options.PrintInfo.Pattern);
                return 0;
            }

            if(options.SeedString != null)
            {
                MPFR.RandomState = options.SeedString;
            }
            else if(options.Seed != null)
            {
                MPFR.RandomState = uint.Parse(options.Seed);
            }

            MPFR.DefaultPrecision = options.Precision;
            MPFR.RoundingMode = options.RoundingMode;
            MPFR.OutputPrecision = options.OutputPrecision;
            Common.InputBase = options.InputBase;
            MPZ.OutputBase = options.OutputBase;

            if(options.BinaryMode)
            {
                SwitchMode();
            }

            foreach(var expr in options.Expressions)
            {
                var result = CurrentParser.Evaluate(expr);
                if(!options.InteractiveMode)
                    Common.Resuls.Add(result);

                Console.WriteLine($"{result}");
            }

            if(options.InteractiveMode)
            {
                ReadLine.HistoryEnabled = false;
                while(true)
                {
                    string inputRaw = null;
                    inputRaw = ReadLine.Read("> ");

                    if(string.IsNullOrWhiteSpace(inputRaw))
                        continue;

                    string input = inputRaw.Trim();
                    object result;
                    if(input[0] == '!')
                    {
                        try
                        {
                            result = Commands.Parser.Execute(input.Remove(0, 1));
                        }
                        catch(Exception e)
                        {
                            result = $"*** Error: {e.Message}";
                        }
                    }
                    else
                    {
                        try
                        {
                            result = CurrentParser.Evaluate(input);
                            Common.Resuls.Add(result);
                        }
                        catch(Exception e)
                        {
                            result = $"*** Error: {e.Message}";
                        }
                    }

                    if(result != null)
                    {
                        Console.WriteLine($"{result}");
                    }

                    ReadLine.AddHistory(inputRaw);
                }
            }

            return 0;
        }
    }
}
