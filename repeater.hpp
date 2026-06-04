#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <vector>

typedef struct Pipe{
    int fds[2];
}Pipe;

// a repeater instance belongs to a single bundle
class Repeater{
    public:
    int commCount;
    std::vector<Pipe*> inputPipes;
    std::vector<Pipe*> outputPipes;

    // constructor
    Repeater(int commCount);
    void distributeInput(std::string inp);
    std::string collectOutput();
    void closeRepeater();
};