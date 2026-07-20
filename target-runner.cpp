#include <dirent.h>
#include <cstdlib>
#include <string>
#include <fstream>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    std::ifstream file;
	std::ofstream fileR;


    int s;

    std::string aux = "./Main ";
    std::string cmd = aux +  argv[5] + " " + argv[6] + " " + argv[7] + " " + argv[8] + " " + argv[9] + " " + argv[10] + " " + argv[11] + " " + argv[12] + " " + argv[13] + " " + argv[14] + " " + argv[15] + " " + argv[16] + " " + argv[17] + " " + argv[18]  + " <" + argv[4];
    const char * c = cmd.c_str();
    s = system(c);
	return 0;
}
