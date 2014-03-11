class QgsMapLayerAction : QAction
{
%TypeHeaderCode
#include <qgsmaplayeractionregistry.h>
%End

  public:
    /**Creates a map layer action which can run on any layer*/
    QgsMapLayerAction( QString name, QObject *parent );
    /**Creates a map layer action which can run only on a specific layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer* layer );
    /**Creates a map layer action which can run on a specific type of layer*/
    QgsMapLayerAction( QString name, QObject *parent, QgsMapLayer::LayerType layerType );

    ~QgsMapLayerAction();

    /** True if action can run using the specified layer */
    bool canRunUsingLayer( QgsMapLayer* layer ) const;

    /** Triggers the action with the specified layer and feature. This also emits the triggeredForLayer( QgsMapLayer *)
     * and triggered() slots */
    void triggerForFeature( QgsMapLayer* layer, QgsFeature* feature );

    /** Triggers the action with the specified layer. This also emits the triggered() slot. */
    void triggerForLayer( QgsMapLayer* layer );

  signals:
    /** Triggered when action has been run for a specific feature */
    void triggeredForFeature( QgsMapLayer* layer, QgsFeature* feature );

    /** Triggered when action has been run for a specific layer */
    void triggeredForLayer( QgsMapLayer* layer );

};

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
    QList<QgsMapLayerAction *> mapLayerActions( QgsMapLayer* layer );

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
