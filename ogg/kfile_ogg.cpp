#include "kfile_ogg.h"
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <kgenericfactory.h>
#include <qdict.h>
#include <qvalidator.h>

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

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
//  I18N_NOOP("Isirc") // dunno what an Isirc number is, the link is broken
};    


K_EXPORT_COMPONENT_FACTORY( kfile_ogg, KGenericFactory<KOggPlugin>( "kfile_ogg" ) );

KOggPlugin::KOggPlugin( QObject *parent, const char *name,
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
}

KFileMetaInfo* KOggPlugin::createInfo( const QString& path )
{
    return new KOggMetaInfo(path);
}



KOggMetaInfo::KOggMetaInfo( const QString& path ) :
    KFileMetaInfo::KFileMetaInfo (path)
{
    // parts of this code taken from ogginfo.c of the vorbis-tools v1.0rc2
    FILE *fp;
    OggVorbis_File vf;
    int rc,i;
    vorbis_comment *vc;
    vorbis_info *vi;
    double playtime;
    long playmin,playsec;

    memset(&vf,0,sizeof(OggVorbis_File));
  
    fp = fopen(QFile::encodeName(path),"rb");
    if (!fp) return;

    rc = ov_open(fp,&vf,NULL,0);

    if (rc < 0) 
    {
        qDebug("Unable to understand \"%s\", errorcode=%d",(const char*)QFile::encodeName(path),rc);
        return;
    }
  
    vc = ov_comment(&vf,-1);

    // get the vorbis comments
    for (i=0; i < vc->comments; i++)
    {
        qDebug("%s",vc->user_comments[i]);
        QStringList split = QStringList::split(QRegExp("="), vc->user_comments[i]);
        split[0] = split[0].lower();
        split[0][0] = split[0][0].upper();
        
        // we have to be sure that the i18n() string always has the same
        // case
        m_items.insert(split[0], new KFileMetaInfoItem(split[0],
            i18n(split[0].utf8()), QVariant(split[1]), true));
    }
    
    // get other information about the file
    vi = ov_info(&vf,-1);
    if (vi)
    {
        m_items.insert("Version", new KFileMetaInfoItem("Version",
          i18n("Version"), QVariant(vi->version)));
    
        m_items.insert("Channels", new KFileMetaInfoItem("Channels",
          i18n("Channels"), QVariant(vi->channels)));

        if (vi->bitrate_upper != -1) 
            m_items.insert("Bitrate upper", new KFileMetaInfoItem("Bitrate upper",
                i18n("Bitrate upper"), QVariant((int)(vi->bitrate_upper+500)/1000),
                false, QString::null, i18n("kbps")));
        else
            m_items.insert("Bitrate upper", new KFileMetaInfoItem("Bitrate upper",
                i18n("Bitrate upper"), QVariant(i18n("none"))));


        if (vi->bitrate_lower != -1) 
            m_items.insert("Bitrate lower", new KFileMetaInfoItem("Bitrate lower",
                i18n("Bitrate lower"), QVariant((int)(vi->bitrate_lower+500)/1000),
                false, QString::null, i18n("kbps")));
        else
            m_items.insert("Bitrate lower", new KFileMetaInfoItem("Bitrate lower",
                i18n("Bitrate lower"), QVariant(i18n("none"))));

    
        if (vi->bitrate_nominal != -1) 
            m_items.insert("Bitrate nominal", new KFileMetaInfoItem("Bitrate nominal",
                i18n("Bitrate nominal"), QVariant((int)(vi->bitrate_nominal+500)/1000),
                false, QString::null, i18n("kbps")));
        else
            m_items.insert("Bitrate nominal", new KFileMetaInfoItem("Bitrate nominal",
                i18n("Bitrate nominal"), i18n("none")));
            
        m_items.insert("Bitrate", new KFileMetaInfoItem("Bitrate",
                i18n("Bitrate average"), QVariant((int)(ov_bitrate(&vf,-1)+500)/1000), false,
                QString::null, i18n("kbps")));
    }

    playtime = ov_time_total(&vf,-1);
    playmin = ((long)playtime) / 60L;
    playsec = ((long)playtime) % 60L;
  
    m_items.insert("Length", new KFileMetaInfoItem("Length",
        i18n("Length"), QVariant(QString("%1:%02").arg(playmin).arg(playsec))));
  
    ov_clear(&vf);
}

KOggMetaInfo::~KOggMetaInfo()
{
}

KFileMetaInfoItem * KOggMetaInfo::item( const QString& key ) const
{
    return m_items[key];
}

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

void KOggMetaInfo::applyChanges()
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

    // todo: add writing support
}

bool KOggMetaInfo::supportsVariableKeys() const
{
    return true;
}

QValidator * KOggMetaInfo::createValidator( const QString& key, QObject *parent,
                                            const char *name ) const
{
    if (m_items[key]->isEditable())
        return new QRegExpValidator(QRegExp(".*"), parent, name);
    else 
        return 0L;
}

QVariant::Type KOggMetaInfo::type( const QString& key ) const
{
    return m_items[key]->type();
}

#include "kfile_ogg.moc"
