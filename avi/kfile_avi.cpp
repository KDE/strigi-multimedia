/* This file is part of the KDE project
 * Copyright (C) 2002 Shane Wright <me@shanewright.co.uk>
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


#include <config.h>
#include "kfile_avi.h"

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

#if !defined(__osf__)
#include <inttypes.h>
#else
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;
#endif

typedef KGenericFactory<KAviPlugin> AviFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_avi, AviFactory( "kfile_avi" ))

KAviPlugin::KAviPlugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "video/x-msvideo" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::Seconds);

    item = addItemInfo(group, "Resolution", i18n("Resolution"), QVariant::Size);

    item = addItemInfo(group, "Frame rate", i18n("Frame Rate"), QVariant::Int);
    setSuffix(item, i18n("fps"));

    item = addItemInfo(group, "Video codec", i18n("Video Codec"), QVariant::String);
    item = addItemInfo(group, "Audio codec", i18n("Audio Codec"), QVariant::String);

}

bool KAviPlugin::read_avi()
{
    static const char sig_riff[] = "RIFF";
    static const char sig_avi[]  = "AVI ";
    static const char sig_list[] = "LIST";
    static const char sig_junk[] = "JUNK";
    uint32_t dwbuf1;

    done_avih = false;
    done_audio = false;

    // read AVI header
    char charbuf1[5];
    charbuf1[4] = '\0';

    // this must be RIFF
    f.readBlock(charbuf1, 4);
    if (memcmp(charbuf1, sig_riff, 4) != 0)
        return false;

    dstream >> dwbuf1;

    // this must be AVI
    f.readBlock(charbuf1, 4);
    if (memcmp(charbuf1, sig_avi, 4) != 0)
        return false;


    // start reading AVI file
    int counter = 0;
    bool done = false;
    do {

        // read header
        f.readBlock(charbuf1, 4);

        kdDebug(7034) << "about to handle chunk with ID: " << charbuf1 << "\n";

        if (memcmp(charbuf1, sig_list, 4) == 0) {
            // if list
            if (!read_list())
                return false;

        } else if (memcmp(charbuf1, sig_junk, 4) == 0) {
            // if junk

            // read chunk size
            dstream >> dwbuf1;

            kdDebug(7034) << "Skipping junk chunk length: " << dwbuf1 << "\n";

            // skip junk
            f.at( f.at() + dwbuf1 );

        } else {
            // something we dont understand yet
            kdDebug(7034) << "Unknown chunk header found: " << charbuf1 << "\n";
            return false;
        };

        if (
          ((done_avih) && (strlen(handler_vids) > 0) && (done_audio)) ||
          f.atEnd()) {
            kdDebug(7034) << "We're done!\n";
            done = true;
        }

        // make sure we dont stay here forever
        ++counter;
        if (counter > 10)
            done = true;

    } while (!done);

    return true;
}


bool KAviPlugin::read_list()
{
    const char sig_hdrl[] = "hdrl";   // header list
    const char sig_strl[] = "strl";   // ...list
    const char sig_movi[] = "movi";   // movie list

    uint32_t dwbuf1;
    char charbuf1[5];
    charbuf1[4] = '\0';

    kdDebug(7034) << "In read_list()\n";

    // read size & list type
    dstream >> dwbuf1;
    f.readBlock(charbuf1, 4);

    // read the relevant bits of the list
    if (memcmp(charbuf1, sig_hdrl, 4) == 0) {
        // should be the main AVI header
        if (!read_avih())
            return false;

    } else if (memcmp(charbuf1, sig_strl, 4) == 0) {
        // should be some stream info
        if (!read_strl())
            return false;

    } else if (memcmp(charbuf1, sig_movi, 4) == 0) {
        // movie list

        kdDebug(7034) << "Skipping movi chunk length: " << dwbuf1 << "\n";

        // skip past it
        f.at( f.at() + dwbuf1 );

    } else {
        // unknown list type
        kdDebug(7034) << "Unknown list type found: " << charbuf1 << "\n";
    }

    return true;
}


bool KAviPlugin::read_avih()
{
    static const char sig_avih[] = "avih";   // header list

    uint32_t dwbuf1;
    char charbuf1[5];

    // read header and length
    f.readBlock(charbuf1, 4);
    dstream >> dwbuf1;

    // not a valid avih?
    if (memcmp(charbuf1, sig_avih, 4) != 0) {
        kdDebug(7034) << "Chunk ID error, expected avih, got: " << charbuf1 << "\n";
        return false;
    }

    // read all the avih fields
    dstream >> avih_microsecperframe;
    dstream >> avih_maxbytespersec;
    dstream >> avih_reserved1;
    dstream >> avih_flags;
    dstream >> avih_totalframes;
    dstream >> avih_initialframes;
    dstream >> avih_streams;
    dstream >> avih_buffersize;
    dstream >> avih_width;
    dstream >> avih_height;
    dstream >> avih_scale;
    dstream >> avih_rate;
    dstream >> avih_start;
    dstream >> avih_length;

    done_avih = true;

    return true;
}


bool KAviPlugin::read_strl()
{
    static const char sig_strh[] = "strh";
    static const char sig_strf[] = "strf";
    //static const char sig_strd[] = "strd";
    static const char sig_strn[] = "strn";
    static const char sig_list[] = "LIST";
    static const char sig_junk[] = "JUNK";

    kdDebug(7034) << "in strl handler\n";

    uint32_t dwbuf1;    // buffer for block sizes
    char charbuf1[5];

    // loop through blocks
    int counter = 0;
    while (true) {

        // read type and size
        f.readBlock(charbuf1, 4);   // type
        dstream >> dwbuf1;          // size

        // detect type
        if (memcmp(charbuf1, sig_strh, 4) == 0) {
            // got strh - stream header
            kdDebug(7034) << "Found strh, calling read_strh()\n";
            read_strh(dwbuf1);

        } else if (memcmp(charbuf1, sig_strf, 4) == 0) {
            // got strf - stream format
            kdDebug(7034) << "Found strf, calling read_strf()\n";
            read_strf(dwbuf1);

        } else if (memcmp(charbuf1, sig_strn, 4) == 0) {
            // we ignore strn, but it can be recorded incorrectly so we have to cope especially

            // skip it
            kdDebug(7034) << "Skipping strn chunk length: " << dwbuf1 << "\n";
            f.at( f.at() + dwbuf1 );

            /*
            this is a pretty annoying hack; many AVIs incorrectly report the
            length of the strn field by 1 byte.  Its possible that strn's
            should be word aligned, but no mention in the specs...

            I'll clean/optimise this a touch soon
            */

            bool done = false;
            unsigned char counter = 0;
            while (!done) {
                // read next marker
                f.readBlock(charbuf1, 4);

                // does it look ok?
                if ((memcmp(charbuf1, sig_list, 4) == 0) ||
                    (memcmp(charbuf1, sig_junk, 4) == 0)) {
                    // yes, go back before it
                    f.at( f.at() - 4);
                    done = true;
                } else {
                    // no, skip one space forward from where we were
                    f.at( f.at() - 3);
                    kdDebug(7034) << "Working around incorrectly marked strn length..." << "\n";
                }

                // make sure we don't stay here too long
                ++counter;
                if (counter>10)
                    done = true;
            }

        } else if ((memcmp(charbuf1, sig_list, 4) == 0) || (memcmp(charbuf1, sig_junk, 4) == 0)) {
            // we have come to the end of our stay here in strl, time to leave

            kdDebug(7034) << "Found LIST/JUNK, returning...\n";

            // rollback before the id and size
            f.at( f.at() - 8 );

            // return back to the main avi parser
            return true;

        } else {
            // we have some other unrecognised block type

            kdDebug(7034) << "Sskipping unrecognised block\n";
            // just skip over it
            f.at( f.at() + dwbuf1);

        } /* switch block type */

        ++counter;
        if (counter > 10)
            return true;

    } /* while (true) */

    // we should never get here
}


