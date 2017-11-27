/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsnetworkspeedstrategy.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsNetworkSpeedStrategy : QgsNetworkStrategy
{
%Docstring
.. versionadded:: 3.0
 Strategy for calculating edge cost based on travel time. Should be
 used for finding fastest path between two points.
%End

%TypeHeaderCode
#include "qgsnetworkspeedstrategy.h"
%End
  public:

    QgsNetworkSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor );
%Docstring
 Default constructor
%End

    virtual QVariant cost( double distance, const QgsFeature &f ) const;

    virtual QSet< int > requiredAttributes() const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsnetworkspeedstrategy.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
