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
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
    kdDebug(7034) << "ogg plugin\n";
}

bool KOggPlugin::readInfo( KFileMetaInfo::Internal& info )
{
    // parts of this code taken from ogginfo.c of the vorbis-tools v1.0rc2
    FILE *fp;
    OggVorbis_File vf;
    int rc,i;
    vorbis_comment *vc;
    vorbis_info *vi;
    double playtime;
    int playmin,playsec;

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
  
    vc = ov_comment(&vf,-1);

//    info.insert(KFileMetaInfoItem("Vendor", i18n("Vendor"),
//                                  QVariant(QString(vi->vendor))));

    // get the vorbis comments
    for (i=0; i < vc->comments; i++)
    {
        kdDebug(7034) << vc->user_comments[i];
        QStringList split = QStringList::split(QRegExp("="), vc->user_comments[i]);
        split[0] = split[0].lower();
        split[0][0] = split[0][0].upper();
        
        // we have to be sure that the i18n() string always has the same
        // case. Oh, and is UTF8 ok here?
        info.insert(KFileMetaInfoItem(split[0], i18n(split[0].utf8()),
                                      QVariant(split[1]), true));
    }
    
    // get other information about the file
    vi = ov_info(&vf,-1);
    if (vi)
    {
        info.insert(KFileMetaInfoItem("Version", i18n("Version"),
                                      QVariant(vi->version)));
    
        info.insert(KFileMetaInfoItem("Channels", i18n("Channels"),
                                      QVariant(vi->channels)));

        info.insert(KFileMetaInfoItem("Frequency", i18n("Frequency"),
                                      QVariant((int)vi->rate)));

        if (vi->bitrate_upper != -1) 
            info.insert(KFileMetaInfoItem("Bitrate upper", i18n("Bitrate upper"),
                              QVariant((int)(vi->bitrate_upper+500)/1000),
                              false, QString::null, i18n("kbps")));
        else
            info.insert(KFileMetaInfoItem("Bitrate upper", i18n("Bitrate upper"),
                              QVariant(i18n("none"))));


        if (vi->bitrate_lower != -1) 
            info.insert(KFileMetaInfoItem("Bitrate lower", i18n("Bitrate lower"),
                              QVariant((int)(vi->bitrate_lower+500)/1000),
                              false, QString::null, i18n("kbps")));
        else
            info.insert(KFileMetaInfoItem("Bitrate lower", i18n("Bitrate lower"),
                              QVariant(i18n("none"))));

    
        if (vi->bitrate_nominal != -1) 
            info.insert(KFileMetaInfoItem("Bitrate nominal", i18n("Bitrate nominal"),
                              QVariant((int)(vi->bitrate_nominal+500)/1000),
                              false, QString::null, i18n("kbps")));
        else
            info.insert(KFileMetaInfoItem("Bitrate nominal", i18n("Bitrate nominal"),
                              i18n("none")));
            
            info.insert(KFileMetaInfoItem("Bitrate", i18n("Bitrate average"),
                              QVariant((int)(ov_bitrate(&vf,-1)+500)/1000),
                              false, QString::null, i18n("kbps")));
    }

    playtime = ov_time_total(&vf,-1);
    playmin = (int)playtime / 60;
    playsec = (int)playtime % 60;
    QString str;
    str = QString("%0:%1").arg(playmin)
                          .arg(QString::number(playsec).rightJustify(2,'0') );
    
    info.insert(KFileMetaInfoItem("Length", i18n("Length"), QVariant(str)));
  
    ov_clear(&vf);
}

bool KOggPlugin::writeInfo(const KFileMetaInfo::Internal& info) const
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
      vc->vendor = new char;
      strcpy(vc->vendor,"");
    }

    QMapIterator<QString, KFileMetaInfoItem> it;
    
    for (it = info.map()->begin(); it!=info.map()->end(); ++it)
    {
        KFileMetaInfoItem item = it.data();
        
        if (!item.isEditable() || !(item.type()==QVariant::String) ) 
            continue;
                  
        QCString key = it.data().key().upper().utf8();
        QCString value = it.data().value().toString().utf8();
        
        kdDebug(7034) << " writing tag " << key << "=" << value << endl;

        vorbis_comment_add_tag(vc,
                        const_cast<char*>(static_cast<const char*>(key)),
                        const_cast<char*>(static_cast<const char*>(value)));
        
    }
    
    // nothing in Qt or KDE to get this?
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
    fclose(outfile);
    sf.close();
    
    return true;
}
  

QValidator* KOggPlugin::createValidator(const KFileMetaInfoItem& item,
                                        QObject *parent,
                                        const char *name ) const
{
    if (item.isEditable())
        return new QRegExpValidator(QRegExp(".*"), parent, name);
    else 
        return 0L;
}

#if 0

QStringList KOggMetaInfo::supportedKeys() const
{
    QDictIterator<KFileMetaInfoItem> it(m_items);
    QStringList list;
    
    for (; it.current(); ++it)
    {
        list.append(it.current()->key());
    }
    return list;
}

QStringList KOggMetaInfo::preferredKeys() const
{
    return supportedKeys();
}

#endif

#include "kfile_ogg.moc"
