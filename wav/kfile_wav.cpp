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

K_EXPORT_COMPONENT_FACTORY(kfile_wav, WavFactory( "kfile_wav" ));

KWavPlugin::KWavPlugin(QObject *parent, const char *name,
                       const QStringList &preferredItems)
    
    : KFilePlugin(parent, name, preferredItems)
{
}

bool KWavPlugin::readInfo( KFileMetaInfo::Internal& info, int )
{
    QFile file(info.path());

    uint32_t format_size;
    uint16_t format_tag;
    uint16_t channel_count;
    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t bytes_per_sample;
    uint16_t sample_size;
    uint32_t data_size;

    const char *riff_signature = "RIFF";
    const char *wav_signature = "WAVE";
    char signature_buffer[4];

    if (!file.open(IO_ReadOnly))
    {
        kdDebug(7034) << "Couldn't open " << QFile::encodeName(info.path()) << endl;
        return false;
    }    

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

    // Read the needed parts of the FORMAT chunk
    file.at(16);
    dstream >> format_size;
    dstream >> format_tag;
    dstream >> channel_count;
    dstream >> sample_rate;
    dstream >> bytes_per_second;
    dstream >> bytes_per_sample;
    dstream >> sample_size;

    // And now read the DATA chunk
    file.at(24 + format_size);
    dstream >> data_size;

    // These values are downright illgeal
    if ((!channel_count) || (!bytes_per_second))
        return false;

    info.insert(KFileMetaInfoItem("Sample Size", i18n("Sample Size"),
                QVariant(sample_size), false,
                QString::null, i18n("bits")));

    info.insert(KFileMetaInfoItem("Sample Rate", i18n("Sample Rate"),
                QVariant(sample_rate), false,
                QString::null, i18n("Hz")));

    info.insert(KFileMetaInfoItem("Channels", i18n("Channels"),
                QVariant(channel_count)));        

    // Use the data size and bytes per second to figure out the audio length in minutes and seconds
    unsigned int wav_seconds = data_size / bytes_per_second;
    unsigned int playmin = wav_seconds / 60;
    unsigned int playsec = wav_seconds % 60;

    // And store that string under "Length"
    QString str;
    str = QString("%0:%1").arg(playmin)
                          .arg(QString::number(playsec).rightJustify(2,'0') );

    info.insert(KFileMetaInfoItem("Length", i18n("Length"),
                QVariant(str), false, QString::null));
    
    info.setPreferredKeys(m_preferred);
    info.setSupportedKeys(m_preferred);
    info.setSupportsVariableKeys(false);
    
    return true;
}

#include "kfile_wav.moc"
