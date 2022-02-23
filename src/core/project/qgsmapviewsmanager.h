/***************************************************************************
    qgsmapviewsmanager.h
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

#ifndef QGSMAPVIEWSMANAGER_H
#define QGSMAPVIEWSMANAGER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QMap>

class QgsProject;

/**
 * \ingroup core
 * \class QgsMapViewsManager
 *
 * \brief Manages storage of a set of views.
 *
 * QgsMapViewsManager handles the storage, serializing and deserializing
 * of views. Usually this class is not constructed directly, but rather
 * accessed through a QgsProject via QgsProject::viewsManager().
 *
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsMapViewsManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapViewsManager. The project will become the parent object for this
     * manager.
     */
    explicit QgsMapViewsManager( QgsProject *project SIP_TRANSFERTHIS );

    /**
     * Reads the manager's state from a DOM element, restoring all views
     * present in the XML document
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QDomDocument &doc );

    /**
     * Returns a DOM element representing the state of the manager.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    //! Removes and deletes all views from the manager.
    void clear();

    /**
     * Returns the DOM element representing the settings of the 3D view named \a name
     *
     * \note Not available in Python bindings
     */
    QDomElement get3DViewSettings( const QString &name ) const SIP_SKIP;

    /**
     * Adds a new 3D view named \a name to the manager with the configuration DOM \a dom
     *
     * \note Not available in Python bindings
     */
    void register3DViewSettings( const QString &name, const QDomElement &dom ) SIP_SKIP;

    /**
     * Returns the names of all 3D views added to the manager
     *
     * \note Not available in Python bindings
     */
    QStringList get3DViewsNames() const SIP_SKIP;

    /**
     * Returns the list of configurations of 3D views added to the manager
     *
     * \note Not available in Python bindings
     */
    QList<QDomElement> get3DViews() const SIP_SKIP;

    //! Removes the configuration of the 3D view named \a name
    void remove3DView( const QString &name );

    //! Renames the 3D view named \a oldTitle to \a newTitle
    void rename3DView( const QString &oldTitle, const QString &newTitle );

    /**
     * Sets whether the 3D view named \a name will be initially visible when the project is opened.
     *
     * \note Not available in Python bindings
     */
    void set3DViewInitiallyVisible( const QString &name, bool visible ) SIP_SKIP;

    /**
     * Returns whether the 3D view named \a name will is opened.
     *
     * \note Not available in Python bindings.
     */
    bool is3DViewOpen( const QString &name ) SIP_SKIP;

  signals:
    //! Emitted when the views list has changed (whenever a view was removed, added, renamed..)
    void views3DListChanged();

  private:
    QMap<QString, QDomElement> m3DMapViewsDom;
};


#endif // QGSMAPVIEWSMANAGER_H
