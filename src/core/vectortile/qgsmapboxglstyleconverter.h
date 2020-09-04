/***************************************************************************
  qgsmapboxglstyleconverter.h
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPBOXGLSTYLECONVERTER_H
#define QGSMAPBOXGLSTYLECONVERTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsproperty.h"
#include <QVariantMap>
#include <memory>

class QgsVectorTileRenderer;
class QgsVectorTileLabeling;
class QgsVectorTileBasicRendererStyle;

/**
 * \ingroup core
 * Handles conversion of MapBox GL styles to QGIS vector tile renderers and labeling
 * settings.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsMapBoxGlStyleConverter
{
  public:

    /**
     * Constructor for QgsMapBoxGlStyleConverter.
     *
     * The specified MapBox GL \a style configuration will be converted.
     */
    QgsMapBoxGlStyleConverter( const QVariantMap &style, const QString &styleName = QString() );

    //! QgsMapBoxGlStyleConverter cannot be copied
    QgsMapBoxGlStyleConverter( const QgsMapBoxGlStyleConverter &other ) = delete;
    //! QgsMapBoxGlStyleConverter cannot be copied
    QgsMapBoxGlStyleConverter &operator=( const QgsMapBoxGlStyleConverter &other ) = delete;

    ~QgsMapBoxGlStyleConverter();

    /**
     * Returns a descriptive error message if an error was encountered during the style conversion,
     * or an empty string if no error was encountered.
     */
    QString errorMessage() const { return mError; }

    /**
     * Returns a new instance of a vector tile renderer representing the converted style,
     * or NULLPTR if the style could not be converted successfully.
     */
    QgsVectorTileRenderer *renderer() const SIP_FACTORY;

    /**
     * Returns a new instance of a vector tile labeling representing the converted style,
     * or NULLPTR if the style could not be converted successfully.
     */
    QgsVectorTileLabeling *labeling() const SIP_FACTORY;

  protected:

    /**
     * Parse list of \a layers from JSON
     */
    void parseLayers( const QVariantList &layers, const QString &styleName );

    /**
     * Parses a fill layer.
     *
     * \param jsonLayer fill layer to parse
     * \param styleName style name
     * \param style generated QGIS vector tile style
     * \returns TRUE if the layer was successfully parsed.
     */
    static bool parseFillLayer( const QVariantMap &jsonLayer, const QString &styleName, QgsVectorTileBasicRendererStyle &style SIP_OUT );

    static QgsProperty parseInterpolateColorByZoom( const QVariantMap &json );

    /**
     * Parses a \a color in one of these supported formats:
     *
     * - #fff or #ffffff
     * - hsl(30, 19%, 90%) or hsla(30, 19%, 90%, 0.4)
     * - rgb(10, 20, 30) or rgba(10, 20, 30, 0.5)
     *
     * Returns an invalid color if the color could not be parsed.
     */
    static QColor parseColor( const QVariant &color );

    /**
     * Takes a QColor object and returns HSLA components in required format for QGIS color_hsla() expression function.
     * \param color input color
     * \param hue an integer value from 0 to 360
     * \param saturation an integer value from 0 to 100
     * \param lightness an integer value from 0 to 100
     * \param alpha an integer value from 0 (completely transparent) to 255 (opaque).
     */
    static void colorAsHslaComponents( const QColor &color, int &hue, int &saturation, int &lightness, int &alpha );

    /**
     * Generates an interpolation for values between \a valueMin and \a valueMax, scaled between the
     * ranges \a zoomMin to \a zoomMax.
     */
    static QString interpolateExpression( int zoomMin, int zoomMax, double valueMin, double valueMax, double base );

  private:

#ifdef SIP_RUN
    QgsMapBoxGlStyleConverter( const QgsMapBoxGlStyleConverter &other );
#endif


    QVariantMap mStyle;
    QString mError;

    std::unique_ptr< QgsVectorTileRenderer > mRenderer;
    std::unique_ptr< QgsVectorTileLabeling > mLabeling;

};

#endif // QGSMAPBOXGLSTYLECONVERTER_H
