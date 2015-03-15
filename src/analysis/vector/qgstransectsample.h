#ifndef QGSTRANSECTSAMPLE_H
#define QGSTRANSECTSAMPLE_H

#include "qgsfeature.h"
#include <QMap>
#include <QString>

class QgsDistanceArea;
class QgsGeometry;
class QgsSpatialIndex;
class QgsVectorLayer;
class QgsPoint;
class QProgressDialog;

/**A class for the creation of transect sample lines based on a set of strata polygons and baselines*/
class ANALYSIS_EXPORT QgsTransectSample
{
  public:

    enum DistanceUnits
    {
      Meters,
      StrataUnits //units are the same as stratum layer
    };

    QgsTransectSample( QgsVectorLayer* strataLayer, QString strataIdAttribute, QString minDistanceAttribute, QString nPointsAttribute,
                       DistanceUnits minDistUnits, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       QString baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer, const QString& usedBaselineLayer, double minTransectLength = 0.0,
                       double baselineBufferDistance = -1.0, double baselineSimplificationTolerance = -1.0 );
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );

  private:
    QgsTransectSample(); //default constructor forbidden

    QgsGeometry* findBaselineGeometry( QVariant strataId );

    /**Returns true if another transect is within the specified minimum distance*/
    static bool otherTransectWithinDistance( QgsGeometry* geom, double minDistLayerUnit, double minDistance, QgsSpatialIndex& sIndex, const QMap< QgsFeatureId, QgsGeometry* >&
        lineFeatureMap, QgsDistanceArea& da );

    QgsVectorLayer* mStrataLayer;
    QString mStrataIdAttribute;
    QString mMinDistanceAttribute;
    QString mNPointsAttribute;

    QgsVectorLayer* mBaselineLayer;
    bool mShareBaseline;
    QString mBaselineStrataId;

    QString mOutputPointLayer;
    QString mOutputLineLayer;
    QString mUsedBaselineLayer;

    DistanceUnits mMinDistanceUnits;

    double mMinTransectLength;

    /**If value is negative, the buffer distance ist set to the same value as the minimum distance*/
    double mBaselineBufferDistance;
    /**If value is negative, no simplification is done to the baseline prior to create the buffer*/
    double mBaselineSimplificationTolerance;

    /**Finds the closest points between two line segments
        @param g1 first input geometry. Must be a linestring with two vertices
        @param g2 second input geometry. Must be a linestring with two vertices
        @param dist out: distance between the segments
        @param pt1 out: closest point on first geometry
        @param pt2 out: closest point on secont geometry
        @return true in case of success*/
    static bool closestSegmentPoints( QgsGeometry& g1, QgsGeometry& g2, double& dist, QgsPoint& pt1, QgsPoint& pt2 );
    /**Returns a copy of the multiline element closest to a point (caller takes ownership)*/
    static QgsGeometry* closestMultilineElement( const QgsPoint& pt, QgsGeometry* multiLine );
    /**Returns clipped buffer line. Iteratively applies reduced tolerances if the result is not a single line
        @param stratumGeom stratum polygon
        @param clippedBaseline base line geometry clipped to the stratum
        @param tolerance buffer distance (in layer units)
        @return clipped buffer line or 0 in case of error*/
    QgsGeometry* clipBufferLine( const QgsGeometry* stratumGeom, QgsGeometry* clippedBaseline, double tolerance );

    /**Returns distance to buffer the baseline (takes care of units and buffer settings*/
    double bufferDistance( double minDistanceFromAttribute ) const;
};

#endif // QGSTRANSECTSAMPLE_H
