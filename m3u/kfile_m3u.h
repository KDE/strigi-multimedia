#ifndef __KMIME_M3U_H__
#define __KMIME_M3U_H__

#include <kfilemetainfo.h>
#include <kurl.h>

class QString;

class KM3uMetaInfo: public KFileMetaInfo
{
public:
    KM3uMetaInfo( const QString& path );
    virtual ~KM3uMetaInfo();
    
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

class KM3uPlugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    KM3uPlugin( QObject *parent, const char *name,
                const QStringList& preferredItems );
    
    virtual KFileMetaInfo* createInfo( const QString& path );
};


#endif
