/***************************************************************************
    qgsselectivemaskingsourcesetmanager.h
    ------------------
    Date                 : January 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSELECTIVEMASKINGSOURCESETMANAGER_H
#define QGSSELECTIVEMASKINGSOURCESETMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsprojectstoredobjectmanager.h"
#include "qgsselectivemaskingsourceset.h"

#include <QObject>

class QDomElement;
class QDomDocument;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsSelectiveMaskingSourceSetManager
 *
 * \brief Manages storage of a set of selective masking source sets.
 *
 * QgsSelectiveMaskingSourceSetManager handles the storage, serializing and deserializing
 * of selective masking source sets. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::selectiveMaskingSourceSetManager().
 *
 * QgsSelectiveMaskingSourceSetManager retains ownership of all the sets contained
 * in the manager.
 *
 * \since QGIS 4.0
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsSelectiveMaskingSourceSetManager : public QgsProjectStoredObjectManagerBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsSelectiveMaskingSourceSetManager : public QgsAbstractProjectStoredObjectManager< QgsSelectiveMaskingSourceSet >
{
#endif
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSelectiveMaskingSourceSetManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsSelectiveMaskingSourceSetManager( QgsProject *project SIP_TRANSFERTHIS = nullptr );

    ~QgsSelectiveMaskingSourceSetManager() override;

    /**
     * Adds a \a set to the manager.
     *
     * Returns TRUE if the addition was successful, or FALSE if the set could not be added (eg
     * as a result of a duplicate set name).
     *
     * \see removeSet()
     * \see setAdded()
     */
    bool addSet( const QgsSelectiveMaskingSourceSet &set );

    /**
     * Updates the definition of a \a set already in the manager.
     *
     * The definition of the existing set with matching ID will be replaced with the updated set.
     */
    bool updateSet( const QgsSelectiveMaskingSourceSet &set );

    /**
     * Removes the set with matching \a name from the manager.
     *
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing a set which is not contained in the manager).
     *
     * \see addSet()
     * \see setRemoved()
     * \see setAboutToBeRemoved()
     * \see clear()
     */
    bool removeSet( const QString &name );

    /**
     * Removes and deletes all sets from the manager.
     *
     * \see removeSet()
     */
    void clear();

    /**
     * Renames a set in the manager from \a oldName to \a newName.
     *
     * Returns TRUE if the rename was successful, or FALSE if it failed (eg
     * no set existed with matching name, or a duplicate new name was used).
     */
    bool renameSet( const QString &oldName, const QString &newName );

    /**
     * Returns a list of all sets contained in the manager.
     */
    QVector< QgsSelectiveMaskingSourceSet > sets() const;

    /**
     * Returns the set with a matching \a id, or an invalid set if no matching sets
     * were found.
     */
    QgsSelectiveMaskingSourceSet setById( const QString &id ) const;

    /**
     * Returns the set with a matching \a name, or an invalid set if no matching sets
     * were found.
     */
    QgsSelectiveMaskingSourceSet setByName( const QString &name ) const;

    /**
     * Reads the manager's state from a DOM element, restoring all sets
     * present in the XML document.
     *
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the state of the manager.
     *
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Generates a unique title for a new set, which does not
     * clash with any already contained by the manager.
     */
    QString generateUniqueTitle() const;

  signals:

    //! Emitted whenever sets stored within the manager are changed
    void changed();

    //! Emitted when a set is about to be added to the manager
    void setAboutToBeAdded( const QString &name );

    //! Emitted when a set has been added to the manager
    void setAdded( const QString &name );

    //! Emitted when a set was removed from the manager
    void setRemoved( const QString &name );

    //! Emitted when a set is about to be removed from the manager
    void setAboutToBeRemoved( const QString &name );

    //! Emitted when a set is renamed
    void setRenamed( const QString &oldName, const QString &newName );

};


#endif // QGSSELECTIVEMASKINGSOURCESETMANAGER_H
