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
#include "kfile_au.h"

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
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
#endif

typedef KGenericFactory<KAuPlugin> AuFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_au, AuFactory( "kfile_au" ))

KAuPlugin::KAuPlugin(QObject *parent, const char *name,
                       const QStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/basic" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setSuffix(item, "s");

    item = addItemInfo(group, "Sample Rate", i18n("Sample Rate"), QVariant::Int);
    setSuffix(item, "Hz");

    item = addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);

    item = addItemInfo(group, "Encoding", i18n("Encoding"), QVariant::String);

}

bool KAuPlugin::readInfo( KFileMetaInfo& info, uint what)
{
    // the file signature, wants to be tidier...
    const char fsig[] = { 0x2e, 0x73, 0x6e, 0x64 };

    // a dword buffer for input
    char inbuf[4];

    // some vars for the file properties
    uint32_t datasize;
    uint32_t encoding;
    uint32_t samplerate;
    uint32_t channels;
    uint16_t bytespersample;

    if ( info.path().isEmpty() ) // remote file
        return false;

    QFile file(info.path());

    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }

    QDataStream dstream(&file);

    // AU files are big-endian
    dstream.setByteOrder(QDataStream::BigEndian);


    // Read and verify the signature
    dstream.readRawBytes(inbuf, 4);
    if (memcmp(fsig, inbuf, 4))
        return false;

    // skip unwanted bits
    file.at(8);

    // grab the bits we want
    dstream >> datasize;
    dstream >> encoding;
    dstream >> samplerate;
    dstream >> channels;

    // add the info
    KFileMetaInfoGroup group = appendGroup(info, "Technical");
    appendItem(group, "Sample Rate", (uint) samplerate);
    appendItem(group, "Channels", (uint) channels);

    // work out the encoding
    switch (encoding) {
    case 1 :
        appendItem(group, "Encoding", i18n("8-bit ISDN u-law"));
        bytespersample = 1;
        break;
    case 2 :
        appendItem(group, "Encoding", i18n("8-bit linear PCM [REF-PCM]"));
        bytespersample = 1;
        break;
    case 3 :
        appendItem(group, "Encoding", i18n("16-bit linear PCM"));
        bytespersample = 2;
        break;
    case 4 :
        appendItem(group, "Encoding", i18n("24-bit linear PCM"));
        bytespersample = 3;
        break;
    case 5 :
        appendItem(group, "Encoding", i18n("32-bit linear PCM"));
        bytespersample = 4;
        break;
    case 6 :
        appendItem(group, "Encoding", i18n("32-bit IEEE floating point"));
        bytespersample = 4;
        break;
    case 7 :
        appendItem(group, "Encoding", i18n("64-bit IEEE floating point"));
        bytespersample = 8;
        break;
    case 23 :
        appendItem(group, "Encoding", i18n("8-bit ISDN u-law compressed"));
        bytespersample = 1;
        break;
    default :
        appendItem(group, "Encoding", i18n("Unknown"));
        bytespersample = 0;
    }

    // work out length from bytespersample + channels + size
    if ((channels > 0) && (datasize > 0) && (datasize != 0xFFFFFFFF) && (bytespersample > 0) && (samplerate > 0)) {
        uint32_t length = datasize / channels / bytespersample / samplerate;
        appendItem(group, "Length", (uint) length);
    } else {
        appendItem(group, "Length", "???");
    }

    return true;
}

#include "kfile_au.moc"
