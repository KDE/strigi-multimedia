/***************************************************************************
 *   Copyright (C) 2004 by Jean-Baptiste Mardelle                          *
 *   bj@altern.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qfile.h>
#include <config.h>
#include "kfile_theora.h"

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "theora/theora.h"
#include "vorbis/codec.h"

ogg_stream_state t_stream_state;
ogg_stream_state v_stream_state;

int              theora_p=0;
int              vorbis_p=0;

static int queue_page(ogg_page *page)
{
    if(theora_p)
        ogg_stream_pagein(&t_stream_state,page);
    if(vorbis_p)
        ogg_stream_pagein(&v_stream_state,page);
    return 0;
}

static int buffer_data(FILE *in,ogg_sync_state *oy)
{
    char *buffer=ogg_sync_buffer(oy,4096);
    int bytes=fread(buffer,1,4096,in);
    ogg_sync_wrote(oy,bytes);
    return(bytes);
}

typedef KGenericFactory<theoraPlugin> theoraFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_theora, theoraFactory( "kfile_theora" ))

theoraPlugin::theoraPlugin(QObject *parent, const char *name,
                           const QStringList &args)
        : KFilePlugin(parent, name, args)
{
//  kdDebug(7034) << "theora plugin\n";

    KFileMimeTypeInfo* info = addMimeTypeInfo( "video/x-theora" );

    KFileMimeTypeInfo::GroupInfo* group = 0;
    KFileMimeTypeInfo::ItemInfo* item;

    // video group

    group = addGroupInfo(info, "Video", i18n("Video Details"));
    setAttributes(group, 0);
    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::Seconds);
    setHint(item, KFileMimeTypeInfo::Length);
    item = addItemInfo(group, "Resolution", i18n("Resolution"), QVariant::Size);
    setHint(item, KFileMimeTypeInfo::Size);
    setUnit(item, KFileMimeTypeInfo::Pixels);
    item = addItemInfo(group, "FrameRate", i18n("Frame Rate"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::FramesPerSecond);
    item = addItemInfo(group, "TargetBitrate", i18n("Target Bitrate"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::Bitrate);
    item = addItemInfo(group, "Quality", i18n("Quality"), QVariant::Int);

    // audio group

    group = addGroupInfo(info, "Audio", i18n("Audio Details"));
    setAttributes(group, 0);
    addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);

    item = addItemInfo(group, "SampleRate", i18n("Sample Rate"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::Hertz);
}

bool theoraPlugin::readInfo( KFileMetaInfo& info, uint what)
{
    // most of the ogg stuff was borrowed from libtheora/examples/player_example.c
    FILE *fp;

    ogg_sync_state   o_sync_state;
    ogg_packet o_packet;
    ogg_page         o_page;

    theora_info      t_info;
    theora_comment   t_comment;
    theora_state     t_state;
    vorbis_info      v_info;
    vorbis_comment   v_comment;

    theora_p=0;
    vorbis_p=0;
    int theora_serial=0;
    int stateflag=0;

    ogg_int64_t duration=0;

    // libtheora is still a bit unstable and sadly the init_ functions don't
    // take care of things the way one would expect.  So, let's do some explicit
    // clearing of these fields.

    memset(&t_info,    0, sizeof(theora_info));
    memset(&t_comment, 0, sizeof(theora_comment));
    memset(&t_state,   0, sizeof(theora_state));

    bool readTech = false;

    if (what & (KFileMetaInfo::Fastest |
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo))
        readTech = true;

    if ( info.path().isEmpty() ) // remote file
        return false;

    fp = fopen(QFile::encodeName(info.path()),"rb");
    if (!fp)
    {
        kdDebug(7034) << "Unable to open " << QFile::encodeName(info.path()) << endl;
        return false;
    }

    ogg_sync_init(&o_sync_state);

    /* init supporting Vorbis structures needed in header parsing */
    vorbis_info_init(&v_info);
    vorbis_comment_init(&v_comment);

    /* init supporting Theora structures needed in header parsing */
    theora_comment_init(&t_comment);
    theora_info_init(&t_info);

    while(!stateflag && buffer_data(fp,&o_sync_state)!=0)
    {
        while (ogg_sync_pageout(&o_sync_state,&o_page)>0)
        {
            ogg_stream_state stream_test;
            /* is this a mandated initial header? If not, stop parsing */
            if(!ogg_page_bos(&o_page))
            {
                queue_page(&o_page);
                stateflag=1;
                break;
            }

            ogg_stream_init(&stream_test,ogg_page_serialno(&o_page));
            ogg_stream_pagein(&stream_test,&o_page);
            ogg_stream_packetout(&stream_test,&o_packet);

            /* identify the codec: try theora */
            if(!theora_p && theora_decode_header(&t_info,&t_comment,&o_packet)>=0)
            {
                /* it is theora */
                memcpy(&t_stream_state,&stream_test,sizeof(stream_test));
                theora_serial=ogg_page_serialno(&o_page);
                theora_p=1;
            }
            else if(!vorbis_p && vorbis_synthesis_headerin(&v_info,&v_comment,&o_packet)>=0)
            {
                /* it is vorbis */
                memcpy(&v_stream_state,&stream_test,sizeof(stream_test));
                vorbis_p=1;
            }
            else
            {
                /* whatever it is, we don't care about it */
                ogg_stream_clear(&stream_test);
            }
        }
    }

    /* we're expecting more header packets. */
    bool corruptedHeaders=false;
    
    while((theora_p && theora_p<3) || (vorbis_p && vorbis_p<3))
    {
        int ret;
        /* look for further theora headers */
        while(theora_p && (theora_p<3) && (ret=ogg_stream_packetout(&t_stream_state,&o_packet)))
        {
            if(ret<0)
            {
                kdDebug(7034)<<"Error parsing Theora stream headers; corrupt stream?\n"<<endl;
                corruptedHeaders=true;
            }
            if(theora_decode_header(&t_info,&t_comment,&o_packet))
            {
                kdDebug(7034)<<"Error parsing Theora stream headers; corrupt stream?"<<endl;
                corruptedHeaders=true;
            }
            theora_p++;
            if(theora_p==3)
                break;
        }

        /* look for more vorbis header packets */
        while(vorbis_p && (vorbis_p<3) && (ret=ogg_stream_packetout(&v_stream_state,&o_packet)))
        {
            if(ret<0)
            {
                kdDebug(7034)<<"Error parsing Vorbis stream headers; corrupt stream"<<endl;
                corruptedHeaders=true;
            }
            if(vorbis_synthesis_headerin(&v_info,&v_comment,&o_packet))
            {
                kdDebug(7034)<<"Error parsing Vorbis stream headers; corrupt stream?"<<endl;
                corruptedHeaders=true;
            }
            vorbis_p++;
            if(vorbis_p==3)
                break;
        }
        /* The header pages/packets will arrive before anything else we
           care about, or the stream is not obeying spec */

        if(ogg_sync_pageout(&o_sync_state,&o_page)>0)
        {
            queue_page(&o_page);
            /* demux into the appropriate stream */
        }
        else
        {
            int ret=buffer_data(fp,&o_sync_state); /* someone needs more data */
            if(ret==0)
            {
                kdDebug(7034)<<"End of file while searching for codec headers."<<endl;
                corruptedHeaders=true;
            }
        }
    }

    /* and now we have it all.  initialize decoders */
    if(theora_p && !corruptedHeaders)
    {
        theora_decode_init(&t_state,&t_info);
    }
    else
    {
        /* tear down the partial theora setup */
        theora_info_clear(&t_info);
        theora_comment_clear(&t_comment);

        vorbis_info_clear(&v_info);
        vorbis_comment_clear(&v_comment);
        ogg_sync_clear(&o_sync_state);
        fclose(fp);
        return false;
    }
    //queue_page(&o_page);

    while (buffer_data(fp,&o_sync_state))
    {
        while (ogg_sync_pageout(&o_sync_state,&o_page)>0)
        {
            // The following line was commented out by Scott Wheeler <wheeler@kde.org>
            // We don't actually need to store all of the pages / packets in memory since
            // (a) libtheora doesn't use them anyway in the one call that we make after this
            // that usese t_state and (b) it basically buffers the entire file to memory if
            // we queue them up like this and that sucks where a typical file size is a few
            // hundred megs.

            // queue_page(&o_page);
        }
        if (theora_serial==ogg_page_serialno(&o_page))
            duration=(ogg_int64_t) theora_granule_time(&t_state,ogg_page_granulepos(&o_page));
    }

    if (readTech)
    {
        int stream_fps=0;
        if (t_info.fps_denominator!=0)
            stream_fps=t_info.fps_numerator/t_info.fps_denominator;
        KFileMetaInfoGroup videogroup = appendGroup(info, "Video");
        appendItem(videogroup, "Length", int (duration));
        appendItem(videogroup, "Resolution", QSize(t_info.frame_width,t_info.frame_height));
        appendItem(videogroup, "FrameRate", stream_fps);
        appendItem(videogroup, "Quality", (int) t_info.quality);

        KFileMetaInfoGroup audiogroup = appendGroup(info, "Audio");
        appendItem(audiogroup, "Channels", v_info.channels);
        appendItem(audiogroup, "SampleRate", int(v_info.rate));
    }
    fclose(fp);

    if (vorbis_p)
    {
        ogg_stream_clear(&v_stream_state);
        vorbis_comment_clear(&v_comment);
        vorbis_info_clear(&v_info);
    }

    ogg_stream_clear(&t_stream_state);
    theora_clear(&t_state);
    theora_comment_clear(&t_comment);
    theora_info_clear(&t_info);
    ogg_sync_clear(&o_sync_state);

    return true;
}

#include "kfile_theora.moc"

