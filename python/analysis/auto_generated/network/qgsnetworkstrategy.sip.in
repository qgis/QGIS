/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsnetworkstrategy.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




%ModuleHeaderCode
#include <qgsnetworkspeedstrategy.h>
#include <qgsnetworkdistancestrategy.h>
%End


class QgsNetworkStrategy
{
%Docstring
QgsNetworkStrategy defines strategy used for calculation of the edge cost. For example it can
take into account travel distance, amount of time or money. Currently there are two strategies
implemented in the analysis library: QgsNetworkDistanceStrategy and QgsNetworkSpeedStrategy.
QgsNetworkStrategy implemented using "strategy" design pattern.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsnetworkstrategy.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast< QgsNetworkDistanceStrategy * >( sipCpp ) != NULL )
      sipType = sipType_QgsNetworkDistanceStrategy;
    else if ( dynamic_cast< QgsNetworkSpeedStrategy * >( sipCpp ) != NULL )
      sipType = sipType_QgsNetworkSpeedStrategy;
    else
      sipType = NULL;
%End
  public:

    QgsNetworkStrategy();
%Docstring
Default constructor
%End

    virtual ~QgsNetworkStrategy();

    virtual QSet< int > requiredAttributes() const;
%Docstring
Returns a list of the source layer attributes needed for cost calculation.
This is method called by :py:class:`QgsGraphDirector`.
%End

    virtual QVariant cost( double distance, const QgsFeature &f ) const = 0;
%Docstring
Returns edge cost
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsnetworkstrategy.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
