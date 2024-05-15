/* utilities to hide object files details */
#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include <unordered_set>

#include "util.h"

static void fillSectionStruct(const Elf64_Shdr &src, Section &dst)
{
    dst.addr = src.sh_addr;
    dst.align = src.sh_addralign;
    dst.flags = src.sh_flags;
    dst.off = src.sh_offset;
    dst.size = src.sh_size;
    dst.type = src.sh_type;
    dst.info = src.sh_info;
}

void discardRela(int fd)
{
    struct stat Stat;
    fstat(fd, &Stat);

    void *mmapRet = mmap(0, Stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    
    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(mmapRet);
    uint64_t shoff = ehdr->e_shoff;
    uint16_t shnum = ehdr->e_shnum;
    Elf64_Shdr *shdr = reinterpret_cast<Elf64_Shdr *>((char *)mmapRet + shoff);
    char *shstrtab = (char *)mmapRet + shdr[ehdr->e_shstrndx].sh_offset;

    for (int i = 0; i < shnum; i++) {
        const char *secName = shstrtab + shdr[i].sh_name;
        if (strcmp(".rela.text", secName) == 0) {
            shdr[i].sh_size = 0;
            break;
        }
    }
    munmap(mmapRet, Stat.st_size);
}

/* rebase symbols from all separate objects into the merged object */
void rebaseSymbols(std::vector<ObjectFile> &v, ObjectFile &mergedObj)
{
    for (auto &o : v) {
        for (auto &sym : o.symbolTable) {
            if (sym.name == "")
                continue;
            /* to enable symbol resolution, we must reject undefined/local symbols */
            if (sym.bind != STB_GLOBAL)
                continue;
            for (const auto &finalSym : mergedObj.symbolTable) {
                if (sym.name == finalSym.name) {
                    sym.value = finalSym.value;
                    sym.offset = finalSym.offset;
                    sym.index  = finalSym.index;
                    goto found;
                }
            }
            assert(false && "Symbol not found! This cannot be possible");
found:
            continue;
        }
    }
}

ObjectFile parseObjectFile(int fd, bool modify)
{
    ObjectFile o;

    struct stat Stat;
    fstat(fd, &Stat);
    
    /* note that during linking, we don't have to map each section w.r.t. 
     * alignment. This is because we are reading it as a file, rather than
     * executing it.
     * This is also a good explanation on "segment for loading,
     * and section for linking".
     */
    void *mmapRet;
    if (!modify)
        mmapRet = mmap(0, Stat.st_size, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0);
    else
        mmapRet = mmap(0, Stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    if (mmapRet == MAP_FAILED)
        ERROR_LOG("cannot create memory-mapped I/O file");
    o.baseAddr = mmapRet;
    o.size = Stat.st_size;
    
    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(mmapRet);
    uint16_t shnum = ehdr->e_shnum;
    uint64_t shoff = ehdr->e_shoff;
    Elf64_Shdr *shdr = reinterpret_cast<Elf64_Shdr *>((char *)mmapRet + shoff);
    char *shstrtab = (char *)mmapRet + shdr[ehdr->e_shstrndx].sh_offset;

    // bool hasSymtab = false, hasStrtab = false;
    uint64_t symtabSize, symtabOff, strtabOff;
    for (int i = 0; i < shnum; i++) {
        const char *secName = shstrtab + shdr[i].sh_name;
        Section &s = o.sections[secName];
        fillSectionStruct(shdr[i], s);
        s.index = i;
        s.name = std::string(secName);
        if (strcmp(secName, ".symtab") == 0) {
            symtabSize = shdr[i].sh_size;
            symtabOff = shdr[i].sh_offset;
        } else if (strcmp(secName, ".strtab") == 0) {
            strtabOff = shdr[i].sh_offset;
        }
    }

    /* build up number index for sections */
    for (int i = 0; i < shnum; i++) {
        const char *secName = shstrtab + shdr[i].sh_name;
        std::string secStr (secName);
        auto secIt = o.sections.find(secStr);
        if (secIt != o.sections.end()) {
            /* This is safe since we will not insert new elements any more */
            o.sectionsByIdx[i] = &secIt->second;
        }
    }
    
    uint64_t symtabEntry = symtabSize / sizeof(Elf64_Sym);
    
    char *strtab = (char *)mmapRet + strtabOff;

    Elf64_Sym *symtab = reinterpret_cast<Elf64_Sym *>((char *)mmapRet + symtabOff);
    for (uint64_t i = 0; i < symtabEntry; i++) {
        const char *name = strtab + symtab[i].st_name;
        /* note here we must strictly keep all symbols, whether they are `useful' or not
         * this is because later we will access symtab using index
         */
        // if (strcmp(name, "") == 0)
        //     continue;
        Symbol s;
        /* initialize name from string table */
        s.name = std::string(name);
        s.value = symtab[i].st_value;
        s.size = symtab[i].st_size;
        s.type = ELF64_ST_TYPE(symtab[i].st_info);
        s.bind = ELF64_ST_BIND(symtab[i].st_info);
        s.visibility = symtab[i].st_other;
        s.offset = symtabOff + sizeof(Elf64_Sym) * i;
        s.index = symtab[i].st_shndx;
        o.symbolTable.push_back(s);
    }

    /* parse and store RELA */
    if (o.sections.find(".rela.text") != o.sections.end()) {
        auto &relaSec = o.sections[".rela.text"];
        auto relaStart = reinterpret_cast<Elf64_Rela *>((char *)mmapRet + relaSec.off);
        auto relaCnt = relaSec.size / sizeof(Elf64_Rela);
        // auto &symtabSec = o.sections[".symtab"];
        // auto symtabStart = reinterpret_cast<Elf64_Sym *>((char *)mmapRet + symtabSec.off);
        for (uint64_t i = 0; i < relaCnt; i++) {
            /* zero initialize */
            RelocEntry re {};
            const auto &sym = o.symbolTable[ELF64_R_SYM(relaStart[i].r_info)];
            re.offset = relaStart[i].r_offset;
            re.type = ELF64_R_TYPE(relaStart[i].r_info);
            re.name = sym.name;
            /* safe since size of symbol table won't change */
            re.sym = &o.symbolTable[ELF64_R_SYM(relaStart[i].r_info)];
            re.addend = relaStart[i].r_addend;
            o.relocTable.emplace_back(re);
        }
    }

    return o;
}

void printSymbolTable(ObjectFile &o)
{
    for (const auto &s : o.symbolTable) {
        std::cout << "Symbol name: " << s.name;
        std::cout << ", value: " << s.value;
        std::cout << ", type: " << s.type;
        std::cout << ", bind: " << s.bind;
        std::cout << ", vis: " << s.visibility << "\n\n";
    }
}
