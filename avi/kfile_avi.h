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

#ifndef __KFILE_AVI_H__
#define __KFILE_AVI_H__

#include <kfilemetainfo.h>
#include <qfile.h>

#if !defined(__osf__)
#include <inttypes.h>
#else
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
#endif



class QStringList;

class KAviPlugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    KAviPlugin( QObject *parent, const char *name, const QStringList& args );



    virtual bool readInfo( KFileMetaInfo& info, uint what);

private:

    bool read_avi();
    bool read_list();
    bool read_avih();
    bool read_strl();
    
    bool read_strf(uint32_t blocksize);
    bool read_strh(uint32_t blocksize);
    
    // methods to sort out human readable names for the codecs
    const char * resolve_audio(uint16_t id);
    
    QFile f;
    QDataStream dstream;

    // AVI header information
    bool done_avih;
    uint32_t avih_microsecperframe;
    uint32_t avih_maxbytespersec;
    uint32_t avih_reserved1;
    uint32_t avih_flags;
    uint32_t avih_totalframes;
    uint32_t avih_initialframes;
    uint32_t avih_streams;
    uint32_t avih_buffersize;
    uint32_t avih_width;
    uint32_t avih_height;
    uint32_t avih_scale;
    uint32_t avih_rate;
    uint32_t avih_start;
    uint32_t avih_length;

    char handler_vids[5];   // leave room for trailing \0
    char handler_auds[5];
    uint16_t handler_audio; // the ID of the audio codec
    bool done_audio;
    
    bool wantstrf;
    
};

#endif
