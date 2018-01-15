/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/diagram/qgshistogramdiagram.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsHistogramDiagram: QgsDiagram
{

%TypeHeaderCode
#include "qgshistogramdiagram.h"
%End
  public:
    QgsHistogramDiagram();

    virtual QgsHistogramDiagram *clone() const /Factory/;


    virtual void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position );


    virtual QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s );

    virtual QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is );

    virtual double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const;

    virtual QString diagramName() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/diagram/qgshistogramdiagram.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
