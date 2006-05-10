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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef __KFILE_MPEG_H__
#define __KFILE_MPEG_H__

#include <kfilemetainfo.h>
#include <QFile>

class QStringList;

class KMpegPlugin: public KFilePlugin
{
    Q_OBJECT

public:
    KMpegPlugin( QObject *parent,  const QStringList& args );

    virtual bool readInfo( KFileMetaInfo& info, uint what);

private:
    int parse_seq();
    void parse_seq_ext();
    long parse_gop();
    int parse_audio();
    int parse_private();
    int skip_packet();

    bool read_mpeg();
    void read_length();

    QFile file;
    QDataStream dstream;

    // MPEG information
    int horizontal_size;
    int vertical_size;
    int aspect_ratio;
    int bitrate;
    float frame_rate;

    int mpeg;
    int audio_type;

    long start_time;
    long end_time;


};

#endif
