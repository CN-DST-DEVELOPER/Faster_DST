#include <fstream>
#include <openssl/md5.h>
#include <set>
#include <unordered_map>

class Cache
{
public:
    enum MODE
    {
        READ,
        WRITE
    };

private:
    void md5(char *filename, unsigned char *md5);

    char *file_path;
    std::fstream cache;
    MODE mode = READ;
    unsigned char *cache_header = nullptr;
    int header_length;

public:
    void SaveCache(std::unordered_map<std::string, uint8_t *> &data);
    void LoadCache(std::unordered_map<char *, uint8_t *> data);
    bool VaildCache();
    void SetMode(MODE mode);

    Cache(char *cache_file, std::set<char *> cache_key);
    ~Cache();
};