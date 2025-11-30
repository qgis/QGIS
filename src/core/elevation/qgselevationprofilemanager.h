/***************************************************************************
    qgselevationprofilemanager.h
    ------------------
    Date                 : July 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILEMANAGER_H
#define QGSELEVATIONPROFILEMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsprojectstoredobjectmanager.h"

#include <QObject>

class QDomElement;
class QDomDocument;
class QgsReadWriteContext;
class QgsElevationProfile;

/**
 * \ingroup core
 * \class QgsElevationProfileManager
 *
 * \brief Manages storage of a set of elevation profiles.
 *
 * QgsElevationProfileManager handles the storage, serializing and deserializing
 * of elevation profiles. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::elevationProfileManager().
 *
 * QgsElevationProfileManager retains ownership of all the profiles contained
 * in the manager.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsElevationProfileManager : public QgsProjectStoredObjectManagerBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsElevationProfileManager : public QgsAbstractProjectStoredObjectManager< QgsElevationProfile >
{
#endif
    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfileManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsElevationProfileManager( QgsProject *project SIP_TRANSFERTHIS = nullptr );

    ~QgsElevationProfileManager() override;

    /**
     * Adds a \a profile to the manager.
     *
     * Ownership of the profile is transferred to the manager.
     *
     * Returns TRUE if the addition was successful, or FALSE if the profile could not be added (eg
     * as a result of a duplicate profile name).
     *
     * \see removeProfile()
     * \see profileAdded()
     */
    bool addProfile( QgsElevationProfile *profile SIP_TRANSFER );

    /**
     * Removes a \a profile from the manager. The profile is deleted.
     *
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing a profile which is not contained in the manager).
     *
     * \see addProfile()
     * \see profileRemoved()
     * \see profileAboutToBeRemoved()
     * \see clear()
     */
    bool removeProfile( QgsElevationProfile *profile );

    /**
     * Removes and deletes all profiles from the manager.
     * \see removeProfile()
     */
    void clear();

    /**
     * Returns a list of all profiles contained in the manager.
     */
    [[nodiscard]] QList< QgsElevationProfile * > profiles() const;

    /**
     * Returns the profile with a matching name, or NULLPTR if no matching profiles
     * were found.
     */
    [[nodiscard]] QgsElevationProfile *profileByName( const QString &name ) const;

    /**
     * Reads the manager's state from a DOM element, restoring all profiles
     * present in the XML document.
     * \see resolveReferences()
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the state of the manager.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * After reading settings from XML, resolves references to any layers in a \a project that have been read as layer IDs.
     *
     * \see readXml()
     */
    void resolveReferences( const QgsProject *project );

    /**
     * Generates a unique title for a new profile, which does not
     * clash with any already contained by the manager.
     */
    [[nodiscard]] QString generateUniqueTitle() const;

  signals:

    //! Emitted when a profile is about to be added to the manager
    void profileAboutToBeAdded( const QString &name );

    //! Emitted when a profile has been added to the manager
    void profileAdded( const QString &name );

    //! Emitted when a profile was removed from the manager
    void profileRemoved( const QString &name );

    //! Emitted when a profile is about to be removed from the manager
    void profileAboutToBeRemoved( const QString &name );

    //! Emitted when a profile is renamed
    void profileRenamed( QgsElevationProfile *profile, const QString &newName );

  protected:

    void setupObjectConnections( QgsElevationProfile *profile ) override;

};


#endif // QGSELEVATIONPROFILEMANAGER_H
