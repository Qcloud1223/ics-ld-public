#include "util.h"
#include "relocation.h"
#include "resolve.h"

#include <iostream>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

std::vector<ObjectFile> allObjects;
std::vector<std::string> allObjectNames;

struct 
{
    const char *outName;
    bool isPIE;
} ldConfig;

void parseArgs(int argc, char **argv)
{
    /* default name for object */
    ldConfig.outName = "test.o";
    ldConfig.isPIE = true;
    for (int i = 1; i < argc; i++) {
        if (strcmp("-o", argv[i]) == 0) {
            if (i + 1 < argc) {
                ldConfig.outName = argv[i + 1];
                i += 1;
            }
            continue;
        } else if (strcmp("-no-pie", argv[i]) == 0) {
            ldConfig.isPIE = false;
            continue;
        }
        allObjectNames.push_back(std::string(argv[i]));
    }
}

void parseObjFile()
{
    for (const auto &name : allObjectNames) {
        int fd = open(name.c_str(), O_RDONLY);
        if (fd == -1)
            ERROR_LOG("cannot open file: %s", name.c_str());
        allObjects.push_back(parseObjectFile(fd));
        // printSymbolTable(allObjects.back());
    }
}

/* TODO: specify name */
void reshapeObjFile(const char *filename)
{
    int fd = open(filename, O_RDWR);
    if (fd == -1)
        ERROR_LOG("cannot open file: %s", filename);
    discardRela(fd);
    close(fd);
}

void relocBinaryFile(const char *filename, bool isPIE)
{
    /* remove .o at the end */
    char *binName = strdup(filename);
    binName[strlen(binName) - 2] = 0;
    int fd = open(binName, O_RDWR);
    if (fd == -1)
        ERROR_LOG("cannot open file: %s", binName);
    auto o = std::move(parseObjectFile(fd, true));
    rebaseSymbols(allObjects, o);
    handleRela(allObjects, o, isPIE);
    close(fd);
}

void mergeObjects(const char *filename)
{
    int status;
    /* TODO: check linker script */
    char *args[16] = {NULL};
    args[0] = strdup("ld");
    args[1] = strdup("-r");
    args[2] = strdup("-o");
    args[3] = strdup(filename);
    // args[4] = strdup("--verbose");
    int idx = 4;
    for (const auto &name : allObjectNames) {
        args[idx++] = strdup(name.c_str());
    }
    if (fork() == 0)
        execv("/usr/bin/ld", args);
    wait(&status);
    if (status != 0)
        ERROR_LOG("merging failed with status %d", status);
}

void binaryGeneration(const char *filename, bool isPIE)
{
    int status;
    char *execName = strdup(filename);
    /* discard .o at the end */
    execName[strlen(execName) - 2] = 0; 
    if (fork() == 0)
        execlp("/usr/bin/gcc", "gcc", filename, "-o", execName, isPIE ? NULL : "-no-pie", NULL);
    /* TODO: waitpid to check ld return value, for some cases will be rejected by it */
    wait(&status);
}

int main(int argc, char **argv)
{
    if (argc < 2) 
        ERROR_LOG("usage: ics-ld object1 [object 2]...");
    
    parseArgs(argc, argv);

    parseObjFile();

    resolveSymbols(allObjects);
    
    mergeObjects(ldConfig.outName);

    reshapeObjFile(ldConfig.outName);

    binaryGeneration(ldConfig.outName, ldConfig.isPIE);

    relocBinaryFile(ldConfig.outName, ldConfig.isPIE);

    return 0;
}