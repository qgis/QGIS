/***************************************************************************
                             qgscoordinatereferencesystemregistry.h
                             -------------------
    begin                : January 2021
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

#ifndef QGSCOORDINATEREFERENCESYSTEMREGISTRY_H
#define QGSCOORDINATEREFERENCESYSTEMREGISTRY_H

#include <QObject>
#include <QMap>
#include "qgscoordinatereferencesystem.h"

class QgsCelestialBody;
class QgsProjOperation;

/**
 * \class QgsCoordinateReferenceSystemRegistry
 * \ingroup core
 * \brief A registry for known coordinate reference system (CRS) definitions, including
 * any user-defined CRSes.
 *
 * QgsCoordinateReferenceSystemRegistry is not usually directly created, but rather accessed through
 * QgsApplication::coordinateReferenceSystemRegistry().
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsCoordinateReferenceSystemRegistry : public QObject
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsCoordinateReferenceSystemRegistry, with the specified \a parent object.
     */
    explicit QgsCoordinateReferenceSystemRegistry( QObject *parent = nullptr );

    ~QgsCoordinateReferenceSystemRegistry();

    /**
     * \brief Contains details of a custom (user defined) CRS.
     * \ingroup core
     * \since QGIS 3.18
     */
    class UserCrsDetails
    {
      public:

        //! CRS ID
        long id = -1;

        //! CRS name (or description)
        QString name;

        //! PROJ string definition of CRS
        QString proj;

        /**
         * WKT definition of CRS. This will be empty for custom CRSes
         * which were defined using a PROJ string only.
         */
        QString wkt;

        //! QgsCoordinateReferenceSystem object representing the user-defined CRS.
        QgsCoordinateReferenceSystem crs;
    };

    /**
     * Returns a list containing the details of all registered
     * custom (user-defined) CRSes.
     */
    QList< QgsCoordinateReferenceSystemRegistry::UserCrsDetails > userCrsList() const;

    /**
     * Adds a new \a crs definition as a custom ("USER") CRS.
     *
     * Returns the new CRS srsid(), or -1 if the CRS could not be saved.
     *
     * The \a nativeFormat argument specifies the format to use when saving the CRS
     * definition. FormatWkt is recommended as it is a lossless format.
     *
     * \warning Not all CRS definitions can be represented as a Proj string, so
     * take care when using the FormatProj option.
     *
     * \see updateUserCrs()
     * \see userCrsAdded()
     */
    long addUserCrs( const QgsCoordinateReferenceSystem &crs, const QString &name, Qgis::CrsDefinitionFormat nativeFormat = Qgis::CrsDefinitionFormat::Wkt );

    /**
     * Updates the definition of the existing user CRS with matching \a id.
     *
     * The \a crs argument specifies a CRS with the desired new definition.
     *
     * Returns FALSE if the new CRS definition could not be saved.
     *
     * The \a nativeFormat argument specifies the format to use when saving the CRS
     * definition. FormatWkt is recommended as it is a lossless format.
     *
     * \warning Not all CRS definitions can be represented as a Proj string, so
     * take care when using the FormatProj option.
     *
     * \see addUserCrs()
     * \see userCrsChanged()
     */
    bool updateUserCrs( long id, const QgsCoordinateReferenceSystem &crs, const QString &name, Qgis::CrsDefinitionFormat nativeFormat = Qgis::CrsDefinitionFormat::Wkt );

    /**
     * Removes the existing user CRS with matching \a id.
     *
     * Returns FALSE if the CRS could not be removed.
     *
     * \see userCrsRemoved()
     */
    bool removeUserCrs( long id );

    /**
     * Returns a map of all valid PROJ operations.
     *
     * The map keys correspond to PROJ operation IDs.
     *
     * \since QGIS 3.20
     */
    QMap< QString, QgsProjOperation > projOperations() const;

    /**
     * Returns a list of all known celestial bodies.
     *
     * \warning This method requires PROJ 8.1 or later
     *
     * \throws QgsNotSupportedException on QGIS builds based on PROJ 8.0 or earlier.
     *
     * \since QGIS 3.20
     */
    QList< QgsCelestialBody > celestialBodies() const;

  signals:

    /**
     * Emitted whenever an existing user CRS definition is changed.
     *
     * The \a id argument specifies the ID of the user CRS which has been changed.
     *
     * Objects which store QgsCoordinateReferenceSystem members should connect to this signal
     * and update any stored CRS definitions to ensure that they always use the current
     * definition of any user defined CRSes.
     *
     * \see crsDefinitionsChanged()
     * \see userCrsAdded()
     */
    void userCrsChanged( const QString &id );

    /**
     * Emitted whenever a new user CRS definition is added.
     *
     * The \a id argument specifies the ID of the user CRS which has been changed.
     *
     * \see userCrsChanged()
     * \see crsDefinitionsChanged()
     */
    void userCrsAdded( const QString &id );

    /**
     * Emitted when the user CRS with matching \a id is removed
     * from the database.
     *
     * \see removeUserCrs()
     */
    void userCrsRemoved( long id );

    /**
     * Emitted whenever an operation has caused any of the known CRS definitions (including
     * user-defined CRS) to change.
     */
    void crsDefinitionsChanged();

  private:

    bool insertProjection( const QString &projectionAcronym );

    mutable QList< QgsCelestialBody > mCelestialBodies;
    mutable QMap< QString, QgsProjOperation > mProjOperations;

};


#endif // QGSCOORDINATEREFERENCESYSTEMREGISTRY_H
