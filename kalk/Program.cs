using System;
using System.Collections.Generic;
using System.IO;
using Math.Mpfr.Native;
using NDesk.Options;
using Libs.Utilities;

namespace kalk
{
    class Program
    {
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

        internal static mpfr_rnd_t ParseRoundingMode(string value)
        {
            if(EnumUtilities.TryParse(value, out mpfr_rnd_t result))
                return result;

            return EnumUtilities.Parse<mpfr_rnd_t>("MPFR_" + value);
        }

        static int Main(string[] args)
        {
            (List<string> Expressions, mpfr_prec_t Precision, mpfr_rnd_t RoundingMode, int OutputPrecision, string Seed, string SeedString, bool InteractiveMode, bool PrintUsage) options = (new List<string>(), 1024, default, 128, null, null, false, false);

            var optionSet = new OptionSet()
            {
                { "x|expr=", "Expression", v => options.Expressions.Add(v) },
                { "p|prec=", "Precision", (uint v) => options.Precision = v },
                { "r|rmode=", "Rounding mode", v => options.RoundingMode = ParseRoundingMode(v) },
                { "n|oprec=", "Ouput precision (decimal count)", (int v) => options.OutputPrecision = v },
                { "z|seed=", "Random seed", v => options.Seed = v },
                { "Z|seedstring=", "Random seed string", v => options.SeedString = v },
                { "i|interactive", "Interactive mode", v => options.InteractiveMode = v != null },
                { "h|help",  "Prints usage", v => options.PrintUsage = v != null },
                { "<>", v => options.Expressions.Add(v) }
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

            if(options.SeedString != null)
            {
                MPFR_Value.RandomState = options.SeedString;
            }
            else if(options.Seed != null)
            {
                MPFR_Value.RandomState = uint.Parse(options.Seed);
            }

            MPFR_Value.DefaultPrecision = options.Precision;
            MPFR_Value.RoundingMode = options.RoundingMode;
            MPFR_Value.OutputPrecision = options.OutputPrecision;

            foreach(var expr in options.Expressions)
            {
                var result = ArithmeticExpressions.Parser.Evaluate(expr);
                if(!options.InteractiveMode)
                    Common.ResultVariables.Add(result);

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
                            result = ArithmeticExpressions.Parser.Evaluate(input);
                            Common.ResultVariables.Add(result);
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
