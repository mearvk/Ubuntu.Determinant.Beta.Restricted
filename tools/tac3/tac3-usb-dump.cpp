// SPDX-License-Identifier: GPL-2.0
// tac3-usb-dump.cpp — safety-gated TAC3 backup bundle writer.
//
// The current TAC3 disk format has shared FILE/HEALTH/ADMIN/RECOVERY extents,
// not independent physical extents for each redundancy layer. This utility
// therefore preserves the complete TAC3 image and authoritative tables, while
// --layers/--layer selects the logical recovery set and writes layer metadata.
// It never invents a physical layer mapping and never modifies the source.
//
// Copyright (C) 2026 MEARVK LLC

#include "tac3.hpp"
#include "tac3_format.hpp"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {
using byte = std::uint8_t;
constexpr std::size_t BS = tac3::format::block_size;

void usage(const char *p) {
    std::fprintf(stderr,
        "Usage: %s --source DEVICE --usb DIRECTORY [--layers N | --layer N]\n\n"
        "  --layers N  select logical layers 0..N-1\n"
        "  --layer N   select one logical layer N\n"
        "\nThe complete TAC3 image and all authoritative tables are preserved.\n"
        "The current disk format does not define physical per-layer extents,\n"
        "so selection is recorded as a logical recovery set rather than a\n"
        "fabricated block mapping. The USB directory must already be mounted.\n", p);
}

std::uint16_t u16(const byte *p) { return (std::uint16_t)p[0] | ((std::uint16_t)p[1] << 8); }
std::uint32_t u32(const byte *p) {
    return (std::uint32_t)p[0] | ((std::uint32_t)p[1] << 8) |
           ((std::uint32_t)p[2] << 16) | ((std::uint32_t)p[3] << 24);
}
std::uint64_t u64(const byte *p) {
    std::uint64_t v = 0; for (unsigned i = 0; i < 8; ++i) v |= (std::uint64_t)p[i] << (8 * i); return v;
}
std::uint32_t crc32c(const byte *p, std::size_t n) {
    std::uint32_t c = 0xffffffffU;
    for (std::size_t i = 0; i < n; ++i) { c ^= p[i]; for (int b = 0; b < 8; ++b) c = (c >> 1) ^ (0x82f63b78U & (std::uint32_t)-(c & 1U)); }
    return ~c;
}

bool put_all(int fd, const byte *p, std::size_t n, off_t off) {
    while (n) {
        ssize_t w = pwrite(fd, p, n, off);
        if (w < 0) { if (errno == EINTR) continue; return false; }
        if (w == 0) return false;
        p += w; n -= (std::size_t)w; off += w;
    }
    return true;
}

bool copy_bytes(int in, int out, std::uint64_t bytes, std::uint64_t in_off = 0) {
    std::vector<byte> buf(1024 * 1024);
    std::uint64_t done = 0;
    while (done < bytes) {
        std::size_t want = (std::size_t)std::min<std::uint64_t>(buf.size(), bytes - done);
        ssize_t r = pread(in, buf.data(), want, (off_t)(in_off + done));
        if (r < 0) { if (errno == EINTR) continue; return false; }
        if (r == 0) return false;
        if (!put_all(out, buf.data(), (std::size_t)r, (off_t)done)) return false;
        done += (std::uint64_t)r;
    }
    return true;
}

bool text(const std::string &path, const std::string &s) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc); if (!f) return false; f << s; return !!f;
}

bool mounted(const char *dev) {
    FILE *f = std::fopen("/proc/self/mountinfo", "r"); if (!f) return false;
    char line[8192]; bool yes = false;
    while (std::fgets(line, sizeof(line), f)) if (std::strstr(line, dev)) { yes = true; break; }
    std::fclose(f); return yes;
}

struct Super {
    std::uint64_t blocks=0, generation=0, file_start=0, file_blocks=0,
                  health_start=0, health_blocks=0, admin_start=0, admin_blocks=0,
                  recovery_start=0, recovery_blocks=0, data_start=0;
    std::uint32_t multitude=0, device_class=0, state=0;
};

