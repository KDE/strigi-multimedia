/* This file is part of the KDE project
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
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
 *  $Id$
 */

#include "kfile_ogg.h"
#include "vcedit.h"

#include <q3cstring.h>
#include <QFile>
#include <QDateTime>
#include <q3dict.h>
#include <qvalidator.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <kurl.h>
#include <k3process.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ksavefile.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

// known translations for common ogg/vorbis keys
// from http://www.ogg.org/ogg/vorbis/doc/v-comment.html
static const char* const knownTranslations[] = {
  I18N_NOOP("Title"),
  I18N_NOOP("Version"),
  I18N_NOOP("Album"),
  I18N_NOOP("Tracknumber"),
  I18N_NOOP("Artist"),
  I18N_NOOP("Organization"),
  I18N_NOOP("Description"),
  I18N_NOOP("Genre"),
  I18N_NOOP("Date"),
  I18N_NOOP("Location"),
  I18N_NOOP("Copyright")
//  I18N_NOOP("Isrc") // dunno what an Isrc number is, the link is broken
};    

K_EXPORT_COMPONENT_FACTORY(kfile_ogg, KGenericFactory<KOggPlugin>("kfile_ogg"))

KOggPlugin::KOggPlugin( QObject *parent, 
                        const QStringList &args )
    : KFilePlugin( parent, args )
{
    kDebug(7034) << "ogg plugin\n";
    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/x-vorbis+ogg" );

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
    
    addItemInfo(group, "Version", i18n("Version"), QVariant::Int);
    addItemInfo(group, "Channels", i18n("Channels"), QVariant::Int);

    item = addItemInfo(group, "Sample Rate", i18n("Sample Rate"), QVariant::Int);
    setSuffix(item, i18n(" Hz"));

    item = addItemInfo(group, "UpperBitrate", i18n("Upper Bitrate"),
                       QVariant::Int);
    setSuffix(item, i18n(" kbps"));
    
    item = addItemInfo(group, "LowerBitrate", i18n("Lower Bitrate"),
                       QVariant::Int);
    setSuffix(item, i18n(" kbps"));

    item = addItemInfo(group, "NominalBitrate", i18n("Nominal Bitrate"),
                       QVariant::Int);
    setSuffix(item, i18n(" kbps"));
    
    item = addItemInfo(group, "Bitrate", i18n("Average Bitrate"), 
                       QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Averaged);
    setHint(item, KFileMimeTypeInfo::Bitrate);
    setSuffix(item, i18n( " kbps"));

    item = addItemInfo(group, "Length", i18n("Length"), QVariant::Int);
    setAttributes(item, KFileMimeTypeInfo::Cummulative);
    setUnit(item, KFileMimeTypeInfo::Seconds);
}

bool KOggPlugin::readInfo( KFileMetaInfo& info, uint what )
{
    // parts of this code taken from ogginfo.c of the vorbis-tools v1.0rc2
    FILE *fp;
    OggVorbis_File vf;
    int rc,i;
    vorbis_comment *vc;
    vorbis_info *vi;
    
    bool readComment = false;
    bool readTech = false;
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::ContentInfo)) readComment = true;
    
    if (what & (KFileMetaInfo::Fastest | 
                KFileMetaInfo::DontCare |
                KFileMetaInfo::TechnicalInfo)) readTech = true;

    memset(&vf, 0, sizeof(OggVorbis_File));

    if ( info.path().isEmpty() ) // remote file
        return false;
 
    fp = fopen(QFile::encodeName(info.path()),"rb");
    if (!fp)
    {
        kDebug(7034) << "Unable to open " << QFile::encodeName(info.path());
        return false;
    }

    rc = ov_open(fp,&vf,NULL,0);

    if (rc < 0) 
    {
        kDebug(7034) << "Unable to understand " << QFile::encodeName(info.path())
                      << ", errorcode=" << rc << endl;
        return false;
    }
  
