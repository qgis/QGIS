/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsidentifymenu.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsIdentifyMenu : QMenu
{
%Docstring
.. seealso:: QgsMapToolIdentify).
.. seealso:: QgsAction) as well as map layer actions (\see QgsMapLayerAction).
 It can also embed custom map layer actions, defined for this menu exclusively.
 If used in a QgsMapToolIdentify, it is accessible via QgsMapToolIdentify.identifyMenu() and can be customized in the map tool sub-class.
%End

%TypeHeaderCode
#include "qgsidentifymenu.h"
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
%Docstring
Constructor for ActionData
%End

      ActionData( QgsMapLayer *layer, QgsMapLayerAction *mapLayerAction = 0 );

      ActionData( QgsMapLayer *layer, QgsFeatureId fid, QgsMapLayerAction *mapLayerAction = 0 );

      bool mIsValid;
      bool mAllResults;
      bool mIsExternalAction;
      QgsMapLayer *mLayer;
      QgsFeatureId mFeatureId;
      QgsIdentifyMenu::MenuLevel mLevel;
      QgsMapLayerAction *mMapLayerAction;
    };

    explicit QgsIdentifyMenu( QgsMapCanvas *canvas );
%Docstring
 QgsIdentifyMenu is a menu to be used to choose within a list of QgsMapTool.IdentifyReults
%End

    ~QgsIdentifyMenu();

    void setAllowMultipleReturn( bool multipleReturn );
%Docstring
define if the menu executed can return multiple results (e.g. all results or all identified features of a vector layer)
%End
    bool allowMultipleReturn();
%Docstring
 :rtype: bool
%End

    void setExecWithSingleResult( bool execWithSingleResult );
%Docstring
define if the menu will be shown with a single idetify result
%End
    bool execWithSingleResult();
%Docstring
 :rtype: bool
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

    void setShowFeatureActions( bool showFeatureActions );
%Docstring
 define if attribute actions(1) and map layer actions(2) can be listed and run from the menu
.. note::

   custom actions will be shown in any case if they exist.
.. seealso:: QgsAction
.. seealso:: QgsMapLayerActionRegistry
%End
    bool showFeatureActions();
%Docstring
 :rtype: bool
%End

    void setResultsIfExternalAction( bool resultsIfExternalAction );
%Docstring
 setResultsIfExternalAction if set to false (default) the menu will not return any results if an external action has been triggered
.. seealso:: setShowFeatureActions)
%End
    bool resultsIfExternalAction();
%Docstring
 :rtype: bool
%End

    void setMaxLayerDisplay( int maxLayerDisplay );
%Docstring
 Defines the maximum number of layers displayed in the menu (default is 10).
.. note::

   0 is unlimited.
%End
    int maxLayerDisplay();
%Docstring
 :rtype: int
%End

    void setMaxFeatureDisplay( int maxFeatureDisplay );
%Docstring
 Defines the maximum number of features displayed in the menu for vector layers (default is 10).
.. note::

   0 is unlimited.
%End
    int maxFeatureDisplay();
%Docstring
 :rtype: int
%End

    void addCustomAction( QgsMapLayerAction *action );
%Docstring
adds a new custom action to the menu
%End

    void removeCustomActions();
%Docstring
remove all custom actions from the menu to be built
%End

    QList<QgsMapToolIdentify::IdentifyResult> exec( const QList<QgsMapToolIdentify::IdentifyResult> &idResults, QPoint pos );
%Docstring
 exec
 \param idResults the list of identify results to choose within
 \param pos the position where the menu will be executed
 :rtype: list of QgsMapToolIdentify.IdentifyResult
%End

  protected:
    virtual void closeEvent( QCloseEvent *e );

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsidentifymenu.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
