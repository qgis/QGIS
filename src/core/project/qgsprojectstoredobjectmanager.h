/***************************************************************************
    qgsprojectstoredobjectmanager.h
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

#ifndef QGSPROJECTSTOREDOBJECTMANAGER_H
#define QGSPROJECTSTOREDOBJECTMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>

class QgsProject;

/**
 * \ingroup core
 * \class QgsProjectStoredObjectManagerBase
 *
 * \brief Manages storage of a set of objects attached to a QgsProject.
 *
 * QgsProjectStoredObjectManagerBase handles the storage, serializing and deserializing
 * of attached objects.
 *
 * QgsProjectStoredObjectManagerBase retains ownership of all the objects contained
 * in the manager.
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsProjectStoredObjectManagerBase : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectStoredObjectManagerBase, for objects attached to the specified \a project.
     */
    explicit QgsProjectStoredObjectManagerBase( QgsProject *project SIP_TRANSFERTHIS = nullptr );

  signals:

    //! Emitted when an object is about to be added to the manager
    void objectAboutToBeAdded( const QString &name );

    //! Emitted when an object has been added to the manager
    void objectAdded( const QString &name );

    //! Emitted when an object was removed from the manager
    void objectRemoved( const QString &name );

    //! Emitted when an object is about to be removed from the manager
    void objectAboutToBeRemoved( const QString &name );

  protected:

    //! Associated project
    QgsProject *mProject = nullptr;

    /**
     * Marks the project as dirty.
     */
    void markProjectDirty();
};

/**
 * \ingroup core
 * \class QgsAbstractProjectStoredObjectManager
 *
 * \brief Template class for storage of a set of objects attached to a QgsProject.
 *
 * QgsAbstractProjectStoredObjectManager handles the storage, serializing and deserializing
 * of attached objects.
 *
 * QgsAbstractProjectStoredObjectManager retains ownership of all the objects contained
 * in the manager.
 *
 * \since QGIS 4.0
 */
template<class T>
class CORE_EXPORT QgsAbstractProjectStoredObjectManager : public QgsProjectStoredObjectManagerBase
{
  public:

    /**
     * Constructor for QgsAbstractProjectStoredObjectManager, for objects attached to the specified \a project.
     */
    explicit QgsAbstractProjectStoredObjectManager( QgsProject *project SIP_TRANSFERTHIS = nullptr );

    ~QgsAbstractProjectStoredObjectManager() override;

    /**
     * Returns the list of objects contained within the manager.
     */
    QList< T * > objects() const;

    /**
     * Returns the object with a matching name, or NULLPTR if no matching objects
     * were found.
     */
    T *objectByName( const QString &name ) const;

  protected:

    //! Attached objects, owned by the manager
    QList< T * > mObjects;

    /**
     * Removes and deletes all objects from the manager.
     * \see removeObject()
     */
    void clearObjects();

    /**
     * Adds an \a object to the manager. Ownership of the object is transferred to the manager.
     * Returns TRUE if the addition was successful, or FALSE if the object could not be added (eg
     * as a result of a duplicate object name).
     * \see removeObject()
     * \see objectAdded()
     */
    bool addObject( T *object SIP_TRANSFER );

    /**
     * Removes an \a object from the manager. The object is deleted.
     * Returns TRUE if the removal was successful, or FALSE if the removal failed (eg as a result
     * of removing an object which is not contained in the manager).
     *
     * \see addObject()
     * \see objectRemoved()
     * \see objectAboutToBeRemoved()
     * \see clearObjects()
     */
    bool removeObject( T *object );

    /**
     * Sets up additional connections to an \a object, called when the object
     * is first added to the manager.
     */
    virtual void setupObjectConnections( T *object );

};


#endif // QGSPROJECTSTOREDOBJECTMANAGER_H
