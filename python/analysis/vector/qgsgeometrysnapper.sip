/**
 * \class QgsGeometrySnapper
 * \ingroup analysis
 * QgsGeometrySnapper allows a geometry to be snapped to the geometries within a
 * different reference layer. Vertices in the geometries will be modified to
 * match the reference layer features within a specified snap tolerance.
 * \note added in QGIS 3.0
 */

class QgsGeometrySnapper : QObject
{
%TypeHeaderCode
#include <qgsgeometrysnapper.h>
%End

  public:

    //! Snapping modes
    enum SnapMode
    {
      PreferNodes,
      PreferClosest,
      EndPointPreferNodes,
      EndPointPreferClosest,
      EndPointToEndPoint,
    };

    /**
     * Constructor for QgsGeometrySnapper. A reference layer which contains geometries to snap to must be
     * set. It is assumed that all geometries snapped using this object will have the
     * same CRS as the reference layer (ie, no reprojection is performed).
     */
    QgsGeometrySnapper( QgsVectorLayer* referenceLayer );

    /**
     * Snaps a geometry to the reference layer and returns the result. The geometry must be in the same
     * CRS as the reference layer, and must have the same type as the reference layer geometry. The snap tolerance
     * is specified in the layer units for the reference layer.
     */
    QgsGeometry snapGeometry( const QgsGeometry& geometry, double snapTolerance, SnapMode mode = PreferNodes ) const;

    /**
     * Snaps a set of features to the reference layer and returns the result. This operation is
     * multithreaded for performance. The featureSnapped() signal will be emitted each time a feature
     * is processed. The snap tolerance is specified in the layer units for the reference layer.
     */
    QgsFeatureList snapFeatures( const QgsFeatureList& features, double snapTolerance, SnapMode mode = PreferNodes );

  signals:

    //! Emitted each time a feature has been processed when calling snapFeatures()
    void featureSnapped();

};

class QgsInternalGeometrySnapper
{
%TypeHeaderCode
#include <qgsgeometrysnapper.h>
%End

  public:
    QgsInternalGeometrySnapper( double snapTolerance, QgsGeometrySnapper::SnapMode mode = QgsGeometrySnapper::PreferNodes );
    QgsGeometry snapFeature( const QgsFeature &feature );
    QgsGeometryMap snappedGeometries() const;
};
