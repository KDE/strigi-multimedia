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

// need this for the mp3info header
#define __MAIN
extern "C" {
#include "mp3info.h"
}
#undef __MAIN

typedef KGenericFactory<KMp3Plugin> Mp3Factory;

K_EXPORT_COMPONENT_FACTORY(kfile_mp3, Mp3Factory( "kfile_mp3" ));

#ifdef _GNUC
#warning ### FIXME ### we have to remove that validator from here so the plugin \
can safely be unloaded
#endif
// we need this one to better validate the id3 tag
class MyValidator: public QValidator
{
  public:
    MyValidator ( unsigned int maxlen, QObject * parent, const char * name = 0 )
      : QValidator(parent, name)
    {
        m_maxlen = maxlen;
    }

    virtual State validate ( QString & input, int & /*pos*/ ) const
    {
      if (input.length()>m_maxlen) return Invalid;
      return Acceptable;
    }
  private:
      int m_maxlen;
};


KMp3Plugin::KMp3Plugin(QObject *parent, const char *name,
                       const QStringList &preferredItems)
    
    : KFilePlugin(parent, name, preferredItems)
{
    kdDebug(7034) << "mp3 plugin\n";
}

bool KMp3Plugin::readInfo( KFileMetaInfo::Internal& info )
{
    kdDebug(7034) << "mp3 plugin readInfo\n";
    
    mp3info mp3;

    memset(&mp3,0,sizeof(mp3info));

    QCString s = QFile::encodeName(info.path());
    mp3.filename = new char[s.length()+1];
    strcpy(mp3.filename, s);
        
    mp3.file = fopen(mp3.filename, "rb");
    if (!mp3.file)
    {
        delete [] mp3.filename;
        kdDebug(7034) << "Couldn't open " << mp3.filename << endl;
        return false;
    }

    ::get_mp3_info(&mp3, ::SCAN_QUICK, ::VBR_VARIABLE);
  
    if (!mp3.header_isvalid) {
        return false;
    }

    QString value;
  
    if (!mp3.id3_isvalid)
        memset(&mp3.id3, sizeof(id3tag), 0);

    if (mp3.id3_isvalid) {
        info.insert(KFileMetaInfoItem("Title", i18n("Title"),
                    QVariant(QString::fromLocal8Bit(mp3.id3.title)), true));

        info.insert(KFileMetaInfoItem("Artist", i18n("Artist"),
                    QVariant(QString::fromLocal8Bit(mp3.id3.artist)), true));

        info.insert(KFileMetaInfoItem("Album", i18n("Album"),
                    QVariant(QString::fromLocal8Bit(mp3.id3.album)), true));

        info.insert(KFileMetaInfoItem("Year", i18n("Year"),
                    QVariant(QString::fromLocal8Bit(mp3.id3.year)), true));

         info.insert(KFileMetaInfoItem("Comment", i18n("Comment"),
                    QVariant(QString::fromLocal8Bit(mp3.id3.comment)), true));

         // the key is "Tracknumber" here because it's in ogg, too
        if (mp3.id3.track[0])
            info.insert(KFileMetaInfoItem("Tracknumber", i18n("Track"),
                        QVariant((int)mp3.id3.track[0]), true));
        
        if (mp3.id3.genre[0]>MAXGENRE)
            mp3.id3.genre[0] = 0;

        if (!mp3.id3_isvalid) {
            info.insert(KFileMetaInfoItem("Genre", i18n("Genre"),
                    QVariant(QString("")), true));
        }
        else  {
            info.insert(KFileMetaInfoItem("Genre", i18n("Genre"),
                        QVariant(QString::fromLocal8Bit(
                        ::typegenre[mp3.id3.genre[0]])), true));
        }
    }    
     
    // end of the id3 part
    
    info.insert(KFileMetaInfoItem("Version", i18n("Version"),
                QVariant(::header_version(&mp3.header)), false, i18n("MPEG")));
    
    info.insert(KFileMetaInfoItem("Layer",
        i18n("Layer"), QVariant(::header_layer(&mp3.header))));
    
    info.insert(KFileMetaInfoItem("CRC", i18n("CRC"),
                QVariant((bool)header_crc(&mp3.header), 42)));
    
    info.insert(KFileMetaInfoItem("Bitrate", i18n("Bitrate"),
                QVariant(::header_bitrate(&mp3.header)), false,
                QString::null, i18n("kbps")));
    
    info.insert(KFileMetaInfoItem("Sample Rate", i18n("Sample Rate"),
                QVariant(::header_frequency(&mp3.header)), false,
                QString::null, i18n("Hz")));
    
    // Modes 0-2 are forms of stereo, mode 3 is mono
    info.insert(KFileMetaInfoItem("Channels", i18n("Channels"),
                QVariant(int((mp3.header.mode == 3) ? 1 : 2))));
    
    info.insert(KFileMetaInfoItem("Copyright", i18n("Copyright"),
                QVariant((bool)mp3.header.copyright, 42)));

    info.insert(KFileMetaInfoItem("Original", i18n("Original"),
                QVariant((bool)mp3.header.original, 42)));

    info.insert(KFileMetaInfoItem("Padding", i18n("Padding"),
                QVariant((bool)mp3.header.padding, 42)));

    int playmin = mp3.seconds / 60;
    int playsec = mp3.seconds % 60;
    QString str;
    str = QString("%0:%1").arg(playmin)
                          .arg(QString::number(playsec).rightJustify(2,'0') );

    info.insert(KFileMetaInfoItem("Length", i18n("Length"),
                QVariant(str), false, QString::null));
    
    info.insert(KFileMetaInfoItem("Emphasis", i18n("Emphasis"),
                QVariant(::header_emphasis(&mp3.header))));

/* Dunno what this is:
      mp3.header.extension
*/
    fclose(mp3.file);
    kdDebug(7034) << "reading finished\n";
    
    delete [] mp3.filename;
    
    kdDebug(7034) << "the preferredItems contain " << m_preferred.size() << endl;
    
    info.setPreferredKeys(m_preferred);
    info.setSupportedKeys(m_preferred);
    info.setSupportsVariableKeys(false);
    
    return true;
}

