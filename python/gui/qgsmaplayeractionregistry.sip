class QgsMapLayerAction : QAction
{
%TypeHeaderCode
#include <qgsmaplayeractionregistry.h>
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

    //! Creates a map layer action which can run on any layer
    //! @note using AllActions as a target probably does not make a lot of sense. This default action was settled for API compatiblity reasons.
    QgsMapLayerAction( QString name, QObject *parent /TransferThis/, Targets targets = AllActions, QIcon icon = QIcon() );

    //! Creates a map layer action which can run only on a specific layer
    QgsMapLayerAction( QString name, QObject *parent /TransferThis/, QgsMapLayer* layer, Targets targets = AllActions, QIcon icon = QIcon() );

    //! Creates a map layer action which can run on a specific type of layer
    QgsMapLayerAction( QString name, QObject *parent /TransferThis/, QgsMapLayer::LayerType layerType, Targets targets = AllActions, QIcon icon = QIcon() );

    ~QgsMapLayerAction();

    /** True if action can run using the specified layer */
    bool canRunUsingLayer( QgsMapLayer* layer ) const;

    /** Triggers the action with the specified layer and list of feature. */
    void triggerForFeatures( QgsMapLayer* layer, const QList<QgsFeature> featureList );

    /** Triggers the action with the specified layer and feature. */
    void triggerForFeature( QgsMapLayer* layer, const QgsFeature* feature );

    /** Triggers the action with the specified layer. */
    void triggerForLayer( QgsMapLayer* layer );

    /** Define the targets of the action */
    void setTargets( Targets targets );
    /** Return availibity of action */
    const Targets& targets() const;

  signals:
    /** Triggered when action has been run for a specific list of features */
    void triggeredForFeatures( QgsMapLayer* layer, const QList<QgsFeature> featureList );

    /** Triggered when action has been run for a specific feature */
    void triggeredForFeature( QgsMapLayer* layer, const QgsFeature& feature );

    /** Triggered when action has been run for a specific layer */
    void triggeredForLayer( QgsMapLayer* layer );

};

QFlags<QgsMapLayerAction::Target> operator|(QgsMapLayerAction::Target f1, QFlags<QgsMapLayerAction::Target> f2);

class QgsMapLayerActionRegistry : QObject
{
%TypeHeaderCode
#include <qgsmaplayeractionregistry.h>
%End

  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsMapLayerActionRegistry * instance();

    ~QgsMapLayerActionRegistry();

    /**Adds a map layer action to the registry*/
    void addMapLayerAction( QgsMapLayerAction * action );

    /**Returns the map layer actions which can run on the specified layer*/
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer* layer, QgsMapLayerAction::Targets targets = QgsMapLayerAction::AllActions );

    /**Removes a map layer action from the registry*/
    bool removeMapLayerAction( QgsMapLayerAction *action );

    /**Sets the default action for a layer*/
    void setDefaultActionForLayer( QgsMapLayer* layer, QgsMapLayerAction* action );
    /**Returns the default action for a layer*/
    QgsMapLayerAction * defaultActionForLayer( QgsMapLayer* layer );

  protected:
    //! protected constructor
    QgsMapLayerActionRegistry( QObject * parent = 0 );

  signals:
    /** Triggered when an action is added or removed from the registry */
    void changed();

};
