/* This file is part of the KDE project
 * Copyright (C) 2003-2004 Allan Sandfeld Jensen <kde@carewolf.com>
 *
 * Originally based upon the kfile_ogg plugin:
 *  Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
 * Interfacing to TagLib is copied from kfile_mp3 plugin:
 *  Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
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

#include "kfile_flac.h"

#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qdict.h>
#include <qvalidator.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ksavefile.h>

#include <tag.h>
#if (TAGLIB_MAJOR_VERSION>1) ||  \
   ((TAGLIB_MAJOR_VERSION==1) && (TAGLIB_MINOR_VERSION>=2))
#define TAGLIB_1_2
#endif

#include <tstring.h>
#include <tfile.h>
#include <flacfile.h>
#ifdef TAGLIB_1_2
#include <oggflacfile.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

K_EXPORT_COMPONENT_FACTORY(kfile_flac, KGenericFactory<KFlacPlugin>("kfile_flac"))

KFlacPlugin::KFlacPlugin( QObject *parent, const char *name,
                        const QStringList &args )
    : KFilePlugin( parent, name, args )
{
    kdDebug(7034) << "flac plugin\n";

    makeMimeTypeInfo( "audio/x-flac" );
#ifdef TAGLIB_1_2
    makeMimeTypeInfo( "audio/x-oggflac" );
#endif

}

void KFlacPlugin::makeMimeTypeInfo(const QString& mimeType)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( mimeType );

    KFileMimeTypeInfo::GroupInfo* group = 0;

    // comment group
    group = addGroupInfo(info, "Comment", i18n("Comment"));
    setAttributes(group, KFileMimeTypeInfo::Addable |
                         KFileMimeTypeInfo::Removable);

    KFileMimeTypeInfo::ItemInfo* item = 0;

    item = addItemInfo(group, "Artist", i18n("Artist"), QVariant::String);
    setHint(item, KFileMimeTypeInfo::Author);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Title", i18n("Title"), QVariant::String);
    setHint(item, KFileMimeTypeInfo::Name);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Album", i18n("Album"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Genre", i18n("Genre"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Tracknumber", i18n("Track Number"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Date", i18n("Date"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Description", i18n("Description"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Organization", i18n("Organization"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Location", i18n("Location"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);

    item = addItemInfo(group, "Copyright", i18n("Copyright"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);


    addVariableInfo(group, QVariant::String, KFileMimeTypeInfo::Addable |
                                             KFileMimeTypeInfo::Removable |
                                             KFileMimeTypeInfo::Modifiable);

    // technical group
    group = addGroupInfo(info, "Technical", i18n("Technical Details"));
    setAttributes(group, 0);

    addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);

    item = addItemInfo(group, "Sample Rate", i18n("Sample Rate"), QVariant::Int);
    setSuffix(item, i18n(" Hz"));

    item = addItemInfo(group, "Sample Width", i18n("Sample Width"), QVariant::Int);
    setSuffix(item, i18n(" bits"));

    item = addItemInfo(group, "Bitrate", i18n("Average Bitrate"),
                       QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    setHint(item, KFileMimeTypeInfo::Bitrate);
    setSuffix(item, i18n( " kbps"));

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Cummulative);
    setHint(item, KFileMimeTypeInfo::Length);
    setUnit(item, KFileMimeTypeInfo::Seconds);
}

bool KFlacPlugin::readInfo( KFileMetaInfo& info, uint what )
{
    if ( info.path().isEmpty() ) // remote file
        return false;

    bool readComment = false;
    bool readTech = false;
    if (what & (KFileMetaInfo::Fastest |
                KFileMetaInfo::DontCare |
                KFileMetaInfo::ContentInfo)) readComment = true;

    if (what & (KFileMetaInfo::Fastest |
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo)) readTech = true;

    TagLib::File *file = 0;

    if (info.mimeType() == "audio/x-flac")
        file = new TagLib::FLAC::File(QFile::encodeName(info.path()).data(), readTech);
#ifdef TAGLIB_1_2
    else
        file = new TagLib::Ogg::FLAC::File(QFile::encodeName(info.path()).data(), readTech);
#endif

    if (!file || !file->isValid())
    {
        kdDebug(7034) << "Couldn't open " << file->name() << endl;
        delete file;
        return false;
    }

    if(readComment && file->tag())
    {
        KFileMetaInfoGroup commentgroup = appendGroup(info, "Comment");

        QString date  = file->tag()->year() > 0 ? QString::number(file->tag()->year()) : QString::null;
        QString track = file->tag()->track() > 0 ? QString::number(file->tag()->track()) : QString::null;

        appendItem(commentgroup, "Title",       TStringToQString(file->tag()->title()).stripWhiteSpace());
        appendItem(commentgroup, "Artist",      TStringToQString(file->tag()->artist()).stripWhiteSpace());
        appendItem(commentgroup, "Album",       TStringToQString(file->tag()->album()).stripWhiteSpace());
        appendItem(commentgroup, "Date",        date);
        appendItem(commentgroup, "Comment",     TStringToQString(file->tag()->comment()).stripWhiteSpace());
        appendItem(commentgroup, "Tracknumber", track);
        appendItem(commentgroup, "Genre",       TStringToQString(file->tag()->genre()).stripWhiteSpace());
    }

    if (readTech && file->audioProperties())
    {
        KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");
        TagLib::FLAC::Properties *properties =
                   (TagLib::FLAC::Properties*)(file->audioProperties());

        appendItem(techgroup, "Bitrate",      properties->bitrate());
        appendItem(techgroup, "Sample Rate",  properties->sampleRate());
        appendItem(techgroup, "Sample Width", properties->sampleWidth());
        appendItem(techgroup, "Channels",     properties->channels());
        appendItem(techgroup, "Length",       properties->length());
    }

    delete file;
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
        return QStringToTString(m_info["Comment"][key].value().toString());
    }
    int toInt(const char *key) const
    {
        return m_info["Comment"][key].value().toInt();
    }
private:
    const KFileMetaInfo &m_info;
};

bool KFlacPlugin::writeInfo(const KFileMetaInfo& info) const
{
    TagLib::File *file;

    if (!TagLib::File::isWritable(QFile::encodeName(info.path()).data())) {
        kdDebug(7034) << "can't write to " << info.path() << endl;
        return false;
    }

    if (info.mimeType() == "audio/x-flac")
        file = new TagLib::FLAC::File(QFile::encodeName(info.path()).data(), false);
#ifdef TAGLIB_1_2
    else
        file = new TagLib::Ogg::FLAC::File(QFile::encodeName(info.path()).data(), false);
#endif

    if(!file->isOpen())
    {
        kdDebug(7034) << "couldn't open " << info.path() << endl;
        delete file;
        return false;
    }

    Translator t(info);

    file->tag()->setTitle(t["Title"]);
    file->tag()->setArtist(t["Artist"]);
    file->tag()->setAlbum(t["Album"]);
    file->tag()->setYear(t.toInt("Date"));
    file->tag()->setComment(t["Comment"]);
    file->tag()->setTrack(t.toInt("Tracknumber"));
    file->tag()->setGenre(t["Genre"]);

    file->save();

    delete file;
    return true;
}

QValidator* KFlacPlugin::createValidator( const QString&,
                                         const QString &group, const QString &key,
                                         QObject* parent, const char* name) const
{
    if(key == "Tracknumber" || key == "Date")
    {
        return new QIntValidator(0, 9999, parent, name);
    }
    else
	return new QRegExpValidator(QRegExp(".*"), parent, name);
}

#include "kfile_flac.moc"
