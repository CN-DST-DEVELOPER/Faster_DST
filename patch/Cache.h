/**
 * Copyright 2023 Fengying <zxcvbnm3057@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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