#ifndef __INI_FILE_HPP__
#define __INI_FILE_HPP__
#include<windows.h>
#include<string>
#include<map>
#include "strings.hpp"

class IniFile: public std::map<std::string,std::string> {
public :
inline IniFile ()  {}
inline IniFile (const std::string& file) { load(file); }
inline ~IniFile () {}
bool load (const std::string& file) ;
bool save (const std::string& file) ;
bool contains (const std::string& key) ;
template<class T> void put (const std::string& key, const T& value) ;
template<class T> T get (const std::string& key, const T& value) ;
//inline std::string get (std::string key, std::string value) { return (*this)[key]=value; }
};

template<class T> void IniFile::put (const std::string& key, const T& value)  {
(*this)[key] = toString(value);
}

template<class T> T IniFile::get (const std::string& key, const T& value)  {
auto it = find(key);
return it==end()? value : fromString<T>(it->second);
}


#endif
