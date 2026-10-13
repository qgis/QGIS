/***************************************************************************
    qgsprocessingtoolboxdock.cpp
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingtoolboxdock.h"

#include "qgis.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingprovideractions.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingtoolboxmodel.h"
#include "qgssettings.h"

#include <QMenu>
#include <QString>
#include <QToolButton>
#include <QWidgetAction>

#include "moc_qgsprocessingtoolboxdock.cpp"

using namespace Qt::StringLiterals;

QgsProcessingToolboxDockWidget::QgsProcessingToolboxDockWidget( QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );

  setAllowedAreas( Qt::DockWidgetArea::LeftDockWidgetArea | Qt::DockWidgetArea::RightDockWidgetArea );

  mProcessingToolbar->setIconSize( QgsGui::iconSize( Qgis::UserInterfaceIconType::DockedToolbar ) );

  mAlgorithmTree->setRegistry( QgsApplication::processingRegistry(), QgsGui::processingRecentAlgorithmLog(), QgsGui::processingFavoriteAlgorithmManager() );

  mAlgorithmTree->setFilters( QgsProcessingToolboxProxyModel::Filter::Toolbox );
  mSearchBox->setShowSearchIcon( true );
  mSearchBox->setPlaceholderText( tr( "Search…" ) );
  connect( mSearchBox, &QgsFilterLineEdit::valueChanged, this, &QgsProcessingToolboxDockWidget::setFilterString );
  setFilterString( QString() );

  connect( mSearchBox, &QgsFilterLineEdit::returnPressed, this, &QgsProcessingToolboxDockWidget::executeCurrentAlgorithm );

  connect( mAlgorithmTree, &QgsProcessingToolboxTreeView::customContextMenuRequested, this, &QgsProcessingToolboxDockWidget::showPopupMenu );
  connect( mAlgorithmTree, &QgsProcessingToolboxTreeView::doubleClicked, this, &QgsProcessingToolboxDockWidget::executeCurrentAlgorithm );

  connect( mTxtTip, &QLabel::linkActivated, this, &QgsProcessingToolboxDockWidget::openSettings );
  mTxtTip->setVisible( hasDisabledProviders() );

  connect( QgsApplication::processingRegistry(), &QgsProcessingRegistry::providerAdded, this, &QgsProcessingToolboxDockWidget::providerAdded );
  connect( QgsApplication::processingRegistry(), &QgsProcessingRegistry::providerRemoved, this, &QgsProcessingToolboxDockWidget::providerRemoved );

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged, this, &QgsProcessingToolboxDockWidget::activeLayerChanged );

  connect( QgisApp::instance()->actionEditFeaturesInPlace(), &QAction::toggled, this, &QgsProcessingToolboxDockWidget::setInPlaceEditMode );

  connect( QgisApp::instance(), &QgisApp::activeLayerChanged, this, &QgsProcessingToolboxDockWidget::syncInPlaceEditState );

  syncInPlaceEditState();
}

void QgsProcessingToolboxDockWidget::initializeActions()
{
  mProcessingToolbar->addAction( QgisApp::instance()->actionProcessingHistory() );
  mProcessingToolbar->addSeparator();
  mProcessingToolbar->addAction( QgisApp::instance()->actionEditFeaturesInPlace() );
  mProcessingToolbar->addSeparator();
  auto optionsAction = new QAction( QgsApplication::getThemeIcon( u"/mActionOptions.svg"_s ), tr( "Options" ), this );
  optionsAction->setObjectName( "optionsAction" );
  connect( optionsAction, &QAction::triggered, this, [this] {
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), u"processingOptions"_s );
    mTxtTip->setVisible( hasDisabledProviders() );
  } );
  mProcessingToolbar->addAction( optionsAction );

  const QList<QgsProcessingProvider *> providers = QgsApplication::processingRegistry()->providers();
  for ( QgsProcessingProvider *provider : providers )
  {
    if ( provider->isActive() )
    {
      addProviderActions( provider );
    }
  }
}

void QgsProcessingToolboxDockWidget::setInPlaceEditMode( bool enabled )
{
  QgsProcessingToolboxProxyModel::Filters filters = QgsProcessingToolboxProxyModel::Filter::Toolbox;
  filters.setFlag( QgsProcessingToolboxProxyModel::Filter::ShowKnownIssues, QgsSettings().value( u"Processing/Configuration/SHOW_ALGORITHMS_KNOWN_ISSUES"_s, false ).toBool() );

  filters.setFlag( QgsProcessingToolboxProxyModel::Filter::InPlace, enabled );

  mAlgorithmTree->setFilters( filters );

  mInPlaceMode = enabled;
}

QToolBar *QgsProcessingToolboxDockWidget::toolBar()
{
  return mProcessingToolbar;
}

void QgsProcessingToolboxDockWidget::syncInPlaceEditState( QgsMapLayer *layer )
{
  if ( !layer )
  {
    layer = QgisApp::instance()->activeLayer();
  }

  QAction *editInPlaceAction = QgisApp::instance()->actionEditFeaturesInPlace();
  const bool oldEnabledState = editInPlaceAction->isEnabled();

  const bool newEnabledState = qobject_cast< QgsVectorLayer * >( layer );
  editInPlaceAction->setEnabled( newEnabledState );

  if ( newEnabledState != oldEnabledState )
  {
    setInPlaceEditMode( newEnabledState && editInPlaceAction->isChecked() );
  }
}

void QgsProcessingToolboxDockWidget::setFilterString( const QString &string )
{
  QgsProcessingToolboxProxyModel::Filters filters = mAlgorithmTree->filters();
  filters.setFlag( QgsProcessingToolboxProxyModel::Filter::ShowKnownIssues, QgsSettings().value( u"Processing/Configuration/SHOW_ALGORITHMS_KNOWN_ISSUES"_s, false ).toBool() );
  mAlgorithmTree->setFilters( filters );
  mAlgorithmTree->setFilterString( string );
}

void QgsProcessingToolboxDockWidget::showPopupMenu( const QPoint &pos )
{
  const QModelIndex index = mAlgorithmTree->indexAt( pos );
  const QgsProcessingAlgorithm *algorithm = mAlgorithmTree->algorithmForIndex( index );
  if ( !algorithm )
  {
    return;
  }

  auto menu = new QMenu();
  auto executeAction = new QAction( tr( "Execute…" ), menu );
  connect( executeAction, &QAction::triggered, this, &QgsProcessingToolboxDockWidget::executeCurrentAlgorithm );
  menu->addAction( executeAction );

  if ( algorithm->flags().testFlag( Qgis::ProcessingAlgorithmFlag::SupportsBatch ) )
  {
    auto executeBatchAction = new QAction( tr( "Execute as Batch Process…" ), menu );
    connect( executeBatchAction, &QAction::triggered, this, &QgsProcessingToolboxDockWidget::executeCurrentAlgorithmAsBatchProcess );
    menu->addAction( executeBatchAction );
  }
  menu->addSeparator();

  const QList<QgsProcessingToolboxContextAction *> actions = QgsGui::processingGuiRegistry()->toolboxContextActions();
  if ( !actions.isEmpty() )
  {
    menu->addSeparator();
  }
  for ( QgsProcessingToolboxContextAction *action : actions )
  {
    Q_NOWARN_DEPRECATED_PUSH
    action->setData( const_cast< QgsProcessingAlgorithm * >( algorithm ), this );
    Q_NOWARN_DEPRECATED_POP

    const QString providerId = algorithm->provider() ? algorithm->provider()->id() : QString();
    if ( action->isSeparator() )
    {
      menu->addSeparator();
    }
    else if ( action->isCompatibleWithAlgorithm( providerId, algorithm->name() ) )
    {
      auto contextMenuAction = new QAction( action->actionName(), menu );
      contextMenuAction->setIcon( action->icon() );
      const QString algorithmName = algorithm->name();
      connect( contextMenuAction, &QAction::triggered, this, [this, action, algorithmName, providerId] {
        QgsProcessingActionContext context;
        context.setParentWidget( this );
        context.setAlgorithmName( algorithmName );
        context.setProviderId( providerId );
        action->trigger( context );
      } );
      menu->addAction( contextMenuAction );
    }
  }

  menu->exec( mAlgorithmTree->mapToGlobal( pos ) );
}

void QgsProcessingToolboxDockWidget::executeCurrentAlgorithm()
{
  const QgsProcessingAlgorithm *algorithm = mAlgorithmTree->selectedAlgorithm();
  if ( !algorithm )
    return;

  QgsGui::instance()->emitExecuteAlgorithm( algorithm->id(), mInPlaceMode, false );
}

void QgsProcessingToolboxDockWidget::executeCurrentAlgorithmAsBatchProcess()
{
  const QgsProcessingAlgorithm *algorithm = mAlgorithmTree->selectedAlgorithm();
  if ( !algorithm )
    return;

  QgsGui::instance()->emitExecuteAlgorithm( algorithm->id(), mInPlaceMode, true );
}

void QgsProcessingToolboxDockWidget::openSettings( const QString &url )
{
  if ( url == "close"_L1 )
  {
    mTxtTip->setVisible( false );
    mTipWasClosed = true;
  }
  else
  {
    QgisApp::instance()->showOptionsDialog( QgisApp::instance(), u"processingOptions"_s );
    mTxtTip->setVisible( hasDisabledProviders() );
  }
}

void QgsProcessingToolboxDockWidget::providerAdded( const QString &id )
{
  if ( QgsProcessingProvider *provider = QgsApplication::processingRegistry()->providerById( id ) )
  {
    addProviderActions( provider );
  }
}

void QgsProcessingToolboxDockWidget::providerRemoved( const QString &id )
{
  if ( auto button = findChild<QToolButton *>( u"provideraction_%1"_s.arg( id ) ) )
  {
    button->deleteLater();
  }
}

void QgsProcessingToolboxDockWidget::activeLayerChanged( QgsMapLayer *layer )
{
  auto vl = qobject_cast< QgsVectorLayer * >( layer );
  if ( !vl )
    return;

  mAlgorithmTree->setInPlaceLayer( vl );
}

bool QgsProcessingToolboxDockWidget::hasDisabledProviders()
{
  const bool showTip = QgsSettings().value( u"Processing/Configuration/SHOW_PROVIDERS_TOOLTIP"_s, true ).toBool();
  if ( !showTip || mTipWasClosed )
    return false;

  const QList<QgsProcessingProvider *> providers = QgsApplication::processingRegistry()->providers();
  for ( QgsProcessingProvider *provider : providers )
  {
    if ( !provider->isActive() && provider->canBeActivated() )
    {
      return true;
    }
  }

  return false;
}

void QgsProcessingToolboxDockWidget::addProviderActions( QgsProcessingProvider *provider )
{
  const QList<QgsProcessingToolboxAction *> actions = QgsGui::processingGuiRegistry()->toolboxActionsForProvider( provider->id() );

  if ( actions.isEmpty() )
    return;

  auto toolbarButton = new QToolButton();
  toolbarButton->setObjectName( "provideraction_" + provider->id() );
  toolbarButton->setIcon( provider->icon() );
  toolbarButton->setToolTip( provider->name() );
  toolbarButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  auto menu = new QMenu( provider->name(), this );
  menu->setObjectName( provider->id() + "_menu" );
  for ( QgsProcessingToolboxAction *toolboxAction : actions )
  {
    Q_NOWARN_DEPRECATED_PUSH
    toolboxAction->setData( this );
    Q_NOWARN_DEPRECATED_POP

    auto action = new QAction( toolboxAction->actionName(), menu );
    action->setObjectName( toolboxAction->actionName() );
    connect( action, &QAction::triggered, this, [this, toolboxAction] {
      QgsProcessingActionContext context;
      context.setParentWidget( this );
      toolboxAction->trigger( context );
    } );
    menu->addAction( action );
  }
  toolbarButton->setMenu( menu );

  // provider action toolbuttons should come FIRST in the toolbar
  const QList< QAction * > existingActions = mProcessingToolbar->actions();

  QAction *insertBeforeAction = nullptr;
  bool foundProviderActions = false;
  for ( QAction *action : existingActions )
  {
    if ( action->isSeparator() )
    {
      if ( foundProviderActions )
      {
        // this is the separator after the last existing provider action, so we need to insert before this one
        insertBeforeAction = action;
        break;
      }
      else
      {
        continue;
      }
    }

    if ( auto widgetAction = qobject_cast< QWidgetAction * >( action ) )
    {
      if ( auto toolbutton = qobject_cast< QToolButton * >( widgetAction->defaultWidget() ) )
      {
        if ( toolbutton->objectName().startsWith( "provideraction"_L1 ) )
        {
          foundProviderActions = true;
          if ( toolbutton->toolTip().toLower().localeAwareCompare( provider->name().toLower() ) > 0 )
          {
            // found an existing provider action which is alphabetically after this provider, so we know where to
            // insert it now...
            insertBeforeAction = action;
            break;
          }
          else
          {
            // a provider action which comes before this one alphabetically, so move to next
            continue;
          }
        }
      }
    }
    insertBeforeAction = action;
    break;
  }

  if ( insertBeforeAction && foundProviderActions )
  {
    mProcessingToolbar->insertWidget( insertBeforeAction, toolbarButton );
  }
  else if ( insertBeforeAction )
  {
    auto separatorAction = new QAction();
    separatorAction->setSeparator( true );
    mProcessingToolbar->insertAction( insertBeforeAction, separatorAction );
    mProcessingToolbar->insertWidget( separatorAction, toolbarButton );
  }
  else
  {
    mProcessingToolbar->addWidget( toolbarButton );
  }
}
