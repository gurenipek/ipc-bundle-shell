#include "parser.h"
#include "repeater.hpp"

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include <fcntl.h>  

typedef struct {
    char** args;
} Command;

typedef struct {
	int command_count;
    std::vector<Command*> commands;
    std::string name;
    Repeater* rep;
} Bundle;

// std::vector<bundle> bundles;
std::unordered_map<std::string, Bundle*> bundleMap;

int main(int argc, char const *argv[]){

    int is_bundle_creation = 0;

    while(1){

        // take output and parse
        char* input = nullptr;
        std::size_t size;
        // std::cout << "bundle shell $ ";
        int i = getline(&input, &size, stdin);
        parsed_input currentLine;
        int parsed = parse(input, is_bundle_creation, &currentLine);


        // special commands check
        if(currentLine.command.type == QUIT){
            return 0;
        }
        else if(currentLine.command.type == PROCESS_BUNDLE_CREATE){
            // take new bundle
            is_bundle_creation = 1;
            Bundle* current_bundle = new Bundle;

            //start filling bundle information
            current_bundle->name = currentLine.command.bundle_name;
            int bundle_finished = -1;

            // a bundle contains at least one command
            while( bundle_finished != 1 and currentLine.command.type != PROCESS_BUNDLE_STOP){
                char* bundle_inp = nullptr;
                std::size_t size;
                int i = getline(&bundle_inp, &size, stdin);

                parsed_input bundle_command;
                bundle_finished = parse(bundle_inp, is_bundle_creation, &bundle_command);
                if(bundle_finished != 1){

                    Command* comm = new Command;
                    
                    // deep copy the arguments of command
                    int commSize = 0;
                    for(int p = 0 ; p < 99 ; p++){
                        if (bundle_command.argv[p]  == nullptr){
                            commSize = p;
                            break;
                        }
                    }
                    comm->args = new char*[commSize+1];
                    for (int i = 0; i < commSize; i++){
                        comm->args[i] = bundle_command.argv[i];
                    }
                    comm->args[commSize] = nullptr;

                    // add command to vector in bundle
                    current_bundle->commands.push_back(comm);

                    
                    current_bundle->command_count++;
                }                
            }

            //bundle creation over
            is_bundle_creation = 0;

            bundleMap.insert(std::make_pair(current_bundle->name, current_bundle));
        
        }
        else if(currentLine.command.type == PROCESS_BUNDLE_EXECUTION){


            // how many bundles are involved in the execution command
            int bundleCount = currentLine.command.bundle_count;

            // pointer to the bundle execution array
            bundle_execution* bundleList = currentLine.command.bundles;

            // retrieve input file if it exists
            char* inputFile = bundleList[0].input;
            std::string inputContent;
            
            // retrieve output file if it exists
            char* outputFile = bundleList[bundleCount - 1].output;
            
            // bundle execution
            for(int i = 0; i< bundleCount; i++){
                // retrieve bundle
                Bundle* bun = nullptr;
                try {
                    bun = bundleMap.at(bundleList[i].name);
                } catch (std::out_of_range err) {
                    break;
                }
                bundleMap[bundleList[i].name] = nullptr;
                bun->rep = new Repeater(bun->command_count);

                // get input from a file or read pipeline
                if(i == 0 && inputFile != nullptr){
                    //load input txt file
                    char ch;
                    int fd = open(inputFile, O_RDONLY);
                    int readFlag = 1;
                    while (readFlag){
                        readFlag = read(fd, &ch, 1);
                        inputContent += ch;
                    } 
                    close(fd);
                }

                // fork and execute commands in bundle
                for(int j = 0; j < bun->command_count; j++){
                    int pid = fork();
                    if(pid == 0){
                        // child

                        //connect pipes for this command
                        int di = dup2(bun->rep->inputPipes[j]->fds[0], STDIN_FILENO);
                        for( int k = 0 ; k < bun->command_count; k++){
                            close(bun->rep->inputPipes[k]->fds[0]);
                            close(bun->rep->inputPipes[k]->fds[1]);
                        }


                        int dout = dup2(bun->rep->outputPipes[j]->fds[1], STDOUT_FILENO);
                        for( int k = 0 ; k < bun->command_count; k++){
                            close(bun->rep->outputPipes[k]->fds[1]);
                            close(bun->rep->outputPipes[k]->fds[0]);
                        }
                        //run command
                        execvp(bun->commands[j]->args[0], bun->commands[j]->args);
                    }
                    

                    //parent
                    close(bun->rep->inputPipes[j]->fds[0]);
                    close(bun->rep->outputPipes[j]->fds[1]);

                    
                }

                // repeater distributes the same input to every running command
                bun->rep->distributeInput(inputContent);
                
                // collect output from whole bundle
                std::string bunOutput = bun->rep->collectOutput();

                // direct output to a file or write to pipeline
                inputContent = bunOutput;

                if(i == bundleCount-1 && outputFile != nullptr){
                    // push output to file
                    int fd = open(outputFile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                    write(fd, &inputContent, inputContent.size());
                    close(fd);
                }
                else if(i == bundleCount-1 && outputFile == nullptr){
                    std::cout << inputContent;
                }

                bun->rep->closeRepeater();
            }
            

        }

        
    }
    return 0;
}