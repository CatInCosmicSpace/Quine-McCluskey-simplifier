#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "Quine_McCluskey_Simplifier.hpp"
#include "log_expr.hpp"

int main(int argc, char* argv[]) {
	if (argc == 2) {
		if (std::string(argv[1]) == "-h" || std::string(argv[1]) == "-help") {
			std::cout << "usage: qms -input_mode -output_mode input_file output_file. \n\
input_mode can take one of the following values: \n \
-f\t if the function in the file is represented by the formula\n \
-s\t if the function in the file is represented by a set of sets on which it is equal to the truth\n \
-v\t if the function in the file is represented by a vector of values\n \
output_mode can take one of the following values: \n \
-f\t for representation by the formula\n \
-s\t for representation by symbols -, 1 and 0 for lack of x, x and not x in the disjuncts\n";
		}
	}
	else if (argc == 5) {
		if (!((std::string(argv[1]) == "-f" ||
			std::string(argv[1]) == "-s" ||
			std::string(argv[1]) == "-v") &&
			(std::string(argv[2]) == "-f" ||
				std::string(argv[2]) == "-s")))
			throw std::logic_error("Invalid flags. Please see the help with -h or -help.");
		std::ifstream input_file(argv[3]);
		std::ofstream output_file(argv[4]);
		if (!(input_file.is_open() || output_file.is_open()))
			throw std::logic_error("Can not open files. Please check your files and try again.");
		std::stringstream ss;
		std::string input_string;
		Quine_McCluskey_Simplifier QMS;

		if (std::string(argv[1]) == "-f") {
			std::getline(input_file, input_string);
			log_expr le(input_string);
			le.print(ss);
			QMS.init(ss, false);
		}
		else if (std::string(argv[1]) == "-s") {
			QMS.init(input_file, true);
		}
		else if (std::string(argv[1]) == "-v") {
			QMS.init(input_file, false);
		}
		input_file.close();
		QMS.simplify();

		if (std::string(argv[2]) == "-f") {
			QMS.print_formula(output_file);
		}
		else if (std::string(argv[2]) == "-s") {
			QMS.print_mdnf(output_file);
		}
		output_file.close();
		std::cout << "Done." << std::endl;
	}
	else {
		std::cout << "Unexpected usage. Try -h or -help to see help.\n";
	}
	return 0;
}
