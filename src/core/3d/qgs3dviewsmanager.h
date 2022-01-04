/***************************************************************************
    qgs3dviewsmanager.h
    ------------------
    Date                 : December 2021
    Copyright            : (C) 2021 Belgacem Nedjima
    Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DVIEWSMANAGER_H
#define QGS3DVIEWSMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QMap>

class QgsProject;

/**
 * \ingroup core
 * \class Qgs3DViewsManager
 *
 * \brief Manages storage of a set of 3D views.
 *
 * QgsLayoutManager handles the storage, serializing and deserializing
 * of 3D views. Usually this class is not constructed directly, but
 * rather accessed through a QgsProject via QgsProject::viewsManager3D().
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT Qgs3DViewsManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for Qgs3DViewsManager. The project will become the parent object for this
     * manager.
     */
    explicit Qgs3DViewsManager( QgsProject *project );

    /**
     * Reads the manager's state from a DOM element, restoring all 3D views
     * present in the XML document
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &doc );

    /**
     *  Returns a DOM element representing the state of the manager.
     *  \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    //! Removes and deletes all 3D views from the manager.
    void clear();

    //! Returns the DOM element representing the settings of the 3D view named \a name
    QDomElement get3DViewSettings( const QString &name ) const;

    //! Adds a new 3D view named \a name to the manager with the configuration DOM \a dom
    void register3DViewSettings( const QString &name, const QDomElement &dom );

    //! Returns the names of all 3D views added to the manager
    QStringList get3DViewsNames() const;

    //! Returns the list of configurations of 3D views added to the manager
    QList<QDomElement> get3DViews() const;

    //! Removes the configuration of the 3D view named \a name
    void remove3DView( const QString &name );

    //! Renames the 3D view named \a oldTitle to \a newTitle
    void rename3DView( const QString &oldTitle, const QString &newTitle );

    //! Sets the configuration of the 3D view named \a name to being opened
    void viewOpened( const QString &name );

    //! Sets the configuration of the 3D view named \a name to being closed
    void viewClosed( const QString &name, const QDomElement &dom );

  signals:
    //! Emitted when the views list has changed (whenever a 3D view was removed, added, renamed..)
    void viewsListChanged();

  private:
    QMap<QString, QDomElement> m3DMapViewsDom;
};


#endif // QGSLAYOUTMANAGER_H
