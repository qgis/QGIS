/***************************************************************************
    qgsrelationeditor.cpp
     --------------------------------------
    Date                 : 17.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationeditorwidget.h"

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeatureselectiondlg.h"
#include "qgsgenericfeatureselectionmanager.h"
#include "qgsrelation.h"
#include "qgsvectorlayertools.h"
#include "qgsproject.h"
#include "qgstransactiongroup.h"
#include "qgslogger.h"
#include "qgsvectorlayerutils.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayerselectionmanager.h"
#include "qgsmaptooldigitizefeature.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

/// @cond PRIVATE
///
QgsFilteredSelectionManager::QgsFilteredSelectionManager( QgsVectorLayer *layer, const QgsFeatureRequest &request, QObject *parent )
  : QgsVectorLayerSelectionManager( layer, parent )
  , mRequest( request )
{
  for ( auto fid : layer->selectedFeatureIds() )
    if ( mRequest.acceptFeature( layer->getFeature( fid ) ) )
      mSelectedFeatureIds << fid;

  connect( layer, &QgsVectorLayer::selectionChanged, this, &QgsFilteredSelectionManager::onSelectionChanged );
}

const QgsFeatureIds &QgsFilteredSelectionManager::selectedFeatureIds() const
{
  return mSelectedFeatureIds;
}

int QgsFilteredSelectionManager::selectedFeatureCount()
{
  return mSelectedFeatureIds.count();
}

void QgsFilteredSelectionManager::onSelectionChanged( const QgsFeatureIds &selected, const QgsFeatureIds &deselected, bool clearAndSelect )
{
  QgsFeatureIds lselected = selected;
  if ( clearAndSelect )
  {
    mSelectedFeatureIds.clear();
  }
  else
  {
    for ( auto fid : deselected )
      mSelectedFeatureIds.remove( fid );
  }

  for ( auto fid : selected )
    if ( mRequest.acceptFeature( layer()->getFeature( fid ) ) )
      mSelectedFeatureIds << fid;
    else
      lselected.remove( fid );

  emit selectionChanged( lselected, deselected, clearAndSelect );
}

/// @endcond

QgsRelationEditorWidget::QgsRelationEditorWidget( QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setContentsMargins( 0, 9, 0, 0 );
  setLayout( topLayout );

  // buttons
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins( 0, 0, 0, 0 );
  // toggle editing
  mToggleEditingButton = new QToolButton( this );
  mToggleEditingButton->setObjectName( QStringLiteral( "mToggleEditingButton" ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mToggleEditingButton->setText( tr( "Toggle Editing" ) );
  mToggleEditingButton->setEnabled( false );
  mToggleEditingButton->setCheckable( true );
  mToggleEditingButton->setToolTip( tr( "Toggle editing mode for child layer" ) );
  buttonLayout->addWidget( mToggleEditingButton );
  // save Edits
  mSaveEditsButton = new QToolButton( this );
  mSaveEditsButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveEdits.svg" ) ) );
  mSaveEditsButton->setText( tr( "Save Child Layer Edits" ) );
  mSaveEditsButton->setToolTip( tr( "Save child layer edits" ) );
  mSaveEditsButton->setEnabled( true );
  buttonLayout->addWidget( mSaveEditsButton );
  // add feature with geometry
  mAddFeatureGeometryButton = new QToolButton( this );
  mAddFeatureGeometryButton->setObjectName( QStringLiteral( "mAddFeatureGeometryButton" ) );
  buttonLayout->addWidget( mAddFeatureGeometryButton );
  // add feature
  mAddFeatureButton = new QToolButton( this );
  mAddFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewTableRow.svg" ) ) );
  mAddFeatureButton->setText( tr( "Add Child Feature" ) );
  mAddFeatureButton->setToolTip( tr( "Add child feature" ) );
  mAddFeatureButton->setObjectName( QStringLiteral( "mAddFeatureButton" ) );
  buttonLayout->addWidget( mAddFeatureButton );
  // duplicate feature
  mDuplicateFeatureButton = new QToolButton( this );
  mDuplicateFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDuplicateFeature.svg" ) ) );
  mDuplicateFeatureButton->setText( tr( "Duplicate Child Feature" ) );
  mDuplicateFeatureButton->setToolTip( tr( "Duplicate child feature" ) );
  mDuplicateFeatureButton->setObjectName( QStringLiteral( "mDuplicateFeatureButton" ) );
  buttonLayout->addWidget( mDuplicateFeatureButton );
  // delete feature
  mDeleteFeatureButton = new QToolButton( this );
  mDeleteFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ) );
  mDeleteFeatureButton->setText( tr( "Delete Child Feature" ) );
  mDeleteFeatureButton->setToolTip( tr( "Delete child feature" ) );
  mDeleteFeatureButton->setObjectName( QStringLiteral( "mDeleteFeatureButton" ) );
  buttonLayout->addWidget( mDeleteFeatureButton );
  // link feature
  mLinkFeatureButton = new QToolButton( this );
  mLinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLink.svg" ) ) );
  mLinkFeatureButton->setText( tr( "Link Existing Features" ) );
  mLinkFeatureButton->setToolTip( tr( "Link existing child features" ) );
  mLinkFeatureButton->setObjectName( QStringLiteral( "mLinkFeatureButton" ) );
  buttonLayout->addWidget( mLinkFeatureButton );
  // unlink feature
  mUnlinkFeatureButton = new QToolButton( this );
  mUnlinkFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUnlink.svg" ) ) );
  mUnlinkFeatureButton->setText( tr( "Unlink Feature" ) );
  mUnlinkFeatureButton->setToolTip( tr( "Unlink child feature" ) );
  mUnlinkFeatureButton->setObjectName( QStringLiteral( "mUnlinkFeatureButton" ) );
  buttonLayout->addWidget( mUnlinkFeatureButton );
  // zoom to linked feature
  mZoomToFeatureButton = new QToolButton( this );
  mZoomToFeatureButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToSelected.svg" ) ) );
  mZoomToFeatureButton->setText( tr( "Zoom To Feature" ) );
  mZoomToFeatureButton->setToolTip( tr( "Zoom to child feature" ) );
  mZoomToFeatureButton->setObjectName( QStringLiteral( "mZoomToFeatureButton" ) );
  buttonLayout->addWidget( mZoomToFeatureButton );
  // spacer
  buttonLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );
  // form view
  mFormViewButton = new QToolButton( this );
  mFormViewButton->setText( tr( "Form View" ) );
  mFormViewButton->setToolTip( tr( "Switch to form view" ) );
  mFormViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPropertyItem.svg" ) ) );
  mFormViewButton->setCheckable( true );
  mFormViewButton->setChecked( mViewMode == QgsDualView::AttributeEditor );
  buttonLayout->addWidget( mFormViewButton );
  // table view
  mTableViewButton = new QToolButton( this );
  mTableViewButton->setText( tr( "Table View" ) );
  mTableViewButton->setToolTip( tr( "Switch to table view" ) );
  mTableViewButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) ) );
  mTableViewButton->setCheckable( true );
  mTableViewButton->setChecked( mViewMode == QgsDualView::AttributeTable );
  buttonLayout->addWidget( mTableViewButton );
  // button group
  mViewModeButtonGroup = new QButtonGroup( this );
  mViewModeButtonGroup->addButton( mFormViewButton, QgsDualView::AttributeEditor );
  mViewModeButtonGroup->addButton( mTableViewButton, QgsDualView::AttributeTable );

  // add buttons layout
  topLayout->addLayout( buttonLayout );

  mRelationLayout = new QGridLayout();
  mRelationLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addLayout( mRelationLayout );

  mDualView = new QgsDualView( this );
  mDualView->setView( mViewMode );

  mRelationLayout->addWidget( mDualView );

  connect( this, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsRelationEditorWidget::onCollapsedStateChanged );
  connect( mViewModeButtonGroup, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ),
           this, static_cast<void ( QgsRelationEditorWidget::* )( int )>( &QgsRelationEditorWidget::setViewMode ) );
  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::toggleEditing );
  connect( mSaveEditsButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::saveEdits );
  connect( mAddFeatureButton, &QAbstractButton::clicked, this, [this]() { addFeature(); } );
  connect( mAddFeatureGeometryButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::addFeatureGeometry );
  connect( mDuplicateFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::duplicateFeature );
  connect( mDeleteFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::deleteSelectedFeatures );
  connect( mLinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::linkFeature );
  connect( mUnlinkFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::unlinkSelectedFeatures );
  connect( mZoomToFeatureButton, &QAbstractButton::clicked, this, &QgsRelationEditorWidget::zoomToSelectedFeatures );

  connect( mDualView, &QgsDualView::showContextMenuExternally, this, &QgsRelationEditorWidget::showContextMenu );

  // Set initial state for add/remove etc. buttons
  updateButtons();
}

void QgsRelationEditorWidget::setRelationFeature( const QgsRelation &relation, const QgsFeature &feature )
{
  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  mRelation = relation;
  mFeature = feature;

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );

  updateTitle();

  QgsVectorLayer *lyr = relation.referencingLayer();

  bool canChangeAttributes = lyr->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !lyr->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
    updateButtons();
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  setObjectName( QStringLiteral( "referenced/" ) + mRelation.name() );

  // If not yet initialized, it is not (yet) visible, so we don't load it to be faster (lazy loading)
  // If it is already initialized, it has been set visible before and the currently shown feature is changing
  // and the widget needs updating

  if ( mVisible )
  {
    QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );
    initDualView( mRelation.referencingLayer(), myRequest );
  }
}

void QgsRelationEditorWidget::initDualView( QgsVectorLayer *layer, const QgsFeatureRequest &request )
{
  QgsAttributeEditorContext ctx { mEditorContext };
  ctx.setParentFormFeature( mFeature );
  mDualView->init( layer, mEditorContext.mapCanvas(), request, ctx );
  mFeatureSelectionMgr = new QgsFilteredSelectionManager( layer, request, mDualView );
  mDualView->setFeatureSelectionManager( mFeatureSelectionMgr );

  connect( mFeatureSelectionMgr, &QgsIFeatureSelectionManager::selectionChanged, this, &QgsRelationEditorWidget::updateButtons );

  QIcon icon;
  QString text;
  if ( layer->geometryType() == QgsWkbTypes::PointGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePoint.svg" ) );
    text = tr( "Add Point child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::LineGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCaptureLine.svg" ) );
    text = tr( "Add Line child Feature" );
  }
  else if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    icon = QgsApplication::getThemeIcon( QStringLiteral( "/mActionCapturePolygon.svg" ) );
    text = tr( "Add Polygon Feature" );
  }

  if ( text.isEmpty() || !mEditorContext.mapCanvas() || !mEditorContext.cadDockWidget() )
  {
    mAddFeatureGeometryButton->setVisible( false );
  }
  else
  {
    mAddFeatureGeometryButton->setIcon( icon );
    mAddFeatureGeometryButton->setText( text );
    mAddFeatureGeometryButton->setToolTip( text );
  }

  updateButtons();
}

void QgsRelationEditorWidget::setRelations( const QgsRelation &relation, const QgsRelation &nmrelation )
{
  if ( mRelation.isValid() )
  {
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    disconnect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  if ( mNmRelation.isValid() )
  {
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    disconnect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  mRelation = relation;
  mNmRelation = nmrelation;

  if ( !mRelation.isValid() )
    return;

  mToggleEditingButton->setVisible( true );

  const auto transactionGroups = QgsProject::instance()->transactionGroups();
  for ( auto it = transactionGroups.constBegin(); it != transactionGroups.constEnd(); ++it )
  {
    if ( it.value()->layers().contains( mRelation.referencingLayer() ) )
    {
      mToggleEditingButton->setVisible( false );
      mSaveEditsButton->setVisible( false );
    }
  }

  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
  connect( mRelation.referencingLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );

  if ( mNmRelation.isValid() )
  {
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStarted, this, &QgsRelationEditorWidget::updateButtons );
    connect( mNmRelation.referencedLayer(), &QgsVectorLayer::editingStopped, this, &QgsRelationEditorWidget::updateButtons );
  }

  updateTitle();

  QgsVectorLayer *lyr = relation.referencingLayer();

  bool canChangeAttributes = lyr->dataProvider()->capabilities() & QgsVectorDataProvider::ChangeAttributeValues;
  if ( canChangeAttributes && !lyr->readOnly() )
  {
    mToggleEditingButton->setEnabled( true );
    updateButtons();
  }
  else
  {
    mToggleEditingButton->setEnabled( false );
  }

  if ( mNmRelation.isValid() )
    mZoomToFeatureButton->setVisible( mNmRelation.referencedLayer()->isSpatial() );
  else
    mZoomToFeatureButton->setVisible( mRelation.referencingLayer()->isSpatial() );

  setObjectName( QStringLiteral( "referenced/" ) + mRelation.name() );

  updateUi();
}

void QgsRelationEditorWidget::setEditorContext( const QgsAttributeEditorContext &context )
{
  mEditorContext = context;

  if ( context.mapCanvas() && context.cadDockWidget() )
  {
    mMapToolDigitize.reset( new QgsMapToolDigitizeFeature( context.mapCanvas(), context.cadDockWidget() ) );
    mMapToolDigitize->setButton( mAddFeatureGeometryButton );
  }
}

QgsAttributeEditorContext QgsRelationEditorWidget::editorContext() const
{
  return mEditorContext;
}

QgsIFeatureSelectionManager *QgsRelationEditorWidget::featureSelectionManager()
{
  return mFeatureSelectionMgr;
}

void QgsRelationEditorWidget::setViewMode( QgsDualView::ViewMode mode )
{
  mDualView->setView( mode );
  mViewMode = mode;
}

void QgsRelationEditorWidget::setFeature( const QgsFeature &feature, bool update )
{
  mFeature = feature;

  mEditorContext.setFormFeature( feature );

  if ( update )
    updateUi();
}

void QgsRelationEditorWidget::updateButtons()
{
  bool editable = false;
  bool linkable = false;
  bool selectionNotEmpty = mFeatureSelectionMgr ? mFeatureSelectionMgr->selectedFeatureCount() : false;

  if ( mRelation.isValid() )
  {
    editable = mRelation.referencingLayer()->isEditable();
    linkable = mRelation.referencingLayer()->isEditable();
  }

  if ( mNmRelation.isValid() )
  {
    editable = mNmRelation.referencedLayer()->isEditable();
  }

  mAddFeatureButton->setEnabled( editable );
  mAddFeatureGeometryButton->setEnabled( editable );
  mDuplicateFeatureButton->setEnabled( editable && selectionNotEmpty );
  mLinkFeatureButton->setEnabled( linkable );
  mDeleteFeatureButton->setEnabled( editable && selectionNotEmpty );
  mUnlinkFeatureButton->setEnabled( linkable && selectionNotEmpty );

  mZoomToFeatureButton->setVisible(
    mEditorContext.mapCanvas() && (
      (
        mNmRelation.isValid() &&
        mNmRelation.referencedLayer()->geometryType() != QgsWkbTypes::NullGeometry &&
        mNmRelation.referencedLayer()->geometryType() != QgsWkbTypes::UnknownGeometry
      )
      ||
      (
        mRelation.isValid() &&
        mRelation.referencedLayer()->geometryType() != QgsWkbTypes::NullGeometry &&
        mRelation.referencedLayer()->geometryType() != QgsWkbTypes::UnknownGeometry
      )
    )
  );

  mZoomToFeatureButton->setEnabled( selectionNotEmpty );

  mToggleEditingButton->setChecked( editable );
  mSaveEditsButton->setEnabled( editable );
}

void QgsRelationEditorWidget::addFeatureGeometry()
{
  QgsVectorLayer *layer = nullptr;
  if ( mNmRelation.isValid() )
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();

  mMapToolDigitize->setLayer( layer );

  // window is always on top, so we hide it to digitize without seeing it
  window()->setVisible( false );
  setMapTool( mMapToolDigitize );

  connect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationEditorWidget::onDigitizingCompleted );
  connect( mEditorContext.mapCanvas(), &QgsMapCanvas::keyPressed, this, &QgsRelationEditorWidget::onKeyPressed );

  if ( mEditorContext.mainMessageBar() )
  {
    QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( layer, mFeature );

    QString title = tr( "Create child feature for parent %1 \"%2\"" ).arg( mRelation.referencedLayer()->name(), displayString );
    QString msg = tr( "Digitize the geometry for the new feature on layer %1. Press &lt;ESC&gt; to cancel." )
                  .arg( layer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    mEditorContext.mainMessageBar()->pushItem( mMessageBarItem );
  }

}

void QgsRelationEditorWidget::addFeature( const QgsGeometry &geometry )
{
  QgsAttributeMap keyAttrs;

  const QgsVectorLayerTools *vlTools = mEditorContext.vectorLayerTools();

  if ( mNmRelation.isValid() )
  {
    // n:m Relation: first let the user create a new feature on the other table
    // and autocreate a new linking feature.
    QgsFeature f;
    if ( vlTools->addFeature( mNmRelation.referencedLayer(), QgsAttributeMap(), geometry, &f ) )
    {
      // Fields of the linking table
      const QgsFields fields = mRelation.referencingLayer()->fields();

      // Expression context for the linking table
      QgsExpressionContext context = mRelation.referencingLayer()->createExpressionContext();

      QgsAttributeMap linkAttributes;
      const auto constFieldPairs = mRelation.fieldPairs();
      for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
      {
        int index = fields.indexOf( fieldPair.first );
        linkAttributes.insert( index,  mFeature.attribute( fieldPair.second ) );
      }

      const auto constNmFieldPairs = mNmRelation.fieldPairs();
      for ( const QgsRelation::FieldPair &fieldPair : constNmFieldPairs )
      {
        int index = fields.indexOf( fieldPair.first );
        linkAttributes.insert( index, f.attribute( fieldPair.second ) );
      }
      QgsFeature linkFeature = QgsVectorLayerUtils::createFeature( mRelation.referencingLayer(), QgsGeometry(), linkAttributes, &context );

      mRelation.referencingLayer()->addFeature( linkFeature );

      updateUi();
    }
  }
  else
  {
    QgsFields fields = mRelation.referencingLayer()->fields();

    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      keyAttrs.insert( fields.indexFromName( fieldPair.referencingField() ), mFeature.attribute( fieldPair.referencedField() ) );
    }

    vlTools->addFeature( mDualView->masterModel()->layer(), keyAttrs, geometry );
  }
}

void QgsRelationEditorWidget::onDigitizingCompleted( const QgsFeature &feature )
{
  addFeature( feature.geometry() );

  unsetMapTool();
}

void QgsRelationEditorWidget::linkFeature()
{
  QgsVectorLayer *layer = nullptr;

  if ( mNmRelation.isValid() )
    layer = mNmRelation.referencedLayer();
  else
    layer = mRelation.referencingLayer();

  QgsFeatureSelectionDlg *selectionDlg = new QgsFeatureSelectionDlg( layer, mEditorContext, this );
  selectionDlg->setAttribute( Qt::WA_DeleteOnClose );

  const QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( mRelation.referencedLayer(), mFeature );
  selectionDlg->setWindowTitle( tr( "Link existing child features for parent %1 \"%2\"" ).arg( mRelation.referencedLayer()->name(), displayString ) );

  connect( selectionDlg, &QDialog::accepted, this, &QgsRelationEditorWidget::onLinkFeatureDlgAccepted );
  selectionDlg->show();
}

void QgsRelationEditorWidget::onLinkFeatureDlgAccepted()
{
  QgsFeatureSelectionDlg *selectionDlg = qobject_cast<QgsFeatureSelectionDlg *>( sender() );
  if ( mNmRelation.isValid() )
  {
    QgsFeatureIterator it = mNmRelation.referencedLayer()->getFeatures(
                              QgsFeatureRequest()
                              .setFilterFids( selectionDlg->selectedFeatures() )
                              .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

    QgsFeature relatedFeature;

    QgsFeatureList newFeatures;

    // Fields of the linking table
    const QgsFields fields = mRelation.referencingLayer()->fields();

    // Expression context for the linking table
    QgsExpressionContext context = mRelation.referencingLayer()->createExpressionContext();

    QgsAttributeMap linkAttributes;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int index = fields.indexOf( fieldPair.first );
      linkAttributes.insert( index,  mFeature.attribute( fieldPair.second ) );
    }

    while ( it.nextFeature( relatedFeature ) )
    {
      const auto constFieldPairs = mNmRelation.fieldPairs();
      for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
      {
        int index = fields.indexOf( fieldPair.first );
        linkAttributes.insert( index, relatedFeature.attribute( fieldPair.second ) );
      }
      const QgsFeature linkFeature = QgsVectorLayerUtils::createFeature( mRelation.referencingLayer(), QgsGeometry(), linkAttributes, &context );

      newFeatures << linkFeature;
    }

    mRelation.referencingLayer()->addFeatures( newFeatures );
    QgsFeatureIds ids;
    const auto constNewFeatures = newFeatures;
    for ( const QgsFeature &f : constNewFeatures )
      ids << f.id();
    mRelation.referencingLayer()->selectByIds( ids );
  }
  else
  {
    QMap<int, QVariant> keys;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int idx = mRelation.referencingLayer()->fields().lookupField( fieldPair.referencingField() );
      QVariant val = mFeature.attribute( fieldPair.referencedField() );
      keys.insert( idx, val );
    }

    const auto constSelectedFeatures = selectionDlg->selectedFeatures();
    for ( QgsFeatureId fid : constSelectedFeatures )
    {
      QMapIterator<int, QVariant> it( keys );
      while ( it.hasNext() )
      {
        it.next();
        mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), it.value() );
      }
    }
  }

  updateUi();
}

void QgsRelationEditorWidget::duplicateFeature()
{
  QgsVectorLayer *layer = mRelation.referencingLayer();

  QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setFilterFids( mFeatureSelectionMgr->selectedFeatureIds() ) );
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    QgsVectorLayerUtils::QgsDuplicateFeatureContext duplicatedFeatureContext;
    QgsVectorLayerUtils::duplicateFeature( layer, f, QgsProject::instance(), 0, duplicatedFeatureContext );
  }
}

void QgsRelationEditorWidget::deleteFeature( const QgsFeatureId featureid )
{
  deleteFeatures( QgsFeatureIds() << featureid );
}

void QgsRelationEditorWidget::deleteSelectedFeatures()
{
  QgsFeatureIds selectedFids = mFeatureSelectionMgr->selectedFeatureIds();
  deleteFeatures( selectedFids );
}

void QgsRelationEditorWidget::deleteFeatures( const QgsFeatureIds &featureids )
{
  bool deleteFeatures = true;

  QgsVectorLayer *layer;
  if ( mNmRelation.isValid() )
  {
    layer = mNmRelation.referencedLayer();

    // When deleting a linked feature within an N:M relation,
    // check if the feature is linked to more than just one feature.
    // In case it is linked more than just once, ask the user for confirmation
    // as it is likely he was not aware of the implications and might delete
    // there may be several linking entries deleted along.

    QgsFeatureRequest deletedFeaturesRequest;
    deletedFeaturesRequest.setFilterFids( featureids );
    deletedFeaturesRequest.setFlags( QgsFeatureRequest::NoGeometry );
    deletedFeaturesRequest.setSubsetOfAttributes( QgsAttributeList() << mNmRelation.referencedFields().first() );

    QgsFeatureIterator deletedFeatures = layer->getFeatures( deletedFeaturesRequest );
    QStringList deletedFeaturesPks;
    QgsFeature feature;
    while ( deletedFeatures.nextFeature( feature ) )
    {
      deletedFeaturesPks.append( QgsExpression::quotedValue( feature.attribute( mNmRelation.referencedFields().first() ) ) );
    }

    QgsFeatureRequest linkingFeaturesRequest;
    linkingFeaturesRequest.setFlags( QgsFeatureRequest::NoGeometry );
    linkingFeaturesRequest.setNoAttributes();

    QString linkingFeaturesRequestExpression;
    if ( !deletedFeaturesPks.empty() )
    {
      linkingFeaturesRequestExpression = QStringLiteral( "%1 IN (%2)" ).arg( QgsExpression::quotedColumnRef( mNmRelation.fieldPairs().first().first ), deletedFeaturesPks.join( ',' ) );
      linkingFeaturesRequest.setFilterExpression( linkingFeaturesRequestExpression );

      QgsFeatureIterator relatedLinkingFeatures = mNmRelation.referencingLayer()->getFeatures( linkingFeaturesRequest );

      int relatedLinkingFeaturesCount = 0;
      while ( relatedLinkingFeatures.nextFeature( feature ) )
      {
        relatedLinkingFeaturesCount++;
      }

      if ( deletedFeaturesPks.size() == 1 && relatedLinkingFeaturesCount > 1 )
      {
        QMessageBox messageBox( QMessageBox::Question, tr( "Really delete entry?" ), tr( "The entry on %1 is still linked to %2 features on %3. Do you want to delete it?" ).arg( mNmRelation.referencedLayer()->name(), QString::number( relatedLinkingFeaturesCount ), mRelation.referencedLayer()->name() ), QMessageBox::NoButton, this );
        messageBox.addButton( QMessageBox::Cancel );
        QAbstractButton *deleteButton = messageBox.addButton( tr( "Delete" ),  QMessageBox::AcceptRole );

        messageBox.exec();
        if ( messageBox.clickedButton() != deleteButton )
          deleteFeatures = false;
      }
      else if ( deletedFeaturesPks.size() > 1 && relatedLinkingFeaturesCount > deletedFeaturesPks.size() )
      {
        QMessageBox messageBox( QMessageBox::Question, tr( "Really delete entries?" ), tr( "The %1 entries on %2 are still linked to %3 features on %4. Do you want to delete them?" ).arg( QString::number( deletedFeaturesPks.size() ), mNmRelation.referencedLayer()->name(), QString::number( relatedLinkingFeaturesCount ), mRelation.referencedLayer()->name() ), QMessageBox::NoButton, this );
        messageBox.addButton( QMessageBox::Cancel );
        QAbstractButton *deleteButton = messageBox.addButton( tr( "Delete" ), QMessageBox::AcceptRole );

        messageBox.exec();
        if ( messageBox.clickedButton() != deleteButton )
          deleteFeatures = false;
      }
    }
  }
  else
  {
    layer = mRelation.referencingLayer();
  }

  QgsVectorLayerUtils::QgsDuplicateFeatureContext infoContext;
  if ( QgsVectorLayerUtils::impactsCascadeFeatures( layer, featureids, QgsProject::instance(), infoContext ) )
  {
    QString childrenInfo;
    int childrenCount = 0;
    const auto infoContextLayers = infoContext.layers();
    for ( QgsVectorLayer *chl : infoContextLayers )
    {
      childrenCount += infoContext.duplicatedFeatures( chl ).size();
      childrenInfo += ( tr( "%1 feature(s) on layer \"%2\", " ).arg( infoContext.duplicatedFeatures( chl ).size() ).arg( chl->name() ) );
    }

    // for extra safety to make sure we know that the delete can have impact on children and joins
    int res = QMessageBox::question( this, tr( "Delete at least %1 feature(s) on other layer(s)" ).arg( childrenCount ),
                                     tr( "Delete %1 feature(s) on layer \"%2\", %3 as well\nand all of its other descendants.\nDelete these features?" ).arg( featureids.count() ).arg( layer->name() ).arg( childrenInfo ),
                                     QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
      deleteFeatures = false;
  }

  if ( deleteFeatures )
  {
    QgsVectorLayer::DeleteContext context( true, QgsProject::instance() );
    layer->deleteFeatures( featureids, &context );
    const auto contextLayers = context.handledLayers();
    if ( contextLayers.size() > 1 )
    {
      int deletedCount = 0;
      QString feedbackMessage;
      for ( QgsVectorLayer *contextLayer : contextLayers )
      {
        feedbackMessage += tr( "%1 on layer %2. " ).arg( context.handledFeatures( contextLayer ).size() ).arg( contextLayer->name() );
        deletedCount += context.handledFeatures( contextLayer ).size();
      }
      mEditorContext.mainMessageBar()->pushMessage( tr( "%1 features deleted: %2" ).arg( deletedCount ).arg( feedbackMessage ), Qgis::Success );
    }

    updateUi();
  }
}

void QgsRelationEditorWidget::unlinkFeature( const QgsFeatureId featureid )
{
  unlinkFeatures( QgsFeatureIds() << featureid );
}

void QgsRelationEditorWidget::unlinkSelectedFeatures()
{
  unlinkFeatures( mFeatureSelectionMgr->selectedFeatureIds() );
}

void QgsRelationEditorWidget::zoomToSelectedFeatures()
{
  QgsMapCanvas *c = mEditorContext.mapCanvas();
  if ( !c )
    return;

  c->zoomToFeatureIds(
    mNmRelation.isValid() ? mNmRelation.referencedLayer() : mRelation.referencingLayer(),
    mFeatureSelectionMgr->selectedFeatureIds()
  );
}

void QgsRelationEditorWidget::unlinkFeatures( const QgsFeatureIds &featureids )
{
  if ( mNmRelation.isValid() )
  {
    QgsFeatureIterator selectedIterator = mNmRelation.referencedLayer()->getFeatures(
                                            QgsFeatureRequest()
                                            .setFilterFids( featureids )
                                            .setSubsetOfAttributes( mNmRelation.referencedFields() ) );

    QgsFeature f;

    QStringList filters;

    while ( selectedIterator.nextFeature( f ) )
    {
      filters << '(' + mNmRelation.getRelatedFeaturesRequest( f ).filterExpression()->expression() + ')';
    }

    QString filter = QStringLiteral( "(%1) AND (%2)" ).arg(
                       mRelation.getRelatedFeaturesRequest( mFeature ).filterExpression()->expression(),
                       filters.join( QStringLiteral( " OR " ) ) );

    QgsFeatureIterator linkedIterator = mRelation.referencingLayer()->getFeatures( QgsFeatureRequest()
                                        .setNoAttributes()
                                        .setFilterExpression( filter ) );

    QgsFeatureIds fids;

    while ( linkedIterator.nextFeature( f ) )
    {
      fids << f.id();
      QgsDebugMsgLevel( FID_TO_STRING( f.id() ), 4 );
    }

    mRelation.referencingLayer()->deleteFeatures( fids );

    updateUi();
  }
  else
  {
    QMap<int, QgsField> keyFields;
    const auto constFieldPairs = mRelation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : constFieldPairs )
    {
      int idx = mRelation.referencingLayer()->fields().lookupField( fieldPair.referencingField() );
      if ( idx < 0 )
      {
        QgsDebugMsg( QStringLiteral( "referencing field %1 not found" ).arg( fieldPair.referencingField() ) );
        return;
      }
      QgsField fld = mRelation.referencingLayer()->fields().at( idx );
      keyFields.insert( idx, fld );
    }

    const auto constFeatureids = featureids;
    for ( QgsFeatureId fid : constFeatureids )
    {
      QMapIterator<int, QgsField> it( keyFields );
      while ( it.hasNext() )
      {
        it.next();
        mRelation.referencingLayer()->changeAttributeValue( fid, it.key(), QVariant( it.value().type() ) );
      }
    }
  }
}

void QgsRelationEditorWidget::toggleEditing( bool state )
{
  if ( state )
  {
    mEditorContext.vectorLayerTools()->startEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->startEditing( mNmRelation.referencedLayer() );
  }
  else
  {
    mEditorContext.vectorLayerTools()->stopEditing( mRelation.referencingLayer() );
    if ( mNmRelation.isValid() )
      mEditorContext.vectorLayerTools()->stopEditing( mNmRelation.referencedLayer() );
  }
}

void QgsRelationEditorWidget::saveEdits()
{
  mEditorContext.vectorLayerTools()->saveEdits( mRelation.referencingLayer() );
  if ( mNmRelation.isValid() )
    mEditorContext.vectorLayerTools()->saveEdits( mNmRelation.referencedLayer() );
}

void QgsRelationEditorWidget::onCollapsedStateChanged( bool collapsed )
{
  if ( !collapsed )
  {
    mVisible = true;
    updateUi();
  }
}

void QgsRelationEditorWidget::updateUi()
{
  // If not yet initialized, it is not (yet) visible, so we don't load it to be faster (lazy loading)
  // If it is already initialized, it has been set visible before and the currently shown feature is changing
  // and the widget needs updating

  if ( mVisible )
  {
    QgsFeatureRequest myRequest = mRelation.getRelatedFeaturesRequest( mFeature );

    if ( mNmRelation.isValid() )
    {
      QgsFeatureIterator it = mRelation.referencingLayer()->getFeatures( myRequest );

      QgsFeature fet;

      QStringList filters;

      while ( it.nextFeature( fet ) )
      {
        QString filter = mNmRelation.getReferencedFeatureRequest( fet ).filterExpression()->expression();
        filters << filter.prepend( '(' ).append( ')' );
      }

      QgsFeatureRequest nmRequest;

      nmRequest.setFilterExpression( filters.join( QStringLiteral( " OR " ) ) );

      initDualView( mNmRelation.referencedLayer(), nmRequest );
    }
    else
    {
      initDualView( mRelation.referencingLayer(), myRequest );
    }
  }
}

bool QgsRelationEditorWidget::showLinkButton() const
{
  return mLinkFeatureButton->isVisible();
}

void QgsRelationEditorWidget::setShowLinkButton( bool showLinkButton )
{
  mLinkFeatureButton->setVisible( showLinkButton );
}

bool QgsRelationEditorWidget::showUnlinkButton() const
{
  return mUnlinkFeatureButton->isVisible();
}

void QgsRelationEditorWidget::setShowSaveChildEditsButton( bool showChildEdits )
{
  mSaveEditsButton->setVisible( showChildEdits );
}

bool QgsRelationEditorWidget::showSaveChildEditsButton() const
{
  return mSaveEditsButton->isVisible();
}

void QgsRelationEditorWidget::setShowUnlinkButton( bool showUnlinkButton )
{
  mUnlinkFeatureButton->setVisible( showUnlinkButton );
}

void QgsRelationEditorWidget::parentFormValueChanged( const QString &attribute, const QVariant &newValue )
{
  mDualView->parentFormValueChanged( attribute, newValue );
}

bool QgsRelationEditorWidget::showLabel() const
{
  return mShowLabel;
}

void QgsRelationEditorWidget::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;

  updateTitle();
}

void QgsRelationEditorWidget::showContextMenu( QgsActionMenu *menu, const QgsFeatureId fid )
{
  if ( mRelation.referencingLayer()->isEditable() )
  {
    QAction *qAction = nullptr;

    qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteSelected.svg" ) ),  tr( "Delete Feature" ) );
    connect( qAction, &QAction::triggered, this, [this, fid]() { deleteFeature( fid ); } );

    qAction = menu->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionUnlink.svg" ) ),  tr( "Unlink Feature" ) );
    connect( qAction, &QAction::triggered, this, [this, fid]() { unlinkFeature( fid ); } );
  }
}

void QgsRelationEditorWidget::setMapTool( QgsMapTool *mapTool )
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  mapCanvas->setMapTool( mapTool );
  mapCanvas->window()->raise();
  mapCanvas->activateWindow();
  mapCanvas->setFocus();
  connect( mapTool, &QgsMapTool::deactivated, this, &QgsRelationEditorWidget::mapToolDeactivated );
}

void QgsRelationEditorWidget::unsetMapTool()
{
  QgsMapCanvas *mapCanvas = mEditorContext.mapCanvas();

  // this will call mapToolDeactivated
  mapCanvas->unsetMapTool( mMapToolDigitize );

  disconnect( mapCanvas, &QgsMapCanvas::keyPressed, this, &QgsRelationEditorWidget::onKeyPressed );
  disconnect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationEditorWidget::onDigitizingCompleted );
}

void QgsRelationEditorWidget::updateTitle()
{
  if ( mShowLabel && mRelation.isValid() )
    setTitle( mRelation.name() );
  else
    setTitle( QString() );
}

QgsFeature QgsRelationEditorWidget::feature() const
{
  return mFeature;
}

void QgsRelationEditorWidget::onKeyPressed( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    unsetMapTool();
  }
}

void QgsRelationEditorWidget::mapToolDeactivated()
{
  window()->setVisible( true );
  window()->raise();
  window()->activateWindow();

  if ( mEditorContext.mainMessageBar() && mMessageBarItem )
  {
    mEditorContext.mainMessageBar()->popWidget( mMessageBarItem );
  }
  mMessageBarItem = nullptr;
}
