#pragma once

#include <stdio.h>
#include <stdint.h>
#include <elf.h>

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#define ERROR_LOG(fmt, ...) \
do { \
    fprintf(stderr, "[%s] " fmt "\n", __func__, ##__VA_ARGS__); \
    abort(); \
} while (0)

#ifdef NDEBUG
#define DEBUG_LOG(fmt, ...)
#else
#define DEBUG_LOG(fmt, ...) \
do { \
    printf("[%s] " fmt "\n", __func__, ##__VA_ARGS__); \
} while (0)
#endif

#define ALIGN_DOWN(base, size) ((base) & -((__typeof__(base))(size)))
#define ALIGN_UP(base, size) ALIGN_DOWN((base) + (size)-1, (size))

struct Symbol
{
    std::string name;
    uint64_t value;
    uint64_t size;
    int type;
    int bind;
    int visibility;

    /* offset in the merged object file */
    uint64_t offset;
    /* section index in the merged object file */
    uint16_t index;

    Symbol& operator= (const Symbol&) = delete;
};

struct RelocEntry
{
    Symbol *sym;
    /* name is necessary when the symbol is not found */
    std::string name;
    uint64_t offset;
    int type;
    int64_t addend;
};

/* a concise version of Elf64_Shdr, 
 * containing just enough info for us to build the final object  
 */
struct Section
{
    /* name is no longer necessary since it's encoded in the key of map */
    std::string name;
    unsigned type;
    unsigned flags;
    unsigned info;
    /* the index this section would be in the final object file */
    unsigned index;
    uint64_t addr;
    uint64_t off;
    uint64_t size;
    uint64_t align;
};

struct ObjectFile
{
    std::vector<Symbol> symbolTable;
    std::vector<RelocEntry> relocTable;

    std::unordered_map<std::string, Section> sections;
    std::unordered_map<unsigned, Section *> sectionsByIdx;
    void *baseAddr;
    uint64_t size;
};

ObjectFile parseObjectFile(int fd, bool modify=false);
void discardRela(int fd);
void printSymbolTable(ObjectFile &o);
void mergeObjectFiles(std::vector<ObjectFile> &allObjects, int fd);
void rebaseSymbols(std::vector<ObjectFile> &allObjects, ObjectFile &m);