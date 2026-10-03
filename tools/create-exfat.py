#!/usr/bin/env python3
"""Pack a title directory into a fresh, unpartitioned exFAT image without mounting.

Requires mkfs.exfat from exfatprogs. Uses FAT chains, 64 KiB clusters, ASCII
names, and one cluster per directory. Refuses overwrites and symlinks. This
small packer is for this title's release tree, not a general filesystem editor.
SPDX-License-Identifier: GPL-3.0-or-later
"""
import argparse
import math
import pathlib
import shutil
import struct
import subprocess


def rotate(v, bits=16):
    return ((v >> 1) | ((v & 1) << (bits - 1)))


def checksum(data):
    value = 0
    for i, byte in enumerate(data):
        if i not in (2, 3):
            value = (rotate(value) + byte) & 0xffff
    return value


def entries(name, first, length, directory):
    if not name.isascii() or len(name) > 255:
        raise ValueError('Only ASCII release paths up to 255 characters supported')
    encoded = name.encode('utf-16le')
    hash_value = 0
    for byte in name.upper().encode('utf-16le'):
        hash_value = (rotate(hash_value) + byte) & 0xffff
    chunks = [encoded[i:i+30] for i in range(0, len(encoded), 30)]
    primary = bytearray(32)
    primary[0] = 0x85
    primary[1] = 1 + len(chunks)
    struct.pack_into('<H', primary, 4, 0x10 if directory else 0x20)
    # A valid DOS timestamp (2026-10-03 00:00:00), deterministic for this release.
    stamp = ((2026-1980) << 25) | (10 << 21) | (3 << 16)
    struct.pack_into('<III', primary, 8, stamp, stamp, stamp)
    primary[22:25] = bytes([0x80]*3)  # UTC offsets known, UTC+0
    stream = bytearray(32)
    stream[0] = 0xc0
    stream[1] = 1  # AllocationPossible; FAT chains, no NoFatChain flag
    stream[3] = len(name)
    struct.pack_into('<H', stream, 4, hash_value)
    struct.pack_into('<Q', stream, 8, length)
    struct.pack_into('<I', stream, 20, first)
    struct.pack_into('<Q', stream, 24, length)
    names = []
    for chunk in chunks:
        entry = bytearray(32)
        entry[0] = 0xc1
        entry[2:2+len(chunk)] = chunk
        names.append(entry)
    record = primary + stream + b''.join(names)
    struct.pack_into('<H', record, 2, checksum(record))
    return record


def pack(source, destination, mkfs):
    source = source.resolve()
    if not (source/'eboot.bin').is_file() or not (source/'sce_sys/param.json').is_file():
        raise ValueError('Source must be a complete title directory')
    paths = list(source.rglob('*'))
    if any(p.is_symlink() for p in paths):
        raise ValueError('Symlinks are not supported')
    for p in paths:
        if not p.name.isascii():
            raise ValueError('Release paths must be ASCII')
    size = sum(math.ceil(p.stat().st_size/65536)*65536 if p.is_file() else 65536 for p in paths)
    total = max(128*1024*1024, math.ceil((size+96*1024*1024)/(1024*1024))*1024*1024)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as f:
        f.truncate(total)
    try:
        subprocess.run([mkfs, '-c', '64K', '-L', 'CRISISPS5', str(destination)], check=True)
        with destination.open('r+b') as f:
            boot = f.read(512)
            if boot[3:11] != b'EXFAT   ':
                raise ValueError('mkfs did not produce exFAT')
            sector = 1 << boot[108]
            cluster_bytes = sector * (1 << boot[109])
            fat_sector, _, heap_sector, cluster_count, root = struct.unpack_from('<IIIII', boot, 80)
            if cluster_bytes != 65536 or boot[110] != 1:
                raise ValueError('Unexpected exFAT geometry')
            def offset(cluster):
                return heap_sector*sector+(cluster-2)*cluster_bytes
            def read_cluster(cluster):
                f.seek(offset(cluster))
                return f.read(cluster_bytes)
            root_data = bytearray(read_cluster(root))
            end = 0
            bitmap_first = bitmap_length = 0
            while root_data[end] != 0:
                if root_data[end] == 0x81:
                    bitmap_first = struct.unpack_from('<I', root_data, end+20)[0]
                    bitmap_length = struct.unpack_from('<Q', root_data, end+24)[0]
                end += 32
                if end >= cluster_bytes:
                    raise ValueError('Unexpected full root directory')
            if not bitmap_first or bitmap_length > cluster_bytes:
                raise ValueError('Unsupported allocation bitmap')
            f.seek(offset(bitmap_first))
            bitmap = bytearray(f.read(bitmap_length))
            cursor = 0
            def allocate(count):
                nonlocal cursor
                clusters = []
                for _ in range(count):
                    while cursor < cluster_count and (bitmap[cursor//8] >> (cursor%8)) & 1:
                        cursor += 1
                    if cursor >= cluster_count:
                        raise ValueError('Image out of space')
                    bitmap[cursor//8] |= 1 << (cursor%8)
                    clusters.append(cursor+2)
                    cursor += 1
                for i, cluster in enumerate(clusters):
                    f.seek(fat_sector*sector+cluster*4)
                    f.write(struct.pack('<I', clusters[i+1] if i+1<len(clusters) else 0xffffffff))
                return clusters
            def directory(path, initial=b''):
                data = bytearray(initial)
                for item in sorted(path.iterdir(), key=lambda p:p.name):
                    if item.is_dir():
                        clusters = allocate(1)
                        child = directory(item)
                        f.seek(offset(clusters[0]))
                        f.write(child)
                        length = cluster_bytes
                    else:
                        length = item.stat().st_size
                        clusters = allocate(math.ceil(length/cluster_bytes))
                        with item.open('rb') as inp:
                            for cluster in clusters:
                                f.seek(offset(cluster))
                                f.write(inp.read(cluster_bytes))
                    data.extend(entries(item.name, clusters[0] if clusters else 0, length, item.is_dir()))
                if len(data)+32 > cluster_bytes:
                    raise ValueError('Directory exceeds one cluster')
                return data + bytes(cluster_bytes-len(data))
            data = directory(source, root_data[:end])
            f.seek(offset(root))
            f.write(data)
            f.seek(offset(bitmap_first))
            f.write(bitmap)
            # PercentInUse is excluded from the boot checksum. Mark unknown in
            # both the main and backup boot sectors, avoiding stale mkfs values.
            for at in (112, 12*sector+112):
                f.seek(at)
                f.write(b'\xff')
    except Exception:
        destination.unlink(missing_ok=True)
        raise
    print(f'Created {destination} ({total} bytes; clusters: {cluster_bytes})')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=pathlib.Path)
    parser.add_argument('destination', type=pathlib.Path)
    parser.add_argument('--mkfs', default=shutil.which('mkfs.exfat'))
    args = parser.parse_args()
    if not args.mkfs:
        parser.error('Install exfatprogs or specify --mkfs /path/to/mkfs.exfat')
    pack(args.source, args.destination, args.mkfs)
