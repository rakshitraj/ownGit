#include "include/hash-object.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/sha.h>
#include <zlib.h>
#include <sstream>
#include <iomanip>

std::string sha1_hash(const std::vector<char>& hashable_data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    const unsigned char* input_data = reinterpret_cast<const unsigned char*>(hashable_data.data());
    size_t input_length = hashable_data.size();

    SHA1(input_data, input_length, hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();

}

int compress_zlib(std::vector<char>& uncompressed_data, std::vector<char>& out) {
    z_stream strm;
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(uncompressed_data.data()));
    strm.avail_in = uncompressed_data.size();

    // Init Deflate
    if (deflateInit(&strm, Z_BEST_SPEED) != Z_OK) {
        std::cout << "err1\n";
        throw std::runtime_error("deflateInit failed");
    }
    // Deflate uncompressed data
    char buffer[CHUNK_SIZE]; // Output buffer
    int ret;
    do {
        strm.next_out = reinterpret_cast<Bytef*>(buffer);
        strm.avail_out = CHUNK_SIZE;
        ret = deflate(&strm, Z_SYNC_FLUSH);
        if (ret == Z_STREAM_ERROR) {
            std::cout << "err2\n";
            throw std::runtime_error("deflate failed in compress_zlib");
        }
        out.insert(out.end(), buffer, buffer + (CHUNK_SIZE - strm.avail_out));
    } while (ret != Z_STREAM_END);
    if (deflateEnd(&strm) != Z_OK) {
        std::cout << "err3\n";
        throw std::runtime_error("deflateEnd failed");
    }

    return strm.total_out;
}

std::string serialize(std::vector<char> uncompressed_data) {
    // Deflate hashable data
    std::vector<char> ret;
    int total_out = compress_zlib(uncompressed_data, ret);
    std::cout << total_out << '\n';
    std::string out = std::string(ret.data(), total_out);
    return out;
}

int hash_object(const int argc, const char* const argv[]) {
    if (argc <=3) {
        std::cerr << "Invalid arguments, Required '-w' <file>\n";
        return EXIT_FAILURE;
    }

    const std::string flag = argv[2];
    if (flag != "-w") {
        std::cerr << "Invalid flag for hash-object, expected '-w' <file>\n";
        return EXIT_FAILURE;
    }

    const std::string target_path = argv[3];

    std::ifstream target(target_path, std::ios::binary);
    if (!target) {
        std::cerr << "Failed to read file: " << target_path << "\n";
        return EXIT_FAILURE;
    }
    std::vector<char> file_contents((std::istreambuf_iterator<char>(target)), std::istreambuf_iterator<char>());

    // Read object type
    std::string type = "blob";

    // Add header 
    std::string header = type + " " + std::to_string(file_contents.size()) + '\0';
    file_contents.insert(file_contents.begin(), header.begin(), header.end());

    std::cout << "hashing...\n";
    std::string hash = sha1_hash(file_contents);
    std::cout << "hash --> " << hash << '\n';

    std::cout << "serailizing...\n";
    std::string out = serialize(file_contents);
    std::cout << "out --> " << out << '\n';

    std::cout << "Writing object...\n";
    if (flag == "-w") {
        std::string object_dir = hash.substr(0,2);
        std::string object_name = hash.substr(2);
        std::string object_path = ".git/objects/" + object_dir + '/' + object_name;

        std::ofstream git_object;
        git_object.open(object_path);
        if (!git_object.is_open()) {
            std::string error_msg = "failed to write git object at " + object_path;
            throw std::runtime_error(error_msg);
        }
        git_object << out;
        git_object.close();
    }

    std::cout << hash;
    return EXIT_SUCCESS;
}