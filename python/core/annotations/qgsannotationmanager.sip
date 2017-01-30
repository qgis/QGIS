class QgsAnnotationManager : QObject
{
%TypeHeaderCode
#include <qgsannotationmanager.h>
%End
  public:

    explicit QgsAnnotationManager( QgsProject* project = nullptr );
    ~QgsAnnotationManager();

    bool addAnnotation( QgsAnnotation* annotation /Transfer/ );
    bool removeAnnotation( QgsAnnotation* annotation );
    void clear();
    QList< QgsAnnotation* > annotations() const;
    bool readXml( const QDomElement& element, const QDomDocument& doc );
    QDomElement writeXml( QDomDocument& doc ) const;

  signals:

    void annotationAdded( QgsAnnotation* annotation );
    void annotationRemoved();
    void annotationAboutToBeRemoved( QgsAnnotation* annotation );

};
