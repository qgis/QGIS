/***************************************************************************
    qgsmaplayeractionregistry.h
    ---------------------------
    begin                : January 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERACTIONREGISTRY_H
#define QGSMAPLAYERACTIONREGISTRY_H

#include "qgis_sip.h"
#include "qgis.h"
#include "qgis_gui.h"
#include "qgsmaplayeractioncontext.h"

#include <QObject>
#include <QList>
#include <QMap>

class QgsFeature;
class QgsMapLayer;
class QgsMapLayerAction;


/**
 * \ingroup gui
* \brief This class tracks map layer actions.
*
* QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
* QgsGui::mapLayerActionRegistry().
*/
class GUI_EXPORT QgsMapLayerActionRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapLayerActionRegistry.
     *
     * QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
     * QgsGui::mapLayerActionRegistry().
     */
    QgsMapLayerActionRegistry( QObject *parent = nullptr );

    //! Adds a map layer action to the registry
    void addMapLayerAction( QgsMapLayerAction *action );

    /**
     * Returns the map layer actions which can run on the specified layer.
     *
     * The \a context argument was added in QGIS 3.30.
     */
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer *layer, Qgis::MapLayerActionTargets targets = Qgis::MapLayerActionTarget::AllActions, const QgsMapLayerActionContext &context = QgsMapLayerActionContext() );

    //! Removes a map layer action from the registry
    bool removeMapLayerAction( QgsMapLayerAction *action );

    //! Sets the default action for a layer
    void setDefaultActionForLayer( QgsMapLayer *layer, QgsMapLayerAction *action );
    //! Returns the default action for a layer
    QgsMapLayerAction *defaultActionForLayer( QgsMapLayer *layer );

  protected:

    QList< QgsMapLayerAction * > mMapLayerActionList;

  signals:
    //! Triggered when an action is added or removed from the registry
    void changed();

  private:

    QMap< QgsMapLayer *, QgsMapLayerAction * > mDefaultLayerActionMap;

};

#endif // QGSMAPLAYERACTIONREGISTRY_H
