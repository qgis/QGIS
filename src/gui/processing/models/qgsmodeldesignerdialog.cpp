/***************************************************************************
                             qgsmodeldesignerdialog.cpp
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldesignerdialog.h"
#include "qgssettings.h"
#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgsmessagebar.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingalgorithm.h"
#include "qgsgui.h"
#include "qgsprocessingparametertype.h"
#include "qgsmodelundocommand.h"
#include "qgsmodelviewtoolselect.h"
#include "qgsmodelviewtoolpan.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "processing/models/qgsprocessingmodelgroupbox.h"
#include "processing/models/qgsmodelinputreorderwidget.h"
#include "qgsmessageviewer.h"
#include "qgsmessagebaritem.h"
#include "qgspanelwidget.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsprocessinghelpeditorwidget.h"

#include <QShortcut>
#include <QDesktopWidget>
#include <QKeySequence>
#include <QFileDialog>
#include <QPrinter>
#include <QSvgGenerator>
#include <QToolButton>
#include <QCloseEvent>
#include <QMessageBox>
#include <QUndoView>
#include <QPushButton>
#include <QUrl>
#include <QTextStream>
#include <QActionGroup>

///@cond NOT_STABLE


QgsModelerToolboxModel::QgsModelerToolboxModel( QObject *parent )
  : QgsProcessingToolboxProxyModel( parent )
{

}

Qt::ItemFlags QgsModelerToolboxModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QgsProcessingToolboxProxyModel::flags( index );
  const QModelIndex sourceIndex = mapToSource( index );
  if ( toolboxModel()->isAlgorithm( sourceIndex ) )
  {
    f = f | Qt::ItemIsDragEnabled;
  }
  return f;
}

Qt::DropActions QgsModelerToolboxModel::supportedDragActions() const
{
  return Qt::CopyAction;
}



QgsModelDesignerDialog::QgsModelDesignerDialog( QWidget *parent, Qt::WindowFlags flags )
  : QMainWindow( parent, flags )
  , mToolsActionGroup( new QActionGroup( this ) )
{
  setupUi( this );

  setAttribute( Qt::WA_DeleteOnClose );
  setDockOptions( dockOptions() | QMainWindow::GroupedDragging );
  setWindowFlags( Qt::WindowMinimizeButtonHint |
                  Qt::WindowMaximizeButtonHint |
                  Qt::WindowCloseButtonHint );

  QgsGui::enableAutoGeometryRestore( this );

  mModel = std::make_unique< QgsProcessingModelAlgorithm >();
  mModel->setProvider( QgsApplication::processingRegistry()->providerById( QStringLiteral( "model" ) ) );

  mUndoStack = new QUndoStack( this );
  connect( mUndoStack, &QUndoStack::indexChanged, this, [ = ]
  {
    if ( mIgnoreUndoStackChanges )
      return;

    mBlockUndoCommands++;
    updateVariablesGui();
    mGroupEdit->setText( mModel->group() );
    mNameEdit->setText( mModel->displayName() );
    mBlockUndoCommands--;
    repaintModel();
  } );

  mPropertiesDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mInputsDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mAlgorithmsDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
  mVariablesDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable );

  mAlgorithmsTree->header()->setVisible( false );
  mAlgorithmSearchEdit->setShowSearchIcon( true );
  mAlgorithmSearchEdit->setPlaceholderText( tr( "Search…" ) );
  connect( mAlgorithmSearchEdit, &QgsFilterLineEdit::textChanged, mAlgorithmsTree, &QgsProcessingToolboxTreeView::setFilterString );

  mInputsTreeWidget->header()->setVisible( false );
  mInputsTreeWidget->setAlternatingRowColors( true );
  mInputsTreeWidget->setDragDropMode( QTreeWidget::DragOnly );
  mInputsTreeWidget->setDropIndicatorShown( true );

  mNameEdit->setPlaceholderText( tr( "Enter model name here" ) );
  mGroupEdit->setPlaceholderText( tr( "Enter group name here" ) );

  mMessageBar = new QgsMessageBar();
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  mainLayout->insertWidget( 0, mMessageBar );

  mView->setAcceptDrops( true );
  QgsSettings settings;

  connect( mActionClose, &QAction::triggered, this, &QWidget::close );
  connect( mActionNew, &QAction::triggered, this, &QgsModelDesignerDialog::newModel );
  connect( mActionZoomIn, &QAction::triggered, this, &QgsModelDesignerDialog::zoomIn );
  connect( mActionZoomOut, &QAction::triggered, this, &QgsModelDesignerDialog::zoomOut );
  connect( mActionZoomActual, &QAction::triggered, this, &QgsModelDesignerDialog::zoomActual );
  connect( mActionZoomToItems, &QAction::triggered, this, &QgsModelDesignerDialog::zoomFull );
  connect( mActionExportImage, &QAction::triggered, this, &QgsModelDesignerDialog::exportToImage );
  connect( mActionExportPdf, &QAction::triggered, this, &QgsModelDesignerDialog::exportToPdf );
  connect( mActionExportSvg, &QAction::triggered, this, &QgsModelDesignerDialog::exportToSvg );
  connect( mActionExportPython, &QAction::triggered, this, &QgsModelDesignerDialog::exportAsPython );
  connect( mActionSave, &QAction::triggered, this, [ = ] { saveModel( false ); } );
  connect( mActionSaveAs, &QAction::triggered, this, [ = ] { saveModel( true ); } );
  connect( mActionDeleteComponents, &QAction::triggered, this, &QgsModelDesignerDialog::deleteSelected );
  connect( mActionSnapSelected, &QAction::triggered, mView, &QgsModelGraphicsView::snapSelected );
  connect( mActionValidate, &QAction::triggered, this, &QgsModelDesignerDialog::validate );
  connect( mActionReorderInputs, &QAction::triggered, this, &QgsModelDesignerDialog::reorderInputs );
  connect( mActionEditHelp, &QAction::triggered, this, &QgsModelDesignerDialog::editHelp );
  connect( mReorderInputsButton, &QPushButton::clicked, this, &QgsModelDesignerDialog::reorderInputs );

  mActionSnappingEnabled->setChecked( settings.value( QStringLiteral( "/Processing/Modeler/enableSnapToGrid" ), false ).toBool() );
  connect( mActionSnappingEnabled, &QAction::toggled, this, [ = ]( bool enabled )
  {
    mView->snapper()->setSnapToGrid( enabled );
    QgsSettings().setValue( QStringLiteral( "/Processing/Modeler/enableSnapToGrid" ), enabled );
  } );
  mView->snapper()->setSnapToGrid( mActionSnappingEnabled->isChecked() );

  connect( mActionSelectAll, &QAction::triggered, this, [ = ]
  {
    mScene->selectAll();
  } );

  QStringList docksTitle = settings.value( QStringLiteral( "ModelDesigner/hiddenDocksTitle" ), QStringList(), QgsSettings::App ).toStringList();
  QStringList docksActive = settings.value( QStringLiteral( "ModelDesigner/hiddenDocksActive" ), QStringList(), QgsSettings::App ).toStringList();
  if ( !docksTitle.isEmpty() )
  {
    for ( const auto &title : docksTitle )
    {
      mPanelStatus.insert( title, PanelStatus( true, docksActive.contains( title ) ) );
    }
  }
  mActionHidePanels->setChecked( !docksTitle.isEmpty() );
  connect( mActionHidePanels, &QAction::toggled, this, &QgsModelDesignerDialog::setPanelVisibility );

  mUndoAction = mUndoStack->createUndoAction( this );
  mUndoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUndo.svg" ) ) );
  mUndoAction->setShortcuts( QKeySequence::Undo );
  mRedoAction = mUndoStack->createRedoAction( this );
  mRedoAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRedo.svg" ) ) );
  mRedoAction->setShortcuts( QKeySequence::Redo );

  mMenuEdit->insertAction( mActionDeleteComponents, mRedoAction );
  mMenuEdit->insertAction( mActionDeleteComponents, mUndoAction );
  mMenuEdit->insertSeparator( mActionDeleteComponents );
  mToolbar->insertAction( mActionZoomIn, mUndoAction );
  mToolbar->insertAction( mActionZoomIn, mRedoAction );
  mToolbar->insertSeparator( mActionZoomIn );

  mGroupMenu = new QMenu( tr( "Zoom To" ), this );
  mMenuView->insertMenu( mActionZoomIn, mGroupMenu );
  connect( mGroupMenu, &QMenu::aboutToShow, this, &QgsModelDesignerDialog::populateZoomToMenu );

  //cut/copy/paste actions. Note these are not included in the ui file
  //as ui files have no support for QKeySequence shortcuts
  mActionCut = new QAction( tr( "Cu&t" ), this );
  mActionCut->setShortcuts( QKeySequence::Cut );
  mActionCut->setStatusTip( tr( "Cut" ) );
  mActionCut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCut.svg" ) ) );
  connect( mActionCut, &QAction::triggered, this, [ = ]
  {
    mView->copySelectedItems( QgsModelGraphicsView::ClipboardCut );
  } );

  mActionCopy = new QAction( tr( "&Copy" ), this );
  mActionCopy->setShortcuts( QKeySequence::Copy );
  mActionCopy->setStatusTip( tr( "Copy" ) );
  mActionCopy->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditCopy.svg" ) ) );
  connect( mActionCopy, &QAction::triggered, this, [ = ]
  {
    mView->copySelectedItems( QgsModelGraphicsView::ClipboardCopy );
  } );

  mActionPaste = new QAction( tr( "&Paste" ), this );
  mActionPaste->setShortcuts( QKeySequence::Paste );
  mActionPaste->setStatusTip( tr( "Paste" ) );
  mActionPaste->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ) );
  connect( mActionPaste, &QAction::triggered, this, [ = ]
  {
    mView->pasteItems( QgsModelGraphicsView::PasteModeCursor );
  } );
  mMenuEdit->insertAction( mActionDeleteComponents, mActionCut );
  mMenuEdit->insertAction( mActionDeleteComponents, mActionCopy );
  mMenuEdit->insertAction( mActionDeleteComponents, mActionPaste );
  mMenuEdit->insertSeparator( mActionDeleteComponents );

  QgsProcessingToolboxProxyModel::Filters filters = QgsProcessingToolboxProxyModel::FilterModeler;
  if ( settings.value( QStringLiteral( "Processing/Configuration/SHOW_ALGORITHMS_KNOWN_ISSUES" ), false ).toBool() )
  {
    filters |= QgsProcessingToolboxProxyModel::FilterShowKnownIssues;
  }
  mAlgorithmsTree->setFilters( filters );
  mAlgorithmsTree->setDragDropMode( QTreeWidget::DragOnly );
  mAlgorithmsTree->setDropIndicatorShown( true );

  mAlgorithmsModel = new QgsModelerToolboxModel( this );
  mAlgorithmsTree->setToolboxProxyModel( mAlgorithmsModel );

  connect( mView, &QgsModelGraphicsView::algorithmDropped, this, [ = ]( const QString & algorithmId, const QPointF & pos )
  {
    addAlgorithm( algorithmId, pos );
  } );
  connect( mAlgorithmsTree, &QgsProcessingToolboxTreeView::doubleClicked, this, [ = ]()
  {
    if ( mAlgorithmsTree->selectedAlgorithm() )
      addAlgorithm( mAlgorithmsTree->selectedAlgorithm()->id(), QPointF() );
  } );
  connect( mInputsTreeWidget, &QgsModelDesignerInputsTreeWidget::doubleClicked, this, [ = ]( const QModelIndex & )
  {
    const QString parameterType = mInputsTreeWidget->currentItem()->data( 0, Qt::UserRole ).toString();
    addInput( parameterType, QPointF() );
  } );

  connect( mView, &QgsModelGraphicsView::inputDropped, this, &QgsModelDesignerDialog::addInput );

  // Ctrl+= should also trigger a zoom in action
  QShortcut *ctrlEquals = new QShortcut( QKeySequence( QStringLiteral( "Ctrl+=" ) ), this );
  connect( ctrlEquals, &QShortcut::activated, this, &QgsModelDesignerDialog::zoomIn );

  mUndoDock = new QgsDockWidget( tr( "Undo History" ), this );
  mUndoDock->setObjectName( QStringLiteral( "UndoDock" ) );
  mUndoView = new QUndoView( mUndoStack, this );
  mUndoDock->setWidget( mUndoView );
  mUndoDock->setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable );
  addDockWidget( Qt::DockWidgetArea::LeftDockWidgetArea, mUndoDock );

  tabifyDockWidget( mUndoDock, mPropertiesDock );
  tabifyDockWidget( mVariablesDock, mPropertiesDock );
  mPropertiesDock->raise();
  tabifyDockWidget( mInputsDock, mAlgorithmsDock );
  mInputsDock->raise();

  connect( mVariablesEditor, &QgsVariableEditorWidget::scopeChanged, this, [ = ]
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Variables" ) );
      mModel->setVariables( mVariablesEditor->variablesInActiveScope() );
      endUndoCommand();
    }
  } );
  connect( mNameEdit, &QLineEdit::textChanged, this, [ = ]( const QString & name )
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Name" ), NameChanged );
      mModel->setName( name );
      endUndoCommand();
      updateWindowTitle();
    }
  } );
  connect( mGroupEdit, &QLineEdit::textChanged, this, [ = ]( const QString & group )
  {
    if ( mModel )
    {
      beginUndoCommand( tr( "Change Model Group" ), GroupChanged );
      mModel->setGroup( group );
      endUndoCommand();
    }
  } );

  fillInputsTree();

  QToolButton *toolbuttonExportToScript = new QToolButton();
  toolbuttonExportToScript->setPopupMode( QToolButton::InstantPopup );
  toolbuttonExportToScript->addAction( mActionExportAsScriptAlgorithm );
  toolbuttonExportToScript->setDefaultAction( mActionExportAsScriptAlgorithm );
  mToolbar->insertWidget( mActionExportImage, toolbuttonExportToScript );
  connect( mActionExportAsScriptAlgorithm, &QAction::triggered, this, &QgsModelDesignerDialog::exportAsScriptAlgorithm );

  mActionShowComments->setChecked( settings.value( QStringLiteral( "/Processing/Modeler/ShowComments" ), true ).toBool() );
  connect( mActionShowComments, &QAction::toggled, this, &QgsModelDesignerDialog::toggleComments );

  mPanTool = new QgsModelViewToolPan( mView );
  mPanTool->setAction( mActionPan );

  mToolsActionGroup->addAction( mActionPan );
  connect( mActionPan, &QAction::triggered, mPanTool, [ = ] { mView->setTool( mPanTool ); } );

  mSelectTool = new QgsModelViewToolSelect( mView );
  mSelectTool->setAction( mActionSelectMoveItem );

  mToolsActionGroup->addAction( mActionSelectMoveItem );
  connect( mActionSelectMoveItem, &QAction::triggered, mSelectTool, [ = ] { mView->setTool( mSelectTool ); } );

  mView->setTool( mSelectTool );
  mView->setFocus();

  connect( mView, &QgsModelGraphicsView::macroCommandStarted, this, [ = ]( const QString & text )
  {
    mIgnoreUndoStackChanges++;
    mUndoStack->beginMacro( text );
    mIgnoreUndoStackChanges--;
  } );
  connect( mView, &QgsModelGraphicsView::macroCommandEnded, this, [ = ]
  {
    mIgnoreUndoStackChanges++;
    mUndoStack->endMacro();
    mIgnoreUndoStackChanges--;
  } );
  connect( mView, &QgsModelGraphicsView::beginCommand, this, [ = ]( const QString & text )
  {
    beginUndoCommand( text );
  } );
  connect( mView, &QgsModelGraphicsView::endCommand, this, [ = ]
  {
    endUndoCommand();
  } );
  connect( mView, &QgsModelGraphicsView::deleteSelectedItems, this, [ = ]
  {
    deleteSelected();
  } );

  connect( mActionAddGroupBox, &QAction::triggered, this, [ = ]
  {
    const QPointF viewCenter = mView->mapToScene( mView->viewport()->rect().center() );
    QgsProcessingModelGroupBox group;
    group.setPosition( viewCenter );
    group.setDescription( tr( "New Group" ) );

    beginUndoCommand( tr( "Add Group Box" ) );
    model()->addGroupBox( group );
    repaintModel();
    endUndoCommand();
  } );

  updateWindowTitle();

  // restore the toolbar and dock widgets positions using Qt settings API
  restoreState( settings.value( QStringLiteral( "ModelDesigner/state" ), QByteArray(), QgsSettings::App ).toByteArray() );
}

QgsModelDesignerDialog::~QgsModelDesignerDialog()
{
  QgsSettings settings;
  if ( !mPanelStatus.isEmpty() )
  {
    QStringList docksTitle;
    QStringList docksActive;

    for ( const auto &panel : mPanelStatus.toStdMap() )
    {
      if ( panel.second.isVisible )
        docksTitle << panel.first;
      if ( panel.second.isActive )
        docksActive << panel.first;
    }
    settings.setValue( QStringLiteral( "ModelDesigner/hiddenDocksTitle" ), docksTitle, QgsSettings::App );
    settings.setValue( QStringLiteral( "ModelDesigner/hiddenDocksActive" ), docksActive, QgsSettings::App );
  }
  else
  {
    settings.remove( QStringLiteral( "ModelDesigner/hiddenDocksTitle" ), QgsSettings::App );
    settings.remove( QStringLiteral( "ModelDesigner/hiddenDocksActive" ), QgsSettings::App );
  }

  // store the toolbar/dock widget settings using Qt settings API
  settings.setValue( QStringLiteral( "ModelDesigner/state" ), saveState(), QgsSettings::App );

  mIgnoreUndoStackChanges++;
  delete mSelectTool; // delete mouse handles before everything else
}

void QgsModelDesignerDialog::closeEvent( QCloseEvent *event )
{
  if ( checkForUnsavedChanges() )
    event->accept();
  else
    event->ignore();
}

void QgsModelDesignerDialog::beginUndoCommand( const QString &text, int id )
{
  if ( mBlockUndoCommands || !mUndoStack )
    return;

  if ( mActiveCommand )
    endUndoCommand();

  mActiveCommand = std::make_unique< QgsModelUndoCommand >( mModel.get(), text, id );
}

void QgsModelDesignerDialog::endUndoCommand()
{
  if ( mBlockUndoCommands || !mActiveCommand || !mUndoStack )
    return;

  mActiveCommand->saveAfterState();
  mIgnoreUndoStackChanges++;
  mUndoStack->push( mActiveCommand.release() );
  mIgnoreUndoStackChanges--;
  setDirty( true );
}

QgsProcessingModelAlgorithm *QgsModelDesignerDialog::model()
{
  return mModel.get();
}

void QgsModelDesignerDialog::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel.reset( model );

  mGroupEdit->setText( mModel->group() );
  mNameEdit->setText( mModel->displayName() );
  repaintModel();
  updateVariablesGui();

  mView->centerOn( 0, 0 );
  setDirty( false );

  mIgnoreUndoStackChanges++;
  mUndoStack->clear();
  mIgnoreUndoStackChanges--;

  updateWindowTitle();
}

void QgsModelDesignerDialog::loadModel( const QString &path )
{
  std::unique_ptr< QgsProcessingModelAlgorithm > alg = std::make_unique< QgsProcessingModelAlgorithm >();
  if ( alg->fromFile( path ) )
  {
    alg->setProvider( QgsApplication::processingRegistry()->providerById( QStringLiteral( "model" ) ) );
    alg->setSourceFilePath( path );
    setModel( alg.release() );
  }
  else
  {
    QgsMessageLog::logMessage( tr( "Could not load model %1" ).arg( path ), tr( "Processing" ), Qgis::MessageLevel::Critical );
    QMessageBox::critical( this, tr( "Open Model" ), tr( "The selected model could not be loaded.\n"
                           "See the log for more information." ) );
  }
}

void QgsModelDesignerDialog::setModelScene( QgsModelGraphicsScene *scene )
{
  QgsModelGraphicsScene *oldScene = mScene;

  mScene = scene;
  mScene->setParent( this );
  mScene->setChildAlgorithmResults( mChildResults );
  mScene->setModel( mModel.get() );
  mScene->setMessageBar( mMessageBar );

  const QPointF center = mView->mapToScene( mView->viewport()->rect().center() );
  mView->setModelScene( mScene );

  mSelectTool->resetCache();
  mSelectTool->setScene( mScene );

  connect( mScene, &QgsModelGraphicsScene::rebuildRequired, this, [ = ]
  {
    if ( mBlockRepaints )
      return;

    repaintModel();
  } );
  connect( mScene, &QgsModelGraphicsScene::componentAboutToChange, this, [ = ]( const QString & description, int id ) { beginUndoCommand( description, id ); } );
  connect( mScene, &QgsModelGraphicsScene::componentChanged, this, [ = ] { endUndoCommand(); } );

  mView->centerOn( center );

  if ( oldScene )
    oldScene->deleteLater();
}

void QgsModelDesignerDialog::activate()
{
  show();
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

void QgsModelDesignerDialog::updateVariablesGui()
{
  mBlockUndoCommands++;

  std::unique_ptr< QgsExpressionContextScope > variablesScope = std::make_unique< QgsExpressionContextScope >( tr( "Model Variables" ) );
  const QVariantMap modelVars = mModel->variables();
  for ( auto it = modelVars.constBegin(); it != modelVars.constEnd(); ++it )
  {
    variablesScope->setVariable( it.key(), it.value() );
  }
  QgsExpressionContext variablesContext;
  variablesContext.appendScope( variablesScope.release() );
  mVariablesEditor->setContext( &variablesContext );
  mVariablesEditor->setEditableScopeIndex( 0 );

  mBlockUndoCommands--;
}

void QgsModelDesignerDialog::setDirty( bool dirty )
{
  mHasChanged = dirty;
  updateWindowTitle();
}

bool QgsModelDesignerDialog::validateSave( SaveAction action )
{
  switch ( action )
  {
    case QgsModelDesignerDialog::SaveAction::SaveAsFile:
      break;
    case QgsModelDesignerDialog::SaveAction::SaveInProject:
      if ( mNameEdit->text().trimmed().isEmpty() )
      {
        mMessageBar->pushWarning( QString(), tr( "Please enter a model name before saving" ) );
        return false;
      }
      break;
  }

  return true;
}

bool QgsModelDesignerDialog::checkForUnsavedChanges()
{
  if ( isDirty() )
  {
    QMessageBox::StandardButton ret = QMessageBox::question( this, tr( "Save Model?" ),
                                      tr( "There are unsaved changes in this model. Do you want to keep those?" ),
                                      QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard, QMessageBox::Cancel );
    switch ( ret )
    {
      case QMessageBox::Save:
        return saveModel( false );

      case QMessageBox::Discard:
        return true;

      default:
        return false;
    }
  }
  else
  {
    return true;
  }
}

void QgsModelDesignerDialog::setLastRunChildAlgorithmResults( const QVariantMap &results )
{
  mChildResults = results;
  if ( mScene )
    mScene->setChildAlgorithmResults( mChildResults );
}

void QgsModelDesignerDialog::setLastRunChildAlgorithmInputs( const QVariantMap &inputs )
{
  mChildInputs = inputs;
  if ( mScene )
    mScene->setChildAlgorithmInputs( mChildInputs );
}

void QgsModelDesignerDialog::setModelName( const QString &name )
{
  mNameEdit->setText( name );
}

void QgsModelDesignerDialog::zoomIn()
{
  mView->setTransformationAnchor( QGraphicsView::NoAnchor );
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  QgsSettings settings;
  const double factor = settings.value( QStringLiteral( "/qgis/zoom_favor" ), 2.0 ).toDouble();
  mView->scale( factor, factor );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomOut()
{
  mView->setTransformationAnchor( QGraphicsView::NoAnchor );
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  QgsSettings settings;
  const double factor = 1.0 / settings.value( QStringLiteral( "/qgis/zoom_favor" ), 2.0 ).toDouble();
  mView->scale( factor, factor );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomActual()
{
  QPointF point = mView->mapToScene( QPoint( mView->viewport()->width() / 2.0, mView->viewport()->height() / 2 ) );
  mView->resetTransform();
  mView->scale( QgsApplication::desktop()->logicalDpiX() / 96, QgsApplication::desktop()->logicalDpiX() / 96 );
  mView->centerOn( point );
}

void QgsModelDesignerDialog::zoomFull()
{
  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  mView->fitInView( totalRect, Qt::KeepAspectRatio );
}

void QgsModelDesignerDialog::newModel()
{
  if ( !checkForUnsavedChanges() )
    return;

  std::unique_ptr< QgsProcessingModelAlgorithm > alg = std::make_unique< QgsProcessingModelAlgorithm >();
  alg->setProvider( QgsApplication::processingRegistry()->providerById( QStringLiteral( "model" ) ) );
  setModel( alg.release() );
}

void QgsModelDesignerDialog::exportToImage()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as Image" ), tr( "PNG files (*.png *.PNG)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "png" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF imageRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QImage img( totalRect.width(), totalRect.height(),
              QImage::Format_ARGB32_Premultiplied );
  img.fill( Qt::white );
  QPainter painter;
  painter.setRenderHint( QPainter::Antialiasing );
  painter.begin( &img );
  mView->scene()->render( &painter, imageRect, totalRect );
  painter.end();

  img.save( filename );

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as image to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::MessageLevel::Success, 0 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportToPdf()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as PDF" ), tr( "PDF files (*.pdf *.PDF)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "pdf" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF printerRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QPrinter printer;
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( filename );
  printer.setPaperSize( QSizeF( printerRect.width(), printerRect.height() ), QPrinter::DevicePixel );
  printer.setFullPage( true );

  QPainter painter( &printer );
  mView->scene()->render( &painter, printerRect, totalRect );
  painter.end();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as PDF to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( filename ).toString(),
                            QDir::toNativeSeparators( filename ) ), Qgis::MessageLevel::Success, 0 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportToSvg()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as SVG" ), tr( "SVG files (*.svg *.SVG)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "svg" ) );

  repaintModel( false );

  QRectF totalRect = mView->scene()->itemsBoundingRect();
  totalRect.adjust( -10, -10, 10, 10 );
  const QRectF svgRect = QRectF( 0, 0, totalRect.width(), totalRect.height() );

  QSvgGenerator svg;
  svg.setFileName( filename );
  svg.setSize( QSize( totalRect.width(), totalRect.height() ) );
  svg.setViewBox( svgRect );
  svg.setTitle( mModel->displayName() );

  QPainter painter( &svg );
  mView->scene()->render( &painter, svgRect, totalRect );
  painter.end();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as SVG to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::MessageLevel::Success, 0 );
  repaintModel( true );
}

void QgsModelDesignerDialog::exportAsPython()
{
  QString filename = QFileDialog::getSaveFileName( this, tr( "Save Model as Python Script" ), tr( "Processing scripts (*.py *.PY)" ) );
  if ( filename.isEmpty() )
    return;

  filename = QgsFileUtils::ensureFileNameHasExtension( filename, QStringList() << QStringLiteral( "py" ) );

  const QString text = mModel->asPythonCode( QgsProcessing::PythonQgsProcessingAlgorithmSubclass, 4 ).join( '\n' );

  QFile outFile( filename );
  if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return;
  }
  QTextStream fout( &outFile );
  fout << text;
  outFile.close();

  mMessageBar->pushMessage( QString(), tr( "Successfully exported model as Python script to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( filename ).toString(), QDir::toNativeSeparators( filename ) ), Qgis::MessageLevel::Success, 0 );
}

void QgsModelDesignerDialog::toggleComments( bool show )
{
  QgsSettings().setValue( QStringLiteral( "/Processing/Modeler/ShowComments" ), show );

  repaintModel( true );
}

void QgsModelDesignerDialog::updateWindowTitle()
{
  QString title = tr( "Model Designer" );
  if ( !mModel->name().isEmpty() )
    title = QStringLiteral( "%1 - %2" ).arg( title, mModel->name() );

  if ( isDirty() )
    title.prepend( '*' );

  setWindowTitle( title );
}

void QgsModelDesignerDialog::deleteSelected()
{
  QList< QgsModelComponentGraphicItem * > items = mScene->selectedComponentItems();
  if ( items.empty() )
    return;

  if ( items.size() == 1 )
  {
    items.at( 0 )->deleteComponent();
    return;
  }

  std::sort( items.begin(), items.end(), []( QgsModelComponentGraphicItem * p1, QgsModelComponentGraphicItem * p2 )
  {
    // try to delete the easy stuff first, so comments, then outputs, as nothing will depend on these...
    if ( dynamic_cast< QgsModelCommentGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelCommentGraphicItem *>( p2 ) )
      return false;
    else if ( dynamic_cast< QgsModelGroupBoxGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelGroupBoxGraphicItem *>( p2 ) )
      return false;
    else if ( dynamic_cast< QgsModelOutputGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelOutputGraphicItem *>( p2 ) )
      return false;
    else if ( dynamic_cast< QgsModelChildAlgorithmGraphicItem *>( p1 ) )
      return true;
    else if ( dynamic_cast< QgsModelChildAlgorithmGraphicItem *>( p2 ) )
      return false;
    return false;
  } );


  beginUndoCommand( tr( "Delete Components" ) );

  QVariant prevState = mModel->toVariant();
  mBlockUndoCommands++;
  mBlockRepaints = true;
  bool failed = false;
  while ( !items.empty() )
  {
    QgsModelComponentGraphicItem *toDelete = nullptr;
    for ( QgsModelComponentGraphicItem *item : items )
    {
      if ( item->canDeleteComponent() )
      {
        toDelete = item;
        break;
      }
    }

    if ( !toDelete )
    {
      failed = true;
      break;
    }

    toDelete->deleteComponent();
    items.removeAll( toDelete );
  }

  if ( failed )
  {
    mModel->loadVariant( prevState );
    QMessageBox::warning( nullptr, QObject::tr( "Could not remove components" ),
                          QObject::tr( "Components depend on the selected items.\n"
                                       "Try to remove them before trying deleting these components." ) );
    mBlockUndoCommands--;
    mActiveCommand.reset();
  }
  else
  {
    mBlockUndoCommands--;
    endUndoCommand();
  }

  mBlockRepaints = false;
  repaintModel();
}

void QgsModelDesignerDialog::populateZoomToMenu()
{
  mGroupMenu->clear();
  for ( const QgsProcessingModelGroupBox &box : model()->groupBoxes() )
  {
    if ( QgsModelComponentGraphicItem *item = mScene->groupBoxItem( box.uuid() ) )
    {
      QAction *zoomAction = new QAction( box.description(), mGroupMenu );
      connect( zoomAction, &QAction::triggered, this, [ = ]
      {
        QRectF groupRect = item->mapToScene( item->boundingRect() ).boundingRect();
        groupRect.adjust( -10, -10, 10, 10 );
        mView->fitInView( groupRect, Qt::KeepAspectRatio );
        mView->centerOn( item );
      } );
      mGroupMenu->addAction( zoomAction );
    }
  }
}

void QgsModelDesignerDialog::setPanelVisibility( bool hidden )
{
  const QList<QDockWidget *> docks = findChildren<QDockWidget *>();
  const QList<QTabBar *> tabBars = findChildren<QTabBar *>();

  if ( hidden )
  {
    mPanelStatus.clear();
    //record status of all docks
    for ( QDockWidget *dock : docks )
    {
      mPanelStatus.insert( dock->windowTitle(), PanelStatus( dock->isVisible(), false ) );
      dock->setVisible( false );
    }

    //record active dock tabs
    for ( QTabBar *tabBar : tabBars )
    {
      QString currentTabTitle = tabBar->tabText( tabBar->currentIndex() );
      mPanelStatus[ currentTabTitle ].isActive = true;
    }
  }
  else
  {
    //restore visibility of all docks
    for ( QDockWidget *dock : docks )
    {
      if ( mPanelStatus.contains( dock->windowTitle() ) )
      {
        dock->setVisible( mPanelStatus.value( dock->windowTitle() ).isVisible );
      }
    }

    //restore previously active dock tabs
    for ( QTabBar *tabBar : tabBars )
    {
      //loop through all tabs in tab bar
      for ( int i = 0; i < tabBar->count(); ++i )
      {
        QString tabTitle = tabBar->tabText( i );
        if ( mPanelStatus.contains( tabTitle ) && mPanelStatus.value( tabTitle ).isActive )
        {
          tabBar->setCurrentIndex( i );
        }
      }
    }
    mPanelStatus.clear();
  }
}

void QgsModelDesignerDialog::editHelp()
{
  QgsProcessingHelpEditorDialog dialog( this );
  dialog.setWindowTitle( tr( "Edit Model Help" ) );
  dialog.setAlgorithm( mModel.get() );
  if ( dialog.exec() )
  {
    beginUndoCommand( tr( "Edit Model Help" ) );
    mModel->setHelpContent( dialog.helpContent() );
    endUndoCommand();
  }
}

void QgsModelDesignerDialog::validate()
{
  QStringList issues;
  if ( model()->validate( issues ) )
  {
    mMessageBar->pushSuccess( QString(), tr( "Model is valid!" ) );
  }
  else
  {
    QgsMessageBarItem *messageWidget = QgsMessageBar::createMessage( QString(), tr( "Model is invalid!" ) );
    QPushButton *detailsButton = new QPushButton( tr( "Details" ) );
    connect( detailsButton, &QPushButton::clicked, detailsButton, [ = ]
    {
      QgsMessageViewer *dialog = new QgsMessageViewer( detailsButton );
      dialog->setTitle( tr( "Model is Invalid" ) );

      QString longMessage = tr( "<p>This model is not valid:</p>" ) + QStringLiteral( "<ul>" );
      for ( const QString &issue : issues )
      {
        longMessage += QStringLiteral( "<li>%1</li>" ).arg( issue );
      }
      longMessage += QLatin1String( "</ul>" );

      dialog->setMessage( longMessage, QgsMessageOutput::MessageHtml );
      dialog->showMessage();
    } );
    messageWidget->layout()->addWidget( detailsButton );
    mMessageBar->clearWidgets();
    mMessageBar->pushWidget( messageWidget, Qgis::MessageLevel::Warning, 0 );
  }
}

void QgsModelDesignerDialog::reorderInputs()
{
  QgsModelInputReorderDialog dlg( this );
  dlg.setModel( mModel.get() );
  if ( dlg.exec() )
  {
    const QStringList inputOrder = dlg.inputOrder();
    beginUndoCommand( tr( "Reorder Inputs" ) );
    mModel->setParameterOrder( inputOrder );
    endUndoCommand();
  }
}

bool QgsModelDesignerDialog::isDirty() const
{
  return mHasChanged && mUndoStack->index() != -1;
}

void QgsModelDesignerDialog::fillInputsTree()
{
  const QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "mIconModelInput.svg" ) );
  std::unique_ptr< QTreeWidgetItem > parametersItem = std::make_unique< QTreeWidgetItem >();
  parametersItem->setText( 0, tr( "Parameters" ) );
  QList<QgsProcessingParameterType *> available = QgsApplication::processingRegistry()->parameterTypes();
  std::sort( available.begin(), available.end(), []( const QgsProcessingParameterType * a, const QgsProcessingParameterType * b ) -> bool
  {
    return QString::localeAwareCompare( a->name(), b->name() ) < 0;
  } );

  for ( QgsProcessingParameterType *param : std::as_const( available ) )
  {
    if ( param->flags() & QgsProcessingParameterType::ExposeToModeler )
    {
      std::unique_ptr< QTreeWidgetItem > paramItem = std::make_unique< QTreeWidgetItem >();
      paramItem->setText( 0, param->name() );
      paramItem->setData( 0, Qt::UserRole, param->id() );
      paramItem->setIcon( 0, icon );
      paramItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
      paramItem->setToolTip( 0, param->description() );
      parametersItem->addChild( paramItem.release() );
    }
  }
  mInputsTreeWidget->addTopLevelItem( parametersItem.release() );
  mInputsTreeWidget->topLevelItem( 0 )->setExpanded( true );
}


//
// QgsModelChildDependenciesWidget
//

QgsModelChildDependenciesWidget::QgsModelChildDependenciesWidget( QWidget *parent,  QgsProcessingModelAlgorithm *model, const QString &childId )
  : QWidget( parent )
  , mModel( model )
  , mChildId( childId )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );

  mLineEdit = new QLineEdit();
  mLineEdit->setEnabled( false );
  hl->addWidget( mLineEdit, 1 );

  mToolButton = new QToolButton();
  mToolButton->setText( QString( QChar( 0x2026 ) ) );
  hl->addWidget( mToolButton );

  setLayout( hl );

  mLineEdit->setText( tr( "%1 dependencies selected" ).arg( 0 ) );

  connect( mToolButton, &QToolButton::clicked, this, &QgsModelChildDependenciesWidget::showDialog );
}

void QgsModelChildDependenciesWidget::setValue( const QList<QgsProcessingModelChildDependency> &value )
{
  mValue = value;

  updateSummaryText();
}

void QgsModelChildDependenciesWidget::showDialog()
{
  const QList<QgsProcessingModelChildDependency> available = mModel->availableDependenciesForChildAlgorithm( mChildId );

  QVariantList availableOptions;
  for ( const QgsProcessingModelChildDependency &dep : available )
    availableOptions << QVariant::fromValue( dep );
  QVariantList selectedOptions;
  for ( const QgsProcessingModelChildDependency &dep : mValue )
    selectedOptions << QVariant::fromValue( dep );

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel )
  {
    QgsProcessingMultipleSelectionPanelWidget *widget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, selectedOptions );
    widget->setPanelTitle( tr( "Algorithm Dependencies" ) );

    widget->setValueFormatter( [ = ]( const QVariant & v ) -> QString
    {
      const QgsProcessingModelChildDependency dep = v.value< QgsProcessingModelChildDependency >();

      const QString description = mModel->childAlgorithm( dep.childId ).description();
      if ( dep.conditionalBranch.isEmpty() )
        return description;
      else
        return tr( "Condition “%1” from algorithm “%2”" ).arg( dep.conditionalBranch, description );
    } );

    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged, this, [ = ]()
    {
      QList< QgsProcessingModelChildDependency > res;
      for ( const QVariant &v : widget->selectedOptions() )
      {
        res << v.value< QgsProcessingModelChildDependency >();
      }
      setValue( res );
    } );
    connect( widget, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked, widget, &QgsPanelWidget::acceptPanel );
    panel->openPanel( widget );
  }
}

void QgsModelChildDependenciesWidget::updateSummaryText()
{
  mLineEdit->setText( tr( "%n dependencies selected", nullptr, mValue.count() ) );
}

///@endcond
