/***************************************************************************
    qgsactionmenu.h
     --------------------------------------
    Date                 : 11.8.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSACTIONMENU_H
#define QGSACTIONMENU_H

#include <QMenu>
#include <QSignalMapper>

#include "qgsattributeaction.h"
#include "qgsmaplayeractionregistry.h"


/**
 * This class is a menu that is populated automatically with the actions defined for a given layer.
 */

class QgsActionMenu : public QMenu
{
    Q_OBJECT

  public:
    enum ActionType
    {
      Invalid,        //!< Invalid
      MapLayerAction, //!< Standard actions (defined by core or plugins)
      AttributeAction //!< Custom actions (manually defined in layer properties)
    };

    struct ActionData
    {
      ActionData()
          : actionType( Invalid )
          , actionId( 0 )
      {}

      ActionData( int actionId, QgsFeatureId featureId, QgsMapLayer* mapLayer )
          : actionType( AttributeAction )
          , actionId( actionId )
          , featureId( featureId )
          , mapLayer( mapLayer )
      {}

      ActionData( QgsMapLayerAction* action, QgsFeatureId featureId, QgsMapLayer* mapLayer )
          : actionType( MapLayerAction )
          , actionId( action )
          , featureId( featureId )
          , mapLayer( mapLayer )
      {}

      ActionType actionType;

      union aid
      {
        aid( int i ) : id( i ) {}
        aid( QgsMapLayerAction* a ) : action( a ) {}
        int id;
        QgsMapLayerAction* action;
      } actionId;

      QgsFeatureId featureId;
      QgsMapLayer* mapLayer;
    };


  public:
    /**
     * Constructs a new QgsActionMenu
     *
     * @param layer    The layer that this action will be run upon.
     * @param feature  The feature that this action will be run upon. Make sure that this feature is available
     *                 for the lifetime of this object.
     * @param parent   The usual QWidget parent.
     */
    explicit QgsActionMenu( QgsVectorLayer* layer, const QgsFeature* feature, QWidget*  parent = 0 );

    /**
     * Constructs a new QgsActionMenu
     *
     * @param layer    The layer that this action will be run upon.
     * @param fid      The feature id of the feature for which this action will be run.
     * @param parent   The usual QWidget parent.
     */
    explicit QgsActionMenu( QgsVectorLayer* layer, const QgsFeatureId fid, QWidget*  parent = 0 );

    /**
     * Destructor
     */
    ~QgsActionMenu();

    /**
     * Change the feature on which actions are performed
     *
     * @param feature  A feature. Will not take ownership. It's the callers responsibility to keep the feature
     *                 as long as the menu is displayed and the action is running.
     */
    void setFeature( QgsFeature* feature );

    /**
     * @brief setFeature
     * @param feature
     */
    void setFeature( QgsFeatureId feature );

  private slots:
    void triggerAction();
    void reloadActions();

  signals:
    void reinit();

  private:
    void init();
    const QgsFeature* feature();

    QgsVectorLayer* mLayer;
    QgsAttributeAction* mActions;
    const QgsFeature* mFeature;
    QgsFeatureId mFeatureId;
    bool mOwnsFeature;
};

Q_DECLARE_METATYPE( QgsActionMenu::ActionData )

#endif // QGSACTIONMENU_H
