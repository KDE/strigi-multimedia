/*  KFile FLAC-plugin

    Copyright (C) 2003 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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

#if HAVE_TAGLIB
#include "tstring.h"
#include "mpegfile.h"
#include "id3v1genres.h"
#include "tag.h"
#else
// The C API is more powerful, and we need the extra power
#include <FLAC/metadata.h>
#endif

#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

// known translations for common ogg/vorbis keys
// from http://www.ogg.org/ogg/vorbis/doc/v-comment.html
static const char* const knownTranslations[] = {
  I18N_NOOP("Title"),
  I18N_NOOP("Album"),
  I18N_NOOP("Tracknumber"),
  I18N_NOOP("Artist"),
  I18N_NOOP("Organization"),
  I18N_NOOP("Description"),
  I18N_NOOP("Genre"),
  I18N_NOOP("Date"),
  I18N_NOOP("Location"),
  I18N_NOOP("Copyright")
};

K_EXPORT_COMPONENT_FACTORY(kfile_flac, KGenericFactory<KFlacPlugin>("kfile_flac"))

KFlacPlugin::KFlacPlugin( QObject *parent, const char *name,
                        const QStringList &args )
    : KFilePlugin( parent, name, args )
{
    kdDebug(7034) << "flac plugin\n";

    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/x-flac" );

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

    item = addItemInfo(group, "Sample Rate", i18n("Sample rate"), QVariant::Int);
    setSuffix(item, i18n(" Hz"));

    item = addItemInfo(group, "Sample Width", i18n("Sample width"), QVariant::Int);
    setSuffix(item, i18n(" bits"));

    item = addItemInfo(group, "Bitrate", i18n("Average bitrate"),
                       QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    setHint(item, KFileMimeTypeInfo::Bitrate);
    setSuffix(item, i18n( " kbps"));

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Cummulative);
    setHint(item, KFileMimeTypeInfo::Length);
    setUnit(item, KFileMimeTypeInfo::Seconds);
}

#if HAVE_TAGLIB

bool KFlacPlugin::readInfo( KFileMetaInfo& info, uint what )
{
    kdDebug(7034) << "flac plugin readInfo\n";

    bool readComment = false;
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

    TagLib::FLAC::File file(QFile::encodeName(info.path()).data(), readTech);

    if(!file.isOpen())  {
        kdDebug(7034) << "Couldn't open " << file.name() << endl;
        return false;
    }

    if(readId3)
    {
        KFileMetaInfoGroup id3group = appendGroup(info, "id3");

        QString date  = file.tag()->year() > 0 ? QString::number(file.tag()->year()) : QString::null;
        QString track = file.tag()->track() > 0 ? QString::number(file.tag()->track()) : QString::null;

        appendItem(id3group, "Title",       TStringToQString(file.tag()->title()));
        appendItem(id3group, "Artist",      TStringToQString(file.tag()->artist()));
        appendItem(id3group, "Album",       TStringToQString(file.tag()->album()));
        appendItem(id3group, "Date",        date);
        appendItem(id3group, "Comment",     TStringToQString(file.tag()->comment()));
        appendItem(id3group, "Tracknumber", track);
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

bool KFlacPlugin::writeInfo( const KFileMetaInfo& info) const
{
    TagLib::FLAC::File file(QFile::encodeName(info.path()).data(), false);

    if(!file.isOpen() || !TagLib::File::isWritable(file.name())) {
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

#else

bool KFlacPlugin::readInfo( KFileMetaInfo& info, uint what )
{

    bool readComment = false;
    bool readTech = false;
    if (what & (KFileMetaInfo::Fastest |
                KFileMetaInfo::DontCare |
                KFileMetaInfo::ContentInfo)) readComment = true;

    if (what & (KFileMetaInfo::Fastest |
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo)) readTech = true;



    FLAC__Metadata_SimpleIterator *simp;
    simp = FLAC__metadata_simple_iterator_new();

        if (!FLAC__metadata_simple_iterator_init(simp, QFile::encodeName(info.path()), true, true) )
        return false;

    do {
        if (FLAC__metadata_simple_iterator_get_block_type(simp) == FLAC__METADATA_TYPE_STREAMINFO && readTech)
        {
	    struct stat buf;
	    int ret = stat(QFile::encodeName(info.path()), &buf); // To get file-size
	    if (ret) return false;

	    KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");
	    const FLAC__StreamMetadata *block = FLAC__metadata_simple_iterator_get_block(simp);
	    const FLAC__StreamMetadata_StreamInfo *si = &block->data.stream_info;

	    appendItem(techgroup, "Channels", int(si->channels));
            appendItem(techgroup, "Sample Rate", int(si->sample_rate));
            appendItem(techgroup, "Sample Width", int(si->bits_per_sample));
	    int slen = si->total_samples/ si->sample_rate; // length in seconds
            appendItem(techgroup, "Length", slen);
	    appendItem(techgroup, "Bitrate", int(buf.st_size/(slen*125))); // 8/1000 = 1/125
	}
	else
	if (FLAC__metadata_simple_iterator_get_block_type(simp) == FLAC__METADATA_TYPE_VORBIS_COMMENT &&
		    readComment)
	{
	    KFileMetaInfoGroup commentgroup = appendGroup(info, "Comment");
	    const FLAC__StreamMetadata *block = FLAC__metadata_simple_iterator_get_block(simp);
	    const FLAC__StreamMetadata_VorbisComment *vc = &block->data.vorbis_comment;
	    for(unsigned int i=0; i< vc->num_comments;i++) {
		const FLAC__StreamMetadata_VorbisComment_Entry *ent = &vc->comments[i];
		QString entry = QString::fromUtf8((const char*)ent->entry, ent->length);
		QString name  = entry.section('=', 0, 0);
		QString value = entry.section('=', 1, 1);
		name = name.lower();	// ARTIST == artist
                name[0] = name[0].upper(); // artist==Artist
		appendItem(commentgroup, name, value);
	    }
	}

    } while (FLAC__metadata_simple_iterator_next(simp));

    return true;

}

bool KFlacPlugin::writeInfo(const KFileMetaInfo& info) const
{
    FLAC__Metadata_SimpleIterator *simp;
    simp = FLAC__metadata_simple_iterator_new();

    FLAC__StreamMetadata *vc = 0;
    bool new_block = false;

    if(!FLAC__metadata_simple_iterator_init(
	    simp, QFile::encodeName(info.path()), false, true)
       ) return false;

    if (!FLAC__metadata_simple_iterator_is_writable(simp))
	return false;

    do {
	FLAC__MetadataType type = FLAC__metadata_simple_iterator_get_block_type(simp);
	if ( type == FLAC__METADATA_TYPE_VORBIS_COMMENT ) {
	    vc = FLAC__metadata_simple_iterator_get_block(simp);
	    break; // Leave iterator pointing at vc
	}
    } while (FLAC__metadata_simple_iterator_next(simp));

    FLAC__StreamMetadata_VorbisComment_Entry ent;
    if(!vc) { // No comment-block found. Create one
	vc = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
	ent.entry = (FLAC__byte*)"KDE 3.2";
	ent.length = 7;
	FLAC__metadata_object_vorbiscomment_set_vendor_string(vc,ent,true);
	new_block = true;
    }

    KFileMetaInfoGroup group = info["Comment"];

    int num_comments = vc->data.vorbis_comment.num_comments;

    QStringList keys = group.keys();
    QStringList::Iterator it;
    for(it=keys.begin();it!=keys.end(); ++it) {
	KFileMetaInfoItem item = group[*it];

	if(!item.isModified() || !item.isEditable()) continue;
	QString key = item.key();
	QCString field = (key+"="+item.value().toString()).utf8();

	int i = FLAC__metadata_object_vorbiscomment_find_entry_from(vc, 0, key.utf8());
        ent.length = field.length();
	ent.entry = (FLAC__byte*)field.data();

	if (i<0) { // No such comment, insert
	    FLAC__metadata_object_vorbiscomment_insert_comment
		    (vc, num_comments++, ent, true);
	}
	else {
	    FLAC__metadata_object_vorbiscomment_set_comment
		    (vc, i, ent, true);
	}
    }

    if (new_block) {
    	FLAC__metadata_simple_iterator_insert_block_after(simp, vc, true);
        FLAC__metadata_object_delete(vc);
    }
    else
	FLAC__metadata_simple_iterator_set_block(simp, vc, true);

    FLAC__metadata_simple_iterator_delete(simp);
    return true;
}

QValidator* KFlacPlugin::createValidator( const QString&,
                                         const QString &, const QString &,
                                         QObject* parent, const char* name) const {
	return new QRegExpValidator(QRegExp(".*"), parent, name);
}

#endif

#include "kfile_flac.moc"
