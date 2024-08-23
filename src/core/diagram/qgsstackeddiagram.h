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

#define DIAGRAM_NAME_STACKED "Stacked"

#include "qgsdiagram.h"
#include <QPen>
#include <QBrush>

class QgsFeature;
class QPointF;
class QgsDiagramSettings;
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

    struct DiagramData
    {
      QgsDiagram *diagram = nullptr;
      QgsDiagramSettings *settings = nullptr;
    };

    QgsStackedDiagram();

    QgsStackedDiagram *clone() const override SIP_FACTORY;

    /**
     * Adds a subdiagram to the stacked diagram object along with its corresponding settings.
     * \param diagram subdiagram to be added to the stacked diagram
     * \param s       subdiagram settings
     * Subdiagrams added first will appear more to the left (if stacked diagram is horizontal),
     * or more to the top (if stacked diagram is vertical).
     */
    void addSubDiagram( QgsDiagram *diagram, QgsDiagramSettings *s );

    /**
     * Returns an ordered list with the subdiagrams of the stacked diagram object.
     * If the stacked diagram orientation is vertical, the list is returned backwards.
     * \param s stacked diagram settings
     */
    QList< QgsDiagram * > subDiagrams( const QgsDiagramSettings &s ) const;

    //! Returns the settings associated to the \a diagram.
    QgsDiagramSettings *subDiagramSettings( const QgsDiagram *diagram ) const;

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

  private:
    QBrush mCategoryBrush;
    QPen   mPen;
    double mScaleFactor;
    QList< DiagramData > mSubDiagrams;
};

#endif // QGSSTACKEDDIAGRAM_H
