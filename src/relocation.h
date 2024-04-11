#pragma once

#include "util.h"

void rebaseRela(std::vector<ObjectFile> &allObject);
void handleRela(std::vector<ObjectFile> &allObject, ObjectFile &mergedObject, bool isPIE);