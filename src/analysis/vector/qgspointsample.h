#ifndef QGSPOINTSAMPLE_H
#define QGSPOINTSAMPLE_H

#include "qgsfeature.h"
#include <QString>

class QgsFeature;
class QgsPoint;
class QgsSpatialIndex;
class QgsVectorFileWriter;
class QgsVectorLayer;
class QProgressDialog;

/**Creates random points in polygons / multipolygons*/
class ANALYSIS_EXPORT QgsPointSample
{
  public:
    QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, QString nPointsAttribute, QString minDistAttribute = QString() );
    ~QgsPointSample();

    /**Starts calculation of random points
        @return 0 in case of success*/
    int createRandomPoints( QProgressDialog* pd );

  private:

    QgsPointSample(); //default constructor is forbidden
    void addSamplePoints( QgsFeature& inputFeature, QgsVectorFileWriter& writer, int nPoints, double minDistance );
    bool checkMinDistance( QgsPoint& pt, QgsSpatialIndex& index, double minDistance, QMap< QgsFeatureId, QgsPoint >& pointMap );

    /**Layer id of input polygon/multipolygon layer*/
    QgsVectorLayer* mInputLayer;
    /**Output path of result layer*/
    QString mOutputLayer;
    /**Attribute containing number of points per feature*/
    QString mNumberOfPointsAttribute;
    /**Attribute containing minimum distance between sample points (or -1 if no min. distance constraint)*/
    QString mMinDistanceAttribute;
    QgsFeatureId mNCreatedPoints; //helper to find free ids
};

#endif // QGSPOINTSAMPLE_H
