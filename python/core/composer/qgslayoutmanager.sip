
class QgsLayoutManager : QObject
{
%TypeHeaderCode
#include <qgslayoutmanager.h>
%End
  public:

    explicit QgsLayoutManager( QgsProject* project = 0 );

    ~QgsLayoutManager();
    bool addComposition( QgsComposition* composition /Transfer/ );
    bool removeComposition( QgsComposition* composition );

    void clear();
    QList< QgsComposition * > compositions() const;
    QgsComposition *compositionByName( const QString &name ) const;
    bool readXml( const QDomElement &element, const QDomDocument &doc );
    QDomElement writeXml( QDomDocument &doc ) const;
    bool saveAsTemplate( const QString &name, QDomDocument &doc ) const;
    QgsComposition *duplicateComposition( const QString &name, const QString &newName );
    QString generateUniqueTitle() const;

  signals:

    void compositionAboutToBeAdded( const QString &name );
    void compositionAdded( const QString &name );
    void compositionRemoved( const QString &name );
    void compositionAboutToBeRemoved( const QString &name );
    void compositionRenamed( QgsComposition *composition, const QString &newName );


};

