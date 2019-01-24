/***************************************************************************
    qgsactionmenu.h
     --------------------------------------
    Date                 : 11.8.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include "qgis_sip.h"
#include <QSignalMapper>

#include "qgsfeature.h"
#include "qgsaction.h"
#include "qgis_gui.h"
#include "qgsattributeform.h"

class QgsMapLayer;
class QgsMapLayerAction;
class QgsVectorLayer;
class QgsActionManager;

/**
 * \ingroup gui
 * This class is a menu that is populated automatically with the actions defined for a given layer.
 */

class GUI_EXPORT QgsActionMenu : public QMenu
{
    Q_OBJECT

  public:
    enum ActionType
    {
      Invalid,        //!< Invalid
      MapLayerAction, //!< Standard actions (defined by core or plugins)
      AttributeAction //!< Custom actions (manually defined in layer properties)
    };

    struct GUI_EXPORT ActionData
    {

      /**
       * Constructor for ActionData.
       */
      ActionData() = default;
      ActionData( const QgsAction &action, QgsFeatureId featureId, QgsMapLayer *mapLayer );
      ActionData( QgsMapLayerAction *action, QgsFeatureId featureId, QgsMapLayer *mapLayer );

      QgsActionMenu::ActionType actionType = Invalid;
      QVariant actionData;
      QgsFeatureId featureId = 0;
      QgsMapLayer *mapLayer = nullptr;
    };

    /**
     * Constructs a new QgsActionMenu
     *
     * \param layer    The layer that this action will be run upon.
     * \param feature  The feature that this action will be run upon. Make sure that this feature is available
     *                 for the lifetime of this object.
     * \param parent   The usual QWidget parent.
     * \param actionScope The action scope this menu will run in
     */
    explicit QgsActionMenu( QgsVectorLayer *layer, const QgsFeature &feature, const QString &actionScope, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructs a new QgsActionMenu
     *
     * \param layer    The layer that this action will be run upon.
     * \param fid      The feature id of the feature for which this action will be run.
     * \param parent   The usual QWidget parent.
     * \param actionScope The action scope this menu will run in
     */
    explicit QgsActionMenu( QgsVectorLayer *layer, QgsFeatureId fid, const QString &actionScope, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Change the feature on which actions are performed
     *
     * \param feature  A feature. Will not take ownership. It's the callers responsibility to keep the feature
     *                 as long as the menu is displayed and the action is running.
     */
    void setFeature( const QgsFeature &feature );

    /**
     * Change the mode of the actions
     *
     * \param mode The mode of the attribute form
     */
    void setMode( QgsAttributeEditorContext::Mode mode );

    /**
     * Sets an expression context scope used to resolve underlying actions.
     *
     * \since QGIS 3.0
     */
    void setExpressionContextScope( const QgsExpressionContextScope &scope );

    /**
     * Returns an expression context scope used to resolve underlying actions.
     *
     * \since QGIS 3.0
     */
    QgsExpressionContextScope expressionContextScope() const;

  signals:
    void reinit();

  private slots:
    void triggerAction();
    void reloadActions();
    void layerWillBeDeleted();

  private:
    void init();
    QgsFeature feature();

    QgsVectorLayer *mLayer = nullptr;
    QList<QgsAction> mActions;
    QgsFeature mFeature;
    QgsFeatureId mFeatureId;
    QString mActionScope;
    QgsExpressionContextScope mExpressionContextScope;
    QgsAttributeEditorContext::Mode mMode;
};


Q_DECLARE_METATYPE( QgsActionMenu::ActionData )

#endif // QGSACTIONMENU_H
