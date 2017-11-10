/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsactionmenu.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsActionMenu : QMenu
{
%Docstring
 This class is a menu that is populated automatically with the actions defined for a given layer.
%End

%TypeHeaderCode
#include "qgsactionmenu.h"
%End
  public:
    enum ActionType
    {
      Invalid,
      MapLayerAction,
      AttributeAction
    };

    struct ActionData
    {

      ActionData();
%Docstring
 Constructor for ActionData.
%End
      ActionData( const QgsAction &action, QgsFeatureId featureId, QgsMapLayer *mapLayer );
      ActionData( QgsMapLayerAction *action, QgsFeatureId featureId, QgsMapLayer *mapLayer );

      QgsActionMenu::ActionType actionType;
      QVariant actionData;
      QgsFeatureId featureId;
      QgsMapLayer *mapLayer;
    };

    explicit QgsActionMenu( QgsVectorLayer *layer, const QgsFeature &feature, const QString &actionScope, QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructs a new QgsActionMenu

 \param layer    The layer that this action will be run upon.
 \param feature  The feature that this action will be run upon. Make sure that this feature is available
                 for the lifetime of this object.
 \param parent   The usual QWidget parent.
 \param actionScope The action scope this menu will run in
%End

    explicit QgsActionMenu( QgsVectorLayer *layer, const QgsFeatureId fid, const QString &actionScope, QWidget *parent /TransferThis/ = 0 );
%Docstring
 Constructs a new QgsActionMenu

 \param layer    The layer that this action will be run upon.
 \param fid      The feature id of the feature for which this action will be run.
 \param parent   The usual QWidget parent.
 \param actionScope The action scope this menu will run in
%End

    void setFeature( const QgsFeature &feature );
%Docstring
 Change the feature on which actions are performed

 \param feature  A feature. Will not take ownership. It's the callers responsibility to keep the feature
                 as long as the menu is displayed and the action is running.
%End

    void setExpressionContextScope( const QgsExpressionContextScope &scope );
%Docstring
 Sets an expression context scope used to resolve underlying actions.

.. versionadded:: 3.0
%End

    QgsExpressionContextScope expressionContextScope() const;
%Docstring
 Returns an expression context scope used to resolve underlying actions.

.. versionadded:: 3.0
 :rtype: QgsExpressionContextScope
%End

  signals:
    void reinit();

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsactionmenu.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
