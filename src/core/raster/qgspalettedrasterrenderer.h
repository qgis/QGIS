/***************************************************************************
                         qgspalettedrasterrenderer.h
                         ---------------------------
    begin                : December 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPALETTEDRASTERRENDERER_H
#define QGSPALETTEDRASTERRENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QVector>

#include "qgsrasterrenderer.h"
#include "qgscolorrampshader.h"

class QColor;
class QDomElement;
class QgsRasterAttributeTable;

/**
 * \ingroup core
 * \brief Renderer for paletted raster images.
*/
class CORE_EXPORT QgsPalettedRasterRenderer: public QgsRasterRenderer
{
  public:

    //! Properties of a single value class
    struct CORE_EXPORT Class
    {
      //! Constructor for Class
      Class( double value, const QColor &color = QColor(), const QString &label = QString() )
        : value( value )
        , color( color )
        , label( label )
      {}

      //! Value
      double value;

      //! Color to render value
      QColor color;
      //! Label for value
      QString label;
    };

    /**
     * \ingroup core
     * \brief Properties of a multi value class: a class that contains multiple values.
     * \since QGIS 3.30
     */
    class CORE_EXPORT MultiValueClass
    {

      public:

        //! Constructor for MultiValueClass from a list of values
        MultiValueClass( const QVector< QVariant > &values, const QColor &color = QColor(), const QString &label = QString() );

        //! Values
        QVector< QVariant > values;

        //! Color to render values
        QColor color;

        //! Label for values
        QString label;
    };


    //! Map of value to class properties
    typedef QList< QgsPalettedRasterRenderer::Class > ClassData;

    //! Map of multi value to class properties
    typedef QList< QgsPalettedRasterRenderer::MultiValueClass > MultiValueClassData;

    /**
     * Constructor for QgsPalettedRasterRenderer.
     */
    QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const ClassData &classes );

    /**
     * Constructor for QgsPalettedRasterRenderer from multi value classes.
     * \since QGIS 3.30
     */
    QgsPalettedRasterRenderer( QgsRasterInterface *input, int bandNumber, const MultiValueClassData &classes ) SIP_SKIP;

    //! QgsPalettedRasterRenderer cannot be copied. Use clone() instead.
    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer & ) = delete;
    //! QgsPalettedRasterRenderer cannot be copied. Use clone() instead.
    const QgsPalettedRasterRenderer &operator=( const QgsPalettedRasterRenderer & ) = delete;

    QgsPalettedRasterRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    //! Returns number of colors
    int nColors() const;

    /**
     * Returns a map of value to classes (colors) used by the renderer.
     */
    ClassData classes() const;

    /**
     * Returns a map of multi value to classes (colors) used by the renderer.
     * \since QGIS 3.30
     */
    MultiValueClassData multiValueClasses( ) const;

    bool canCreateRasterAttributeTable( ) const override;

    /**
     * Sets the multi value classes to \a setMultiValueClasses.
     * \since QGIS 3.30
     */
    void setMultiValueClasses( const MultiValueClassData &classes );

    /**
     * Returns optional category label
     * \since QGIS 2.1
    */
    QString label( double idx ) const;

    /**
     * Set category label
     * \since QGIS 2.1
    */
    void setLabel( double idx, const QString &label );

    /**
     * Returns the raster band used for rendering the raster.
     * \since QGIS 3.0
     */
    int band() const { return mBand; }

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;
    QList< QPair< QString, QColor > > legendSymbologyItems() const override;
    QList<QgsLayerTreeModelLegendNode *> createLegendNodes( QgsLayerTreeLayer *nodeLayer ) SIP_FACTORY override;
    QList<int> usesBands() const override;
    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;
    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

    /**
     * Set the source color \a ramp. Ownership is transferred to the renderer.
     * \see sourceColorRamp()
     * \since QGIS 3.0
     */
    void setSourceColorRamp( QgsColorRamp *ramp SIP_TRANSFER );

    /**
     * Gets the source color ramp
     * \see setSourceColorRamp()
     * \since QGIS 3.0
     */
    QgsColorRamp *sourceColorRamp() const;

    /**
     * Converts a raster color \a table to paletted renderer class data.
     * \since QGIS 3.0
     */
    static QgsPalettedRasterRenderer::ClassData colorTableToClassData( const QList<QgsColorRampShader::ColorRampItem> &table );

    /**
     * Reads and returns classes from the Raster Attribute Table \a attributeTable, optionally classifying the attribute table
     * by \a classificationColumn and setting the colors from \a ramp.
     * The default value of -1 for the classificationColumn uses the first available value column.
     *
     * \note The method will return an empty list of classes in case the Raster Attribute Table is not thematic.
     * \since QGIS 3.30
     */
    static QgsPalettedRasterRenderer::MultiValueClassData rasterAttributeTableToClassData( const QgsRasterAttributeTable *attributeTable, int classificationColumn = -1, QgsColorRamp *ramp = nullptr );

    /**
     * Converts a \a string containing a color table or class data to to paletted renderer class data.
     *
     * \see classDataFromFile()
     * \see classDataToString()
     * \since QGIS 3.0
     */
    static QgsPalettedRasterRenderer::ClassData classDataFromString( const QString &string );

    /**
     * Opens a color table file and returns corresponding paletted renderer class data.
     * \see classDataFromString()
     * \since QGIS 3.0
     */
    static QgsPalettedRasterRenderer::ClassData classDataFromFile( const QString &path );

    /**
     * Converts classes to a string representation, using the .clr/gdal color table file format.
     * \see classDataFromString()
     * \since QGIS 3.0
     */
    static QString classDataToString( const QgsPalettedRasterRenderer::ClassData &classes );

    /**
     * Generates class data from a \a raster, for the specified \a bandNumber. An optional
     * color \a ramp can be specified to automatically assign colors from the ramp.
     * \since QGIS 3.0
     */
    static QgsPalettedRasterRenderer::ClassData classDataFromRaster( QgsRasterInterface *raster, int bandNumber, QgsColorRamp *ramp = nullptr,
        QgsRasterBlockFeedback *feedback = nullptr );


  private:
#ifdef SIP_RUN
    QgsPalettedRasterRenderer( const QgsPalettedRasterRenderer & );
    const QgsPalettedRasterRenderer &operator=( const QgsPalettedRasterRenderer & );
#endif


    int mBand;
    MultiValueClassData mMultiValueClassData;

    ClassData classData() const;

    //! Source color ramp
    std::unique_ptr<QgsColorRamp> mSourceColorRamp;

    //! Premultiplied color map
    QMap< double, QRgb > mColors;
    void updateArrays();

    // Maximum number of allowed classes for float rasters
    static const int MAX_FLOAT_CLASSES;
};

#endif // QGSPALETTEDRASTERRENDERER_H
