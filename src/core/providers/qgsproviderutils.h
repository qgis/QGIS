/***************************************************************************
                             qgsproviderutils.h
                             ----------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
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
#ifndef QGSPROVIDERUTILS_H
#define QGSPROVIDERUTILS_H

#include "qgis_core.h"
#include "qgis.h"
#include <QList>

class QgsProviderSublayerDetails;

/**
 * \class QgsProviderUtils
 * \ingroup core
 *
 * \brief Contains utility functions for working with data providers.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderUtils
{
  public:

    /**
     * Returns TRUE if the sublayer \a details are incomplete, and require a more in-depth
     * scan.
     *
     * For instance, if the details contain any vector sublayers with unknown geometry types
     * then a query with the Qgis::SublayerQueryFlag::ResolveGeometryType flag is required.
     *
     * If \a ignoreUnknownFeatureCount is TRUE then sublayers with an unknown feature count
     * will not be considered as incomplete.
     */
    static bool sublayerDetailsAreIncomplete( const QList< QgsProviderSublayerDetails > &details, bool ignoreUnknownFeatureCount );

    /**
     * Suggests a suitable layer name given only a file \a path.
     *
     * Usually this corresponds to the base file name of \a path (e.g.
     * "rivers" for a path of "c:/my data/water/rivers.shp"). However
     * some data formats which use fixed file paths (such as aigrid files)
     * will instead return the parent directory name (e.g. "rivers" for
     * a path of "c:/my data/water/rivers/hdr.adf").
     */
    static QString suggestLayerNameFromFilePath( const QString &path );

};

#endif //QGSPROVIDERUTILS_H



