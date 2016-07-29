class QgsVectorLayerEditPassthrough : QgsVectorLayerEditBuffer
{
%TypeHeaderCode
#include <qgsvectorlayereditpassthrough.h>
%End
  public:
    QgsVectorLayerEditPassthrough( QgsVectorLayer* layer );
    bool isModified() const;
    bool addFeature( QgsFeature& f );
    bool addFeatures( QgsFeatureList& features );
    bool deleteFeature( QgsFeatureId fid );
    bool deleteFeatures( const QgsFeatureIds& fids );
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );
    bool addAttribute( const QgsField &field );
    bool deleteAttribute( int attr );
    bool renameAttribute( int attr, const QString& newName );
    bool commitChanges( QStringList& commitErrors );
    void rollBack();

};
