/***************************************************************************
    qgspiediagram.h
    ---------------------
    begin                : March 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPIEDIAGRAM_H
#define QGSPIEDIAGRAM_H

#define DIAGRAM_NAME_PIE "Pie"

#include "qgis_core.h"
#include "qgis.h"
#include "qgsdiagram.h"
#include <QPen>
#include <QBrush>

class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;
class QgsFeature;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsPieDiagram
 * \brief A pie chart diagram.
 */
class CORE_EXPORT QgsPieDiagram: public QgsDiagram SIP_NODEFAULTCTORS
{
  public:
    QgsPieDiagram();

    QgsPieDiagram *clone() const override SIP_FACTORY;

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) override;

    QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) override;
    QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &interpolationSettings ) override;
    double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &interpolationSettings ) const override;
    QString diagramName() const override;

  private:
    QBrush mCategoryBrush;
    QPen mPen;

};

#endif // QGSPIEDIAGRAM_H
