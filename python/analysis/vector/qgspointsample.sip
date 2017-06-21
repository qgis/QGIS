/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgspointsample.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsPointSample
{
%Docstring
 Creates random points in polygons / multipolygons*
%End

%TypeHeaderCode
#include "qgspointsample.h"
%End
  public:
    QgsPointSample( QgsVectorLayer *inputLayer, const QString &outputLayer, const QString &nPointsAttribute, const QString &minDistAttribute = QString() );

    int createRandomPoints( QProgressDialog *pd );
%Docstring
 Starts calculation of random points
:return: 0 in case of success*
 :rtype: int
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/vector/qgspointsample.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
