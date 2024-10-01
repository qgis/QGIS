/***************************************************************************
    qgsstackedbardiagram.h
    ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDBARDIAGRAM_H
#define QGSSTACKEDBARDIAGRAM_H

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
 * \class QgsStackedBarDiagram
 *
 * \brief A stacked bar chart diagram.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsStackedBarDiagram: public QgsDiagram SIP_NODEFAULTCTORS
{
  public:
    static const QString DIAGRAM_NAME_STACKED_BAR SIP_SKIP;

    QgsStackedBarDiagram();

    QgsStackedBarDiagram *clone() const override SIP_FACTORY;

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) override;

    QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) override;
    QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &interpolationSettings ) override;
    double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &interpolationSettings ) const override;
    QString diagramName() const override;

  private:
    QBrush mCategoryBrush;
    QPen   mPen;
    bool mApplySpacingAdjust = false;
};


#endif // QGSSTACKEDBARDIAGRAM_H
