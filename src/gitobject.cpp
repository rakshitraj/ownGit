#include <iostream>
#include <filesystem>
#include <vector>
#include <zlib.h>
#include <openssl/sha.h>

#define CHUNK_SIZE 1024

enum class GitCommands {
    Init,
    CatFile,
    HashObject,
    LSTree,
    Unknown
};

class GitCommand {
public:
    GitCommand();
    ~GitCommand() = default;
};

enum class GitObjectTypes {
    blob,
    commit,
    tree
};

class GitObject {
public:
    GitObject() = default;

    virtual void serialize(std::vector<char>& uncompressed_data, std::vector<char>& out);
    virtual void deserialize(std::vector<char>& compressed_data, std::vector<char>& out);
    
    void sha1_hash(const std::vector<char>& hashable_data, std::string& hashed) {
        unsigned char hash[SHA_DIGEST_LENGTH];
        const unsigned char* input_data = reinterpret_cast<const unsigned char*>(hashable_data.data());
        size_t input_length = hashable_data.size();

        SHA1(input_data, input_length, hash);

        std::ostringstream oss;
        for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        hashed = oss.str();
    }


    void compress_zlib(std::vector<char>& uncompressed_data, std::vector<char>& out) {
        z_stream strm = {};
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(uncompressed_data.data()));
        strm.avail_in = uncompressed_data.size();

        // Init Deflate
        if (deflateInit(&strm, Z_BEST_SPEED) != Z_OK) {
            throw std::runtime_error("deflateInit failed");
        }
        // Deflate uncompressed data
        char buffer[CHUNK_SIZE]; // Output buffer
        int ret;
        do {
            strm.next_out = reinterpret_cast<Bytef*>(buffer);
            strm.avail_out = CHUNK_SIZE;
            ret = deflate(&strm, Z_FINISH);
            if (ret == Z_STREAM_ERROR) {
                throw std::runtime_error("deflate failed in compress_zlib");
            }
            out.insert(out.end(), buffer, buffer + (CHUNK_SIZE - strm.avail_out));
        } while (strm.avail_out == 0);
        // End deflate
        if (deflateEnd(&strm) != Z_OK) {
            throw std::runtime_error("deflateEnd failed");
        }
    }


    void decompress_zlib(const std::vector<char>& compressed, std::vector<char>& out) {
        z_stream strm = {};
        strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed.data()));
        strm.avail_in = compressed.size();

        if (inflateInit(&strm) != Z_OK) {
            throw std::runtime_error("inflateInit failed");
        }
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
    }

    virtual ~GitObject() = default;
};

class GitBlob : public GitObject {
    GitBlob();
    ~GitBlob() = default;
};