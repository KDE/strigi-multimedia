#ifndef __KMIME_OGG_H__
#define __KMIME_OGG_H__

#include <kfilemetainfo.h>
#include <kurl.h>

class QString;

class KOggMetaInfo: public KFileMetaInfo
{
public:
    KOggMetaInfo( const QString& path );
    virtual ~KOggMetaInfo();
    
    virtual KFileMetaInfoItem * item( const QString& key ) const;
    
    virtual QStringList supportedKeys() const;
    virtual QStringList preferredKeys() const;
    virtual bool supportsVariableKeys() const;
    
    virtual void applyChanges();
    virtual QValidator * createValidator( const QString& key, QObject *parent,
                                          const char *name ) const;

    QVariant::Type type( const QString& key ) const;  
        
private:
    char* m_path;
};

class KOggPlugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    KOggPlugin( QObject *parent, const char *name,
                const QStringList& preferredItems );
    
    virtual KFileMetaInfo* createInfo( const QString& path );
};


#endif