bool KAviPlugin::read_strh(uint32_t blocksize)
{
    static const char sig_vids[] = "vids";   // ...video
    static const char sig_auds[] = "auds";   // ...audio

    uint32_t strh_flags;
    uint32_t strh_reserved1;
    uint32_t strh_initialframes;
    uint32_t strh_scale;
    uint32_t strh_rate;
    uint32_t strh_start;
    uint32_t strh_length;
    uint32_t strh_buffersize;
    uint32_t strh_quality;
    uint32_t strh_samplesize;

    char charbuf1[5];
    char charbuf2[5];


    // get stream info type, and handler id
    f.readBlock(charbuf1, 4);
    f.readBlock(charbuf2, 4);

    // read the strh fields
    dstream >> strh_flags;
    dstream >> strh_reserved1;
    dstream >> strh_initialframes;
    dstream >> strh_scale;
    dstream >> strh_rate;
    dstream >> strh_start;
    dstream >> strh_length;
    dstream >> strh_buffersize;
    dstream >> strh_quality;
    dstream >> strh_samplesize;

    if (memcmp(&charbuf1, sig_vids, 4) == 0) {
        // we are video!

        // save the handler
        memcpy(handler_vids, charbuf2, 4);
        kdDebug(7034) << "Video handler: " << handler_vids << "\n";


    } else if (memcmp(&charbuf1, sig_auds, 4) == 0) {
        // we are audio!

        // save the handler
        memcpy(handler_auds, charbuf2, 4);
        kdDebug(7034) << "Audio handler: " << handler_auds << "\n";

        // we want strf to get the audio codec
        wantstrf = true;

    } else {
        // we are something that we don't understand

    }

    // do we need to skip ahead any more?  (usually yes , contrary to
    // the AVI specs I've read...)
    // note: 48 is 10 * uint32_t + 2*FOURCC; the 10 fields we read above, plus the two character fields
    if (blocksize > 48)
        f.at( f.at() + (blocksize - 48) );

    return true;
}


