/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgstransectsample.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsTransectSample
{
%Docstring
 A class for the creation of transect sample lines based on a set of strata polygons and baselines*
%End

%TypeHeaderCode
#include "qgstransectsample.h"
%End
  public:

    enum DistanceUnits
    {
      Meters,
      StrataUnits
    };

    QgsTransectSample( QgsVectorLayer *strataLayer, const QString &strataIdAttribute, const QString &minDistanceAttribute, const QString &nPointsAttribute,
                       DistanceUnits minDistUnits, QgsVectorLayer *baselineLayer, bool shareBaseline,
                       const QString &baselineStrataId, const QString &outputPointLayer, const QString &outputLineLayer, const QString &usedBaselineLayer, double minTransectLength = 0.0,
                       double baselineBufferDistance = -1.0, double baselineSimplificationTolerance = -1.0 );

    int createSample( QProgressDialog *pd );
%Docstring
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgstransectsample.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
