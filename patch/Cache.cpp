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

#include "Cache.h"

#define BUFFER_SIZE 1024
#define MD5_DIGEST_LENGTH 16
#define FUNCTION_NAME_MAX_LENGTH 25

void Cache::md5(char *filename, unsigned char *md5)
{
    std::ifstream ifile(filename, std::ios::in | std::ios::binary);
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    char DataBuff[BUFFER_SIZE];
    while (!ifile.eof())
    {
        ifile.read(DataBuff, BUFFER_SIZE);
        int length = ifile.gcount();
        if (length)
        {
            MD5_Update(&md5_ctx, DataBuff, length);
        }
    }
    ifile.close();
    MD5_Final(md5, &md5_ctx);
}

void Cache::SaveCache(std::unordered_map<std::string, uint8_t *> &data)
{
    unsigned short cache_size = 0;
    cache.seekp(0, std::ios_base::beg);
    cache.write(reinterpret_cast<const char *>(&cache_size), sizeof(unsigned short));

    cache.write(reinterpret_cast<const char *>(cache_header), header_length);
    for (auto it = data.begin(); it != data.end(); it++)
    {
        cache.write(it->first.c_str(), it->first.length() + 1);
        cache.write(reinterpret_cast<const char *>(&it->second), sizeof(uint8_t *));
    }

    cache_size = cache.tellg();
    cache.seekp(0, std::ios_base::beg);
    cache.write(reinterpret_cast<const char *>(&cache_size), sizeof(unsigned short));
}

void Cache::LoadCache(std::unordered_map<char *, uint8_t *> data)
{
    char name[FUNCTION_NAME_MAX_LENGTH];
    uint8_t *src;
    cache.seekg(header_length, std::ios_base::beg);
    while (!cache.eof())
    {
        cache.getline(name, sizeof(char) * FUNCTION_NAME_MAX_LENGTH, '\0');
        cache.read(reinterpret_cast<char *>(&src), sizeof(uint8_t *));
        data[name] = src;
    }
}

bool Cache::VaildCache()
{
    if (!cache.is_open())
        return false;

    unsigned short cache_size = 0;
    cache.seekg(0, std::ios::beg);
    cache.read(reinterpret_cast<char *>(&cache_size), sizeof(unsigned short));

    cache.seekg(0, std::ios::end);
    if (cache.tellg() != cache_size)
        return false;

    cache.seekg(sizeof(unsigned short), std::ios::beg);
    unsigned char cache_header_old[header_length];
    cache.read(reinterpret_cast<char *>(cache_header_old), header_length);
    for (int i = 0; i < header_length; i++)
        if (cache_header_old[i] != cache_header[i])
            return false;
    return true;
}

void Cache::SetMode(MODE value)
{
    if (value == mode)
        return;
    cache.close();
    cache.clear();
    switch (value)
    {
    case READ:
        cache.open(file_path, std::ios_base::in | std::ios_base::binary);
        break;
    case WRITE:
        cache.open(file_path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        break;
    }
}
Cache::Cache(char *cache_file, std::set<char *> cache_key)
{
    file_path = cache_file;
    cache.open(file_path, std::ios_base::in | std::ios_base::binary);

    header_length = cache_key.size() * MD5_DIGEST_LENGTH;
    cache_header = new unsigned char[header_length];
    int i = 0;
    for (auto it = cache_key.begin(); it != cache_key.end(); it++)
    {
        md5(*it, cache_header + i * MD5_DIGEST_LENGTH);
        i++;
    }
}

Cache::~Cache()
{
    cache.close();
    cache.clear();
    delete cache_header;
}