#!/usr/bin/env python3
"""Read exFAT chains independently and compare every release file byte for byte.
SPDX-License-Identifier: GPL-3.0-or-later
"""
import pathlib,struct,sys
image=pathlib.Path(sys.argv[1]);source=pathlib.Path(sys.argv[2])
with image.open('rb') as f:
    boot=f.read(512);assert boot[3:11]==b'EXFAT   '
    sector=1<<boot[108];cluster_size=sector*(1<<boot[109])
    fat,_,heap,count,root=struct.unpack_from('<IIIII',boot,80)
    def chain(first,length):
        data=bytearray();seen=set();cluster=first
        while len(data)<length:
            assert 2<=cluster<count+2 and cluster not in seen
            seen.add(cluster);f.seek(heap*sector+(cluster-2)*cluster_size);data.extend(f.read(cluster_size))
            f.seek(fat*sector+cluster*4);cluster=struct.unpack('<I',f.read(4))[0]
        return data[:length]
    found=set()
    def walk(first,length,relative=pathlib.Path()):
        data=chain(first,length);at=0
        while at<len(data) and data[at]:
            if data[at]!=0x85:at+=32;continue
            second=data[at+1];record=data[at:at+32*(second+1)];value=0
            for i,b in enumerate(record):
                if i not in (2,3):value=(((value>>1)|((value&1)<<15))+b)&0xffff
            assert value==struct.unpack_from('<H',record,2)[0]
            attr=struct.unpack_from('<H',record,4)[0];stream=record[32:64]
            n=stream[3];name=b''.join(record[x+2:x+32] for x in range(64,len(record),32))[:n*2].decode('utf-16le')
            first_child=struct.unpack_from('<I',stream,20)[0];size=struct.unpack_from('<Q',stream,24)[0]
            path=relative/name
            if attr&0x10:walk(first_child,size,path)
            else:
                assert bytes(chain(first_child,size))==(source/path).read_bytes(),str(path)
                found.add(path)
            at+=32*(second+1)
    walk(root,cluster_size)
    expected={p.relative_to(source) for p in source.rglob('*') if p.is_file()}
    assert found==expected,(found,expected)
    print(f'PASS: {len(found)} files match their sources; directory checksums and FAT chains valid')