bool KMp3Plugin::writeInfo( const KFileMetaInfo::Internal& info) const
{
    mp3info mp3;
    memset(&mp3,0,sizeof(mp3info));

    QCString name = QFile::encodeName(info.path());
    mp3.filename = new char[name.length()+1];
    strcpy(mp3.filename, name);

    mp3.file = fopen(mp3.filename, "rb+");
    
    if (!mp3.file) 
    {
      kdDebug(7034) << "couldn't open " << info.path() << endl;
      delete [] mp3.filename;
      return false;
    }

    ::get_mp3_info(&mp3, ::SCAN_NONE, ::VBR_VARIABLE);
    
    strncpy(mp3.id3.title,  info["Title"]  .value().toString().local8Bit(), 31);
    strncpy(mp3.id3.artist, info["Artist"] .value().toString().local8Bit(), 31);
    strncpy(mp3.id3.album,  info["Album"]  .value().toString().local8Bit(), 31);
    strncpy(mp3.id3.year,   info["Year"]   .value().toString().local8Bit(),  5);
    strncpy(mp3.id3.comment,info["Comment"].value().toString().local8Bit(), 29);

    KFileMetaInfoItem track = info["Tracknumber"];
    if (track.isValid()) mp3.id3.track[0] = track.value().toInt();
    
    QString s = info["Genre"].value().toString();

    for (mp3.id3.genre[0] = 0; mp3.id3.genre[0] <= MAXGENRE; mp3.id3.genre[0]++)
    {
        if (s == ::typegenre[mp3.id3.genre[0]]) break;
    }
      
    bool success = ::write_tag(&mp3);
    fclose(mp3.file);
    
    delete [] mp3.filename;
  
    return success;
}

QValidator* KMp3Plugin::createValidator(const KFileMetaInfoItem& item,
                                        QObject* parent, const char* name) const
{
    if ((item.key() == "Title") || (item.key() == "Artist")||
        (item.key() == "Album"))
    {
        return new MyValidator(30, parent, name);
    }
    else if (item.key() == "Year")
    {
        return new MyValidator(4, parent, name);
    }
    else if (item.key() == "Comment")
    {
        return new MyValidator(28, parent, name);
    }
    else if (item.key() == "Tracknumber")
    {
        return new QIntValidator(0, 255, parent, name);
    }
    else if (item.key() == "Genre")
    {
        QStringList list;
        for (int index = 0; index <= MAXGENRE; index++)
        {
           list += ::typegenre[galphagenreindex[index]];
         }

        return new KStringListValidator(list, false, true, parent, name);
    }
    else return 0;
}

#include "kfile_mp3.moc"
