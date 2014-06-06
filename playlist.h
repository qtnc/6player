#ifndef ___QC_PLAYLIST___
#define ___QC_PLAYLIST___
#include<string>
#include<vector>

struct playlistitem {
std::string file, title, artist, album, genre, subtitle, composer, copyright, year, disc, tracknum, comment;
double length;
BOOL metadataSet;
playlistitem () : title(), artist(), album(), genre(), subtitle(), length(0), metadataSet(false) {}
};

struct radio {
std::string name, description, genre, country, lang;
std::vector<std::string> urls;
};


#endif
