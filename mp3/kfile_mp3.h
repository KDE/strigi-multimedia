#ifndef __KMIME_MP3_H__
#define __KMIME_MP3_H__

#include <kfilemetainfo.h>

class QString;

class KMp3MetaInfo: public KFileMetaInfo
{
public:
    KMp3MetaInfo( const QString& path );
    virtual ~KMp3MetaInfo();
    
    virtual KFileMetaInfoItem * item( const QString& key ) const;
    
    virtual QStringList supportedKeys() const;
    virtual QStringList preferredKeys() const;
    
    virtual void applyChanges();
    virtual QValidator * createValidator( const QString& key, QObject *parent,
                                          const char *name ) const;

    QVariant::Type type( const QString& key ) const;  

private:
    char* m_path;
};

class KMp3Plugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    KMp3Plugin( QObject *parent, const char *name,
                const QStringList& preferredItems );
    
    virtual KFileMetaInfo* createInfo( const QString& path );
};


#endif