bool read_super(int fd, Super &s, std::array<byte, BS> &raw) {
    if (pread(fd, raw.data(), BS, 0) != (ssize_t)BS) return false;
    if (std::memcmp(raw.data(), tac3::format::magic, tac3::format::magic_size)) return false;
    if (u16(raw.data()+TAC3_SB_OFF_MAJOR) != tac3::format::major ||
        u16(raw.data()+TAC3_SB_OFF_MINOR) != tac3::format::minor ||
        u32(raw.data()+TAC3_SB_OFF_BLOCK_SIZE) != tac3::format::block_size) return false;
    const std::uint32_t stored = u32(raw.data()+TAC3_SB_OFF_CHECKSUM);
    auto verify = raw; std::fill(verify.begin()+TAC3_SB_OFF_CHECKSUM, verify.end(), 0);
    if (stored != crc32c(verify.data(), BS-4)) return false;
    s.blocks=u64(raw.data()+TAC3_SB_OFF_TOTAL_BLOCKS); s.generation=u64(raw.data()+TAC3_SB_OFF_GENERATION);
    s.multitude=u32(raw.data()+TAC3_SB_OFF_MULTITUDE); s.device_class=u32(raw.data()+TAC3_SB_OFF_DEVICE_CLASS);
    s.file_start=u64(raw.data()+TAC3_SB_OFF_FILE_START); s.file_blocks=u64(raw.data()+TAC3_SB_OFF_FILE_BLOCKS);
    s.health_start=u64(raw.data()+TAC3_SB_OFF_HEALTH_START); s.health_blocks=u64(raw.data()+TAC3_SB_OFF_HEALTH_BLOCKS);
    s.admin_start=u64(raw.data()+TAC3_SB_OFF_ADMIN_START); s.admin_blocks=u64(raw.data()+TAC3_SB_OFF_ADMIN_BLOCKS);
    s.recovery_start=u64(raw.data()+TAC3_SB_OFF_RECOVERY_START); s.recovery_blocks=u64(raw.data()+TAC3_SB_OFF_RECOVERY_BLOCKS);
    s.data_start=u64(raw.data()+TAC3_SB_OFF_DATA_START); s.state=u32(raw.data()+TAC3_SB_OFF_STATE);
    if (!s.multitude || s.multitude > tac3::kMultMax || s.blocks < tac3::format::minimum_blocks) return false;
    auto ok=[&](std::uint64_t a,std::uint64_t n){return n && a < s.blocks && n <= s.blocks-a;};
    return ok(s.file_start,s.file_blocks)&&ok(s.health_start,s.health_blocks)&&ok(s.admin_start,s.admin_blocks)&&ok(s.recovery_start,s.recovery_blocks)&&s.data_start<s.blocks;
}

bool extent(int in, const std::string &path, std::uint64_t start, std::uint64_t blocks) {
    int out=open(path.c_str(),O_CREAT|O_WRONLY|O_TRUNC,0644); if(out<0)return false;
    bool ok=copy_bytes(in,out,blocks*BS,start*BS); if(ok)ok=(fsync(out)==0); close(out); return ok;
}
}

