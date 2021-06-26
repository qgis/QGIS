/***************************************************************************
                         qgslocatorcontext.h
                         ------------------
    begin                : May 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCATORCONTEXT_H
#define QGSLOCATORCONTEXT_H

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"

/**
 * \class QgsLocatorContext
 * \ingroup core
 * Encapsulates the properties relating to the context of a locator search.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLocatorContext
{
  public:

    /**
     * Constructor for QgsLocatorContext.
     */
    QgsLocatorContext() = default;

    /**
     * Map extent to target in results. This can be used to prioritize searching
     * for results close to the current map extent. The CRS for the extent
     * is specified by targetExtentCrs.
     * \see targetExtentCrs
     */
    QgsRectangle targetExtent;

    /**
     * Coordinate reference system for the map extent variable.
     * \see targetExtent
     */
    QgsCoordinateReferenceSystem targetExtentCrs;

    /**
     * Coordinate transform context, to use whenever performing coordinate transformations inside
     * a locator.
     *
     * \since QGIS 3.16
     */
    QgsCoordinateTransformContext transformContext;

    /**
     * Will be TRUE if search is being conducted using a filter prefix.
     */
    bool usingPrefix = false;

};

#endif // QGSLOCATORCONTEXT_H


