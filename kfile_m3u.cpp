#include "kfile_m3u.h"
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <kgenericfactory.h>
#include <qdict.h>
#include <qvalidator.h>

K_EXPORT_COMPONENT_FACTORY( kfile_m3u, KGenericFactory<KM3uPlugin>( "kfile_m3u" ) );

KM3uPlugin::KM3uPlugin( QObject *parent, const char *name,
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
}

KFileMetaInfo* KM3uPlugin::createInfo( const QString& path )
{
    return new KM3uMetaInfo(path);
}



KM3uMetaInfo::KM3uMetaInfo( const QString& path ) :
    KFileMetaInfo::KFileMetaInfo (path)
{
  
    QFile f(path);
    f.open(IO_ReadOnly);
    
    // for now treat all lines like entries
    int num = 1;
    while (!f.atEnd())
    {
        QString s;
        f.readLine(s, 1000);
        if (!s.startsWith("#"))
        {
            QString key; key.sprintf("%04d", num);
            if (s.endsWith("\n")) s.truncate(s.length()-1);
            m_items.insert(key, new KFileMetaInfoItem(key,
                            i18n("Track %1").arg(num), QVariant(s)));
            num++;
        }
    }
}

KM3uMetaInfo::~KM3uMetaInfo()
{
}

KFileMetaInfoItem * KM3uMetaInfo::item( const QString& key ) const
{
    return m_items[key];
}

QStringList KM3uMetaInfo::supportedKeys() const
{
    QDictIterator<KFileMetaInfoItem> it(m_items);
    QStringList list;
    
    for (; it.current(); ++it)
    {
        list.append(it.current()->key());
    }
    return list;
}

QStringList KM3uMetaInfo::preferredKeys() const
{
    QStringList l = supportedKeys();
    l.sort();
    return l;
}

void KM3uMetaInfo::applyChanges()
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

    // todo
}

bool KM3uMetaInfo::supportsVariableKeys() const
{
    return true;
}

QValidator * KM3uMetaInfo::createValidator( const QString& key, QObject *parent,
                                            const char *name ) const
{
    if (m_items[key]->isEditable())
        return new QRegExpValidator(QRegExp(".*"), parent, name);
    else 
        return 0L;
}

QVariant::Type KM3uMetaInfo::type( const QString& key ) const
{
    return m_items[key]->type();
}

#include "kfile_m3u.moc"
