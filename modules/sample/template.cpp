/********************************************************************/
/* File: template.cpp                                                 */
/* Last Edition: 30/01/2017, 07:12 PM.                              */
/********************************************************************/
/* Programmed by:                                                   */
/* Jose Cappelletto                                                 */
/********************************************************************/
/* Project: imageproc								                */
/* File: 	template.cpp							                */
/********************************************************************/

#include <iostream>
#include <sstream>
#include <fstream>

/*
	Usage: Template -i input [-r argument -o output]
*/

using namespace std;

void printUsage(){
        cout << "'Template' for tool that provides a basic skeleton for modules in 'imageproc'" << endl;
        cout << "\tUsage: Template -i input [-r argument1 -o output]" << endl;
        cout << "\t-i\tModule input file" << endl;
        cout << "\t-h\tShow this help message" << endl;
        cout << "\t-r\t[optional] a random argument for testing purporses" << endl;
	    cout << "\t-o\t[optional] Output file (if not specified, a file will be created with from input filename)" << endl;
        cout <<  endl << "\tExample:" << endl;
        cout << "\t$ Template -i input.dat -o output.dat -r 0" << endl;
        cout << "\tThis will open 'input.dat' file, do some stuff and write output into 'output.dat' file" << endl;
}

//********************************
int main(int argc, char* argv[]){

	// If not enough arguments or '-h' argument is called, the print usage instructions
	if (argc<3 || (string (argv[1]) == "-h")){
		printUsage();
		return 0;
	}
	//read the arguments
	string argument, argval;
	string InputFile; //strings storing parameters
	ostringstream OutputFile;
	int opt = 0;
	float rand_arg;

	while ((opt = getopt(argc, argv, "i:r:o")) != -1) {
	switch(opt) {
		case 'i':
			InputFile = string (optarg);
			break;
		case 'r':
			rand_arg = atof (optarg);
			break;
		case 'o':
			OutputFile << optarg;
			break;
		case '?':
			/* Case when user enters the command as
			* $ ./cmd_exe -i
			*/
			if (optopt == 'i') {
				cout << "Missing mandatory input option" << endl;
			}
			else if (optopt == 'o') {
				cout << "Missing mandatory output option" << endl;
			} 
			else {
				cout << "Invalid option received" << endl;
				}
			break;
		}
	}

	// Example of how to parse input file name

	//gets the path of the input source
	string FileName = InputFile.substr(InputFile.find_last_of("/")+1);
	string BasePath = InputFile.substr(0,InputFile.length() - FileName.length());
	//determines the filetype
	string FileType = InputFile.substr(InputFile.find_last_of("."));
	string FileBase = FileName.substr(0,FileName.length() - FileType.length());

	cout << "Input: " << InputFile << endl;
	cout << "Path:  " << BasePath << endl;
	cout << "File:  " << FileName << endl;
	cout << "Type:  " << FileType << endl;
	cout << "Base:  " << FileBase << endl;

	cout << "Output:" << OutputFile.str().c_str() << endl;

//*****************************************************************************
	return 0;
}																																																											

