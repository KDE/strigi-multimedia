/* This file is part of the KDE project
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
 * Copyright (C) 2002 Ryan Cumming <bodnar42@phalynx.dhs.org>
 * Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
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
 *  $Id$
 */

#include "config.h"

#include "kfile_mp3.h"

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

#if HAVE_TAGLIB
#include "tstring.h"
#include "mpegfile.h"
#include "id3v1genres.h"
#include "tag.h"
#else
// need this for the mp3info header
#define __MAIN
extern "C" {
#include "mp3info.h"
}
#undef __MAIN
#endif

typedef KGenericFactory<KMp3Plugin> Mp3Factory;

K_EXPORT_COMPONENT_FACTORY(kfile_mp3, Mp3Factory( "kfile_mp3" ))

KMp3Plugin::KMp3Plugin(QObject *parent, const char *name,
                       const QStringList &args)
    
    : KFilePlugin(parent, name, args)
{
    kdDebug(7034) << "mp3 plugin\n";
    
    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/x-mp3" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    // id3 group
    group = addGroupInfo(info, "id3", i18n("ID3 Tag"));

    setAttributes(group, KFileMimeTypeInfo::Addable | 
                         KFileMimeTypeInfo::Removable);

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Title", i18n("Title"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Name);

    item = addItemInfo(group, "Artist", i18n("Artist"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Author);

    item = addItemInfo(group, "Album", i18n("Album"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    
    item = addItemInfo(group, "Date", i18n("Year"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    
    item = addItemInfo(group, "Comment", i18n("Comment"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Description);

    item = addItemInfo(group, "Tracknumber", i18n("Track"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Genre", i18n("Genre"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    // technical group
    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    item = addItemInfo(group, "Version", i18n("Version"), QVariant::Int);
    setPrefix(item,  i18n("MPEG "));

    item = addItemInfo(group, "Layer", i18n("Layer"), QVariant::Int);
    item = addItemInfo(group, "CRC", i18n("CRC"), QVariant::Bool);
    item = addItemInfo(group, "Bitrate", i18n("Bitrate"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    setHint(item, KFileMimeTypeInfo::Bitrate);
    setSuffix(item, i18n(" kbps"));

    item = addItemInfo(group, "Sample Rate", i18n("Sample Rate"), QVariant::Int);
    setSuffix(item, i18n("Hz"));
    
    item = addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);
    item = addItemInfo(group, "Copyright", i18n("Copyright"), QVariant::Bool);
    item = addItemInfo(group, "Original", i18n("Original"), QVariant::Bool);
    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setAttributes(item,  KFileMimeTypeInfo::Cummulative);
    setUnit(item, KFileMimeTypeInfo::Seconds);
    item = addItemInfo(group, "Emphasis", i18n("Emphasis"), QVariant::String);
}

#if HAVE_TAGLIB

bool KMp3Plugin::readInfo( KFileMetaInfo& info, uint what )
{
    kdDebug(7034) << "mp3 plugin readInfo\n";
    
    bool readId3 = false;
    bool readTech = false;
    typedef enum KFileMetaInfo::What What;
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::ContentInfo)) readId3 = true;
    
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo)) readTech = true;

    if(!readId3 && !readTech)
	return true;

    TagLib::MPEG::File file(QFile::encodeName(info.path()).data(), readTech);

    if(!file.isOpen())  {
        kdDebug(7034) << "Couldn't open " << file.name().toCString() << endl;
        return false;
    }

    if(readId3)
    {
        KFileMetaInfoGroup id3group = appendGroup(info, "id3");
        
        appendItem(id3group, "Title",       TStringToQString(file.tag()->title()));
        appendItem(id3group, "Artist",      TStringToQString(file.tag()->artist()));
        appendItem(id3group, "Album",       TStringToQString(file.tag()->album()));
        appendItem(id3group, "Date",        QString::number(file.tag()->year()));
        appendItem(id3group, "Comment",     TStringToQString(file.tag()->comment()));
        appendItem(id3group, "Tracknumber", QString::number(file.tag()->track()));
        appendItem(id3group, "Genre",       TStringToQString(file.tag()->genre()));
    }
    
    if(readTech) {

        KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");

        QString version;
        switch(file.audioProperties()->version()) {
        case TagLib::MPEG::Header::Version1:
            version = "1.0";
            break;
        case TagLib::MPEG::Header::Version2:
            version = "2.0";
            break;
        case TagLib::MPEG::Header::Version2_5:
            version = "2.5";
            break;
        }

	static const int dummy = 0; // QVariant's bool constructor requires a dummy int value.

        // CRC and Emphasis aren't yet implemented in TagLib (not that I think anyone cares)
        
        appendItem(techgroup, "Version",     version);
        appendItem(techgroup, "Layer",       file.audioProperties()->layer());
        // appendItem(techgroup, "CRC",      file.audioProperties()->crc());
        appendItem(techgroup, "Bitrate",     file.audioProperties()->bitrate());
        appendItem(techgroup, "Sample Rate", file.audioProperties()->sampleRate());
        appendItem(techgroup, "Channels",    file.audioProperties()->channels());
        appendItem(techgroup, "Copyright",   QVariant(file.audioProperties()->isCopyrighted(), dummy));
        appendItem(techgroup, "Original",    QVariant(file.audioProperties()->isOriginal(), dummy));
        appendItem(techgroup, "Length",      file.audioProperties()->length());
        // appendItem(techgroup, "Emphasis", file.audioProperties()->empahsis());
    }

    kdDebug(7034) << "reading finished\n";
    return true;
}

class Translator
{
public:
    Translator(const KFileMetaInfo &info) : m_info(info) {}
    TagLib::String operator[](const char *key) const
    {
        return QStringToTString(m_info["id3"][key].value().toString());
    }
    int toInt(const char *key) const
    {
        return m_info["id3"][key].value().toInt();
    }
private:
    const KFileMetaInfo &m_info;
};

bool KMp3Plugin::writeInfo( const KFileMetaInfo& info) const
{
    TagLib::MPEG::File file(QFile::encodeName(info.path()).data(), false);

    if(!file.isOpen() || !TagLib::File::isWritable(file.name())) {
        kdDebug(7034) << "couldn't open " << info.path() << endl;
        return false;
    }

    Translator t(info);

    file.tag()->setTitle(t["title"]);
    file.tag()->setArtist(t["Artist"]);
    file.tag()->setAlbum(t["Album"]);
    file.tag()->setYear(t.toInt("Date"));
    file.tag()->setComment(t["Comment"]);
    file.tag()->setTrack(t.toInt("Tracknumber"));
    file.tag()->setGenre(t["Genre"]);

    file.save();

    return true;
}

#else // non-TagLib version

bool KMp3Plugin::readInfo( KFileMetaInfo& info, uint what )
{
    kdDebug(7034) << "mp3 plugin readInfo\n";
    
    bool readId3 = false;
    bool readTech = false;
    typedef enum KFileMetaInfo::What What;
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::ContentInfo)) readId3 = true;
    
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo)) readTech = true;
    
    mp3info mp3;

    memset(&mp3,0,sizeof(mp3info));

    QCString s = QFile::encodeName(info.path());
    mp3.filename = strdup(s);
        
    mp3.file = fopen(mp3.filename, "rb");
    if (!mp3.file)
    {
        if (mp3.filename) free(mp3.filename);
        mp3.filename = 0;
        kdDebug(7034) << "Couldn't open " << mp3.filename << endl;
        return false;
    }

    ::get_mp3_info(&mp3, ::SCAN_QUICK, ::VBR_VARIABLE);
  
    // here we go
    QString value;
  
    if (mp3.id3_isvalid && readId3)
    {
        KFileMetaInfoGroup id3group = appendGroup(info, "id3");
        
        appendItem(id3group, "Title", QString::fromLocal8Bit(mp3.id3.title));
        appendItem(id3group, "Artist", QString::fromLocal8Bit(mp3.id3.artist));
        appendItem(id3group, "Album", QString::fromLocal8Bit(mp3.id3.album));
        appendItem(id3group, "Date", QString::fromLocal8Bit(mp3.id3.year));
        appendItem(id3group, "Comment", QString::fromLocal8Bit(mp3.id3.comment));

        if (mp3.id3.track[0])
            appendItem(id3group, "Tracknumber", mp3.id3.track[0]);
        
      	// Could we find a valid genre?
        if (mp3.id3.genre[0]>MAXGENRE) {
            // No, set it to 12 ("Other")
          	mp3.id3.genre[0] = 12;
      	}

        appendItem(id3group, "Genre", QString::fromLocal8Bit(
                                              ::typegenre[mp3.id3.genre[0]]));
    }
    else
        memset(&mp3.id3, sizeof(id3tag), 0);
    
    // end of the id3 part
    // now the technical stuff
    if (mp3.header_isvalid && readTech) {

        KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");
        
        appendItem(techgroup, "Version", mp3.header.version);
        appendItem(techgroup, "Layer", ::header_layer(&mp3.header));
        appendItem(techgroup, "CRC", QVariant(header_crc(&mp3.header), 42));
        appendItem(techgroup, "Bitrate", ::header_bitrate(&mp3.header));
        appendItem(techgroup, "Sample Rate", ::header_frequency(&mp3.header));
        // Modes 0-2 are forms of stereo, mode 3 is mono
        appendItem(techgroup, "Channels", int((mp3.header.mode == 3) ? 1 : 2));
        appendItem(techgroup, "Copyright", QVariant(mp3.header.copyright, 42));
        appendItem(techgroup, "Original", QVariant(mp3.header.original, 42));
        appendItem(techgroup, "Length", mp3.seconds);
        appendItem(techgroup, "Emphasis", QString(::header_emphasis(&mp3.header)));
    }

    fclose(mp3.file);
    kdDebug(7034) << "reading finished\n";
    
    if (mp3.filename) free(mp3.filename);
    mp3.filename = 0;
    
    return true;
}

bool KMp3Plugin::writeInfo( const KFileMetaInfo& info) const
{
    mp3info mp3;
    memset(&mp3,0,sizeof(mp3info));

    QCString name = QFile::encodeName(info.path());
    mp3.filename = strdup(name);

    mp3.file = fopen(mp3.filename, "rb+");
    
    if (!mp3.file) 
    {
      kdDebug(7034) << "couldn't open " << info.path() << endl;
      if (mp3.filename) free(mp3.filename);
      mp3.filename = 0;
      return false;
    }

    ::get_mp3_info(&mp3, ::SCAN_NONE, ::VBR_VARIABLE);
    
    if (!info.groups().contains("id3") || !info["id3"].isValid() )
    {
        fclose(mp3.file);
        // that's how original mp3info does it
        truncate(mp3.filename,mp3.datasize);
        if (mp3.filename) free(mp3.filename);
        mp3.filename = 0;
        return true;
    }
    
    strncpy(mp3.id3.title,  info["id3"]["Title"]  .value().toString().local8Bit(), 31);
    mp3.id3.title[ 30 ] = '\0';
    strncpy(mp3.id3.artist, info["id3"]["Artist"] .value().toString().local8Bit(), 31);
    mp3.id3.artist[ 30 ] = '\0';
    strncpy(mp3.id3.album,  info["id3"]["Album"]  .value().toString().local8Bit(), 31);
    mp3.id3.album[ 30 ] = '\0';
    strncpy(mp3.id3.year,   info["id3"]["Date"]   .value().toString().local8Bit(),  5);
    mp3.id3.year[ 4 ] = '\0';
    strncpy(mp3.id3.comment,info["id3"]["Comment"].value().toString().local8Bit(), 29);
    mp3.id3.comment[ 28 ] = '\0';

    KFileMetaInfoItem track = info["id3"]["Tracknumber"];
    if (track.isValid()) mp3.id3.track[0] = track.value().toInt();
    
    QString s = info["id3"]["Genre"].value().toString();

    for (mp3.id3.genre[0] = 0; mp3.id3.genre[0] <= MAXGENRE; mp3.id3.genre[0]++)
    {
        if (s == ::typegenre[mp3.id3.genre[0]])
            break;
    }

    // Could we find a valid genre?
    if (mp3.id3.genre[0] > MAXGENRE) {
         // No, set it to 12 ("Other")
         mp3.id3.genre[0] = 12;
    }
      
    bool success = ::write_tag(&mp3);
    fclose(mp3.file);
    
    if (mp3.filename) free(mp3.filename);
    mp3.filename = 0;
  
    return success;
}

#endif // HAVE_TAGLIB

QValidator* KMp3Plugin::createValidator(const QString& /* mimetype */,
                                        const QString &group, const QString &key,
                                        QObject* parent, const char* name) const
{
    kdDebug(7034) << "making a validator for " << group << "/" << key << endl;

#if HAVE_TAGLIB

    if (key == "Genre")
    {
        QStringList l;
        TagLib::StringList genres = TagLib::ID3v1::genreList();
        for(TagLib::StringList::ConstIterator it = genres.begin(); it != genres.end(); ++it)
        {
	    l.append(TStringToQString((*it)));
        }

        return new KStringListValidator(l, false, true, parent, name);
    }

#else

    if ((key == "Title") || (key == "Artist")||
        (key == "Album"))
    {
        return new QRegExpValidator(QRegExp(".{,30}"), parent, name);
    }
    else if (key == "Date")
    {
        return new QRegExpValidator(QRegExp(".{,4}"), parent, name);
    }
    else if (key == "Comment")
    {
        return new QRegExpValidator(QRegExp(".{,28}"), parent, name);
    }
    else if (key == "Tracknumber")
    {
        return new QIntValidator(0, 255, parent, name);
    }
    else if (key == "Genre")
    {
        QStringList list;
        for (int index = 0; index <= MAXGENRE; index++)
        {
           list += ::typegenre[galphagenreindex[index]];
        }

        return new KStringListValidator(list, false, true, parent, name);
    }

#endif

    return 0;
}

#include "kfile_mp3.moc"
