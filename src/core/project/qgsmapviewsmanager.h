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
 * accessed through a QgsProject via QgsProject::get3DViewsManager() for example.
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
    explicit QgsMapViewsManager( QgsProject *project );

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
     * Returns the DOM element representing the settings of the view named \a name
     *
     * \note Not available in Python bindings
     */
    QDomElement getViewSettings( const QString &name ) const SIP_SKIP;

    /**
     * Adds a new view named \a name to the manager with the configuration DOM \a dom
     *
     * \note Not available in Python bindings
     */
    void registerViewSettings( const QString &name, const QDomElement &dom ) SIP_SKIP;

    /**
     * Returns the names of all views added to the manager
     *
     * \note Not available in Python bindings
     */
    QStringList getViewsNames() const SIP_SKIP;

    /**
     * Returns the list of configurations of views added to the manager
     *
     * \note Not available in Python bindings
     */
    QList<QDomElement> getViews() const SIP_SKIP;

    //! Removes the configuration of the view named \a name
    void removeView( const QString &name );

    //! Renames the view named \a oldTitle to \a newTitle
    void renameView( const QString &oldTitle, const QString &newTitle );

    //! Sets whether the view named \a name will be initially visible when the project is opened
    void setViewInitiallyVisible( const QString &name, bool visible );

    //! Returns whether the view named \a name will is opened
    bool isViewOpen( const QString &name );

  signals:
    //! Emitted when the views list has changed (whenever a view was removed, added, renamed..)
    void viewsListChanged();

  private:
    QMap<QString, QDomElement> mMapViewsDom;
};


#endif // QGSMAPVIEWSMANAGER_H
