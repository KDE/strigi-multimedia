#include "kfile_mp3.h"
#include <kprocess.h>
#include <klocale.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <kgenericfactory.h>
#include <qdict.h>
#include <qvalidator.h>

// need this for the mp3info header
#define __MAIN
extern "C" {
#include "mp3info.h"
}
#undef __MAIN

K_EXPORT_COMPONENT_FACTORY( kfile_mp3, KGenericFactory<KMp3Plugin> );

KMp3Plugin::KMp3Plugin( QObject *parent, const char *name,
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
}

KFileMetaInfo* KMp3Plugin::createInfo( const QString& path )
{
    return new KMp3MetaInfo(path);
}



KMp3MetaInfo::KMp3MetaInfo( const QString& path ) :
    KFileMetaInfo (path)
{
    mp3info mp3;

  	memset(&mp3,0,sizeof(mp3info));

    QCString s = QFile::encodeName(path);
    mp3.filename = m_path = new char[s.length()+1];
    strcpy(mp3.filename, s);
        
    mp3.file = fopen(mp3.filename, "rb");
    ::get_mp3_info(&mp3, ::SCAN_QUICK, ::VBR_VARIABLE);
  
    if (!mp3.header_isvalid) return;

    QString value;
  
    if (mp3.id3_isvalid)
    {
        m_items.insert("Title", new KFileMetaInfoItem("Title",
            i18n("Title"), QVariant(mp3.id3.title), true));

        m_items.insert("Artist", new KFileMetaInfoItem("Artist",
            i18n("Artist"), QVariant(mp3.id3.artist), true));

        m_items.insert("Album", new KFileMetaInfoItem("Album",
            i18n("Album"), QVariant(mp3.id3.album), true));

        m_items.insert("Year", new KFileMetaInfoItem("Year",
            i18n("Year"), QVariant(mp3.id3.year), true));

        m_items.insert("Comment", new KFileMetaInfoItem("Comment",
            i18n("Comment"), QVariant(mp3.id3.comment), true));
        
        m_items.insert("Track", new KFileMetaInfoItem("Track",
            i18n("Track"), QVariant(mp3.id3.track[0]), true));
        
        if (mp3.id3.genre[0]<=MAXGENRE)
        {
            m_items.insert("Genre", new KFileMetaInfoItem("Genre",
                i18n("Genre"), QVariant(::typegenre[mp3.id3.genre[0]])));

            m_items.insert("GenreNo", new KFileMetaInfoItem("Genre No.",
                i18n("Genre No."), QVariant(mp3.id3.genre[0]), true));
        }
        
    }
    else
        m_items.insert("Title", new KFileMetaInfoItem("Title",
            i18n("Title"), QVariant(path)));
    
    m_items.insert("Version", new KFileMetaInfoItem("Version",
        i18n("Version"), QVariant(mp3.header.version)));
    
    m_items.insert("Layer", new KFileMetaInfoItem("Layer",
        i18n("Layer"), QVariant(::header_layer(&mp3.header))));
    
    m_items.insert("CRC", new KFileMetaInfoItem("CRC",
        i18n("CRC"), QVariant((bool)mp3.header.crc)));
    
    m_items.insert("Bitrate", new KFileMetaInfoItem("Bitrate",
        i18n("Bitrate"), QVariant(::header_bitrate(&mp3.header)), false,
        QString::null, i18n("kbps")));
    
    m_items.insert("Frequency", new KFileMetaInfoItem("Frequency",
        i18n("Frequency"), QVariant(::header_frequency(&mp3.header)), false,
        QString::null, i18n("Hz")));
    
    m_items.insert("Copyright", new KFileMetaInfoItem("Copyright",
        i18n("Copyright"),
        QVariant((bool)mp3.header.copyright)));

    m_items.insert("Original", new KFileMetaInfoItem("Original",
        i18n("Original"),
        QVariant((bool)mp3.header.original)));

    // someone know a better way?
    QTime time(0, 0, 0);
    time = time.addSecs(mp3.seconds);

    m_items.insert("Length", new KFileMetaInfoItem("Length",
        i18n("Length"), QVariant(time), false, QString::null));
        
/* I need to look those up
      l->insert("Padding", QString().setNum(mp3.header.padding));
      l->insert("Extension", QString().setNum(mp3.header.extension));
      l->insert("Mode", QString().setNum(mp3.header.mode));
      l->insert("Mode Extension", QString().setNum(mp3.header.mode_extension));
      l->insert("Emphasis", QString().setNum(mp3.header.emphasis));
*/
    fclose(mp3.file);
}

KMp3MetaInfo::~KMp3MetaInfo()
{
    delete [] m_path;
}

KFileMetaInfoItem * KMp3MetaInfo::item( const QString& key ) const
{
    return m_items[key];
}

QStringList KMp3MetaInfo::supportedKeys() const
{
    QDictIterator<KFileMetaInfoItem> it(m_items);
    QStringList list;
    
    for (; it.current(); ++it)
    {
        list.append(it.current()->key());
    }
    return list;
}

QStringList KMp3MetaInfo::preferredKeys() const
{
    return supportedKeys();
}

void KMp3MetaInfo::applyChanges()
{
    bool doit = false;
    
    // look up if we need to write to the file
    QDictIterator<KFileMetaInfoItem> it(m_items);
    for( ; it.current(); ++it )
    {
        if (it.current()->isModified())
        {
            doit = true;
            break;
        }
    }
    
    if (!doit) return;

    mp3info mp3;
  	memset(&mp3,0,sizeof(mp3info));

    mp3.filename = m_path;

    mp3.file = fopen(mp3.filename, "rb");
    
    strncpy(mp3.id3.title, m_items["Title"]->value().toString().latin1(), 31);
    strncpy(mp3.id3.artist, m_items["Artist"]->value().toString().latin1(), 31);
    strncpy(mp3.id3.album, m_items["Album"]->value().toString().latin1(), 31);
    strncpy(mp3.id3.year, m_items["Year"]->value().toString().latin1(), 5);
    strncpy(mp3.id3.comment, m_items["Comment"]->value().toString().latin1(), 29);
    mp3.id3.track[0] = m_items["Track"]->value().toInt();
    
    int genre = m_items["Genre"]->value().toInt();
    if (genre<=MAXGENRE)
    {
        mp3.id3.genre[0] = genre;
    }
    
    ::write_tag(&mp3);
    fclose(mp3.file);
}

QValidator * KMp3MetaInfo::createValidator( const QString& key, QObject *parent,
                                            const char *name ) const
{
    if ((key == "Title") || (key == "Artist")|| (key == "Album"))
    {
        return new QRegExpValidator(QRegExp(".*{,31}"), parent, name);
    }
    else if (key == "Year")
    {
        // perhaps we should make the range smaller?
        return new QIntValidator(0, 9999, parent, name);
    }
    else if (key == "Comment")
    {
        return new QRegExpValidator(QRegExp(".*{,29}"), parent, name);
    }
    else if (key == "Track")
    {
        return new QIntValidator(0, 255, parent, name);
    }
    else if (key == "GenreNo")
    {
        return new QIntValidator(0, MAXGENRE, parent, name);
    }
    else return 0L;
}

QVariant::Type KMp3MetaInfo::type( const QString& key ) const
{
    return m_items[key]->type();
}

#include "kfile_mp3.moc"
