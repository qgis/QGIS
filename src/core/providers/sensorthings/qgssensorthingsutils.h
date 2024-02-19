/***************************************************************************
    qgssensorthingsutils.h
    --------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORTHINGSUTILS_H
#define QGSSENSORTHINGSUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"

class QgsFields;
class QgsFeedback;

/**
 * \ingroup core
 * \brief Utility functions for working with OGC SensorThings API services.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSensorThingsUtils
{

  public:

    //! Default page size
    static constexpr int DEFAULT_PAGE_SIZE = 200; SIP_SKIP

    /**
     * Converts a string value to a Qgis::SensorThingsEntity type.
     *
     * Returns Qgis::SensorThingsEntity::Invalid if the string could not be converted to a known entity type.
     */
    static Qgis::SensorThingsEntity stringToEntity( const QString &type );

    /**
     * Converts a Qgis::SensorThingsEntity \a type to a user-friendly translated string.
     *
     * If \a plural is TRUE then a plural string is returned (ie "Things" instead of "Thing").
    */
    static QString displayString( Qgis::SensorThingsEntity type, bool plural = false );

    /**
    * Converts a string value corresponding to a SensorThings entity set to a Qgis::SensorThingsEntity type.
    *
    * Returns Qgis::SensorThingsEntity::Invalid if the string could not be converted to a known entity set type.
    */
    static Qgis::SensorThingsEntity entitySetStringToEntity( const QString &type );

    /**
     * Returns the fields which correspond to a specified entity \a type.
     */
    static QgsFields fieldsForEntityType( Qgis::SensorThingsEntity type );

    /**
     * Returns the geometry field for a specified entity \a type.
     */
    static QString geometryFieldForEntityType( Qgis::SensorThingsEntity type );

    /**
     * Returns TRUE if the specified entity \a type can have geometry attached.
     */
    static bool entityTypeHasGeometry( Qgis::SensorThingsEntity type );

    /**
     * Returns a filter string which restricts results to those matching the specified
     * \a entityType and \a wkbType.
     */
    static QString filterForWkbType( Qgis::SensorThingsEntity entityType, Qgis::WkbType wkbType );

    /**
     * Returns a list of available geometry types for the server at the specified \a uri
     * and entity \a type.
     *
     * This method will block while network requests are made to the server.
     */
    static QList< Qgis::GeometryType > availableGeometryTypes( const QString &uri, Qgis::SensorThingsEntity type, QgsFeedback *feedback = nullptr, const QString &authCfg = QString() );

};

#endif // QGSSENSORTHINGSUTILS_H
