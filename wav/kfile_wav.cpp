/* This file is part of the KDE project
 * Copyright (C) 2002 Ryan Cumming <bodnar42@phalynx.dhs.org>
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
#include "kfile_wav.h"

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
#endif

typedef KGenericFactory<KWavPlugin> WavFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_wav, WavFactory( "kfile_wav" ))

KWavPlugin::KWavPlugin(QObject *parent, const char *name,
                       const QStringList &args)
    
    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/x-wav" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    // "the" group
    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Sample Size", i18n("Sample Size"), QVariant::Int);
    setSuffix(item, i18n(" bits"));

    item = addItemInfo(group, "Sample Rate", i18n("Sample Rate"), QVariant::Int);
    setSuffix(item, i18n(" Hz"));

    item = addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setAttributes(item,  KFileMimeTypeInfo::Cummulative);
    setUnit(item, KFileMimeTypeInfo::Seconds);
}

bool KWavPlugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    if ( info.path().isEmpty() ) // remote file
        return false;

    QFile file(info.path());

    uint32_t format_size;
    uint16_t format_tag;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t bytes_per_sample;
    uint16_t sample_size;
    uint32_t data_size;
    uint32_t unknown_chunk_size;
    uint16_t unknown_chunk16;
    bool have_fmt = false;
    bool have_data = false;
    QIODevice::Offset file_length;

    const char *riff_signature = "RIFF";
    const char *wav_signature = "WAVE";
    const char *fmt_signature = "fmt ";
    const char *data_signature = "data";
    char signature_buffer[4];

    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }    

    file_length = file.size() - 100; // a bit of insurance
    QDataStream dstream(&file);

    // WAV files are little-endian
    dstream.setByteOrder(QDataStream::LittleEndian);

    // Read and verify the RIFF signature
    dstream.readRawBytes(signature_buffer, 4);
    if (memcmp(signature_buffer, riff_signature, 4))
         return false;

    // Skip the next bit (total file size, pretty useless)
    file.at(8);

    // Read and verify the WAVE signature
    dstream.readRawBytes(signature_buffer, 4);
    if (memcmp(signature_buffer, wav_signature, 4))
         return false;

    // pretty dumb scanner, but better than what we had!
    do
    {
	dstream.readRawBytes(signature_buffer, 4);
	if (!memcmp(signature_buffer, fmt_signature, 4)) {
	    dstream >> format_size;
	    dstream >> format_tag;
	    dstream >> channel_count;
	    dstream >> sample_rate;
	    dstream >> bytes_per_second;
	    dstream >> bytes_per_sample;
	    dstream >> sample_size;
	    have_fmt = true;
	    if ( format_size > 16 ) {
		for (unsigned int i = 0; i < (format_size-16+1)/2; i++)
		    dstream >> unknown_chunk16;
	    }
	} else if (!memcmp(signature_buffer, data_signature, 4)) {
	    dstream >> data_size;
	    have_data = true;
	} else {
	    dstream >> unknown_chunk_size;
	    for (unsigned int i = 0; i < (unknown_chunk_size+1)/2; i++)
		dstream >> unknown_chunk16;
	}
	if (have_data && have_fmt)
	    break;
    } while (file.at() < file_length);

    if ( (!have_data) || (!have_fmt) )
	return false;

    // These values are downright illegal
    if ((!channel_count) || (!bytes_per_second))
        return false;
    
    KFileMetaInfoGroup group = appendGroup(info, "Technical");
    

    appendItem(group, "Sample Size", int(sample_size));
    appendItem(group, "Sample Rate", int(sample_rate));
    appendItem(group, "Channels", int(channel_count));
    unsigned int wav_seconds = data_size / bytes_per_second;
    appendItem(group, "Length", int(wav_seconds));

    return true;
}

#include "kfile_wav.moc"
