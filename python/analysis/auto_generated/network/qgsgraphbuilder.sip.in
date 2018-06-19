/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphbuilder.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGraphBuilder : QgsGraphBuilderInterface
{
%Docstring
This class used for making the QgsGraph object
%End

%TypeHeaderCode
#include "qgsgraphbuilder.h"
%End
  public:

    QgsGraphBuilder( const QgsCoordinateReferenceSystem &crs, bool otfEnabled = true, double topologyTolerance = 0.0, const QString &ellipsoidID = "WGS84" );
%Docstring
Default constructor
%End

    ~QgsGraphBuilder();

    virtual void addVertex( int id, const QgsPointXY &pt );

%Docstring
MANDATORY BUILDER PROPERTY DECLARATION
%End

    virtual void addEdge( int pt1id, const QgsPointXY &pt1, int pt2id, const QgsPointXY &pt2, const QVector< QVariant > &prop );


    QgsGraph *graph() /Factory/;
%Docstring
Returns generated :py:class:`QgsGraph`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/network/qgsgraphbuilder.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
