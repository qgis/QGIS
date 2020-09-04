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
#include <QVariantMap>
#include <memory>

class QgsVectorTileRenderer;
class QgsVectorTileLabeling;

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
    QgsMapBoxGlStyleConverter( const QVariantMap &style );

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

    void parseLayers( const QVariantList &layers );

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
