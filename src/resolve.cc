#include "resolve.h"

#include <iostream>
#include <signal.h>
#include <set>
#include <vector>

using std::set, std::string, std::cout, std::endl, std::vector, std::cerr;

struct errsym {
    enum errtype {
        OK,
        MULTIDEF,
        NODEF
    };
    int err;    // Error type
    string sym; // Error symbol name
};

errsym callResolveSymbols(std::vector<ObjectFile> &allObjects);

void resolveSymbols(std::vector<ObjectFile> &allObjects) {
    errsym ret = callResolveSymbols(allObjects);
    switch (ret.err)
    {
    case errsym::MULTIDEF:
        cerr << "multiple definition for symbol " << ret.sym << endl;
        abort();
        break;
    case errsym::NODEF:
        cerr << "undefined reference for symbol " << ret.sym << endl;
        abort();
        break;
    default:
        break;
    }
}

/* bind each undefined reference (reloc entry) to the exact valid symbol table entry
 * Throw correct errors when a reference is not bound to definition,
 * or there is more than one definition.
 */
errsym callResolveSymbols(std::vector<ObjectFile> &allObjects)
{
    /* Your code here */
    // if found multiple definition, return {errsym::MULTIDEF, "symbol name"};
    // if no definition is found, return {errsym::NODEF, "symbol name"};

    return {errsym::OK, ""};
}
