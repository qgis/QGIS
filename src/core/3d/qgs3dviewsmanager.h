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
 * Qgs3DViewsManager retains ownership of all the 3d views contained
 * in the manager.
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

    bool readXml( const QDomElement &element, const QDomDocument &doc );

    QDomElement writeXml( QDomDocument &doc ) const;

    void clear();

    QDomElement get3DViewSettings( const QString &name );
    void register3DViewSettings( const QString &name, const QDomElement &dom );

    QStringList get3DViewsNames();
    QList<QDomElement> get3DViews();

    void remove3DView( const QString &name );
    void rename3DView( const QString &oldTitle, const QString &newTitle );

    void viewOpened( const QString &name );
    void viewClosed( const QString &name );

  signals:
    void viewsListChanged();

  private:
    QMap<QString, QDomElement> m3DMapViewsDom;
};


#endif // QGSLAYOUTMANAGER_H
