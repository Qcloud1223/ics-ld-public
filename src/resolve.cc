#include "resolve.h"

#include <iostream>

#define MULTIDEF 1
#define NODEF 2

std::string errSymName;

int callResolveSymbols(std::vector<ObjectFile> &allObjects);

void resolveSymbols(std::vector<ObjectFile> &allObjects) {
    int ret = callResolveSymbols(allObjects);
    if (ret == MULTIDEF) {
        std::cerr << "multiple definition for symbol " << errSymName << std::endl;
        abort();
    } else if (ret == NODEF) {
        std::cerr << "undefined reference for symbol " << errSymName << std::endl;
        abort();
    }
}

/* bind each undefined reference (reloc entry) to the exact valid symbol table entry
 * Throw correct errors when a reference is not bound to definition,
 * or there is more than one definition.
 */
int callResolveSymbols(std::vector<ObjectFile> &allObjects)
{
    /* Your code here */
    // if found multiple definition, set the errSymName to problematic symbol name and return MULTIDEF;
    // if no definition is found, set the errSymName to problematic symbol name and return NODEF;

    return 0;
}
