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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "kfile_ogg.h"
#include "vcedit.h"

#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <qdict.h>
#include <qvalidator.h>

#include <kdebug.h>
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ksavefile.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include <sys/stat.h>
#include <unistd.h>

// known translations for common ogg/vorbis keys
// from http://www.ogg.org/ogg/vorbis/doc/v-comment.html
static const char* knownTranslations[] = {
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

K_EXPORT_COMPONENT_FACTORY(kfile_ogg, KGenericFactory<KOggPlugin>("kfile_ogg"));

KOggPlugin::KOggPlugin( QObject *parent, const char *name,
                        const QStringList &args )
    : KFilePlugin( parent, name, args )
{
    kdDebug(7034) << "ogg plugin\n";
    
    KFileMimeTypeInfo* info = addMimeTypeInfo( "application/x-ogg" );

    KFileMimeTypeInfo::GroupInfo* group = 0;

    // comment group
    group = info->addGroupInfo("Comment", i18n("Comment"),
                               KFileMimeTypeInfo::Addable |
                               KFileMimeTypeInfo::Removable);


    group->addVariableInfo(QVariant::String, KFileMimeTypeInfo::Addable |
                                             KFileMimeTypeInfo::Removable |
                                             KFileMimeTypeInfo::Modifiable);
    


    // technical group
    group = info->addGroupInfo("Technical", i18n("Technical details"), 0);

    
    
    group->addItemInfo("Version", i18n("Version"), QVariant::Int);
    group->addItemInfo("Channels", i18n("Channels"), QVariant::Int);
    group->addItemInfo("Sample Rate", i18n("Sample Rate"), QVariant::Int, 0,
                      KFileMimeTypeInfo::NoUnit,  KFileMimeTypeInfo::NoHint,
                      QString::null, i18n("Hz"));

    group->addItemInfo("Bitrate upper", i18n("Bitrate upper"), QVariant::Int,
                       0, KFileMimeTypeInfo::NoUnit,
                       KFileMimeTypeInfo::NoHint, QString::null, i18n("kbps"));

    group->addItemInfo("Bitrate lower", i18n("Bitrate lower"), QVariant::Int,
                       0, KFileMimeTypeInfo::NoUnit,
                       KFileMimeTypeInfo::NoHint, QString::null, i18n("kbps"));

    group->addItemInfo("Bitrate nominal", i18n("Bitrate nominal"), QVariant::Int,
                       0, KFileMimeTypeInfo::NoUnit,
                       KFileMimeTypeInfo::NoHint, QString::null, i18n("kbps"));

    group->addItemInfo("Bitrate", i18n("Bitrate average"), QVariant::Int,
                       KFileMimeTypeInfo::Averaged, 
                       KFileMimeTypeInfo::NoUnit, KFileMimeTypeInfo::Bitrate, 
                       QString::null, i18n("kbps"));

    group->addItemInfo("Length", i18n("Length"), QVariant::Int, 
                       KFileMimeTypeInfo::Cummulative,
                       KFileMimeTypeInfo::Seconds);
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

    memset(&vf,0,sizeof(OggVorbis_File));
  
    fp = fopen(QFile::encodeName(info.path()),"rb");
    if (!fp) return false;

    rc = ov_open(fp,&vf,NULL,0);

    if (rc < 0) 
    {
        kdDebug(7034) << "Unable to understand " << QFile::encodeName(info.path())
                      << ", errorcode=" << rc << endl;
        return false;
    }
  
//    info.insert(KFileMetaInfoItem("Vendor", i18n("Vendor"),
//                                  QVariant(QString(vi->vendor))));

    // get the vorbis comments
    if (readComment)
    {
        vc = ov_comment(&vf,-1);

        KFileMetaInfoGroup commentGroup = info.appendGroup("Comment");
            
        for (i=0; i < vc->comments; i++)
        {
            kdDebug(7034) << vc->user_comments[i] << endl;
            QStringList split = QStringList::split(QRegExp("="), QString::fromUtf8(vc->user_comments[i]));
            split[0] = split[0].lower();
            split[0][0] = split[0][0].upper();
        
            // we have to be sure that the i18n() string always has the same
            // case. Oh, and is UTF8 ok here?
            commentGroup.appendItem(split[0].utf8(), split[1]);
        }
    }
    
    if (readTech)
    {  
        KFileMetaInfoGroup techgroup = info.appendGroup("Technical");
        // get other information about the file
        vi = ov_info(&vf,-1);
        if (vi)
        {

            techgroup.appendItem("Version", int(vi->version));
            techgroup.appendItem("Channels", int(vi->channels));
            techgroup.appendItem("Sample Rate", int(vi->rate));

            if (vi->bitrate_upper > 0) 
                techgroup.appendItem("Bitrate upper",
                                     int(vi->bitrate_upper+500)/1000);
            if (vi->bitrate_lower > 0) 
                techgroup.appendItem("Bitrate lower",
                                     int(vi->bitrate_lower+500)/1000);
            if (vi->bitrate_nominal > 0) 
                techgroup.appendItem("Bitrate nominal",
                                     int(vi->bitrate_nominal+500)/1000);

            techgroup.appendItem("Bitrate",
                                     int(ov_bitrate(&vf,-1)+500)/1000);
            
        }
        
        techgroup.appendItem("Length", int(ov_time_total(&vf,-1)));
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
        kdDebug(7034) << "couldn't open " << info.path() << endl;
        return false;
    }

    vcedit_state *state=vcedit_new_state();
    
    if ( vcedit_open(state, infile)==-1 ) 
    {
        kdDebug(7034) << "error in vcedit_open for " << info.path() << endl;
        return false;
    }
    
    struct vorbis_comment* oc = vcedit_comments(state);
    struct vorbis_comment* vc = state->vc;
    
    if(vc) vorbis_comment_clear(vc);

    if (oc && oc->vendor) 
    {
        vc->vendor = new char[strlen(oc->vendor)+1];
        strcpy(vc->vendor,oc->vendor);
    }
    else
    {
      vc->vendor = new char[1];
      strcpy(vc->vendor,"");
    }
    
    KFileMetaInfoGroup group = info["Comment"];

    QStringList keys = group.keys();
    QStringList::Iterator it;
    for (it = keys.begin(); it!=keys.end(); ++it)
    {
        KFileMetaInfoItem item = group[*it];
        
        if (!item.isEditable() || !(item.type()==QVariant::String) ) 
            continue;
                  
        QCString key = item.key().upper().utf8();
        if (item.value().canCast(QVariant::String))
        {
            QCString value = item.value().toString().utf8();

            kdDebug(7034) << " writing tag " << key << "=" << value << endl;
       
            vorbis_comment_add_tag(vc,
                        const_cast<char*>(static_cast<const char*>(key)),
                        const_cast<char*>(static_cast<const char*>(value)));
        }
        else
          kdWarning(7034) << "ignoring " << key << endl;
        
    }
    
    // nothing in Qt or KDE to get the mode as an int?
    struct stat s;
    stat(QFile::encodeName(info.path()), &s);

    KSaveFile sf(info.path(), s.st_mode);
    FILE* outfile = sf.fstream();

    if ( sf.status() || !outfile)
    {
        kdDebug(7034) << "couldn't create temp file\n";
        vcedit_clear(state); // frees comment entries and vendor
        sf.abort();
        delete vc->vendor;
        return false;
    }

    
    vcedit_write(state,outfile); // calls vcedit_clear() itself so we don't free anything

    delete vc->vendor;

    fclose(infile);
    sf.close();
    
    return true;
}

QValidator* KOggPlugin::createValidator( const QString &, const QString &,
                                         QObject* parent, const char* name) const {
	return new QRegExpValidator(QRegExp(".*"), parent, name);
}

#include "kfile_ogg.moc"
