#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

int main() {
    string gen_workload_command = "../load_gen --insert=1000 --range_delete=10 --range_delete_selectivity=0.01 --entry_size=8; mv workload.txt ../";
    // The command you want to execute, for example, "ls" to list files in the current directory.
    string move_workload_command = "cat ../workload.txt | sed 's/^R/D Range/g' > ./workload_new.txt ";

    cout << "Executing command: " << gen_workload_command << endl;
    // Use the system function to execute the command.
    int returnCode = system(gen_workload_command.c_str());

    // Check the return code to see if the command was executed successfully.
    if (returnCode == 0) {
        printf("Gen workload command executed successfully.\n");
    } else {
        printf("Gen workload command failed to execute.\n");
    }


    cout << "Executing command: " << move_workload_command << endl;
    // Use the system function to execute the command.
    returnCode = system(move_workload_command.c_str());

    // Check the return code to see if the command was executed successfully.
    if (returnCode == 0) {
        printf("Move workload command executed successfully.\n");
    } else {
        printf("Move workload command failed to execute.\n");
    }

    return 0;
}

