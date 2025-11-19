#include "parser.h"

bool readID3v1(File &file, ID3v1Tag &tag) {
    if (!file) return false;

    if (file.size() < 128) return false; // file too small for ID3v1

    // Go to last 128 bytes
    file.seek(file.size() - 128);

    char buffer[128];
    if (file.readBytes(buffer, 128) != 128) return false;

    if (strncmp(buffer, "TAG", 3) != 0) return false; // Not an ID3v1 tag

    // Copy fields
    memcpy(tag.title, buffer + 3, 30); tag.title[30] = 0;
    memcpy(tag.artist, buffer + 33, 30); tag.artist[30] = 0;
    memcpy(tag.album, buffer + 63, 30); tag.album[30] = 0;

    // Check for ID3v1.1 track byte
    if (buffer[125] == 0 && buffer[126] != 0) {
        tag.track = buffer[126];
    } else {
        tag.track = 0;
    }

    return true;
}