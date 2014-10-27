/***************************************************************************
    qgsdualview.cpp
     --------------------------------------
    Date                 : 10.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsattributeaction.h"
#include "qgsattributeform.h"
#include "qgsattributetablemodel.h"
#include "qgsdualview.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfeaturelistmodel.h"
#include "qgsifeatureselectionmanager.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmessagelog.h"
#include "qgsvectordataprovider.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayercache.h"

#include <QDialog>
#include <QMenu>
#include <QMessageBox>
#include <QProgressDialog>

QgsDualView::QgsDualView( QWidget* parent )
    : QStackedWidget( parent )
    , mEditorContext()
    , mMasterModel( 0 )
    , mFilterModel( 0 )
    , mFeatureListModel( 0 )
    , mAttributeForm( 0 )
    , mLayerCache( 0 )
    , mProgressDlg( 0 )
    , mFeatureSelectionManager( 0 )
{
  setupUi( this );

  mPreviewActionMapper = new QSignalMapper( this );

  mPreviewColumnsMenu = new QMenu( this );
  mActionPreviewColumnsMenu->setMenu( mPreviewColumnsMenu );

  // Set preview icon
  mActionExpressionPreview->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionPreview.svg" ) );

  // Connect layer list preview signals
  connect( mActionExpressionPreview, SIGNAL( triggered() ), SLOT( previewExpressionBuilder() ) );
  connect( mPreviewActionMapper, SIGNAL( mapped( QObject* ) ), SLOT( previewColumnChanged( QObject* ) ) );
  connect( mFeatureList, SIGNAL( displayExpressionChanged( QString ) ), this, SLOT( previewExpressionChanged( QString ) ) );
}

void QgsDualView::init( QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, const QgsFeatureRequest &request, QgsAttributeEditorContext context )
{
  mEditorContext = context;

  connect( mTableView, SIGNAL( willShowContextMenu( QMenu*, QModelIndex ) ), this, SLOT( viewWillShowContextMenu( QMenu*, QModelIndex ) ) );

  initLayerCache( layer );
  initModels( mapCanvas, request );

  mTableView->setModel( mFilterModel );
  mFeatureList->setModel( mFeatureListModel );
  mAttributeForm = new QgsAttributeForm( layer, QgsFeature(), mEditorContext );
  mAttributeEditorScrollArea->setLayout( new QGridLayout() );
  mAttributeEditorScrollArea->layout()->addWidget( mAttributeForm );
  mAttributeEditorScrollArea->setWidget( mAttributeForm );

  mAttributeForm->hideButtonBox();

  connect( mAttributeForm, SIGNAL( attributeChanged( QString, QVariant ) ), this, SLOT( featureFormAttributeChanged() ) );

  if ( mFeatureListPreviewButton->defaultAction() )
    mFeatureList->setDisplayExpression( mDisplayExpression );
  else
    columnBoxInit();
}

void QgsDualView::columnBoxInit()
{
  // load fields
  QList<QgsField> fields = mLayerCache->layer()->pendingFields().toList();

  QString defaultField;

  // default expression: saved value
  QString displayExpression = mLayerCache->layer()->displayExpression();

  // if no display expression is saved: use display field instead
  if ( displayExpression == "" )
  {
    if ( mLayerCache->layer()->displayField() != "" )
    {
      defaultField = mLayerCache->layer()->displayField();
      displayExpression = QString( "COALESCE(\"%1\", '<NULL>')" ).arg( defaultField );
    }
  }

  // if neither diaplay expression nor display field is saved...
  if ( displayExpression == "" )
  {
    QgsAttributeList pkAttrs = mLayerCache->layer()->pendingPkAttributesList();

    if ( pkAttrs.size() > 0 )
    {
      if ( pkAttrs.size() == 1 )
        defaultField = pkAttrs.at( 0 );

      // ... If there are primary key(s) defined
      QStringList pkFields;

      Q_FOREACH ( int attr, pkAttrs )
      {
        pkFields.append( "COALESCE(\"" + fields[attr].name() + "\", '<NULL>')" );
      }

      displayExpression = pkFields.join( "||', '||" );
    }
    else if ( fields.size() > 0 )
    {
      if ( fields.size() == 1 )
        defaultField = fields.at( 0 ).name();

      // ... concat all fields
      QStringList fieldNames;
      foreach ( QgsField field, fields )
      {
        fieldNames.append( "COALESCE(\"" + field.name() + "\", '<NULL>')" );
      }

      displayExpression = fieldNames.join( "||', '||" );
    }
    else
    {
      // ... there isn't really much to display
      displayExpression = "'[Please define preview text]'";
    }
  }

  mFeatureListPreviewButton->addAction( mActionExpressionPreview );
  mFeatureListPreviewButton->addAction( mActionPreviewColumnsMenu );

  Q_FOREACH ( const QgsField& field, fields )
  {
    if ( mLayerCache->layer()->editorWidgetV2( mLayerCache->layer()->fieldNameIndex( field.name() ) ) != "Hidden" )
    {
      QIcon icon = QgsApplication::getThemeIcon( "/mActionNewAttribute.png" );
      QString text = field.name();

      // Generate action for the preview popup button of the feature list
      QAction* previewAction = new QAction( icon, text, mFeatureListPreviewButton );
      mPreviewActionMapper->setMapping( previewAction, previewAction );
      connect( previewAction, SIGNAL( triggered() ), mPreviewActionMapper, SLOT( map() ) );
      mPreviewColumnsMenu->addAction( previewAction );

      if ( text == defaultField )
      {
        mFeatureListPreviewButton->setDefaultAction( previewAction );
      }
    }
  }

  // If there is no single field found as preview
  if ( !mFeatureListPreviewButton->defaultAction() )
  {
    mFeatureList->setDisplayExpression( displayExpression );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mDisplayExpression = mFeatureList->displayExpression();
  }
  else
  {
    mFeatureListPreviewButton->defaultAction()->trigger();
  }
}

void QgsDualView::setView( QgsDualView::ViewMode view )
{
  setCurrentIndex( view );
}

void QgsDualView::setFilterMode( QgsAttributeTableFilterModel::FilterMode filterMode )
{
  mFilterModel->setFilterMode( filterMode );
  emit filterChanged();
}

void QgsDualView::setSelectedOnTop( bool selectedOnTop )
{
  mFilterModel->setSelectedOnTop( selectedOnTop );
}

void QgsDualView::initLayerCache( QgsVectorLayer* layer )
{
  // Initialize the cache
  QSettings settings;
  int cacheSize = settings.value( "/qgis/attributeTableRowCache", "10000" ).toInt();
  mLayerCache = new QgsVectorLayerCache( layer, cacheSize, this );
  mLayerCache->setCacheGeometry( false );
  if ( 0 == cacheSize || 0 == ( QgsVectorDataProvider::SelectAtId & mLayerCache->layer()->dataProvider()->capabilities() ) )
  {
    connect( mLayerCache, SIGNAL( progress( int, bool & ) ), this, SLOT( progress( int, bool & ) ) );
    connect( mLayerCache, SIGNAL( finished() ), this, SLOT( finished() ) );

    mLayerCache->setFullCache( true );
  }
}

void QgsDualView::initModels( QgsMapCanvas* mapCanvas, const QgsFeatureRequest& request )
{
  delete mFeatureListModel;
  delete mFilterModel;
  delete mMasterModel;

  mMasterModel = new QgsAttributeTableModel( mLayerCache, this );
  mMasterModel->setRequest( request );
  mMasterModel->setEditorContext( mEditorContext );

  connect( mMasterModel, SIGNAL( progress( int, bool & ) ), this, SLOT( progress( int, bool & ) ) );
  connect( mMasterModel, SIGNAL( finished() ), this, SLOT( finished() ) );

  mMasterModel->loadLayer();

  mFilterModel = new QgsAttributeTableFilterModel( mapCanvas, mMasterModel, mMasterModel );

  connect( mFeatureList, SIGNAL( displayExpressionChanged( QString ) ), this, SIGNAL( displayExpressionChanged( QString ) ) );

  mFeatureListModel = new QgsFeatureListModel( mFilterModel, mFilterModel );
}

void QgsDualView::on_mFeatureList_aboutToChangeEditSelection( bool& ok )
{
  if ( mLayerCache->layer()->isEditable() && !mAttributeForm->save() )
    ok = false;
}

void QgsDualView::on_mFeatureList_currentEditSelectionChanged( const QgsFeature &feat )
{
  if ( !mLayerCache->layer()->isEditable() || mAttributeForm->save() )
  {
    mAttributeForm->setFeature( feat );
    setCurrentEditSelection( QgsFeatureIds() << feat.id() );
  }
  else
  {
    // Couldn't save feature
  }
}

void QgsDualView::setCurrentEditSelection( const QgsFeatureIds& fids )
{
  mFeatureList->setCurrentFeatureEdited( false );
  mFeatureList->setEditSelection( fids );
}

bool QgsDualView::saveEditChanges()
{
  return mAttributeForm->save();
}

void QgsDualView::previewExpressionBuilder()
{
  // Show expression builder
  QgsExpressionBuilderDialog dlg( mLayerCache->layer(), mFeatureList->displayExpression(), this );
  dlg.setWindowTitle( tr( "Expression based preview" ) );
  dlg.setExpressionText( mFeatureList->displayExpression() );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFeatureList->setDisplayExpression( dlg.expressionText() );
    mFeatureListPreviewButton->setDefaultAction( mActionExpressionPreview );
    mFeatureListPreviewButton->setPopupMode( QToolButton::MenuButtonPopup );
  }

  mDisplayExpression = mFeatureList->displayExpression();
}

void QgsDualView::previewColumnChanged( QObject* action )
{
  QAction* previewAction = qobject_cast< QAction* >( action );

  if ( previewAction )
  {
    if ( !mFeatureList->setDisplayExpression( QString( "COALESCE( \"%1\", '<NULL>' )" ).arg( previewAction->text() ) ) )
    {
      QMessageBox::warning( this,
                            tr( "Could not set preview column" ),
                            tr( "Could not set column '%1' as preview column.\nParser error:\n%2" )
                            .arg( previewAction->text() )
                            .arg( mFeatureList->parserErrorString() )
                          );
    }
    else
    {
      mFeatureListPreviewButton->setDefaultAction( previewAction );
      mFeatureListPreviewButton->setPopupMode( QToolButton::InstantPopup );
    }
  }

  mDisplayExpression = mFeatureList->displayExpression();

  Q_ASSERT( previewAction );
}

int QgsDualView::featureCount()
{
  return mMasterModel->rowCount();
}

int QgsDualView::filteredFeatureCount()
{
  return mFilterModel->rowCount();
}

void QgsDualView::viewWillShowContextMenu( QMenu* menu, QModelIndex atIndex )
{
  QModelIndex sourceIndex = mFilterModel->mapToSource( atIndex );

  //add user-defined actions to context menu
  if ( mLayerCache->layer()->actions()->size() != 0 )
  {

    QAction *a = menu->addAction( tr( "Run layer action" ) );
    a->setEnabled( false );

    for ( int i = 0; i < mLayerCache->layer()->actions()->size(); i++ )
    {
      const QgsAction &action = mLayerCache->layer()->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsAttributeTableAction *a = new QgsAttributeTableAction( action.name(), this, i, sourceIndex );
      menu->addAction( action.name(), a, SLOT( execute() ) );
    }
  }

  //add actions from QgsMapLayerActionRegistry to context menu
  QList<QgsMapLayerAction *> registeredActions = QgsMapLayerActionRegistry::instance()->mapLayerActions( mLayerCache->layer() );
  if ( registeredActions.size() > 0 )
  {
    //add a separator between user defined and standard actions
    menu->addSeparator();

    QList<QgsMapLayerAction*>::iterator actionIt;
    for ( actionIt = registeredActions.begin(); actionIt != registeredActions.end(); ++actionIt )
    {
      QgsAttributeTableMapLayerAction *a = new QgsAttributeTableMapLayerAction(( *actionIt )->text(), this, ( *actionIt ), sourceIndex );
      menu->addAction(( *actionIt )->text(), a, SLOT( execute() ) );
    }
  }

  menu->addSeparator();
  QgsAttributeTableAction *a = new QgsAttributeTableAction( tr( "Open form" ), this, -1, sourceIndex );
  menu->addAction( tr( "Open form" ), a, SLOT( featureForm() ) );
}

void QgsDualView::previewExpressionChanged( const QString expression )
{
  mLayerCache->layer()->setDisplayExpression( expression );
}

void QgsDualView::featureFormAttributeChanged()
{
  mFeatureList->setCurrentFeatureEdited( true );
}

void QgsDualView::setFilteredFeatures( QgsFeatureIds filteredFeatures )
{
  mFilterModel->setFilteredFeatures( filteredFeatures );
}

void QgsDualView::setRequest( const QgsFeatureRequest& request )
{
  mMasterModel->setRequest( request );
}

void QgsDualView::setFeatureSelectionManager( QgsIFeatureSelectionManager* featureSelectionManager )
{
  mTableView->setFeatureSelectionManager( featureSelectionManager );
  // mFeatureList->setFeatureSelectionManager( featureSelectionManager );

  if ( mFeatureSelectionManager && mFeatureSelectionManager->parent() == this )
    delete mFeatureSelectionManager;

  mFeatureSelectionManager = featureSelectionManager;
}

void QgsDualView::progress( int i, bool& cancel )
{
  if ( !mProgressDlg )
  {
    mProgressDlg = new QProgressDialog( tr( "Loading features..." ), tr( "Abort" ), 0, 0, this );
    mProgressDlg->setWindowTitle( tr( "Attribute table" ) );
    mProgressDlg->setWindowModality( Qt::WindowModal );
    mProgressDlg->show();
  }

  mProgressDlg->setValue( i );
  mProgressDlg->setLabelText( tr( "%1 features loaded." ).arg( i ) );

  QCoreApplication::processEvents();

  cancel = mProgressDlg->wasCanceled();
}

void QgsDualView::finished()
{
  delete mProgressDlg;
  mProgressDlg = 0;
}

/*
 * QgsAttributeTableAction
 */

void QgsAttributeTableAction::execute()
{
  mDualView->masterModel()->executeAction( mAction, mFieldIdx );
}

void QgsAttributeTableAction::featureForm()
{
  QgsFeatureIds editedIds;
  editedIds << mDualView->masterModel()->rowToId( mFieldIdx.row() );
  mDualView->setCurrentEditSelection( editedIds );
  mDualView->setView( QgsDualView::AttributeEditor );
}

/*
 * QgsAttributeTableMapLayerAction
 */

void QgsAttributeTableMapLayerAction::execute()
{
  mDualView->masterModel()->executeMapLayerAction( mAction, mFieldIdx );
}
