/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphdirector.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
#include <qgsvectorlayerdirector.h>
%End

class QgsGraphDirector : QObject
{
%Docstring
Determine making the graph. QgsGraphBuilder and QgsGraphDirector implemented
using "builder" design patter.
%End

%TypeHeaderCode
#include "qgsgraphdirector.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast< QgsVectorLayerDirector * >( sipCpp ) != NULL )
      sipType = sipType_QgsVectorLayerDirector;
    else
      sipType = NULL;
%End
  public:

    ~QgsGraphDirector();

    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPointXY > &additionalPoints,
                            QVector< QgsPointXY > &snappedPoints /Out/,
                            QgsFeedback *feedback = 0 ) const;
%Docstring
Make a graph using :py:class:`QgsGraphBuilder`

:param builder: the graph builder
:param additionalPoints: list of points that should be snapped to the graph
:param snappedPoints: list of snapped points
:param feedback: feedback object for reporting progress

.. note::

   if snappedPoints[i] == QgsPointXY(0.0,0.0) then snapping failed.
%End

    void addStrategy( QgsNetworkStrategy *prop /Transfer/ );
%Docstring
Add optimization strategy
%End

    virtual QString name() const = 0;
%Docstring
Returns director name
%End

  protected:
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphdirector.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
