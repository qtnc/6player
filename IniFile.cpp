#include "IniFile.hpp"
#include<fstream>
#include<windows.h>
#include "strings.hpp"
using namespace std;

bool IniFile::load (const std::string& file) {
string line, name, value;
ifstream in(file);
if (!in) return false;
while(getline(in,line)) {
trim(line);
if (line.size()<1 || line[0]==';' || line[0]=='#') continue;
if (!split(line, '=', name, value)) continue;
trim(name); trim(value);
if (name.size()<1) continue;
(*this)[name]=value;
}
return true;
}

bool IniFile::save (const std::string& file) {
ofstream out(file);
if (!out) return false;
for(auto it: *this) {
out << it.first << '=' << it.second << endl;
}
return true;
}

bool IniFile::contains (const std::string& key) {
return find(key)!=end();
}

