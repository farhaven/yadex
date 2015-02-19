/*
 *	editlev.h
 *	AYM 1998-09-06
 */

#include <string>
#include <utility>

using std::string;
using std::pair;

pair<string, char*> find_level (const string name_given);
void EditLevel (string, bool);
