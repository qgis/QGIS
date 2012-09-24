class QgsUndoCommand : QUndoCommand
{
%TypeHeaderCode
#include <qgsvectorlayerundocommand.h>
%End
  public:

    /** change structure for attribute for undo/redo purpose */
    class AttributeChangeEntry
    {
      public:
        bool isFirstChange;
        QVariant original;
        QVariant target;
    };

    typedef QMap<int, QgsUndoCommand::AttributeChangeEntry> AttributeChanges;

    /** change structure to geometry for undo/redo purpose */
    class GeometryChangeEntry
    {
      public:
        GeometryChangeEntry();
        ~GeometryChangeEntry();

        void setOriginalGeometry( QgsGeometry& orig );
        void setTargetGeometry( QgsGeometry& target );

        QgsGeometry* original;
        QgsGeometry* target;
    };


    QgsUndoCommand( QgsVectorLayer* layer, QString text );

    /**
     * Necessary function to provide undo operation
     */
    void undo();

    /**
     * Necessary function to provide redo operation
     */
    void redo();

    /**
     * Function to store changes in geometry to be returned to this state after undo/redo
     * @param featureId id of feature edited
     * @param original original geometry of feature which was changed
     * @param target changed geometry which was changed
     */
    void storeGeometryChange( QgsFeatureId featureId, QgsGeometry& original, QgsGeometry& target );

    /**
     * Stores changes of attributes for the feature to be returned to this state after undo/redo
     * @param featureId id of feature for which this chaged is stored
     * @param field field identifier of field which was changed
     * @param original original value of attribute before change
     * @param target target value of attribute after change
     * @param isFirstChange flag if this change is the first one
     */
    void storeAttributeChange( QgsFeatureId featureId, int field, QVariant original, QVariant target, bool isFirstChange );

    /**
     * Add id of feature to deleted list to be reverted if needed afterwards
     * @param featureId id of feature which is to be deleted
     */
    void storeFeatureDelete( QgsFeatureId featureId );

    /**
     * Add new feature to list of new features to be stored for undo/redo operations.
     * @param feature feature which is to be added
     */
    void storeFeatureAdd( QgsFeature& feature );

    /**
     * Add new attribute to list of attributes to be used for attributes of features for undo/redo operations.
     * @param index index of attribute which is to be added
     * @param value field description which is to be stored
     */
    void storeAttributeAdd( int index, const QgsField & value );

    /**
     * Add deleted attribute which is to be stored for undo/redo operations.
     * @param index index od attribute definition which is to be deleted
     * @param orig deleted field's description
     */
    void storeAttributeDelete( int index, const QgsField & orig );
};