bool KAviPlugin::read_strf(uint32_t blocksize)
{
    // do we want to do the strf?
    if (wantstrf) {
        // yes.  we want the audio codec identifier out of it

        // get the 16bit audio codec ID
        dstream >> handler_audio;
        kdDebug(7034) << "Read audio codec ID: " << handler_audio << "\n";
        // skip past the rest of the stuff here for now
        f.at( f.at() + blocksize - 2);
        // we have audio
        done_audio = true;

    } else {
        // no, skip the strf
        f.at( f.at() + blocksize );
    }

    return true;
}



const char * KAviPlugin::resolve_audio(uint16_t id)
{
    /*
        this really wants to use some sort of KDE global
        list.  To avoid bloat for the moment it only does
        a few common codecs
    */

    static const char codec_unknown[] = I18N_NOOP("Unknown");
    static const char codec_01[]  = "Microsoft PCM";
    static const char codec_02[]  = "Microsoft ADPCM";
    static const char codec_50[]  = "MPEG";
    static const char codec_55[]  = "MP3";
    static const char codec_92[]  = "AC3";
    static const char codec_160[] = "WMA1";
    static const char codec_161[] = "WMA2";
    static const char codec_162[] = "WMA3";
    static const char codec_2000[] = "DVM";
    switch (id) {
    case 0x000 : return codec_unknown; break;
    case 0x001 : return codec_01; break;
    case 0x002 : return codec_02; break;
    case 0x050 : return codec_50; break;
    case 0x055 : return codec_55; break;
    case 0x092 : return codec_92; break;
    case 0x160 : return codec_160; break;
    case 0x161 : return codec_161; break;
    case 0x162 : return codec_162; break;
    case 0x2000 : return codec_2000; break;
    default : return codec_unknown;
    }

    return NULL;
}


bool KAviPlugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    /***************************************************/
    // prep

    memset(handler_vids, 0x00, 5);
    memset(handler_auds, 0x00, 5);


    /***************************************************/
    // sort out the file

    if (f.isOpen())
        f.close();

    if ( info.path().isEmpty() ) // remote file
        return false;

    f.setName(info.path());

    // open file, set up stream and set endianness
    if (!f.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }
    //QDataStream dstream(&file);
    dstream.setDevice(&f);

    dstream.setByteOrder(QDataStream::LittleEndian);


    /***************************************************/
    // start reading stuff from it

    wantstrf = false;

    if (!read_avi()) {
        kdDebug(7034) << "read_avi() failed!" << endl;
    }

    /***************************************************/
    // set up our output

    if (done_avih) {

        KFileMetaInfoGroup group = appendGroup(info, "Technical");

	if (0 != avih_microsecperframe) {
	    appendItem(group, "Frame rate", int(1000000 / avih_microsecperframe));
	}
        appendItem(group, "Resolution", QSize(avih_width, avih_height));

        // work out and add length
        uint64_t mylength = (uint64_t) ((float) avih_totalframes * (float) avih_microsecperframe / 1000000.0);
        appendItem(group, "Length", int(mylength));


        if (strlen(handler_vids) > 0)
            appendItem(group, "Video codec", handler_vids);
        else
            appendItem(group, "Video codec", i18n("Unknown"));

        if (done_audio)
            appendItem(group, "Audio codec", i18n(resolve_audio(handler_audio)));
        else
            appendItem(group, "Audio codec", i18n("None"));

    }

    f.close();
    return true;
}

#include "kfile_avi.moc"
