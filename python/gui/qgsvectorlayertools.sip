class QgsVectorLayerTools
{
%TypeHeaderCode
#include <qgsvectorlayertools.h>
%End
  public:

    /**
     * This method should/will be called, whenever a new feature will be added to the layer
     *
     * @param layer           The layer to which the feature should be added
     * @param defaultValues   Default values for the feature to add
     * @param defaultGeometry A default geometry to add to the feature
     * @return                True in case of success, False if the operation failed/was aborted
     */
    virtual bool addFeature( QgsVectorLayer* layer, QgsAttributeMap defaultValues = QgsAttributeMap(), const QgsGeometry& defaultGeometry = QgsGeometry() ) const = 0;


    /**
     * This will be called, whenever a vector layer should be switched to edit mode. Check the providers
     * capability to edit in here.
     * If successful layer->startEditing() will be called and true returned.
     *
     * @param layer  The layer on which to start an edit session
     *
     * @return       True, if the editing session was started
     */
    virtual bool startEditing( QgsVectorLayer* layer ) const = 0;

    /**
     * Will be called, when an editing session is ended and the features should be commited.
     * Appropriate dialogs should be shown like
     *
     * @param layer       The layer to commit
     * @param allowCancel True if a cancel button should be offered
     * @return            True if successful
     */
    virtual bool stopEditing( QgsVectorLayer* layer, bool allowCancel = true ) const = 0;

    /**
     * Should be called, when the features should be commited but the editing session is not ended.
     *
     * @param layer       The layer to commit
     * @return            True if successful
     */
    virtual bool saveEdits( QgsVectorLayer* layer ) const = 0;
};
