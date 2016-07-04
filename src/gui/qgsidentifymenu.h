/***************************************************************************
    qgsidentifymenu.h  -  menu to be used in identify map tool
    ---------------------
    begin                : August 2014
    copyright            : (C) 2014 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIDENTIFYMENU_H
#define QGSIDENTIFYMENU_H

#include <QMenu>

#include "qgsactionmenu.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsvectorlayer.h"

/// @cond PRIVATE
class CustomActionRegistry : public QgsMapLayerActionRegistry
{
    Q_OBJECT

  public:
    explicit CustomActionRegistry( QObject *parent );
    // remove all actions
    void clear() { mMapLayerActionList.clear(); }
};
///@endcond

/** \ingroup gui
 * @brief The QgsIdentifyMenu class builds a menu to be used with identify results (@see QgsMapToolIdentify).
 * It is customizable and can display attribute actions (@see QgsAction) as well as map layer actions (@see QgsMapLayerAction).
 * It can also embed custom map layer actions, defined for this menu exclusively.
 * If used in a QgsMapToolIdentify, it is accessible via QgsMapToolIdentify::identifyMenu() and can be customized in the map tool sub-class.
 */
class GUI_EXPORT QgsIdentifyMenu : public QMenu
{

    Q_OBJECT

  public:
    enum MenuLevel
    {
      LayerLevel,
      FeatureLevel
    };

    struct ActionData
    {
      ActionData()
          : mIsValid( false )
          , mAllResults( false )
          , mIsExternalAction( false )
          , mLayer( nullptr )
          , mFeatureId( 0 )
          , mLevel( LayerLevel )
          , mMapLayerAction( nullptr )
      {}

      ActionData( QgsMapLayer* layer, QgsMapLayerAction* mapLayerAction = nullptr )
          : mIsValid( true )
          , mAllResults( !layer )
          , mIsExternalAction( nullptr != mapLayerAction )
          , mLayer( layer )
          , mFeatureId( 0 )
          , mLevel( LayerLevel )
          , mMapLayerAction( mapLayerAction )
      {}

      ActionData( QgsMapLayer* layer, QgsFeatureId fid, QgsMapLayerAction* mapLayerAction = nullptr )
          : mIsValid( true )
          , mAllResults( false )
          , mIsExternalAction( nullptr != mapLayerAction )
          , mLayer( layer )
          , mFeatureId( fid )
          , mLevel( FeatureLevel )
          , mMapLayerAction( mapLayerAction )
      {}

      bool mIsValid;
      bool mAllResults;
      bool mIsExternalAction;
      QgsMapLayer* mLayer;
      QgsFeatureId mFeatureId;
      MenuLevel mLevel;
      QgsMapLayerAction* mMapLayerAction;
    };

    /**
     * @brief QgsIdentifyMenu is a menu to be used to choose within a list of QgsMapTool::IdentifyReults
     */
    explicit QgsIdentifyMenu( QgsMapCanvas* canvas );

    ~QgsIdentifyMenu();

    //! define if the menu executed can return multiple results (e.g. all results or all identified features of a vector layer)
    void setAllowMultipleReturn( bool multipleReturn ) { mAllowMultipleReturn = multipleReturn;}
    bool allowMultipleReturn() { return mAllowMultipleReturn;}

    //! define if the menu will be shown with a single idetify result
    void setExecWithSingleResult( bool execWithSingleResult ) { mExecWithSingleResult = execWithSingleResult;}
    bool execWithSingleResult() { return mExecWithSingleResult;}

    /**
     * @brief define if attribute actions(1) and map layer actions(2) can be listed and run from the menu
     * @note custom actions will be shown in any case if they exist.
     * @note (1) attribute actions are defined by the user in the layer properties @see QgsAction
     * @note (2) map layer actions are built-in c++ actions or actions which are defined by a python plugin @see QgsMapLayerActionRegistry
     */
    void setShowFeatureActions( bool showFeatureActions ) { mShowFeatureActions = showFeatureActions; }
    bool showFeatureActions() { return mShowFeatureActions;}

    /**
     * @brief setResultsIfExternalAction if set to false (default) the menu will not return any results if an external action has been triggered
     * @note external action can be either custom actions or feature / map layer actions (@see setShowFeatureActions)
     */
    void setResultsIfExternalAction( bool resultsIfExternalAction ) {mResultsIfExternalAction = resultsIfExternalAction;}
    bool resultsIfExternalAction() {return mResultsIfExternalAction;}

    //! Defines the maximimum number of layers displayed in the menu (default is 10).
    //! @note 0 is unlimited.
    void setMaxLayerDisplay( int maxLayerDisplay );
    int maxLayerDisplay() {return mMaxLayerDisplay;}

    //! Defines the maximimum number of features displayed in the menu for vector layers (default is 10).
    //! @note 0 is unlimited.
    void setMaxFeatureDisplay( int maxFeatureDisplay );
    int maxFeatureDisplay() {return mMaxFeatureDisplay;}

    //! adds a new custom action to the menu
    void addCustomAction( QgsMapLayerAction* action ) {mCustomActionRegistry.addMapLayerAction( action );}

    //! remove all custom actions from the menu to be built
    void removeCustomActions();

    /**
     * @brief exec
     * @param idResults the list of identify results to choose within
     * @param pos the position where the menu will be executed
     */
    QList<QgsMapToolIdentify::IdentifyResult> exec( const QList<QgsMapToolIdentify::IdentifyResult>& idResults, QPoint pos );

  protected:
    virtual void closeEvent( QCloseEvent *e ) override;

  private slots:
    void handleMenuHover();
    void deleteRubberBands();
    void layerDestroyed();
    void triggerMapLayerAction();

  private:

    //! adds a raster layer in the menu being built
    void addRasterLayer( QgsMapLayer* layer );

    //! adds a vector layer and its results in the menu being built
    //! if singleLayer is true, results will be displayed on the top level item (not in QMenu with the layer name)
    void addVectorLayer( QgsVectorLayer* layer, const QList<QgsMapToolIdentify::IdentifyResult>& results, bool singleLayer = false );

    //! get the lists of results corresponding to an action in the menu
    QList<QgsMapToolIdentify::IdentifyResult> results( QAction* action, bool& externalAction );

    QgsMapCanvas* mCanvas;
    QList<QgsHighlight*> mRubberBands;
    bool mAllowMultipleReturn;
    bool mExecWithSingleResult;
    bool mShowFeatureActions;
    bool mResultsIfExternalAction;
    int mMaxLayerDisplay;
    int mMaxFeatureDisplay;

    // name of the action to be displayed for feature default action, if other actions are shown
    QString mDefaultActionName;

    // custom menu actions regirstry
    CustomActionRegistry mCustomActionRegistry;

    // map layers with their results, this is the odering of the menu
    QMap <QgsMapLayer*, QList<QgsMapToolIdentify::IdentifyResult> > mLayerIdResults;
};

Q_DECLARE_METATYPE( QgsIdentifyMenu::ActionData )

#endif // QGSIDENTIFYMENU_H
