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
    //item = addItemInfo(group, "Audeo codec", i18n("Audio codec"), QVariant::String);

}


unsigned short KAviPlugin::readHeader()
{
    // defs for block markers
	static const unsigned short mlen = 4;
	static const char *sig_riff = "RIFF";   // main RIFF marker
    static const char *sig_list = "LIST";   // list marker
    static const char *sig_junk = "JUNK";   // junk marker

	static const char *sig_avi  = "AVI ";   // main AVI marker
    static const char *sig_hdrl = "hdrl";   // header list
    static const char *sig_strl = "strl";   // ...list
    //static const char *sig_movi = "movi";   // movie data

    // buffers for holding stuff we want to look at
    uint32_t dwbuf1;
    uint32_t dwbuf2;

    unsigned short retval = 0;
    bool done = false;
    while (!done) {
        // read a chunk of block, and the dword after it which is its size
        dstream >> dwbuf1;
        dstream >> dwbuf2;

        // check what block marker it is
        if (memcmp(&dwbuf1, sig_riff, mlen) == 0) {
            // we have a RIFF block, this should be the start of the AVI

            // check for avi
            dstream >> dwbuf1;
            if (memcmp(&dwbuf1, sig_avi, mlen) != 0) {
                // we are not a valid machine
                kdDebug(7034) << "Not a valid AVI - failed 2nd block\n";
                done = true;
            }

        } else if (memcmp(&dwbuf1, sig_list, mlen) == 0) {
            // found a list marker, need to see what sort of list and do stuff

            // hdrl | movi (anything else means we should be in readList instead!
            dstream >> dwbuf1;
            //f->readBlock(&dwbuf1, mlen);
            if (memcmp(&dwbuf1, sig_hdrl, mlen) == 0) {
                // we are a header list!
                retval = readList_hdrl();
            } else if (memcmp(&dwbuf1, sig_strl, mlen) == 0) {
                // we are a stream list, pass the size to the handler method
                retval = readList_strl();
            }

        } else if (memcmp(&dwbuf1, sig_junk, mlen) == 0) {
            // found a junk marker, need to just skip to the end of the block

            // size is in dwbuf2, seek ahead by that amount...
            // FIXME: implement this!

            //f->at( f->at() + (unsigned long) dwbuf2 );

        } else {
            //kdDebug(7034) << "Not a valid AVI - failed a primary marker\n";
            return 1;
        }

    }

    return 1;
}

unsigned short KAviPlugin::readList_hdrl()
{
    static const char *sig_avih = "avih";   // AVI header data
    uint32_t dwbuf1;
	uint32_t blksize;


    kdDebug(7034) << "Reading hdrl\n";

    // this should start with 'avih'
    dstream >> dwbuf1;
    if (memcmp(&dwbuf1, sig_avih, 4) != 0) {
	    // not an avih header
        return 1;
	}

	// then we should have a size
    dstream >> blksize;

    // check the size is the same as the data we're after

	// now grab the data
    avih = true;

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

    return 1;
}


unsigned short KAviPlugin::readList_strl()
{
	// FIXME: we only currently support one set of strf/strh!

    // defs for block markers
	static const unsigned short mlen = 4;
    //static const char *sig_strl = "strl";   // ...list
    static const char *sig_strh = "strh";   // ...header
    //static const char *sig_strf = "strf";   // ...format
    
    static const char *sig_vids = "vids";   // ...video
    static const char *sig_auds = "auds";   // ...audio

    // defs for strh data fields
    uint32_t strh_type;
    uint32_t strh_handler;
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


    kdDebug(7034) << "Reading strl\n";


    // buffers for holding stuff we want to look at
    uint32_t dwbuf1;
    uint32_t dwbuf2;

    dstream >> dwbuf1; // stream header
    dstream >> dwbuf2; // // header structure length

    // check the marker
    if (memcmp(&dwbuf1, sig_strh, mlen) != 0) {
        // we needed a strh!
        return 1;
    }

    // now read the strh data
    dstream >> strh_type;
    dstream >> strh_handler;
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

    // are we video or audio??
    if (memcmp(&strh_type, sig_vids, 4) == 0) {
        // we are video!

        // save the handler
        memcpy(handler_vids, &strh_handler, 4);
        kdDebug(7034) << "Video handler: " << handler_vids << "\n";


    } else if (memcmp(&dwbuf1, sig_auds, 4) == 0) {
        // we are audio!

        // save the handler
        memcpy(handler_auds, &strh_handler, 4);
        kdDebug(7034) << "Audio handler: " << handler_auds << "\n";

    } else {
        // we are something that we don't understand

    }


    return 0;
}


bool KAviPlugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    /***************************************************/
    // prep

    avih = false;
    memset(handler_vids, 0x00, 5);
    memset(handler_auds, 0x00, 5);

    /***************************************************/
    // sort out the file

    QFile file(info.path());


    // open file, set up stream and set endianness
    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }
    //QDataStream dstream(&file);
    dstream.setDevice(&file);

    dstream.setByteOrder(QDataStream::LittleEndian);


    /***************************************************/
    // start reading stuff from it

    readHeader();

    /***************************************************/
    // set up our output

    if (avih) {

        KFileMetaInfoGroup group = appendGroup(info, "Technical");

        appendItem(group, "Frame rate", int(1000000 / avih_microsecperframe));
        appendItem(group, "Resolution", QSize(avih_width, avih_height));

        // work out and add length
        uint64_t mylength = (uint64_t) ((float) avih_totalframes * (float) avih_microsecperframe / 1000000.0);
        appendItem(group, "Length", int(mylength));


        if (strlen(handler_vids) > 0)
            appendItem(group, "Video codec", handler_vids);
        else
            appendItem(group, "Video codec", i18n("None"));

        /* we havent implemented this yet
        if (strlen(handler_auds) > 0)
            appendItem(group, "Audio codec", handler_auds);
        else
            appendItem(group, "Audio codec", i18n("None"));
        */
            
    }

    return true;
}

#include "kfile_avi.moc"
