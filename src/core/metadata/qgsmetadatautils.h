/***************************************************************************
                             qgsmetadatautils.h
                             -------------------
    begin                : April 2021
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

#ifndef QGSMETADATAUTILS_H
#define QGSMETADATAUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"

class QgsLayerMetadata;
class QDomDocument;

/**
 * \ingroup core
 * \class QgsMetadataUtils
 * \brief Contains utility functions for working with metadata.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsMetadataUtils
{
  public:

    /**
     * Converts ESRI layer metadata to QgsLayerMetadata.
     */
    static QgsLayerMetadata convertFromEsri( const QDomDocument &document );

};

#endif // QGSMETADATAUTILS_H
