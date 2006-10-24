/* This file is part of the KDE project
 * Copyright (C) 2005 Allan Sandfeld Jensen <kde@carewolf.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

// MPEG KFile plugin
// Based on reading sourcecode of mpeglib, xinelib and libmpeg3
// and studying MPEG dumps.

#include <config.h>
#include "kfile_mpeg.h"

#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <qdict.h>
#include <qvalidator.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>

// #include <iostream>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef KGenericFactory<KMpegPlugin> MpegFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_mpeg, MpegFactory( "kfile_mpeg" ))

KMpegPlugin::KMpegPlugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "video/mpeg" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::Seconds);

    item = addItemInfo(group, "Resolution", i18n("Resolution"), QVariant::Size);

    item = addItemInfo(group, "Frame rate", i18n("Frame Rate"), QVariant::Double);
    setSuffix(item, i18n("fps"));

    item = addItemInfo(group, "Video codec", i18n("Video Codec"), QVariant::String);
    item = addItemInfo(group, "Audio codec", i18n("Audio Codec"), QVariant::String);

    item = addItemInfo(group, "Aspect ratio", i18n("Aspect ratio"), QVariant::String);
}

// Frame-rate table from libmpeg3
float frame_rate_table[16] =
{
    0.0,   /* Pad */
    (float)24000.0/1001.0,       /* Official frame rates */
    (float)24.0,
    (float)25.0,
    (float)30000.0/1001.0,
    (float)30.0,
    (float)50.0,
    (float)60000.0/1001.0,
    (float)60.0,

    1,                    /* Unofficial economy rates */
    5,
    10,
    12,
    15,
    0,
    0,
};

