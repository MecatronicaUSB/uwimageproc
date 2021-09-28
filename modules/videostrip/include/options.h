/**
 * @file options.h
 * @brief Argument parser options based on args.hxx
 * @version 1.0
 * @date 20/01/2018
 * @author Victor Garcia
 * @author Jose Cappelletto
 */

#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <iostream>
#include "../../common/args.hxx"

args::ArgumentParser 	argParser("","");
args::HelpFlag 	argHelp(argParser, "help", "Display this help menu", {'h', "help"});
//CompletionFlag completion(cliParser, {"complete"});	//TODO: figure out why is missing in current version of args.hxx
args::ValueFlag	<int> 		argWindowSize(argParser, "window size", "Size of the search window for the best frame", {'k', "windowSize"});
args::ValueFlag	<int> 		argTimeSkip(argParser, "time skip", "Time (in seconds) skipped from the start of video", {'s', "timeSkip"});
args::ValueFlag	<double> 	argOverlap(argParser, "overlap", "Desired maximum overlap among frames", {'p',"minOverlap"});
args::Positional<std::string> 	argInput(argParser, "input", "Input file name");
args::Positional<std::string> 	argOutput(argParser, "output", "Prefix for output JPG image files");
args::ValueFlag <bool>		argReport(argParser, "report", "Generate report file containing detailed information for each exported frame", {'r', "--report"});
args::ValueFlag <bool>		argFeatures(argParser, "features", "Export features in PTO compatible format", {'f', "--features"});

#endif

const string green("\033[1;32m");
const string yellow("\033[1;33m");
const string cyan("\033[1;36m");
const string red("\033[1;31m");
const string reset("\033[0m");
