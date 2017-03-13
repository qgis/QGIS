/**
 * This class is a menu that is populated automatically with the actions defined for a given layer.
 */

class QgsActionMenu : QMenu
{
%TypeHeaderCode
#include <qgsactionmenu.h>
%End

  public:
    enum ActionType
    {
      Invalid,        //!< Invalid
      MapLayerAction, //!< Standard actions (defined by core or plugins)
      AttributeAction //!< Custom actions (manually defined in layer properties)
    };

    struct ActionData
    {
      ActionData();
      ActionData( const QgsAction& action, QgsFeatureId featureId, QgsMapLayer* mapLayer );
      ActionData( QgsMapLayerAction* action, QgsFeatureId featureId, QgsMapLayer* mapLayer );

      QgsActionMenu::ActionType actionType;
      QVariant actionData;
      QgsFeatureId featureId;
      QgsMapLayer* mapLayer;
    };

    /**
     * Constructs a new QgsActionMenu
     *
     * @param layer    The layer that this action will be run upon.
     * @param feature  The feature that this action will be run upon. Make sure that this feature is available
     *                 for the lifetime of this object.
     * @param parent   The usual QWidget parent.
     * @param actionScope The action scope this menu will run in
     */
    explicit QgsActionMenu( QgsVectorLayer* layer, const QgsFeature& feature, const QString& actionScope, QWidget *parent /TransferThis/ = nullptr );

    /**
     * Constructs a new QgsActionMenu
     *
     * @param layer    The layer that this action will be run upon.
     * @param fid      The feature id of the feature for which this action will be run.
     * @param parent   The usual QWidget parent.
     * @param actionScope The action scope this menu will run in
     */
    explicit QgsActionMenu( QgsVectorLayer *layer, const QgsFeatureId fid, const QString& actionScope, QWidget *parent /TransferThis/ = nullptr );

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
    void setFeature( const QgsFeature& feature );

  signals:
    void reinit();

};
