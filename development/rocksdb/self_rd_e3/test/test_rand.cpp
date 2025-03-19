 //gen random number
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <chrono>
#include <regex>
#include <thread>
#include <cstdlib>

using namespace std;

int main()
{
    // srand(time(NULL));
    for(int i = 0; i < 10000; i++){
        int random = rand() % 100000 + 1;
        cout << random << endl;
    }

    //store random number in a file
    ofstream myfile;
    myfile.open ("random.txt");
    for(int i = 0; i < 10000; i++){
        int random = rand() % 100000 + 1;
        myfile << random << endl;
    } 
}
// int random = rand() % 100000 + 1;
