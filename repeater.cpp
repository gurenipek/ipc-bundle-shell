#include "repeater.hpp"

// constructor
Repeater::Repeater(int commCount){
    this->commCount = commCount;
    // create input pipes for a bundle
    for(int k = 0 ; k < commCount; k++){
        Pipe* aPipe = new Pipe;
        Pipe* bPipe = new Pipe;
        inputPipes.push_back(aPipe);
        outputPipes.push_back(bPipe);
    }

    for(int i = 0; i < commCount; i++){
        int newInpPipe = pipe(inputPipes[i]->fds);
        int newOutPipe = pipe(outputPipes[i]->fds);
    }
}

void Repeater::distributeInput(std::string inp){
    for(int i = 0; i < commCount; i++){
        // copy the same string to every pipe
        write(inputPipes[i]->fds[1], const_cast<char *>(inp.c_str()), inp.size());
        
        // close the remaining read end of the pipe
        close(inputPipes[i]->fds[1]);
        close(inputPipes[i]->fds[0]);
    }
}

std::string Repeater::collectOutput(){
    std::string output;

    for(int i = 0; i < commCount; i++){
        char ch;
        int readOut = 1;
        while(readOut > 0){
            readOut = read(outputPipes[i]->fds[0], &ch, 1);
            output += ch;
        }
        // close remaining end of pipe
        close(outputPipes[i]->fds[0]);
        close(outputPipes[i]->fds[1]);

    }
    return output;
}

void Repeater::closeRepeater(){
    // making sure everything is closed when job is done
    for(int i = 0; i < commCount; i++){
        close(inputPipes[i]->fds[0]);
        close(inputPipes[i]->fds[1]);
        close(outputPipes[i]->fds[0]);
        close(outputPipes[i]->fds[1]);
    }
}