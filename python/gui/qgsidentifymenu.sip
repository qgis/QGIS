class QgsIdentifyMenu : QMenu
{
%TypeHeaderCode
#include <qgsidentifymenu.h>
%End

  public:
    enum MenuLevel
    {
      LayerLevel,
      FeatureLevel
    };

    struct ActionData
    {
      ActionData();


      ActionData( QgsMapLayer* layer, QgsMapLayerAction* mapLayerAction = 0 );

      ActionData( QgsMapLayer* layer, QgsFeatureId fid, QgsMapLayerAction* mapLayerAction = 0 );

      bool mIsValid;
      bool mAllResults;
      bool mIsExternalAction;
      QgsMapLayer* mLayer;
      QgsFeatureId mFeatureId;
      QgsIdentifyMenu::MenuLevel mLevel;
      QgsMapLayerAction* mMapLayerAction;
    };

    /**
     * @brief QgsIdentifyMenu is a menu to be used to choose within a list of QgsMapTool::IdentifyReults
     */
    explicit QgsIdentifyMenu( QgsMapCanvas* canvas );

    ~QgsIdentifyMenu();

    //! define if the menu executed can return multiple results (e.g. all results or all identified features of a vector layer)
    void setAllowMultipleReturn( bool multipleReturn );
    bool allowMultipleReturn();

    //! define if the menu will be shown with a single idetify result
    void setExecWithSingleResult( bool execWithSingleResult );
    bool execWithSingleResult();

    /**
     * @brief define if attribute actions(1) and map layer actions(2) can be listed and run from the menu
     * @note custom actions will be shown in any case if they exist.
     * @note (1) attribute actions are defined by the user in the layer properties @see QgsAttributeAction
     * @note (2) map layer actions are built-in c++ actions or actions which are defined by a python plugin @see QgsMapLayerActionRegistry
     */
    void setShowFeatureActions( bool showFeatureActions );
    bool showFeatureActions();

    /**
     * @brief setResultsIfExternalAction if set to false (default) the menu will not return any results if an external action has been triggered
     * @note external action can be either custom actions or feature / map layer actions (@see setShowFeatureActions)
     */
    void setResultsIfExternalAction( bool resultsIfExternalAction );
    bool resultsIfExternalAction();

    //! Defines the maximimum number of layers displayed in the menu (default is 10).
    //! @note 0 is unlimited.
    void setMaxLayerDisplay( int maxLayerDisplay );
    int maxLayerDisplay();

    //! Defines the maximimum number of features displayed in the menu for vector layers (default is 10).
    //! @note 0 is unlimited.
    void setMaxFeatureDisplay( int maxFeatureDisplay );
    int maxFeatureDisplay();

    //! adds a new custom action to the menu
    void addCustomAction( QgsMapLayerAction* action );

    //! remove all custom actions from the menu to be built
    void removeCustomActions();

    /**
     * @brief exec
     * @param idResults the list of identify results to choose within
     * @param pos the position where the menu will be executed
     */
    QList<QgsMapToolIdentify::IdentifyResult> exec( const QList<QgsMapToolIdentify::IdentifyResult> idResults, QPoint pos );

  protected:
    virtual void closeEvent( QCloseEvent *e );
};
