/***************************************************************************
    qgsstackeddiagram.h
    ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDDIAGRAM_H
#define QGSSTACKEDDIAGRAM_H

#include "qgsdiagram.h"

class QgsFeature;
class QPointF;
//class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;
class QgsRenderContext;


/**
 * \ingroup core
 * \class QgsStackedDiagram
 * \brief A diagram composed of several subdiagrams, located side by side.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsStackedDiagram : public QgsDiagram SIP_NODEFAULTCTORS
{
  public:
    static const QString DIAGRAM_NAME_STACKED SIP_SKIP;

    QgsStackedDiagram();

    QgsStackedDiagram *clone() const override SIP_FACTORY;

    /**
     * Calculates the position for the next subdiagram, updating the \a newPos object.
     * \param newPos out: position of the previous diagram
     * \param c           renderer context
     * \param s           stacked diagram settings
     * \param subSettings previous subdiagram settings
     */
    void subDiagramPosition( QPointF &newPos, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramSettings &subSettings );

    void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) override;

    QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) override;
    QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) override;
    double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const override;
    QString diagramName() const override;
};

#endif // QGSSTACKEDDIAGRAM_H
