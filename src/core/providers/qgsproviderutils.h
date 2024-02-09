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
     * Flags which control how QgsProviderUtils::sublayerDetailsAreIncomplete() tests for completeness.
     */
    enum class SublayerCompletenessFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      IgnoreUnknownFeatureCount = 1 << 0, //!< Indicates that an unknown feature count should not be considered as incomplete
      IgnoreUnknownGeometryType = 1 << 1, //!< Indicates that an unknown geometry type should not be considered as incomplete
    };
    Q_DECLARE_FLAGS( SublayerCompletenessFlags, SublayerCompletenessFlag )

    /**
     * Returns TRUE if the sublayer \a details are incomplete, and require a more in-depth
     * scan.
     *
     * For instance, if the details contain any vector sublayers with unknown geometry types
     * then a query with the Qgis::SublayerQueryFlag::ResolveGeometryType flag is required.
     *
     * The \a flags argument can be used to control the level of completeness required during the test.
     */
    static bool sublayerDetailsAreIncomplete( const QList< QgsProviderSublayerDetails > &details, QgsProviderUtils::SublayerCompletenessFlags flags = QgsProviderUtils::SublayerCompletenessFlags() );

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

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProviderUtils::SublayerCompletenessFlags )

#endif //QGSPROVIDERUTILS_H



