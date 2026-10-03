// Simple checked asset reader; RGBA data prepared by tools/prepare-art.py.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <new>
extern "C"
{
    int open(const char *, int, ...);
    long read(int, void *, std::size_t);
    int close(int);
}
namespace crisis
{
struct Art
{
    unsigned width = 0, height = 0;
    std::unique_ptr<std::uint32_t[]> pixels;
    bool load(const char *path)
    {
        int fd = open(path, 0);
        if (fd < 0)
            return false;
        auto exact = [fd](void *data, std::size_t count)
        {
            auto *p = static_cast<unsigned char *>(data);
            while (count)
            {
                long n = read(fd, p, count);
                if (n <= 0 || std::size_t(n) > count)
                    return false;
                p += n;
                count -= std::size_t(n);
            }
            return true;
        };
        std::uint32_t header[3]{};
        bool ok = exact(header, sizeof(header)) && header[0] == 0x41454443u && header[1] > 0 &&
                  header[2] > 0 && header[1] <= 4096 && header[2] <= 4096;
        if (ok)
        {
            width = header[1];
            height = header[2];
            pixels.reset(new (std::nothrow) std::uint32_t[std::size_t(width) * height]);
            ok = bool(pixels) && exact(pixels.get(), std::size_t(width) * height * 4);
        }
        close(fd);
        if (!ok)
        {
            pixels.reset();
            width = height = 0;
        }
        return ok;
    }
};
} // namespace crisis
