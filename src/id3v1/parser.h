#include <cstdint>
#include <FS.h>

struct ID3v1Tag {
    char title[31];
    char artist[31];
    char album[31];
    uint8_t track;
};

bool readID3v1(File &file, ID3v1Tag &tag);