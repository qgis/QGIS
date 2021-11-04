/***************************************************************************
                         qgslegendpatchshape.h
                         -------------------
begin                : April 2020
copyright            : (C) 2020 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLEGENDPATCHSHAPE_H
#define QGSLEGENDPATCHSHAPE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsgeometry.h"

class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief Represents a patch shape for use in map legends.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsLegendPatchShape
{
  public:

    /**
     * Constructor for a null QgsLegendPatchShape.
     *
     * A null QgsLegendPatchShape indicates that the default legend patch shape
     * should be used instead.
     */
    QgsLegendPatchShape() = default;

    /**
     * Constructor for QgsLegendPatchShape.
     *
     * The \a type argument specifies the symbol type associated with this patch.
     *
     * The \a geometry argument gives the shape of the patch to render. See setGeometry()
     * for further details on the geometry requirements.
     *
     * If \a preserveAspectRatio is TRUE, then the patch shape should preserve its aspect ratio when
     * it is resized to fit a desired legend patch size.
     */
    QgsLegendPatchShape( Qgis::SymbolType type,
                         const QgsGeometry &geometry,
                         bool preserveAspectRatio = true );

    /**
     * Returns TRUE if the patch shape is a null QgsLegendPatchShape,
     * which indicates that the default legend patch shape should be used instead.
     */
    bool isNull() const;

    /**
     * Returns the symbol type associated with this patch.
     *
     * \see setSymbolType()
     */
    Qgis::SymbolType symbolType() const;

    /**
     * Sets the symbol \a type associated with this patch.
     *
     * \see symbolType()
     */
    void setSymbolType( Qgis::SymbolType type );

    /**
     * Returns the geometry for the patch shape.
     *
     * \see setGeometry()
     */
    QgsGeometry geometry() const;

    /**
     * Sets the \a geometry for the patch shape.
     *
     * The origin and size of the \a geometry is not important, as the legend
     * renderer will automatically scale and transform the geometry to match
     * the desired overall patch bounds.
     *
     * Geometries for legend patches are rendered respecting the traditional
     * "y values increase toward the top of the map" convention, as opposed
     * to the standard computer graphics convention of "y values increase toward
     * the bottom of the display".
     *
     * \warning The geometry type should match the patch shape's symbolType(),
     * e.g. a fill symbol type should only have Polygon or MultiPolygon geometries
     * set, while a line symbol type must have LineString or MultiLineString geometries.
     *
     * \see geometry()
     */
    void setGeometry( const QgsGeometry &geometry );

    /**
     * Returns TRUE if the patch shape should preserve its aspect ratio when
     * it is resized to fit a desired legend patch size.
     *
     * \see setPreserveAspectRatio()
     */
    bool preserveAspectRatio() const;

    /**
     * Sets whether the patch shape should \a preserve its aspect ratio when
     * it is resized to fit a desired legend patch size.
     *
     * The default behavior is to respect the geometry()'s aspect ratio.
     *
     * \see setPreserveAspectRatio()
     */
    void setPreserveAspectRatio( bool preserve );

    /**
     * Returns TRUE if the patch shape should by resized to the desired target size
     * when rendering.
     *
     * Resizing to the target size is the default behavior.
     *
     * \see setScaleToOutputSize()
     * \since QGIS 3.22
     */
    bool scaleToOutputSize() const;

    /**
     * Sets whether the patch shape should by resized to the desired target size
     * when rendering.
     *
     * Resizing to the target size is the default behavior.
     *
     * \see scaleToOutputSize()
     * \since QGIS 3.22
     */
    void setScaleToOutputSize( bool scale );

    /**
     * Returns the patch shape's geometry, scaled to the given size.
     *
     * Note that if scaleToOutputSize() is FALSE then no scaling will be applied.
     *
     * \since QGIS 3.22
     */
    QgsGeometry scaledGeometry( QSizeF size ) const;

    /**
     * Converts the patch shape to a set of QPolygonF objects representing
     * how the patch should be drawn for a symbol of the given \a type at the specified \a size (as
     * geometry parts and rings).
     */
    QList< QList< QPolygonF > > toQPolygonF( Qgis::SymbolType type, QSizeF size ) const;

    /**
     * Read settings from a DOM \a element.
     * \see writeXml()
     */
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Write settings into a DOM \a element.
     * \see readXml()
     */
    void writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const;

  private:
    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Fill;
    QgsGeometry mGeometry;
    bool mPreserveAspectRatio = true;
    bool mScaleToTargetSize = true;

};

#endif // QGSLEGENDPATCHSHAPE_H
