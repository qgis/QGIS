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
  signals:
    void buildProgress( int, int ) const;
%Docstring
Emitted to report graph building progress
%End
    void buildMessage( const QString & ) const;
%Docstring
Emitted to report information about graph building
%End

  public:

    virtual ~QgsGraphDirector();

    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPointXY > &additionalPoints,
                            QVector< QgsPointXY > &snappedPoints /Out/ ) const;
%Docstring
 Make a graph using QgsGraphBuilder

 \param builder the graph builder
 \param additionalPoints list of points that should be snapped to the graph
 \param snappedPoints list of snapped points
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
 :rtype: str
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
