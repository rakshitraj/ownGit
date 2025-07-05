#include "include/cat-file.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <zlib.h>

std::string decompress_zlib(const std::vector<char>& compressed) {

    z_stream strm = {};
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
    strm.avail_in = compressed.size();

    if (inflateInit(&strm) != Z_OK) {
        throw std::runtime_error("inflateInit failed");
    }
    std::vector<char> out;
    char buffer[CHUNK_SIZE];
    int ret;
    int count = 1;
    do {
        strm.next_out = reinterpret_cast<Bytef*>(buffer);
        strm.avail_out = CHUNK_SIZE; // bytes available to write to 
        ret = inflate(&strm, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            throw std::runtime_error("inflate failed in decompress_zlib");
        }
        out.insert(out.end(), buffer, buffer + (CHUNK_SIZE - strm.avail_out));
    } while (strm.avail_out == 0);
    if (inflateEnd(&strm) != Z_OK) {
        throw std::runtime_error("inflateEnd failed");
    }

    return std::string(out.data(), strm.total_out);
}

std::string deserialize(std::string path) {
    // Read zlib compressed file into a vector of bytes
    std::ifstream file(path, std::ios::binary);
    auto compressed = std::vector<char>(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    return decompress_zlib(compressed);
}

int cat_file(const int argc, const char *const argv[]) {
    if (argc <= 3) {
        std::cerr << "Invalid arguments, Required '-p' <blob-sha>\n";
        return EXIT_FAILURE;
    }

    const std::string flag = argv[2];
    if (flag != "-p") {
        std::cerr << "Invalid flag for cat-file, expected `-p`\n";
        return EXIT_FAILURE;
    }

    const std::string value = argv[3];
    const std::string dir_name = value.substr(0,2);
    const std::string file_name = value.substr(2);
    std::string path = ".git/objects/" + dir_name + '/' + file_name;

    const std::string object_content = deserialize(path);

    // Read object type
    int space_pos = object_content.find(' ');
    std::string type = object_content.substr(0, space_pos);

    // Read object size
    int null_pos = object_content.find('\0');
    std::string size = object_content.substr(space_pos+1,null_pos-space_pos-1);

    // Read content
    std::string content = object_content.substr(null_pos+1);

    std::cout<< content << std::flush;
    return EXIT_SUCCESS;
}