int main(int argc,char **argv) {
    const char *source=nullptr,*usb=nullptr; std::uint32_t selected=0; bool one=false;
    for(int i=1;i<argc;++i){
        std::string a(argv[i]);
        if(a=="--source"&&i+1<argc)source=argv[++i];
        else if(a=="--usb"&&i+1<argc)usb=argv[++i];
        else if(a=="--layers"&&i+1<argc){selected=(std::uint32_t)std::stoul(argv[++i]);one=false;}
        else if(a=="--layer"&&i+1<argc){selected=(std::uint32_t)std::stoul(argv[++i]);one=true;}
        else if(a=="--help"||a=="-h"){usage(argv[0]);return 0;}
        else{usage(argv[0]);return 2;}
    }
    if(!source||!usb){usage(argv[0]);return 2;}
    struct stat st{}; if(stat(usb,&st)||!S_ISDIR(st.st_mode)){std::fprintf(stderr,"tac3-usb-dump: USB destination must be an existing mounted directory\n");return 2;}
    if(mounted(source)){std::fprintf(stderr,"tac3-usb-dump: refusing a mounted source: %s\n",source);return 2;}
    int in=open(source,O_RDONLY); if(in<0){std::fprintf(stderr,"tac3-usb-dump: open: %s\n",std::strerror(errno));return 2;}
    Super s{}; std::array<byte,BS> sb{};
    if(!read_super(in,s,sb)){std::fprintf(stderr,"tac3-usb-dump: invalid TAC3 %u.%u source\n",tac3::format::major,tac3::format::minor);close(in);return 2;}
    if(one){if(selected>=s.multitude){std::fprintf(stderr,"tac3-usb-dump: layer %u is outside multitude %u\n",selected,s.multitude);close(in);return 2;}}
    else {if(selected==0)selected=s.multitude;if(selected>s.multitude){std::fprintf(stderr,"tac3-usb-dump: requested %u layers, TAC3 has %u\n",selected,s.multitude);close(in);return 2;}}

    std::ostringstream n;n<<"tac3-usb-backup-g"<<s.generation<<(one?"-layer":"-layers")<<selected;
    std::string root=std::string(usb)+"/"+n.str();
    if(!stat(root.c_str(),&st)){std::fprintf(stderr,"tac3-usb-dump: backup already exists: %s\n",root.c_str());close(in);return 2;}
    if(mkdir(root.c_str(),0755)||mkdir((root+"/tables").c_str(),0755)||mkdir((root+"/metadata").c_str(),0755)||mkdir((root+"/layers").c_str(),0755)){std::fprintf(stderr,"tac3-usb-dump: cannot create bundle: %s\n",std::strerror(errno));close(in);return 1;}

    int out=open((root+"/tac3-image.bin").c_str(),O_CREAT|O_WRONLY|O_TRUNC,0644);
    if(out<0||!copy_bytes(in,out,s.blocks*BS)||fsync(out)){std::fprintf(stderr,"tac3-usb-dump: complete image copy failed\n");if(out>=0)close(out);close(in);return 1;} close(out);
    out=open((root+"/tables/superblock.bin").c_str(),O_CREAT|O_WRONLY|O_TRUNC,0644);
    if(out<0||!put_all(out,sb.data(),BS,0)){std::fprintf(stderr,"tac3-usb-dump: superblock export failed\n");if(out>=0)close(out);close(in);return 1;}fsync(out);close(out);
    if(!extent(in,root+"/tables/file.bin",s.file_start,s.file_blocks)||!extent(in,root+"/tables/health.bin",s.health_start,s.health_blocks)||!extent(in,root+"/tables/admin.bin",s.admin_start,s.admin_blocks)||!extent(in,root+"/tables/recovery.bin",s.recovery_start,s.recovery_blocks)){std::fprintf(stderr,"tac3-usb-dump: table export failed\n");close(in);return 1;}

    std::ostringstream sel;sel<<"format="<<tac3::format::major<<"."<<tac3::format::minor<<"\ngeneration="<<s.generation<<"\nmultitude="<<s.multitude<<"\n"<<(one?"mode=single-layer\nlayer=":"mode=first-N-layers\nlayers=")<<selected<<"\ncomplete_image_preserved=true\nlayer_physical_extents=not_defined_by_current_format\n";
    if(!text(root+"/metadata/selected-layers.txt",sel.str())){close(in);return 1;}
    for(std::uint32_t i=0;i<(one?1u:selected);++i){std::uint32_t layer=one?selected:i;std::ostringstream p,m;p<<root<<"/layers/layer-"<<layer<<".meta";m<<"layer="<<layer<<"\nmultitude="<<s.multitude<<"\ngeneration="<<s.generation<<"\nfile_table=tables/file.bin\nhealth_table=tables/health.bin\nadmin_table=tables/admin.bin\nrecovery_table=tables/recovery.bin\ncomplete_image=tac3-image.bin\nstatus=selected-logical-layer\n";if(!text(p.str(),m.str())){close(in);return 1;}}
    for(const char *leaf:{"status","health","admin"}){std::ifstream f(std::string("/proc/tac3/")+leaf);if(f){std::ostringstream x;x<<f.rdbuf();text(root+"/metadata/proc-"+leaf+".txt",x.str());}}
    std::ostringstream man;man<<"TAC3 USB BACKUP\nformat="<<tac3::format::major<<"."<<tac3::format::minor<<"\ngeneration="<<s.generation<<"\nmultitude="<<s.multitude<<"\ndevice_class="<<s.device_class<<"\nstate="<<s.state<<"\ntotal_blocks="<<s.blocks<<"\nselected_mode="<<(one?"single":"first-N")<<"\nselected_value="<<selected<<"\ncomplete_tac3_image=tac3-image.bin\ntables=superblock.bin,file.bin,health.bin,admin.bin,recovery.bin\nlayer_data_policy=complete-image-plus-selected-layer-metadata\nsource_unchanged=true\n";
    if(!text(root+"/MANIFEST.txt",man.str())){close(in);return 1;} close(in);
    std::printf("TAC3 USB backup complete: %s\n",root.c_str());std::printf("Selected %s: %u\n",one?"layer":"layers",selected);std::printf("Complete image and FILE/HEALTH/ADMIN/RECOVERY tables preserved.\n");return 0;
}
