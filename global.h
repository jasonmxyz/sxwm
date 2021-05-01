#pragma once

#define die(X) die_(__LINE__, __FILE__, X)
void die_(int line, char* file, char* message);