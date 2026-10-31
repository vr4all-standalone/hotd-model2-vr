// SPDX-License-Identifier: GPL-2.0-or-later
package com.vrlightgunhub.hotdmodel2;
import java.io.*;
import java.security.*;
import java.util.*;
import java.util.zip.*;

/** Validate original packed ROMs in place; never unpack game data into app state. */
public final class GameFiles {
    static final class Rom {
        final String name, sha1; final long size, crc;
        Rom(String name, long size, long crc, String sha1) {
            this.name=name; this.size=size; this.crc=crc; this.sha1=sha1;
        }
    }
    private static final Rom[] REQUIRED = {
        new Rom("epr-19696.15", 524288, 0x03da5623L, "be0bd34a9216375c7204445f084f6c74c4d3b0c8"),
        new Rom("epr-19697.16", 524288, 0xa9722d87L, "0b14f9a81272f79a5b294bc024711042c5fb2637"),
        new Rom("epr-19694.13", 524288, 0xe85ca1a3L, "3d688be98f78fe40c2af1e91df6decd500400ae9"),
        new Rom("epr-19695.14", 524288, 0xcd52b461L, "bc96ab2a4ba7f30c0b89814acc8931c8bf800a82"),
        new Rom("mpr-19704.11", 4194304, 0xaa80dbb0L, "24e63f4392847f288971469cd10448536eb435d4"),
        new Rom("mpr-19705.12", 4194304, 0xf906843bL, "bee4f43b3ad15d93a2f9f07b873c9cf5d228e2f9"),
        new Rom("mpr-19702.9", 4194304, 0xfc8aa3b7L, "b64afb17d9c97277d8c4f20811f14f65a61cbb56"),
        new Rom("mpr-19703.10", 4194304, 0x208d993dL, "e5c45ea5621f99661a87ffe88e24764d2bbcb51e"),
        new Rom("mpr-19700.7", 4194304, 0x0558cfd3L, "94440839d3325176c2d03f39a78949d0ef040bba"),
        new Rom("mpr-19701.8", 4194304, 0x224a8929L, "933770546d46abca400e7f524eff2ae89241e56d"),
        new Rom("epr-19698.5", 524288, 0xe7a7b6eaL, "77cb53f8730fdb55080b70910ab8c750d79acb02"),
        new Rom("epr-19699.6", 524288, 0x8160b3d9L, "9dab483c60624dddba8085e94a4325739592ec17"),
        new Rom("epr-19707.29", 524288, 0x384fd133L, "6d060378d0f801b04d12e7ee874f2fa0572992d9"),
        new Rom("epr-19706.30", 524288, 0x1277531cL, "08d3e733ba9989fcd32290634171c73f26ab6e2b"),
        new Rom("mpr-19715.17", 4194304, 0x3ff7dda7L, "0a61b091bb0bc659f0cbca8ad401d0925a1dc2ea"),
        new Rom("mpr-19711.21", 4194304, 0x080d13f1L, "4167428a2a903aea2c14631ccf924afb81338b89"),
        new Rom("mpr-19714.18", 4194304, 0x3e55ab49L, "70b4c1627db80e6734112c02265495e2b4a53278"),
        new Rom("mpr-19710.22", 4194304, 0x80df1036L, "3cc59bb4910aa5382e95762f63325c06b763bd23"),
        new Rom("mpr-19713.19", 4194304, 0x4d092cd3L, "b6d0be283c25235249186751c7f025a7c38d2f36"),
        new Rom("mpr-19709.23", 4194304, 0xd08937bfL, "c92571e35960f27dc8b0b059f12167026d0666d1"),
        new Rom("mpr-19712.20", 4194304, 0x41577943L, "25a0d921c8662043c5860dc7a226d4895ff9fff6"),
        new Rom("mpr-19708.24", 4194304, 0x5cb790f2L, "d3cae450186bc62fd746b14d6a05cb397efcfe40"),
        new Rom("mpr-19718.27", 4194304, 0xa9de5924L, "3ebac2aeb1467939337c9a5c87ad9c293560dae2"),
        new Rom("mpr-19716.25", 4194304, 0x45c7dcceL, "f602cabd879c69afee544848feafb9fb9f5d51e2"),
        new Rom("mpr-19719.28", 4194304, 0x838f8343L, "fe6622b5917f9a99c097fd60d9446ac6b481fa75"),
        new Rom("mpr-19717.26", 4194304, 0x393e440bL, "927ac9cad22f87b339cc86043678470ff139ce1f"),
        new Rom("epr-19720.31", 524288, 0xb367d21dL, "1edaed489a3518ddad85728e416319f940ea02bb"),
        new Rom("mpr-19721.32", 4194304, 0xf5d8fa9aL, "6836973a687c59dd80f8e6c30d33155e306be199"),
        new Rom("mpr-19722.34", 4194304, 0xa56fa539L, "405a892bc368ba862ba71bda7525b421d6973c0e"),
    };
    private static final long MAX_ARCHIVE_BYTES=128L*1024*1024;
    private static final int MAX_MEMBERS=4096;
    private GameFiles() {}

