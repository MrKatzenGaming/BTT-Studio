#pragma once

#include "nn/fs.h"
#include <vapours/results.hpp>
namespace FsHelper {

    struct LoadData {
        const char *path;
        void *buffer;
        long bufSize;
    };

    nn::Result writeFileToPath(void *buf, size_t size, const char *path);

    void loadFileFromPath(LoadData &loadData);

    long getFileSize(const char *path);

    bool isFileExist(const char *path);

    bool isDirExist(const char *path);
}
