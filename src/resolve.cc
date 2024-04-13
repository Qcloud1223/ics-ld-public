#include "resolve.h"

#include <iostream>

/* bind each undefined reference (reloc entry) to the exact valid symbol table entry
 * Throw correct errors when a reference is not bound to definition,
 * or there is more than one definition.
 */
void resolveSymbols(std::vector<ObjectFile> &allObjects)
{
    /* Your code here */

    /* 
     * if (has_undefined_symbol)
     *      std::cerr << "undefined reference for symbol " << sym.name << std::endl;
     * 
     * if (has_multiple_definition)
     *      std::cerr << "multiple definition for symbol " << sym.name << std::endl;
     */
}