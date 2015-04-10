/*==============================================================================
 * FILE:       util.h
 * OVERVIEW:   Provides the definition for the miscellaneous bits and pieces
 *               implemented in the util.so library
 *============================================================================*/

#ifndef UTIL_H
#define UTIL_H

#include <sstream>
#include <string>

// was a workaround
#define STR(x) (char *)(x.str().c_str())
// Upper case a C string: s is source, d is dest
void upperStr(const char *s, char *d);
// Add string and integer
std::string operator+(const std::string &s, int i);

void escapeXMLChars(std::string &s);
char *escapeStr(const char *str);

int lockFileRead(const char *fname);
int lockFileWrite(const char *fname);
void unlockFile(int n);

#endif
