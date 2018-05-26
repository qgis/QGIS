/***************************************************************************
    qgsdiagram.h
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
#ifndef QGSDIAGRAM_H
#define QGSDIAGRAM_H

#include "qgis_core.h"
#include "qgis.h"
#include <QPen>
#include <QBrush>
#include "qgsexpression.h" //for QMap with QgsExpression

class QPainter;
class QPointF;
class QgsDiagramSettings;
class QgsDiagramInterpolationSettings;
class QgsFeature;
class QgsRenderContext;
class QgsExpressionContext;
class QgsFields;
class QgsAttributes;


/**
 * \ingroup core
 * Base class for all diagram types*/
class CORE_EXPORT QgsDiagram
{
  public:

    virtual ~QgsDiagram() { clearCache(); }

    /**
     * Returns an instance that is equivalent to this one
     * \since QGIS 2.4 */
    virtual QgsDiagram *clone() const = 0 SIP_FACTORY;

    void clearCache();

    /**
     * Returns a prepared expression for the specified context.
     * \param expression expression string
     * \param context expression context
     * \since QGIS 2.12
     */
    QgsExpression *getExpression( const QString &expression, const QgsExpressionContext &context );

    //! Draws the diagram at the given position (in pixel coordinates)
    virtual void renderDiagram( const QgsFeature &feature, QgsRenderContext &c, const QgsDiagramSettings &s, QPointF position ) = 0;

    /**
     * Gets a descriptive name for this diagram type.
     */
    virtual QString diagramName() const = 0;
    //! Returns the size in map units the diagram will use to render.
    virtual QSizeF diagramSize( const QgsAttributes &attributes, const QgsRenderContext &c, const QgsDiagramSettings &s ) = 0;
    //! Returns the size in map units the diagram will use to render. Interpolate size
    virtual QSizeF diagramSize( const QgsFeature &feature, const QgsRenderContext &c, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) = 0;

    /**
     * Returns the size of the legend item for the diagram corresponding to a specified value.
     * \param value value to return legend item size for
     * \param s diagram settings
     * \param is interpolation settings
     * \since QGIS 2.16
     */
    virtual double legendSize( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const = 0;

  protected:

    /**
     * Constructor for QgsDiagram.
     */
    QgsDiagram() = default;
    QgsDiagram( const QgsDiagram &other );

    /**
     * Changes the pen width to match the current settings and rendering context
     *  \param pen The pen to modify
     *  \param s   The settings that specify the pen width
     *  \param c   The rendering specifying the proper scale units for pixel conversion
     */
    void setPenWidth( QPen &pen, const QgsDiagramSettings &s, const QgsRenderContext &c );

    /**
     * Calculates a size to match the current settings and rendering context
     *  \param size The size to convert
     *  \param s    The settings that specify the size type
     *  \param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  \returns The converted size for rendering
     */
    QSizeF sizePainterUnits( QSizeF size, const QgsDiagramSettings &s, const QgsRenderContext &c );

    /**
     * Calculates a length to match the current settings and rendering context
     *  \param l    The length to convert
     *  \param s    Unused
     *  \param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  \returns The converted length for rendering
     */
    double sizePainterUnits( double l, const QgsDiagramSettings &s, const QgsRenderContext &c );

    /**
     * Calculates a size to match the current settings and rendering context
     *  \param s    The settings that contain the font size and size type
     *  \param c    The rendering specifying the proper scale units for pixel conversion
     *
     *  \returns The properly scaled font for rendering
     */
    QFont scaledFont( const QgsDiagramSettings &s, const QgsRenderContext &c );

    /**
     * Returns the scaled size of a diagram for a value, respecting the specified diagram interpolation settings.
     * \param value value to calculate corresponding circular size for
     * \param s diagram settings
     * \param is interpolation settings
     * \since QGIS 2.16
     */
    QSizeF sizeForValue( double value, const QgsDiagramSettings &s, const QgsDiagramInterpolationSettings &is ) const;

  private:
    QMap<QString, QgsExpression *> mExpressions;
};

#endif // QGSDIAGRAM_H
