/**
 * @brief The QgsDatumTransformStore class keeps track of datum transformations
 * as chosen by the user.
 *
 * @note added in 2.4
 */
class QgsDatumTransformStore
{
%TypeHeaderCode
#include <qgsdatumtransformstore.h>
%End

  public:
    explicit QgsDatumTransformStore( const QgsCoordinateReferenceSystem& destCrs );

    void clear();

    void setDestinationCrs( const QgsCoordinateReferenceSystem& destCrs );

    void addEntry( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform, int destDatumTransform );

    bool hasEntryForLayer( QgsMapLayer* layer ) const;

    /** Will return transform from layer's CRS to current destination CRS.
     *  Will emit datumTransformInfoRequested signal if the layer has no entry.
     *  @returns transformation associated with layer, or an invalid QgsCoordinateTransform
     *  if no transform is associated with the layer
     */
    QgsCoordinateTransform transformation( QgsMapLayer* layer ) const;


    void readXml( const QDomNode& parentNode );

    void writeXml( QDomNode& parentNode, QDomDocument& theDoc ) const;

};
