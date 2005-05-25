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

#include <tstring.h>
#include <tag.h>
#include <mpegfile.h>
#include <id3v1genres.h>
#include <id3v2framefactory.h>

typedef KGenericFactory<KMp3Plugin> Mp3Factory;

K_EXPORT_COMPONENT_FACTORY(kfile_mp3, Mp3Factory( "kfile_mp3" ))

KMp3Plugin::KMp3Plugin(QObject *parent, const char *name, const QStringList &args)
    : KFilePlugin(parent, name, args)
{
    kdDebug(7034) << "mp3 plugin\n";

    KFileMimeTypeInfo *info = addMimeTypeInfo("audio/x-mp3");

    // id3 group

    KFileMimeTypeInfo::GroupInfo *group = addGroupInfo(info, "id3", i18n("ID3 Tag"));

    setAttributes(group, KFileMimeTypeInfo::Addable |
                         KFileMimeTypeInfo::Removable);

    KFileMimeTypeInfo::ItemInfo *item;

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

bool KMp3Plugin::readInfo(KFileMetaInfo &info, uint what)
{
    kdDebug(7034) << "mp3 plugin readInfo\n";

    bool readId3 = false;
    bool readTech = false;

    typedef enum KFileMetaInfo::What What;

    if(what & (KFileMetaInfo::Fastest |
               KFileMetaInfo::DontCare |
               KFileMetaInfo::ContentInfo))
    {
        readId3 = true;
    }

    if(what & (KFileMetaInfo::Fastest |
               KFileMetaInfo::DontCare |
               KFileMetaInfo::TechnicalInfo))
    {
        readTech = true;
    }

    if(!readId3 && !readTech)
        return true;

    if ( info.path().isEmpty() ) // remote file
        return false;

    TagLib::MPEG::File file(QFile::encodeName(info.path()).data(), readTech);

    if(!file.isOpen())
    {
        kdDebug(7034) << "Couldn't open " << file.name() << endl;
        return false;
    }

    if(readId3)
    {
        KFileMetaInfoGroup id3group = appendGroup(info, "id3");

        QString date  = file.tag()->year() > 0 ? QString::number(file.tag()->year()) : QString::null;
        QString track = file.tag()->track() > 0 ? QString::number(file.tag()->track()) : QString::null;

        QString title = TStringToQString(file.tag()->title()).stripWhiteSpace();
        if (!title.isEmpty())
            appendItem(id3group, "Title", title);
        QString artist = TStringToQString(file.tag()->artist()).stripWhiteSpace();
        if (!artist.isEmpty())
            appendItem(id3group, "Artist", artist);
        QString album = TStringToQString(file.tag()->album()).stripWhiteSpace();
        if (!album.isEmpty())
            appendItem(id3group, "Album", album);
        appendItem(id3group, "Date",        date);
        QString comment = TStringToQString(file.tag()->comment()).stripWhiteSpace();
        if (!comment.isEmpty())
            appendItem(id3group, "Comment", comment);
        appendItem(id3group, "Tracknumber", track);
        QString genre = TStringToQString(file.tag()->genre()).stripWhiteSpace();
        if (!genre.isEmpty())
            appendItem(id3group, "Genre", genre);
    }

    if(readTech)
    {
        KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");

        QString version;
        switch(file.audioProperties()->version())
        {
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

/**
 * Do translation between KFileMetaInfo items and TagLib::String in a tidy way.
 */

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

bool KMp3Plugin::writeInfo(const KFileMetaInfo &info) const
{
    TagLib::ID3v2::FrameFactory::instance()->setDefaultTextEncoding(TagLib::String::UTF8);
    TagLib::MPEG::File file(QFile::encodeName(info.path()).data(), false);

    if(!file.isOpen() || !TagLib::File::isWritable(file.name()))
    {
        kdDebug(7034) << "couldn't open " << info.path() << endl;
        return false;
    }

    Translator t(info);

    file.tag()->setTitle(t["Title"]);
    file.tag()->setArtist(t["Artist"]);
    file.tag()->setAlbum(t["Album"]);
    file.tag()->setYear(t.toInt("Date"));
    file.tag()->setComment(t["Comment"]);
    file.tag()->setTrack(t.toInt("Tracknumber"));
    file.tag()->setGenre(t["Genre"]);

    file.save();

    return true;
}

/**
 * A validator that will suggest a list of strings, but allow for free form
 * strings as well.
 */

class ComboValidator : public KStringListValidator
{
public:
    ComboValidator(const QStringList &list, bool rejecting,
                   bool fixupEnabled, QObject *parent, const char *name) :
        KStringListValidator(list, rejecting, fixupEnabled, parent, name)
    {

    }

    virtual QValidator::State validate(QString &, int &) const
    {
        return QValidator::Acceptable;
    }
};

QValidator *KMp3Plugin::createValidator(const QString & /* mimetype */,
                                        const QString &group, const QString &key,
                                        QObject *parent, const char *name) const
{
    kdDebug(7034) << "making a validator for " << group << "/" << key << endl;

    if(key == "Tracknumber" || key == "Date")
    {
        return new QIntValidator(0, 9999, parent, name);
    }

    if(key == "Genre")
    {
        QStringList l;
        TagLib::StringList genres = TagLib::ID3v1::genreList();
        for(TagLib::StringList::ConstIterator it = genres.begin(); it != genres.end(); ++it)
        {
            l.append(TStringToQString((*it)));
        }
        return new ComboValidator(l, false, true, parent, name);
    }

    return 0;
}

#include "kfile_mp3.moc"