//    info.insert(KFileMetaInfoItem("Vendor", i18n("Vendor"),
//                                  QVariant(QString(vi->vendor))));

    // get the vorbis comments
    if (readComment)
    {
        vc = ov_comment(&vf,-1);
        
        KFileMetaInfoGroup commentGroup = appendGroup(info, "Comment");
            
        for (i=0; i < vc->comments; i++)
        {
            kDebug(7034) << vc->user_comments[i];
            QStringList split = QString::fromUtf8(vc->user_comments[i]).split(QChar('='));
            split[0] = split[0].toLower();
            split[0][0] = split[0][0].toUpper();
 
            // we have to be sure that the i18n() string always has the same
            // case. Oh, and is UTF8 ok here?
            appendItem(commentGroup, split[0], split[1]);
        }
    }
 
    if (readTech)
    {  
        KFileMetaInfoGroup techgroup = appendGroup(info, "Technical");
        // get other information about the file
        vi = ov_info(&vf,-1);
        if (vi)
        {

            appendItem(techgroup, "Version", int(vi->version));
            appendItem(techgroup, "Channels", int(vi->channels));
            appendItem(techgroup, "Sample Rate", int(vi->rate));

            if (vi->bitrate_upper > 0) 
                appendItem(techgroup, "UpperBitrate",
                           int(vi->bitrate_upper+500)/1000);
            if (vi->bitrate_lower > 0) 
                appendItem(techgroup, "LowerBitrate",
                           int(vi->bitrate_lower+500)/1000);
            if (vi->bitrate_nominal > 0) 
                appendItem(techgroup, "NominalBitrate",
                           int(vi->bitrate_nominal+500)/1000);

            if (ov_bitrate(&vf,-1) > 0)
                appendItem(techgroup, "Bitrate", int(ov_bitrate(&vf,-1)+500)/1000);
            
        }
        
        appendItem(techgroup, "Length", int(ov_time_total(&vf,-1)));
    }

    ov_clear(&vf);

    return true;
}

bool KOggPlugin::writeInfo(const KFileMetaInfo& info) const
{
    // todo: add writing support
    FILE*   infile;
    
    infile = fopen(QFile::encodeName(info.path()), "r");
  
    if (!infile) 
    {
        kDebug(7034) << "couldn't open " << info.path();
        return false;
    }

    vcedit_state *state=vcedit_new_state();
    
    if ( vcedit_open(state, infile)==-1 ) 
    {
        kDebug(7034) << "error in vcedit_open for " << info.path();
        vcedit_clear(state);
        return false;
    }
    
    struct vorbis_comment* oc = vcedit_comments(state);
    struct vorbis_comment* vc = state->vc;
    
    if(vc) vorbis_comment_clear(vc);

    if (oc && oc->vendor) 
    {
        vc->vendor = strdup(oc->vendor);
    }
    else
    {
        vc->vendor = strdup("");
    }
    
    KFileMetaInfoGroup group = info["Comment"];

    QStringList keys = group.keys();
    QStringList::Iterator it;
    for (it = keys.begin(); it!=keys.end(); ++it)
    {
        KFileMetaInfoItem item = group[*it];
        
        if (!item.isEditable() || !(item.type()==QVariant::String) ) 
            continue;
                  
        QByteArray key = item.key().toUpper().toUtf8();
        if (item.value().canCast(QVariant::String))
        {
            QByteArray value = item.value().toString().utf8();

            kDebug(7034) << " writing tag " << key << "=" << value;
       
            vorbis_comment_add_tag(vc,
                        const_cast<char*>(static_cast<const char*>(key)),
                        const_cast<char*>(static_cast<const char*>(value)));
        }
        else
          kWarning(7034) << "ignoring " << key;
        
    }
    
    QString filename;
    
    QFileInfo fileinfo(info.path());
    
    // follow symlinks
    if (fileinfo.isSymLink())
        filename = fileinfo.readLink();
    else
        filename = info.path();
    
    KSaveFile sf(filename);
    if ( !sf.open() )
    {
        kDebug(7034) << "couldn't create temp file\n";
        vcedit_clear(state); // frees comment entries and vendor
        sf.abort();
        if (vc->vendor) free(vc->vendor);
        vc->vendor = 0;
        return false;
    }
    FILE* outfile = sf.fstream();

    vcedit_write(state,outfile); // calls vcedit_clear() itself so we don't free anything

    if (vc->vendor) free(vc->vendor);
    vc->vendor = 0;

    fclose(infile);
    sf.finalize(); //check for error here?
    
    return true;
}

QValidator* KOggPlugin::createValidator( const QString&, 
                                         const QString &, const QString &,
                                         QObject* parent, const char* name) const {
	return new QRegExpValidator(QRegExp(".*"), parent);
}

#include "kfile_ogg.moc"
