/***************************************************************************
                         qgsmaptoolsdigitizingtechniquemanager.cpp
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsdigitizingtechniquemanager.h"
#include "moc_qgsmaptoolsdigitizingtechniquemanager.cpp"
#include "qgisapp.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptoolshapecircle2points.h"
#include "qgsmaptoolshaperegistry.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsspinbox.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"


#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QActionGroup>

const QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique> *QgsMapToolsDigitizingTechniqueManager::settingsDigitizingTechnique = new QgsSettingsEntryEnumFlag<Qgis::CaptureTechnique>( QStringLiteral( "technique" ), QgsSettingsTree::sTreeDigitizing, Qgis::CaptureTechnique::StraightSegments, QObject::tr( "Current digitizing technique" ), Qgis::SettingsOption::SaveFormerValue ) SIP_SKIP;
const QgsSettingsEntryString *QgsMapToolsDigitizingTechniqueManager::settingMapToolShapeCurrent = new QgsSettingsEntryString( QStringLiteral( "current" ), sTreeShapeMapTools, QgsMapToolShapeCircle2PointsMetadata::TOOL_ID, QObject::tr( "Current shape map tool" ) ) SIP_SKIP;
const QgsSettingsEntryString *QgsMapToolsDigitizingTechniqueManager::settingMapToolShapeDefaultForCategory = new QgsSettingsEntryString( QStringLiteral( "default" ), sTreeShapeMapToolsCategories, QString(), QObject::tr( "Default map tool for given shape category" ) ) SIP_SKIP;

QgsMapToolsDigitizingTechniqueManager::QgsMapToolsDigitizingTechniqueManager( QObject *parent )
  : QObject( parent )
{
  mTechniqueActions.insert( Qgis::CaptureTechnique::StraightSegments, QgisApp::instance()->mActionDigitizeWithSegment );
  mTechniqueActions.insert( Qgis::CaptureTechnique::CircularString, QgisApp::instance()->mActionDigitizeWithCurve );
  mTechniqueActions.insert( Qgis::CaptureTechnique::Streaming, QgisApp::instance()->mActionStreamDigitize );
  mTechniqueActions.insert( Qgis::CaptureTechnique::Shape, QgisApp::instance()->mActionDigitizeShape );

  mDigitizeModeToolButton = new QToolButton();
  mDigitizeModeToolButton->setPopupMode( QToolButton::MenuButtonPopup );

  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::mapToolSet, this, &QgsMapToolsDigitizingTechniqueManager::mapToolSet );
}

void QgsMapToolsDigitizingTechniqueManager::setupCanvasTools()
{
  const QList<QgsMapToolCapture *> captureTools = QgisApp::instance()->captureTools();
  for ( QgsMapToolCapture *tool : captureTools )
  {
    setupTool( tool );
  }
}

void QgsMapToolsDigitizingTechniqueManager::setupToolBars()
{
  // digitize mode button
  QMenu *digitizeMenu = new QMenu( mDigitizeModeToolButton );
  QActionGroup *actionGroup = new QActionGroup( digitizeMenu );

  QMap<Qgis::CaptureTechnique, QAction *>::const_iterator it = mTechniqueActions.constBegin();
  for ( ; it != mTechniqueActions.constEnd(); ++it )
  {
    digitizeMenu->addAction( it.value() );
    actionGroup->addAction( it.value() );
  }
  QgisApp::instance()->mActionStreamDigitize->setShortcut( tr( "R", "Keyboard shortcut: toggle stream digitizing" ) );
  connect( digitizeMenu, &QMenu::triggered, this, [=]( QAction *action ) {
    Qgis::CaptureTechnique technique = mTechniqueActions.key( action, Qgis::CaptureTechnique::StraightSegments );
    if ( mDigitizeModeToolButton->defaultAction() != action )
    {
      setCaptureTechnique( technique );
    }
    else
    {
      Qgis::CaptureTechnique formerTechnique = settingsDigitizingTechnique->formerValue();
      setCaptureTechnique( formerTechnique );
    }
  } );

  mStreamDigitizingSettingsAction = new QgsStreamDigitizingSettingsAction( QgisApp::instance() );
  digitizeMenu->addSeparator();
  digitizeMenu->addAction( mStreamDigitizingSettingsAction );

  mDigitizeModeToolButton->setMenu( digitizeMenu );

  updateDigitizeModeButton( settingsDigitizingTechnique->value() );

  QgisApp::instance()->mDigitizeToolBar->insertWidget( QgisApp::instance()->mDigitizeToolBar->actions().at( 3 ), mDigitizeModeToolButton );

  // Digitizing shape tools
  const QList<QgsMapToolShapeMetadata *> mapTools = QgsGui::mapToolShapeRegistry()->mapToolMetadatas();
  for ( const QgsMapToolShapeMetadata *metadata : mapTools )
  {
    QToolButton *shapeButton = nullptr;
    if ( !mShapeCategoryButtons.contains( metadata->category() ) )
    {
      shapeButton = new QToolButton( QgisApp::instance()->mShapeDigitizeToolBar );
      shapeButton->setPopupMode( QToolButton::MenuButtonPopup );
      shapeButton->setMenu( new QMenu() );

      QgisApp::instance()->mShapeDigitizeToolBar->addWidget( shapeButton );
      QObject::connect( shapeButton, &QToolButton::triggered, this, [=]( QAction *action ) { setShapeTool( action->data().toString() ); } );

      mShapeCategoryButtons.insert( metadata->category(), shapeButton );
    }
    else
    {
      shapeButton = mShapeCategoryButtons[metadata->category()];
    }

    QMenu *shapeMenu = shapeButton->menu();
    QAction *action = new QAction( metadata->icon(), metadata->name(), shapeMenu );
    action->setCheckable( true );
    action->setData( metadata->id() );
    shapeMenu->addAction( action );
    QString defaultToolId = settingMapToolShapeDefaultForCategory->value( qgsEnumValueToKey( metadata->category() ) );
    if ( defaultToolId.isEmpty() )
    {
      // if no default tool for category, take the first one
      defaultToolId = metadata->id();
      settingMapToolShapeDefaultForCategory->setValue( metadata->id(), qgsEnumValueToKey( metadata->category() ) );
    }
    if ( defaultToolId == metadata->id() )
      shapeButton->setDefaultAction( action );

    mShapeActions.insert( metadata->id(), action );
  }

  // Remove the dropdown arrows from buttons with a single action
  QList<QToolButton *> buttons = QgisApp::instance()->mShapeDigitizeToolBar->findChildren<QToolButton *>();
  for ( QToolButton *button : buttons )
  {
    QMenu *menu = button->menu();
    if ( menu && button->menu()->actions().count() == 1 )
    {
      // Reparent action to button (otherwise it will be deleted with the menu)
      QAction *action = menu->actions().at( 0 );
      action->setParent( button );

      // Delete menu and set popup mode to delayed (remove dropdown arrow)
      button->setMenu( nullptr );
      button->setPopupMode( QToolButton::DelayedPopup );
      menu->deleteLater();
    }
  }
}

QgsMapToolsDigitizingTechniqueManager::~QgsMapToolsDigitizingTechniqueManager()
{
}

void QgsMapToolsDigitizingTechniqueManager::setCaptureTechnique( Qgis::CaptureTechnique technique, bool alsoSetShapeTool )
{
  settingsDigitizingTechnique->setValue( technique );

  mTechniqueActions.value( technique )->setChecked( true );

  updateDigitizeModeButton( technique );

  // QgisApp::captureTools returns all registered capture tools + the eventual current capture tool
  const QList<QgsMapToolCapture *> tools = QgisApp::instance()->captureTools();
  for ( QgsMapToolCapture *tool : tools )
  {
    if ( tool->supportsTechnique( technique ) )
    {
      tool->setCurrentCaptureTechnique( technique );
    }
  }

  if ( technique == Qgis::CaptureTechnique::Shape && alsoSetShapeTool )
  {
    setShapeTool( settingMapToolShapeCurrent->value() );
  }
  else if ( technique != Qgis::CaptureTechnique::Shape )
  {
    // uncheck all the shape tools
    QHash<QString, QAction *>::iterator sit = mShapeActions.begin();
    for ( ; sit != mShapeActions.end(); ++sit )
      sit.value()->setChecked( false );
  }
}

void QgsMapToolsDigitizingTechniqueManager::setShapeTool( const QString &shapeToolId )
{
  QAction *action = nullptr;

  const QgsMapToolShapeMetadata *md = QgsGui::mapToolShapeRegistry()->mapToolMetadata( shapeToolId );
  if ( md )
  {
    settingMapToolShapeDefaultForCategory->setValue( md->id(), qgsEnumValueToKey( md->category() ) );
    settingMapToolShapeCurrent->setValue( md->id() );
    QToolButton *bt = mShapeCategoryButtons.value( md->category() );
    action = mShapeActions.value( md->id() );
    if ( bt && action )
      bt->setDefaultAction( action );
  }
  QHash<QString, QAction *>::iterator sit = mShapeActions.begin();
  for ( ; sit != mShapeActions.end(); ++sit )
    sit.value()->setChecked( sit.value() == action );

  setCaptureTechnique( Qgis::CaptureTechnique::Shape, false );

  // QgisApp::captureTools returns all registered capture tools + the eventual current capture tool
  const QList<QgsMapToolCapture *> tools = QgisApp::instance()->captureTools();
  for ( QgsMapToolCapture *tool : tools )
  {
    if ( tool->supportsTechnique( Qgis::CaptureTechnique::Shape ) )
    {
      tool->setCurrentShapeMapTool( md );
    }
  }
}

void QgsMapToolsDigitizingTechniqueManager::mapToolSet( QgsMapTool *newTool, QgsMapTool * )
{
  if ( QgsMapToolCapture *captureTool = qobject_cast<QgsMapToolCapture *>( newTool ) )
  {
    if ( mInitializedTools.contains( captureTool ) )
      return;

    // this is a non-standard tool, e.g. a plugin tool. But still support setting the digitizing modes for those!
    enableDigitizingTechniqueActions( true, captureTool->action() );
  }
  else
  {
    enableDigitizingTechniqueActions( false, nullptr );
  }
}

void QgsMapToolsDigitizingTechniqueManager::setupTool( QgsMapToolCapture *tool )
{
  if ( tool->action() )
  {
    connect( tool->action(), &QAction::toggled, this, [this, tool]( bool checked ) { enableDigitizingTechniqueActions( checked, tool->action() ); } );
  }

  mInitializedTools.insert( tool );
  connect( tool, &QObject::destroyed, this, [=] {
    mInitializedTools.remove( tool );
  } );
}

void QgsMapToolsDigitizingTechniqueManager::updateDigitizeModeButton( const Qgis::CaptureTechnique technique )
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
      mDigitizeModeToolButton->setDefaultAction( QgisApp::instance()->mActionDigitizeWithSegment );
      break;
    case Qgis::CaptureTechnique::CircularString:
      mDigitizeModeToolButton->setDefaultAction( QgisApp::instance()->mActionDigitizeWithCurve );
      break;
    case Qgis::CaptureTechnique::Streaming:
      mDigitizeModeToolButton->setDefaultAction( QgisApp::instance()->mActionStreamDigitize );
      break;
    case Qgis::CaptureTechnique::Shape:
      mDigitizeModeToolButton->setDefaultAction( QgisApp::instance()->mActionDigitizeShape );
      break;
  }
}

void QgsMapToolsDigitizingTechniqueManager::enableDigitizingTechniqueActions( bool enabled, QAction *triggeredFromToolAction )
{
  QgsSettings settings;

  // QgisApp::captureTools returns all registered capture tools + the eventual current capture tool
  const QList<QgsMapToolCapture *> tools = QgisApp::instance()->captureTools();

  const Qgis::CaptureTechnique settingsCurrentTechnique = settingsDigitizingTechnique->value();
  const QString currentShapeToolId = settingMapToolShapeCurrent->value();

  QSet<Qgis::CaptureTechnique> supportedTechniques;

  QgsMapToolCapture *currentTool = nullptr;

  if ( enabled )
  {
    for ( QgsMapToolCapture *tool : tools )
    {
      if ( triggeredFromToolAction == tool->action() || ( !triggeredFromToolAction && QgisApp::instance()->mapCanvas()->mapTool() == tool ) )
      {
        currentTool = tool;
        for ( auto technique = mTechniqueActions.keyBegin(); technique != mTechniqueActions.keyEnd(); technique++ )
        {
          if ( tool->supportsTechnique( *technique ) )
            supportedTechniques.insert( *technique );
        }
        break;
      }
    }
  }

  // if the global current technique is not supported by the current tool,
  // the actual current technique is not the one stored in the settings
  Qgis::CaptureTechnique actualCurrentTechnique = settingsCurrentTechnique;
  if ( currentTool && !currentTool->supportsTechnique( settingsCurrentTechnique ) )
  {
    actualCurrentTechnique = currentTool->currentCaptureTechnique();
  }

  // Ensure the digitizing mode tool button is set to the correct action
  updateDigitizeModeButton( actualCurrentTechnique );

  QMap<Qgis::CaptureTechnique, QAction *>::const_iterator cit = mTechniqueActions.constBegin();
  for ( ; cit != mTechniqueActions.constEnd(); ++cit )
  {
    cit.value()->setEnabled( enabled && supportedTechniques.contains( cit.key() ) );
    cit.value()->setChecked( cit.value()->isEnabled() && actualCurrentTechnique == cit.key() );
  }

  QHash<QString, QAction *>::const_iterator sit = mShapeActions.constBegin();
  for ( ; sit != mShapeActions.constEnd(); ++sit )
  {
    sit.value()->setEnabled( enabled && supportedTechniques.contains( Qgis::CaptureTechnique::Shape ) );
    sit.value()->setChecked( actualCurrentTechnique == Qgis::CaptureTechnique::Shape && sit.value()->isEnabled() && sit.key() == currentShapeToolId );
  }

  for ( QgsMapToolCapture *tool : tools )
  {
    if ( tool->supportsTechnique( settingsCurrentTechnique ) )
    {
      tool->setCurrentCaptureTechnique( settingsCurrentTechnique );
      if ( settingsCurrentTechnique == Qgis::CaptureTechnique::Shape )
      {
        QgsMapToolShapeMetadata *md = QgsGui::mapToolShapeRegistry()->mapToolMetadata( settingMapToolShapeCurrent->value() );
        tool->setCurrentShapeMapTool( md );
      }
    }
  }
}

//
// QgsStreamDigitizingSettingsAction
//

QgsStreamDigitizingSettingsAction::QgsStreamDigitizingSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mStreamToleranceSpinBox = new QgsSpinBox();
  mStreamToleranceSpinBox->setSuffix( tr( "px" ) );
  mStreamToleranceSpinBox->setKeyboardTracking( false );
  mStreamToleranceSpinBox->setRange( 1, 200 );
  mStreamToleranceSpinBox->setWrapping( false );
  mStreamToleranceSpinBox->setSingleStep( 1 );
  mStreamToleranceSpinBox->setClearValue( 2 );
  mStreamToleranceSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingStreamTolerance->value() );

  QLabel *label = new QLabel( tr( "Streaming Tolerance" ) );
  gLayout->addWidget( label, 1, 0 );
  gLayout->addWidget( mStreamToleranceSpinBox, 1, 1 );
  connect( mStreamToleranceSpinBox, qOverload<int>( &QgsSpinBox::valueChanged ), this, [=]( int value ) {
    QgsSettingsRegistryCore::settingsDigitizingStreamTolerance->setValue( value );
  } );

  QWidget *w = new QWidget( parent );
  w->setLayout( gLayout );
  setDefaultWidget( w );
}

QgsStreamDigitizingSettingsAction::~QgsStreamDigitizingSettingsAction() = default;
