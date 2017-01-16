/***************************************************************************
                         qgsprocessingprovider.h
                         ------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPROCESSINGPROVIDER_H
#define QGSPROCESSINGPROVIDER_H

#include "qgis_core.h"
#include <QIcon>

/**
 * \class QgsProcessingProvider
 * \ingroup core
 * Abstract base class for processing providers. An algorithm provider is a set of
 * related algorithms, typically from the same external application or related
 * to a common area of analysis.
 * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsProcessingProvider
{

  public:

    /**
     * Constructor for QgsProcessingProvider.
     */
    QgsProcessingProvider() = default;

    virtual ~QgsProcessingProvider() = default;

    //! Providers cannot be copied
    QgsProcessingProvider( const QgsProcessingProvider& other ) = delete;
    //! Providers cannot be copied
    QgsProcessingProvider& operator=( const QgsProcessingProvider& other ) = delete;

    /**
     * Returns an icon for the provider.
     * @see svgIcon()
     */
    virtual QIcon icon() const;

    /**
     * Returns a path to an SVG version of the provider's icon.
     * @see icon()
     */
    virtual QString svgIconPath() const;

    /**
     * Returns the unique provider id, used for identifying the provider. This string
     * should be a unique, short, character only string, eg "qgis" or "gdal". This
     * string should not be localised.
     * @see name()
     */
    virtual QString id() const = 0;

    /**
     * Returns the full provider name, which is used to describe the provider within the GUI.
     * This string should be localised.
     * @see id()
     */
    virtual QString name() const = 0;

    /**
     * Returns true if the provider can be activated, or false if it cannot be activated (e.g. due to
     * missing external dependencies).
     */
    virtual bool canBeActivated() const { return true; }

};

#endif // QGSPROCESSINGPROVIDER_H


