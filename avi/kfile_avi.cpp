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

 /*
 * this plugin isnt finished yet - there are some flaws in the parsing
 * structure; the end result being that it can't continue and find the
 * audio strh/strf blocks
 *
 * however, it works and is useful already ;)
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

K_EXPORT_COMPONENT_FACTORY(kfile_avi, AviFactory( "kfile_avi" ));

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

    item = addItemInfo(group, "Frame rate", i18n("Frame rate"), QVariant::Int);
    setSuffix(item, i18n("fps"));

    item = addItemInfo(group, "Video codec", i18n("Video codec"), QVariant::String);
    //item = addItemInfo(group, "Audio codec", i18n("Audio codec"), QVariant::String);

}

bool KAviPlugin::read_avi()
{
    static const char *sig_riff = "RIFF";
    static const char *sig_avi  = "AVI ";
    static const char *sig_list = "LIST";
    static const char *sig_junk = "JUNK";
    uint32_t dwbuf1;

    done_avih = false;
        
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
          ((done_avih) && (strlen(handler_vids) > 0)/* && (strlen(handler_auds) > 0)*/) || 
          f.atEnd()) {
            kdDebug(7034) << "We're done!\n";
            done = true;
        }
                    
    } while (!done);

    return true;                        
}


bool KAviPlugin::read_list()
{
    static const char *sig_hdrl = "hdrl";   // header list
    static const char *sig_strl = "strl";   // ...list
    static const char *sig_movi = "movi";   // movie list
    
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
    static const char *sig_avih = "avih";   // header list

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
    static const char *sig_strh = "strh";
    static const char *sig_strf = "strf";
    static const char *sig_vids = "vids";   // ...video
    static const char *sig_auds = "auds";   // ...audio

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

    kdDebug(7034) << "in strl handler\n";
    
        
    /*
        strl lists contain exactly  two chunks - one strh and one strf
    
        // start with strh
    */

    
    uint32_t dwbuf1;
    char charbuf1[5];
    char charbuf2[5];

    // read type and size
    f.readBlock(charbuf1, 4);
    dstream >> dwbuf1;
    
    // not a valid strh?
    if (memcmp(charbuf1, sig_strh, 4) != 0) {
        kdDebug(7034) << "Chunk ID error, expected strh, got: " << charbuf1 << "\n";
        return false;
    }
            
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

    } else {
        // we are something that we don't understand

    }

    // do we need to skip ahead any more?  (usually yes , contrary to
    // the AVI specs I've read...)
    // note: 48 is 10 * uint32_t + 2*FOURCC; the 10 fields we read above, plus the two character fields
    if (dwbuf1 > 48)
        f.at( f.at() + (dwbuf1 - 48) );
    
    /*
        now do strf
    */
    
    // read type and size
    f.readBlock(charbuf1, 4);
    dstream >> dwbuf1;

    // not a valid strh?
    if (memcmp(charbuf1, sig_strf, 4) != 0) {
        kdDebug(7034) << "Chunk ID error, expected strf, got: " << charbuf1 << "\n";
        return false;
    }
        
    // skip the strf
    f.at( f.at() + dwbuf1 );
        
    return true;    
}


bool KAviPlugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    /***************************************************/
    // prep

    memset(handler_vids, 0x00, 5);
    memset(handler_auds, 0x00, 5);

    
    /***************************************************/
    // sort out the file

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

    if (!read_avi()) {
        kdDebug(7034) << "read_avi() failed!" << endl;
    }

    /***************************************************/
    // set up our output

    if (done_avih) {

        KFileMetaInfoGroup group = appendGroup(info, "Technical");

        appendItem(group, "Frame rate", int(1000000 / avih_microsecperframe));
        appendItem(group, "Resolution", QSize(avih_width, avih_height));

        // work out and add length
        uint64_t mylength = (uint64_t) ((float) avih_totalframes * (float) avih_microsecperframe / 1000000.0);
        appendItem(group, "Length", int(mylength));


        if (strlen(handler_vids) > 0)
            appendItem(group, "Video codec", handler_vids);
        else
            appendItem(group, "Video codec", i18n("Unknown"));

        /* we havent implemented this yet
        if (strlen(handler_auds) > 0)
            appendItem(group, "Audio codec", handler_auds);
        else
            appendItem(group, "Audio codec", i18n("Unknown"));
        */
            
    }

    return true;
}

#include "kfile_avi.moc"