    public static void validate(File game, File alternate) throws IOException {
        if((game==null || !game.isFile()) && alternate!=null && alternate.isFile()) game=alternate;
        if(game==null || !game.isFile()) throw new IOException("Missing Download/hotdo.zip. Copy your owned original arcade ROM archive to Download.");
        File[] paths=new File[]{game};
        validateArchives(paths, REQUIRED);
    }

    static void validateArchives(File[] paths, Rom[] requirements) throws IOException {
        Map<String,Rom> wanted=new HashMap<>();
        for(Rom rom:requirements) wanted.put(rom.name,rom);
        Set<String> found=new HashSet<>();
        for(File path:paths) {
            if(!path.isFile() || !path.canRead()) throw new IOException("Cannot read "+path.getName()+". Allow game file access and retry.");
            if(path.length()>MAX_ARCHIVE_BYTES) throw new IOException("ROM archive is too large: "+path.getName());
            try(ZipFile zip=new ZipFile(path)) {
                if(zip.size()>MAX_MEMBERS) throw new IOException("Too many ZIP members in "+path.getName());
                Set<String> seen=new HashSet<>();
                Enumeration<? extends ZipEntry> entries=zip.entries();
                while(entries.hasMoreElements()) {
                    ZipEntry entry=entries.nextElement(); String name=entry.getName();
                    if(name.isEmpty() || name.equals(".") || name.equals("..") || name.indexOf('/')>=0 || name.indexOf('\\')>=0 || name.indexOf(':')>=0 || name.indexOf('\0')>=0)
                        throw new IOException("Unsafe ZIP member: "+name);
                    String key=name.toLowerCase(Locale.ROOT);
                    if(!seen.add(key)) throw new IOException("Duplicate ZIP member: "+name);
                    Rom rom=wanted.get(key);
                    if(rom==null) continue;
                    if(entry.isDirectory() || entry.getSize()!=rom.size) throw new IOException("Wrong ROM size: "+rom.name);
                    MessageDigest sha;
                    try { sha=MessageDigest.getInstance("SHA-1"); }
                    catch(NoSuchAlgorithmException error) { throw new IOException("SHA-1 validation is unavailable",error); }
                    CRC32 crc=new CRC32(); byte[] buffer=new byte[16384]; long count=0;
                    try(InputStream in=zip.getInputStream(entry)) {
                        int bytes;
                        while((bytes=in.read(buffer,0,(int)Math.min(buffer.length,rom.size-count+1)))!=-1) {
                            if(bytes==0) continue;
                            count+=bytes;
                            if(count>rom.size) throw new IOException("Oversized ROM payload: "+rom.name);
                            sha.update(buffer,0,bytes); crc.update(buffer,0,bytes);
                        }
                    }
                    if(count!=rom.size || crc.getValue()!=rom.crc || !hex(sha.digest()).equals(rom.sha1)) throw new IOException("ROM checksum mismatch: "+rom.name+". Copy a complete, unmodified archive.");
                    found.add(key);
                }
            } catch(ZipException error) {
                throw new IOException("Cannot read "+path.getName()+": invalid, corrupt or encrypted ZIP ("+error.getMessage()+").",error);
            }
        }
        for(Rom rom:requirements) if(!found.contains(rom.name)) {
            String source="Download/hotdo.zip (original revision)";
            throw new IOException("Missing ROM: "+rom.name+". Required in "+source+" or a complete hotdo.zip.");
        }
    }
    private static String hex(byte[] bytes) {
        char[] symbols="0123456789abcdef".toCharArray(); char[] result=new char[bytes.length*2];
        for(int i=0;i<bytes.length;i++) { result[i*2]=symbols[(bytes[i]&255)>>>4]; result[i*2+1]=symbols[bytes[i]&15]; }
        return new String(result);
    }
}