/* Bitrate indexes */
int bitrate_123[3][16] =
   { {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
     {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
     {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,} };

static const uint16_t sequence_start = 0x01b3;
static const uint16_t ext_sequence_start = 0x01b5;
static const uint16_t gop_start = 0x01b8;
static const uint16_t audio1_packet = 0x01c0;
static const uint16_t audio2_packet = 0x01d0;
static const uint16_t private1_packet = 0x01bd;
static const uint16_t private2_packet = 0x01bf;

int KMpegPlugin::parse_seq() {
    uint32_t buf;
    dstream >> buf;

    horizontal_size = (buf >> 20);
    vertical_size = (buf >> 8) & ((1<<12)-1);
    aspect_ratio = (buf >> 4) & ((1<<4)-1);
    int framerate_code = buf & ((1<<4)-1);
    frame_rate = frame_rate_table[framerate_code];

    dstream >> buf;

    bitrate = (buf >> 14);
//     kdDebug(7034) << "bitrate: " << bitrate << endl;
    bool has_intra_matrix = buf & 2;
    bool has_non_intra_matrix = buf & 1;

    int matrix = 0;
    if (has_intra_matrix) matrix +=64;
    if (has_non_intra_matrix) matrix +=64;

    mpeg = 1;
    return matrix;
}

void KMpegPlugin::parse_seq_ext() {
    uint32_t buf;
    dstream >> buf;

    uint8_t type = buf >> 28;
    if (type == 1)
        mpeg = 2;

    /*
    else
    if (type == 2) {
        dstream >> buf;
        // These are display-sizes. I let them override physical sizes.
        horizontal_size = (buf >> 18);
        vertical_size = (buf >> 1) & ((1<<14)-1);
} */
}

long KMpegPlugin::parse_gop() {
    uint32_t buf;
    dstream >> buf;
    dstream >> buf;

    int gop_hour = (buf>>26) & ((1<<5)-1);
    kdDebug(7034) << "gop_hour: " << gop_hour << endl;
    int gop_minute = (buf>>20) & ((1<<6)-1);
    kdDebug(7034) << "gop_minute: " << gop_minute << endl;
    int gop_second = (buf>>13) & ((1<<6)-1);
    kdDebug(7034) << "gop_second: " << gop_second << endl;
    int gop_frame = (buf>>7) & ((1<<6)-1);
    kdDebug(7034) << "gop_frame: " << gop_frame << endl;

    long seconds = gop_hour*60*60 + gop_minute*60 + gop_second;
    return (long)seconds;
}

int KMpegPlugin::parse_audio() {
    uint16_t len;
    dstream >> len;
//     kdDebug(7034) << "Length of audio packet: " << len << endl;

    uint8_t buf;
    int i = 0;
    for(i=0; i<20; i++) {
        dstream >> buf;
        if (buf == 0xff) {
            dstream >> buf;
            if ((buf & 0xe0) == 0xe0)
                goto found_sync;
        }
    }
    kdDebug(7034) << "MPEG audio sync not found" << endl;
    return len-i;

found_sync:

    int layer = ((buf >> 1) & 0x3);
    if (layer == 1)
        audio_type = 3;
    else if (layer == 2)
        audio_type = 2;
    else if (layer == 3)
        audio_type = 1;
    else
        kdDebug(7034) << "Invalid MPEG audio layer" << endl;

    dstream >> buf;
    int bitrate_index = (buf & 0xf0) >> 4;
    audio_rate = bitrate_123[3-layer][bitrate_index];

    return len-3-i;
}

int KMpegPlugin::skip_packet() {
    uint16_t len;
    dstream >> len;
//     kdDebug(7034) << "Length of skipped packet: " << len << endl;

    return len;
}

int KMpegPlugin::skip_riff_chunk() {
    dstream.setByteOrder(QDataStream::LittleEndian);
    uint32_t len;
    dstream >> len;
//     std::cerr << "Length of skipped chunk: " << len << std::endl;

    dstream.setByteOrder(QDataStream::BigEndian);
    return len;
}

int KMpegPlugin::parse_private() {
    uint16_t len;
    dstream >> len;
//     kdDebug(7034) << "Length of private packet: " << len << endl;

    // Match AC3 packets
    uint8_t subtype;
    dstream >> subtype;
    subtype = subtype >> 4;
    if (subtype == 8)   // AC3
        audio_type = 5;
    else
    if (subtype == 10)  // LPCM
        audio_type = 7;

    return len-1;
}

bool KMpegPlugin::find_mpeg_in_cdxa()
{
    int skip_len = 0;
    uint32_t magic;
    uint32_t data_len;
    // search for data chunk
    while (true) {
        dstream >> magic;
        if (magic != 0x64617461) { // "fmt "
            skip_len = skip_riff_chunk();
            if (!file.at(file.at()+skip_len)) return false;
            continue;
        } else {
            // size of chunk
            dstream >> data_len;
            int block = 0;
            // search for mpeg part
            while(block < 32) {
                // check for CDXA sync thingy
                dstream >> magic;
                // 00 ff ff ff ff ff ff ff ff ff ff ff 00
                if (magic == 0x00ffffff) {
//                     std::cerr << "Found CD sync" << std::endl;
                    // skip 20 bytes
                    if (!file.at(file.at()+20)) return false;
                    dstream >> magic;
                    if (magic == 0x000001ba) {
//                         std::cerr << "Found CDXA mpeg" << std::endl;
                        return true;
                    }
                    else {
//                         std::cerr << "CDXA block: #" <<block << ": " << magic << std::endl;
                        if (!file.at(file.at()+2324)) return false;
                        block++;
                        continue;
                    }
                } else {
//                     std::cerr << "Incorrect CDXA block" << std::endl;
                    // shouldn't happen
                    return true;
                }
            }
            return false;
        }
    }
}

bool KMpegPlugin::read_mpeg()
{
    mpeg = 0;
    audio_type = 0;
    audio_rate = 0;

    uint32_t magic;
    dstream >> magic;
    if (magic == 0x52494646) // == "RIFF"
    {
        dstream >> magic;
        dstream >> magic;
        if (magic != 0x43445841) { // 0x43445841 == "CDXA"
            kdDebug(7034) << "Unknown RIFF file" << endl;
            return false;
        } else {
            if (!find_mpeg_in_cdxa()) return false;
        }
    }
    else
    if (magic != 0x000001ba) {
        kdDebug(7034) << "Not a MPEG-PS file" << endl;
        return false;
    }
//     file.at(0);

    uint8_t byte;
    int skip_len = 0;
    int state = 0;
    int skimmed = 0;
    int video_len = 0;
    int searched = 0;
    bool video_found = false, audio_found = false, gop_found = false;
    // Search for MPEG packets
    for(int i=0; i < 2048; i++) {
        dstream >> byte;
        skimmed++;
        searched++;
        if (video_len > 0) video_len--;
        // Use a fast state machine to find 00 00 01 sync code
        switch (state) {
            case 0:
                if (byte == 0)
                    state = 1;
                else
                    state = 0;
                break;
            case 1:
                if (byte == 0)
                    state = 2;
                else
                    state = 0;
                break;
            case 2:
                if (byte == 0)
                    state = 2;
                else
                if (byte == 1)
                    state = 3;
                else
                    state = 0;
                break;
            case 3: {
                skimmed -= 4;
                if (skimmed) {
//                     kdDebug(7034) << "Bytes skimmed:" << skimmed << endl;
                    skimmed = 0;
                }
//                 kdDebug(7034) << "Packet of type:" << QString::number(byte,16) << endl;
                switch (byte) {
                case 0xb3:
                    if (video_found) break;
                    skip_len = parse_seq();
                    video_found = true;
                    video_len -= 8;
                    video_len -= skip_len;
                    break;
                case 0xb5:
                    parse_seq_ext();
                    video_len -= 4;
                    break;
                case 0xb8:
                /*
                    if (!gop_found) {
                        start_time = parse_gop();
                        gop_found = true;
                        kdDebug(7034) << "start_time: " << start_time << endl;
                    }
                */
                    /* nobreak */
                case 0x00:
                case 0x01:
                    // skip the rest of the video data
                    if (video_len > 0 && video_found)
                        skip_len = video_len;
                    break;
                /*
                case 0xb2:
                    skip_len = parse_user();
                    break;
                */
                case 0xba:
                    skip_len = 8;
                    break;
                case 0xbe:
                    // padding
                    skip_len = skip_packet();
                    break;
                case 0xe0:
                    // video data
                    if (video_found)
                        skip_len = skip_packet();
                    else
                        video_len = skip_packet();
                    break;
                case 0xbd:
                case 0xbf:
                    skip_len = parse_private();
                    break;
                case 0xc0:
                case 0xd0:
                    skip_len = parse_audio();
                    audio_found = true;
                    break;
                 default:
//                      kdDebug(7034) << "Unhandled packet of type:" << QString::number(byte,16) << endl;
                    break;
                }
                state = 0;
                break;
            }
        }

        if (video_found && audio_found /*&& gop_found*/) break;
        if (skip_len) {
            if (!file.at(file.at()+skip_len))
                return false;
            searched += skip_len;
            skip_len = 0;
        }
    }
    /*
    if (skimmed)
        kdDebug(7034) << "Bytes skimmed:" << skimmed << endl;
    kdDebug(7034) << "Bytes searched:" << searched << endl;
    */

    if (mpeg == 0) {
        kdDebug(7034) << "No sequence-start found" << endl;
        return false;
    }
    return true;
}

// Search for the last GOP packet and read the time field
void KMpegPlugin::read_length()
{
    end_time = 0;
    uint8_t byte;
    int state = 0;
    // Search for the last gop
    file.at(file.size()-1024);
    for(int j=1; j<64; j++) {
//        dstream.setDevice(&file);
//        dstream.setByteOrder(QDataStream::BigEndian);
        for(int i=0; i<1024; i++) {
            dstream >> byte;
            switch (state) {
                case 0:
                    if (byte == 0)
                        state = 1;
                    else
                        state = 0;
                    break;
                case 1:
                    if (byte == 0)
                        state = 2;
                    else
                        state = 0;
                case 2:
                    if (byte == 0)
                        state = 2;
                    else
                    if (byte == 1)
                        state = 3;
                    else
                        state = 0;
                case 3:
                    if (byte == 0xb8) {
                        end_time = parse_gop();
                        kdDebug(7034) << "end_time: " << end_time << endl;
                        return;
                    }
                    state = 0;
            }
        }
        state = 0;
        file.at(file.size()-j*1024);
    }
    kdDebug(7034) << "No end GOP found" << endl;
}

bool KMpegPlugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    if ( info.path().isEmpty() ) // remote file
        return false;

    file.setName(info.path());

    // open file, set up stream and set endianness
    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }

    dstream.setDevice(&file);
    dstream.setByteOrder(QDataStream::BigEndian);

    start_time = end_time = 0L;

    if (!read_mpeg()) {
        kdDebug(7034) << "read_mpeg() failed!" << endl;
    }
    else {
        KFileMetaInfoGroup group = appendGroup(info, "Technical");

        appendItem(group, "Frame rate", double(frame_rate));

        appendItem(group, "Resolution", QSize(horizontal_size, vertical_size));
        /* The GOP timings are completely bogus
        read_length();
        if (end_time != 0) {
            //long total_frames = end_time-start_time + 1;
            long total_time = end_time;
            appendItem(group, "Length", int(total_time));
        }
        // and so is bitrate
        long total_time = file.size()/((bitrate+audio_rate)*50);
        appendItem(group, "Length", int(total_time));
        */
        if (mpeg == 1)
            appendItem(group, "Video codec", "MPEG-1");
        else
            appendItem(group, "Video codec", "MPEG-2");

        switch (audio_type) {
            case 1:
                appendItem(group, "Audio codec", "MP1");
                break;
            case 2:
                appendItem(group, "Audio codec", "MP2");
                break;
            case 3:
                appendItem(group, "Audio codec", "MP3");
                break;
            case 5:
                appendItem(group, "Audio codec", "AC3");
                break;
            case 7:
                appendItem(group, "Audio codec", "PCM");
                break;
            default:
                appendItem(group, "Audio codec", i18n("Unknown"));
        }
        // MPEG 1 also has an aspect ratio setting, but it works differently,
        // and I am not sure if it is used.
        if (mpeg == 2) {
            switch (aspect_ratio) {
                case 1:
                    appendItem(group, "Aspect ratio", i18n("default"));
                    break;
                case 2:
                    appendItem(group, "Aspect ratio", "4/3");
                    break;
                case 3:
                    appendItem(group, "Aspect ratio", "16/9");
                    break;
                case 4:
                    appendItem(group, "Aspect ratio", "2.11/1");
                    break;
            }
        }
    }

    file.close();
    return true;
}

#include "kfile_mpeg.moc"
