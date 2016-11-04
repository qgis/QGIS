/**
 * \class QgsGeometrySnapper
 * \ingroup analysis
 * QgsGeometrySnapper allows a geometry to be snapped to the geometries within a
 * different refence layer. Vertices in the geometries will be modified to
 * match the reference layer features within a specified snap tolerance.
 * \note added in QGIS 3.0
 */

class QgsGeometrySnapper : QObject
{
%TypeHeaderCode
#include <qgsgeometrysnapper.h>
%End

  public:

    /**
     * Constructor for QgsGeometrySnapper. A reference layer which contains geometries to snap to must be
     * set. The snap tolerance is specified in the layer units for the
     * reference layer, and it is assumed that all geometries snapped using this object will have the
     * same CRS as the reference layer (ie, no reprojection is performed).
     */
    QgsGeometrySnapper( QgsVectorLayer* referenceLayer, double snapTolerance );

    /**
     * Snaps a geometry to the reference layer and returns the result. The geometry must be in the same
     * CRS as the reference layer.
     */
    QgsGeometry snapGeometry( const QgsGeometry& geometry ) const;

    /**
     * Snaps a set of features to the reference layer and returns the result. This operation is
     * multithreaded for performance. The featureSnapped() signal will be emitted each time a feature
     * is processed. This method is not safe to call from multiple threads concurrently.
     */
    QgsFeatureList snapFeatures( const QgsFeatureList& features );

  signals:

    //! Emitted each time a feature has been processed when calling snapFeatures()
    void featureSnapped();
};
