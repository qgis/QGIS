/***************************************************************************
    qgshistogramdiagram.h
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHISTOGRAMDIAGRAM_H
#define QGSHISTOGRAMDIAGRAM_H

#define DIAGRAM_NAME_HISTOGRAM "Histogram"

#include "qgis_core.h"
#include "qgis.h"
#include "qgsdiagram.h"
#include <QPen>
#include <QBrush>

class QgsFeature;
class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;

class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsHistogramDiagram
 */
class CORE_EXPORT QgsHistogramDiagram: public QgsDiagram SIP_NODEFAULTCTORS
{
  public:
    QgsHistogramDiagram();

    QgsHistogramDiagram *clone() const override SIP_FACTORY;

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) override;

    QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) override;
    QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) override;
    double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const override;
    QString diagramName() const override;

  private:
    QBrush mCategoryBrush;
    QPen   mPen;
    double mScaleFactor;
};


#endif // QGSHISTOGRAMDIAGRAM_H
