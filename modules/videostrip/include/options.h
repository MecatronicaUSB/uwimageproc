/**
 * @file options.h
 * @brief Argument parser options based on args.hxx
 * @version 1.0
 * @date 20/01/2018
 * @author Victor Garcia
 * @author Jose Cappelletto
 */

#ifndef _OPTIONS_
#define _OPTIONS_

#include <iostream>
#include "../../common/args.hxx"

args::ArgumentParser 	argParser("","");
args::HelpFlag 	argHelp(argParser, "help", "Display this help menu", {'h', "help"});
//CompletionFlag completion(cliParser, {"complete"});	//TODO: figure out why is missing in current version of args.hxx
args::ValueFlag	<int> 		argWindowSize(argParser, "window size", "Size of the search window for the best frame", {'k'});
args::ValueFlag	<int> 		argTimeSkip(argParser, "time skip", "Time (in seconds) skipped from the start of video", {'s'});
args::ValueFlag	<double> 	argOverlap(argParser, "overlap", "Desired maximum overlap among frames", {'p'});
args::Positional<std::string> 	argInput(argParser, "input", "Input file name");
args::Positional<std::string> 	argOutput(argParser, "output", "Prefix for output JPG image files");

#endif
