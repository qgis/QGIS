/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayeractionregistry.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapLayerAction : QAction
{
%Docstring
An action which can run on map layers
%End

%TypeHeaderCode
#include "qgsmaplayeractionregistry.h"
%End
  public:
    enum Target
    {
      Layer,
      SingleFeature,
      MultipleFeatures,
      AllActions
    };
    typedef QFlags<QgsMapLayerAction::Target> Targets;


    enum Flag
    {
      EnabledOnlyWhenEditable,
    };

    typedef QFlags<QgsMapLayerAction::Flag> Flags;


    QgsMapLayerAction( const QString &name, QObject *parent /TransferThis/, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = 0 );
%Docstring
Creates a map layer action which can run on any layer

.. note::

   using AllActions as a target probably does not make a lot of sense. This default action was settled for API compatibility reasons.
%End

    QgsMapLayerAction( const QString &name, QObject *parent /TransferThis/, QgsMapLayer *layer, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = 0 );
%Docstring
Creates a map layer action which can run only on a specific layer
%End

    QgsMapLayerAction( const QString &name, QObject *parent /TransferThis/, QgsMapLayerType layerType, Targets targets = AllActions, const QIcon &icon = QIcon(), QgsMapLayerAction::Flags flags = 0 );
%Docstring
Creates a map layer action which can run on a specific type of layer
%End

    ~QgsMapLayerAction();

    QgsMapLayerAction::Flags flags() const;
%Docstring
Layer behavior flags.

.. versionadded:: 3.0
%End

    bool canRunUsingLayer( QgsMapLayer *layer ) const;
%Docstring
True if action can run using the specified layer
%End

    void triggerForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList );
%Docstring
Triggers the action with the specified layer and list of feature.
%End

    void triggerForFeature( QgsMapLayer *layer, const QgsFeature *feature );
%Docstring
Triggers the action with the specified layer and feature.
%End

    void triggerForLayer( QgsMapLayer *layer );
%Docstring
Triggers the action with the specified layer.
%End

    void setTargets( Targets targets );
%Docstring
Define the targets of the action
%End
    const Targets &targets() const;
%Docstring
Returns availibity of action
%End

    bool isEnabledOnlyWhenEditable() const;
%Docstring
Returns ``True`` if the action is only enabled for layers in editable mode.

.. versionadded:: 3.0
%End

  signals:
    void triggeredForFeatures( QgsMapLayer *layer, const QList<QgsFeature> &featureList );
%Docstring
Triggered when action has been run for a specific list of features
%End

    void triggeredForFeature( QgsMapLayer *layer, const QgsFeature &feature );
%Docstring
Triggered when action has been run for a specific feature
%End

    void triggeredForLayer( QgsMapLayer *layer );
%Docstring
Triggered when action has been run for a specific layer
%End

};

QFlags<QgsMapLayerAction::Target> operator|(QgsMapLayerAction::Target f1, QFlags<QgsMapLayerAction::Target> f2);


class QgsMapLayerActionRegistry : QObject
{
%Docstring
This class tracks map layer actions.

QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
:py:func:`QgsGui.mapLayerActionRegistry()`
%End

%TypeHeaderCode
#include "qgsmaplayeractionregistry.h"
%End
  public:

    QgsMapLayerActionRegistry( QObject *parent = 0 );
%Docstring
Constructor for QgsMapLayerActionRegistry.

QgsMapLayerActionRegistry is not usually directly created, but rather accessed through
:py:func:`QgsGui.mapLayerActionRegistry()`
%End

    void addMapLayerAction( QgsMapLayerAction *action );
%Docstring
Adds a map layer action to the registry
%End

    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer *layer, QgsMapLayerAction::Targets targets = QgsMapLayerAction::AllActions );
%Docstring
Returns the map layer actions which can run on the specified layer
%End

    bool removeMapLayerAction( QgsMapLayerAction *action );
%Docstring
Removes a map layer action from the registry
%End

    void setDefaultActionForLayer( QgsMapLayer *layer, QgsMapLayerAction *action );
%Docstring
Sets the default action for a layer
%End
    QgsMapLayerAction *defaultActionForLayer( QgsMapLayer *layer );
%Docstring
Returns the default action for a layer
%End

  protected:


  signals:
    void changed();
%Docstring
Triggered when an action is added or removed from the registry
%End

};

QFlags<QgsMapLayerAction::Flag> operator|(QgsMapLayerAction::Flag f1, QFlags<QgsMapLayerAction::Flag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaplayeractionregistry.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
