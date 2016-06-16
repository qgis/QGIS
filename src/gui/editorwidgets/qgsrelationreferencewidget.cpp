/***************************************************************************
    qgsrelationreferencewidget.cpp
     --------------------------------------
    Date                 : 20.4.2013
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

#include "qgsrelationreferencewidget.h"

#include <QPushButton>
#include <QDialog>
#include <QHBoxLayout>
#include <QTimer>

#include "qgsattributeform.h"
#include "qgsattributedialog.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgseditorwidgetfactory.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsrelationreferenceconfigdlg.h"
#include "qgsvectorlayer.h"
#include "qgsattributetablemodel.h"

QgsRelationReferenceWidget::QgsRelationReferenceWidget( QWidget* parent )
    : QWidget( parent )
    , mEditorContext( QgsAttributeEditorContext() )
    , mCanvas( nullptr )
    , mMessageBar( nullptr )
    , mForeignKey( QVariant() )
    , mReferencedFieldIdx( -1 )
    , mReferencingFieldIdx( -1 )
    , mAllowNull( true )
    , mHighlight( nullptr )
    , mMapTool( nullptr )
    , mMessageBarItem( nullptr )
    , mRelationName( "" )
    , mReferencedAttributeForm( nullptr )
    , mReferencedLayer( nullptr )
    , mReferencingLayer( nullptr )
    , mMasterModel( nullptr )
    , mFilterModel( nullptr )
    , mFeatureListModel( nullptr )
    , mWindowWidget( nullptr )
    , mShown( false )
    , mIsEditable( true )
    , mEmbedForm( false )
    , mReadOnlySelector( false )
    , mAllowMapIdentification( false )
    , mOrderByValue( false )
    , mOpenFormButtonVisible( true )
    , mChainFilters( false )
    , mAllowAddFeatures( false )
{
  mTopLayout = new QVBoxLayout( this );
  mTopLayout->setContentsMargins( 0, 0, 0, 0 );

  setSizePolicy( sizePolicy().horizontalPolicy(), QSizePolicy::Fixed );

  setLayout( mTopLayout );

  QHBoxLayout* editLayout = new QHBoxLayout();
  editLayout->setContentsMargins( 0, 0, 0, 0 );
  editLayout->setSpacing( 2 );

  // Prepare the container and layout for the filter comboboxes
  mChooserContainer = new QWidget;
  editLayout->addWidget( mChooserContainer );
  QHBoxLayout* chooserLayout = new QHBoxLayout;
  chooserLayout->setContentsMargins( 0, 0, 0, 0 );
  mFilterLayout = new QHBoxLayout;
  mFilterLayout->setContentsMargins( 0, 0, 0, 0 );
  mFilterContainer = new QWidget;
  mFilterContainer->setLayout( mFilterLayout );
  mChooserContainer->setLayout( chooserLayout );
  chooserLayout->addWidget( mFilterContainer );

  // combobox (for non-geometric relation)
  mComboBox = new QComboBox();
  mChooserContainer->layout()->addWidget( mComboBox );

  // read-only line edit
  mLineEdit = new QLineEdit();
  mLineEdit->setReadOnly( true );
  editLayout->addWidget( mLineEdit );

  // open form button
  mOpenFormButton = new QToolButton();
  mOpenFormButton->setIcon( QgsApplication::getThemeIcon( "/mActionPropertyItem.png" ) );
  mOpenFormButton->setText( tr( "Open related feature form" ) );
  editLayout->addWidget( mOpenFormButton );

  mAddEntryButton = new QToolButton();
  mAddEntryButton->setIcon( QgsApplication::getThemeIcon( "/mActionAdd.svg" ) );
  mAddEntryButton->setText( tr( "Add new entry" ) );
  editLayout->addWidget( mAddEntryButton );

  // highlight button
  mHighlightFeatureButton = new QToolButton( this );
  mHighlightFeatureButton->setPopupMode( QToolButton::MenuButtonPopup );
  mHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionHighlightFeature.svg" ), tr( "Highlight feature" ), this );
  mScaleHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionScaleHighlightFeature.svg" ), tr( "Scale and highlight feature" ), this );
  mPanHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionPanHighlightFeature.svg" ), tr( "Pan and highlight feature" ), this );
  mHighlightFeatureButton->addAction( mHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mScaleHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mPanHighlightFeatureAction );
  mHighlightFeatureButton->setDefaultAction( mHighlightFeatureAction );
  editLayout->addWidget( mHighlightFeatureButton );

  // map identification button
  mMapIdentificationButton = new QToolButton( this );
  mMapIdentificationButton->setIcon( QgsApplication::getThemeIcon( "/mActionMapIdentification.svg" ) );
  mMapIdentificationButton->setText( tr( "Select on map" ) );
  mMapIdentificationButton->setCheckable( true );
  editLayout->addWidget( mMapIdentificationButton );

  // remove foreign key button
  mRemoveFKButton = new QToolButton( this );
  mRemoveFKButton->setIcon( QgsApplication::getThemeIcon( "/mActionRemove.svg" ) );
  mRemoveFKButton->setText( tr( "No selection" ) );
  editLayout->addWidget( mRemoveFKButton );

  // add line to top layout
  mTopLayout->addLayout( editLayout );

  // embed form
  mAttributeEditorFrame = new QgsCollapsibleGroupBox( this );
  mAttributeEditorLayout = new QVBoxLayout( mAttributeEditorFrame );
  mAttributeEditorFrame->setLayout( mAttributeEditorLayout );
  mAttributeEditorFrame->setSizePolicy( mAttributeEditorFrame->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding );
  mTopLayout->addWidget( mAttributeEditorFrame );

  // invalid label
  mInvalidLabel = new QLabel( tr( "The relation is not valid. Please make sure your relation definitions are ok." ) );
  mInvalidLabel->setWordWrap( true );
  QFont font = mInvalidLabel->font();
  font.setItalic( true );
  mInvalidLabel->setStyleSheet( "QLabel { color: red; } " );
  mInvalidLabel->setFont( font );
  mTopLayout->addWidget( mInvalidLabel );

  // default mode is combobox, no geometric relation and no embed form
  mLineEdit->hide();
  mMapIdentificationButton->hide();
  mHighlightFeatureButton->hide();
  mAttributeEditorFrame->hide();
  mInvalidLabel->hide();

  // connect buttons
  connect( mOpenFormButton, SIGNAL( clicked() ), this, SLOT( openForm() ) );
  connect( mHighlightFeatureButton, SIGNAL( triggered( QAction* ) ), this, SLOT( highlightActionTriggered( QAction* ) ) );
  connect( mMapIdentificationButton, SIGNAL( clicked() ), this, SLOT( mapIdentification() ) );
  connect( mRemoveFKButton, SIGNAL( clicked() ), this, SLOT( deleteForeignKey() ) );
  connect( mAddEntryButton, SIGNAL( clicked( bool ) ), this, SLOT( addEntry() ) );
  connect( mComboBox, SIGNAL( editTextChanged( QString ) ), this, SLOT( updateAddEntryButton() ) );
}

QgsRelationReferenceWidget::~QgsRelationReferenceWidget()
{
  deleteHighlight();
  unsetMapTool();
  if ( mMapTool )
    delete mMapTool;
}

void QgsRelationReferenceWidget::setRelation( const QgsRelation& relation, bool allowNullValue )
{
  mAllowNull = allowNullValue;
  mRemoveFKButton->setVisible( allowNullValue && mReadOnlySelector );

  if ( relation.isValid() )
  {
    mInvalidLabel->hide();

    mRelation = relation;
    mReferencingLayer = relation.referencingLayer();
    mRelationName = relation.name();
    mReferencedLayer = relation.referencedLayer();
    mReferencedFieldIdx = mReferencedLayer->fieldNameIndex( relation.fieldPairs().at( 0 ).second );
    mReferencingFieldIdx = mReferencingLayer->fieldNameIndex( relation.fieldPairs().at( 0 ).first );

    QgsAttributeEditorContext context( mEditorContext, relation, QgsAttributeEditorContext::Single, QgsAttributeEditorContext::Embed );

    if ( mEmbedForm )
    {
      mAttributeEditorFrame->setTitle( mReferencedLayer->name() );
      mReferencedAttributeForm = new QgsAttributeForm( relation.referencedLayer(), QgsFeature(), context, this );
      mAttributeEditorLayout->addWidget( mReferencedAttributeForm );
    }

    connect( mReferencedLayer, SIGNAL( editingStarted() ), this, SLOT( updateAddEntryButton() ) );
    connect( mReferencedLayer, SIGNAL( editingStopped() ), this, SLOT( updateAddEntryButton() ) );
    updateAddEntryButton();
  }
  else
  {
    mInvalidLabel->show();
  }

  if ( mShown && isVisible() )
  {
    init();
  }
}

void QgsRelationReferenceWidget::setRelationEditable( bool editable )
{
  if ( !editable )
    unsetMapTool();

  mFilterContainer->setEnabled( editable );
  mComboBox->setEnabled( editable );
  mComboBox->setEditable( true );
  mMapIdentificationButton->setEnabled( editable );
  mRemoveFKButton->setEnabled( editable );
  mIsEditable = editable;
}

void QgsRelationReferenceWidget::setForeignKey( const QVariant& value )
{
  if ( !value.isValid() )
  {
    return;
  }
  if ( value.isNull() )
  {
    deleteForeignKey();
    return;
  }

  if ( !mReferencedLayer )
    return;

  // Attributes from the referencing layer
  QgsAttributes attrs = QgsAttributes( mReferencingLayer->fields().count() );
  // Set the value on the foreign key field of the referencing record
  attrs[ mReferencingLayer->fieldNameIndex( mRelation.fieldPairs().at( 0 ).first )] = value;

  QgsFeatureRequest request = mRelation.getReferencedFeatureRequest( attrs );

  mReferencedLayer->getFeatures( request ).nextFeature( mFeature );

  if ( !mFeature.isValid() )
  {
    return;
  }

  mForeignKey = mFeature.attribute( mReferencedFieldIdx );

  if ( mReadOnlySelector )
  {
    QgsExpression expr( mReferencedLayer->displayExpression() );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( mReferencedLayer );
    context.setFeature( mFeature );
    QString title = expr.evaluate( &context ).toString();
    if ( expr.hasEvalError() )
    {
      title = mFeature.attribute( mReferencedFieldIdx ).toString();
    }
    mLineEdit->setText( title );
  }
  else
  {
    int i = mComboBox->findData( mFeature.id(), QgsAttributeTableModel::FeatureIdRole );
    if ( i == -1 && mAllowNull )
    {
      mComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mComboBox->setCurrentIndex( i );
    }
  }

  mRemoveFKButton->setEnabled( mIsEditable );
  highlightFeature( mFeature );
  updateAttributeEditorFrame( mFeature );
  emit foreignKeyChanged( foreignKey() );
}

void QgsRelationReferenceWidget::deleteForeignKey()
{
  QVariant nullValue = QSettings().value( "qgis/nullValue", "NULL" );
  if ( mReadOnlySelector )
  {
    QString nullText = "";
    if ( mAllowNull )
    {
      nullText = tr( "%1 (no selection)" ).arg( nullValue.toString() );
    }
    mLineEdit->setText( nullText );
    mForeignKey = QVariant();
    mFeature.setValid( false );
  }
  else
  {
    if ( mAllowNull )
    {
      mComboBox->setCurrentIndex( 0 );
    }
    else
    {
      mComboBox->setCurrentIndex( -1 );
    }
  }
  mRemoveFKButton->setEnabled( false );
  updateAttributeEditorFrame( QgsFeature() );
  emit foreignKeyChanged( QVariant( QVariant::Int ) );
}

QgsFeature QgsRelationReferenceWidget::referencedFeature() const
{
  QgsFeature f;
  if ( mReferencedLayer )
  {
    QgsFeatureId fid;
    if ( mReadOnlySelector )
    {
      fid = mFeature.id();
    }
    else
    {
      fid = mComboBox->itemData( mComboBox->currentIndex(), QgsAttributeTableModel::FeatureIdRole ).value<QgsFeatureId>();
    }
    mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( f );
  }
  return f;
}

void QgsRelationReferenceWidget::showIndeterminateState()
{
  if ( mReadOnlySelector )
  {
    whileBlocking( mLineEdit )->setText( QString() );
  }
  else
  {
    whileBlocking( mComboBox )->setCurrentIndex( -1 );
  }
  mRemoveFKButton->setEnabled( false );
  updateAttributeEditorFrame( QgsFeature() );
}

QVariant QgsRelationReferenceWidget::foreignKey() const
{
  if ( mReadOnlySelector )
  {
    return mForeignKey;
  }
  else
  {
    if ( mReferencingFieldIdx < 0 || mReferencingFieldIdx >= mReferencingLayer->fields().count() )
    {
      return QVariant();
    }
    else if ( !mFeature.isValid() )
    {
      return QVariant( mReferencingLayer->fields().at( mReferencingFieldIdx ).type() );
    }
    else
    {
      return mFeature.attribute( mReferencedFieldIdx );
    }
  }
}

void QgsRelationReferenceWidget::setEditorContext( const QgsAttributeEditorContext &context, QgsMapCanvas* canvas, QgsMessageBar* messageBar )
{
  mEditorContext = context;
  mCanvas = canvas;
  mMessageBar = messageBar;

  if ( mMapTool )
    delete mMapTool;
  mMapTool = new QgsMapToolIdentifyFeature( mCanvas );
  mMapTool->setButton( mMapIdentificationButton );
}

void QgsRelationReferenceWidget::setEmbedForm( bool display )
{
  if ( display )
  {
    setSizePolicy( sizePolicy().horizontalPolicy(), QSizePolicy::MinimumExpanding );
    mTopLayout->setAlignment( Qt::AlignTop );
  }

  mAttributeEditorFrame->setVisible( display );
  mEmbedForm = display;
}

void QgsRelationReferenceWidget::setReadOnlySelector( bool readOnly )
{
  mChooserContainer->setHidden( readOnly );
  mLineEdit->setVisible( readOnly );
  mRemoveFKButton->setVisible( mAllowNull && readOnly );
  mReadOnlySelector = readOnly;
}

void QgsRelationReferenceWidget::setAllowMapIdentification( bool allowMapIdentification )
{
  mHighlightFeatureButton->setVisible( allowMapIdentification );
  mMapIdentificationButton->setVisible( allowMapIdentification );
  mAllowMapIdentification = allowMapIdentification;
}

void QgsRelationReferenceWidget::setOrderByValue( bool orderByValue )
{
  mOrderByValue = orderByValue;
}

void QgsRelationReferenceWidget::setFilterFields( const QStringList& filterFields )
{
  mFilterFields = filterFields;
}

void QgsRelationReferenceWidget::setOpenFormButtonVisible( bool openFormButtonVisible )
{
  mOpenFormButton->setVisible( openFormButtonVisible );
  mOpenFormButtonVisible = openFormButtonVisible;
}

void QgsRelationReferenceWidget::setChainFilters( bool chainFilters )
{
  mChainFilters = chainFilters;
}

void QgsRelationReferenceWidget::showEvent( QShowEvent* e )
{
  Q_UNUSED( e )

  mShown = true;

  init();
}

void QgsRelationReferenceWidget::init()
{
  if ( !mReadOnlySelector && mComboBox->count() == 0 && mReferencedLayer )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QSet<QString> requestedAttrs;

    QgsVectorLayerCache* layerCache = new QgsVectorLayerCache( mReferencedLayer, 100000, this );

    if ( !mFilterFields.isEmpty() )
    {
      Q_FOREACH ( const QString& fieldName, mFilterFields )
      {
        QVariantList uniqueValues;
        int idx = mReferencedLayer->fieldNameIndex( fieldName );
        QComboBox* cb = new QComboBox();
        cb->setProperty( "Field", fieldName );
        mFilterComboBoxes << cb;
        mReferencedLayer->uniqueValues( idx, uniqueValues );
        cb->addItem( mReferencedLayer->attributeAlias( idx ).isEmpty() ? fieldName : mReferencedLayer->attributeAlias( idx ) );
        QVariant nullValue = QSettings().value( "qgis/nullValue", "NULL" );
        cb->addItem( nullValue.toString(), QVariant( mReferencedLayer->fields().at( idx ).type() ) );

        Q_FOREACH ( const QVariant& v, uniqueValues )
        {
          cb->addItem( v.toString(), v );
        }

        connect( cb, SIGNAL( currentIndexChanged( int ) ), this, SLOT( filterChanged() ) );

        // Request this attribute for caching
        requestedAttrs << fieldName;

        mFilterLayout->addWidget( cb );
      }

      if ( mChainFilters )
      {
        QVariant nullValue = QSettings().value( "qgis/nullValue", "NULL" );

        QgsFeature ft;
        QgsFeatureIterator fit = layerCache->getFeatures();
        while ( fit.nextFeature( ft ) )
        {
          for ( int i = 0; i < mFilterComboBoxes.count() - 1; ++i )
          {
            QVariant cv = ft.attribute( mFilterFields[i] );
            QVariant nv = ft.attribute( mFilterFields[i + 1] );
            QString cf = cv.isNull() ? nullValue.toString() : cv.toString();
            QString nf = nv.isNull() ? nullValue.toString() : nv.toString();
            mFilterCache[mFilterFields[i]][cf] << nf;
          }
        }
      }
    }
    else
    {
      mFilterContainer->hide();
    }

    QgsExpression exp( mReferencedLayer->displayExpression() );

    requestedAttrs += exp.referencedColumns().toSet();
    requestedAttrs << mRelation.fieldPairs().at( 0 ).second;

    QgsAttributeList attributes;
    Q_FOREACH ( const QString& attr, requestedAttrs )
      attributes << mReferencedLayer->fieldNameIndex( attr );

    layerCache->setCacheSubsetOfAttributes( attributes );
    mMasterModel = new QgsAttributeTableModel( layerCache );
    mMasterModel->setRequest( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( requestedAttrs.toList(), mReferencedLayer->fields() ) );
    mFilterModel = new QgsAttributeTableFilterModel( mCanvas, mMasterModel, mMasterModel );
    mFeatureListModel = new QgsFeatureListModel( mFilterModel, this );
    mFeatureListModel->setDisplayExpression( mReferencedLayer->displayExpression() );

    mMasterModel->loadLayer();

    mFeatureListModel->setInjectNull( mAllowNull );
    if ( mOrderByValue )
    {
      const QStringList referencedColumns = QgsExpression( mReferencedLayer->displayExpression() ).referencedColumns();
      if ( !referencedColumns.isEmpty() )
      {
        int sortIdx = mReferencedLayer->fieldNameIndex( referencedColumns.first() );
        mFilterModel->sort( sortIdx );
      }
    }

    mComboBox->setModel( mFeatureListModel );

    QVariant nullValue = QSettings().value( "qgis/nullValue", "NULL" );

    if ( mChainFilters && mFeature.isValid() )
    {
      for ( int i = 0; i < mFilterFields.size(); i++ )
      {
        QVariant v = mFeature.attribute( mFilterFields[i] );
        QString f = v.isNull() ? nullValue.toString() : v.toString();
        mFilterComboBoxes.at( i )->setCurrentIndex( mFilterComboBoxes.at( i )->findText( f ) );
      }
    }

    QVariant featId = mFeature.isValid() ? mFeature.id() : QVariant( QVariant::Int );
    mComboBox->setCurrentIndex( mComboBox->findData( featId, QgsAttributeTableModel::FeatureIdRole ) );

    // Only connect after iterating, to have only one iterator on the referenced table at once
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( comboReferenceChanged( int ) ) );
    updateAttributeEditorFrame( mFeature );
    QApplication::restoreOverrideCursor();
  }
}

void QgsRelationReferenceWidget::highlightActionTriggered( QAction* action )
{
  if ( action == mHighlightFeatureAction )
  {
    highlightFeature();
  }
  else if ( action == mScaleHighlightFeatureAction )
  {
    highlightFeature( QgsFeature(), Scale );
  }
  else if ( action == mPanHighlightFeatureAction )
  {
    highlightFeature( QgsFeature(), Pan );
  }
}

void QgsRelationReferenceWidget::openForm()
{
  QgsFeature feat = referencedFeature();

  if ( !feat.isValid() )
    return;

  QgsAttributeEditorContext context( mEditorContext, mRelation, QgsAttributeEditorContext::Single, QgsAttributeEditorContext::StandaloneDialog );
  QgsAttributeDialog attributeDialog( mReferencedLayer, new QgsFeature( feat ), true, this, true, context );
  attributeDialog.exec();
}

void QgsRelationReferenceWidget::highlightFeature( QgsFeature f, CanvasExtent canvasExtent )
{
  if ( !mCanvas )
    return;

  if ( !f.isValid() )
  {
    f = referencedFeature();
    if ( !f.isValid() )
      return;
  }

  if ( !f.constGeometry() )
  {
    return;
  }

  const QgsGeometry* geom = f.constGeometry();

  // scale or pan
  if ( canvasExtent == Scale )
  {
    QgsRectangle featBBox = geom->boundingBox();
    featBBox = mCanvas->mapSettings().layerToMapCoordinates( mReferencedLayer, featBBox );
    QgsRectangle extent = mCanvas->extent();
    if ( !extent.contains( featBBox ) )
    {
      extent.combineExtentWith( featBBox );
      extent.scale( 1.1 );
      mCanvas->setExtent( extent );
      mCanvas->refresh();
    }
  }
  else if ( canvasExtent == Pan )
  {
    QgsGeometry* centroid = geom->centroid();
    QgsPoint center = centroid->asPoint();
    delete centroid;
    center = mCanvas->mapSettings().layerToMapCoordinates( mReferencedLayer, center );
    mCanvas->zoomByFactor( 1.0, &center ); // refresh is done in this method
  }

  // highlight
  deleteHighlight();
  mHighlight = new QgsHighlight( mCanvas, f, mReferencedLayer );
  QSettings settings;
  QColor color = QColor( settings.value( "/Map/highlight/color", QGis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int alpha = settings.value( "/Map/highlight/colorAlpha", QGis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  double buffer = settings.value( "/Map/highlight/buffer", QGis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  double minWidth = settings.value( "/Map/highlight/minWidth", QGis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();

  mHighlight->setColor( color ); // sets also fill with default alpha
  color.setAlpha( alpha );
  mHighlight->setFillColor( color ); // sets fill with alpha
  mHighlight->setBuffer( buffer );
  mHighlight->setMinWidth( minWidth );
  mHighlight->show();

  QTimer* timer = new QTimer( this );
  timer->setSingleShot( true );
  connect( timer, SIGNAL( timeout() ), this, SLOT( deleteHighlight() ) );
  timer->start( 3000 );
}

void QgsRelationReferenceWidget::deleteHighlight()
{
  if ( mHighlight )
  {
    mHighlight->hide();
    delete mHighlight;
  }
  mHighlight = nullptr;
}

void QgsRelationReferenceWidget::mapIdentification()
{
  if ( !mAllowMapIdentification || !mReferencedLayer )
    return;

  const QgsVectorLayerTools* tools = mEditorContext.vectorLayerTools();
  if ( !tools )
    return;
  if ( !mCanvas )
    return;

  mMapTool->setLayer( mReferencedLayer );
  mCanvas->setMapTool( mMapTool );

  mWindowWidget = window();

  mCanvas->window()->raise();
  mCanvas->activateWindow();
  mCanvas->setFocus();

  connect( mMapTool, SIGNAL( featureIdentified( QgsFeature ) ), this, SLOT( featureIdentified( const QgsFeature ) ) );
  connect( mMapTool, SIGNAL( deactivated() ), this, SLOT( mapToolDeactivated() ) );

  if ( mMessageBar )
  {
    QString title = tr( "Relation %1 for %2." ).arg( mRelationName, mReferencingLayer->name() );
    QString msg = tr( "Identify a feature of %1 to be associated. Press &lt;ESC&gt; to cancel." ).arg( mReferencedLayer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    mMessageBar->pushItem( mMessageBarItem );
  }
}

void QgsRelationReferenceWidget::comboReferenceChanged( int index )
{
  QgsFeatureId fid = mComboBox->itemData( index, QgsAttributeTableModel::FeatureIdRole ).value<QgsFeatureId>();
  mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( mFeature );
  highlightFeature( mFeature );
  updateAttributeEditorFrame( mFeature );
  emit foreignKeyChanged( mFeature.attribute( mReferencedFieldIdx ) );
}

void QgsRelationReferenceWidget::updateAttributeEditorFrame( const QgsFeature& feature )
{
  mOpenFormButton->setEnabled( feature.isValid() );
  // Check if we're running with an embedded frame we need to update
  if ( mAttributeEditorFrame && mReferencedAttributeForm )
  {
    mReferencedAttributeForm->setFeature( feature );
  }
}

bool QgsRelationReferenceWidget::allowAddFeatures() const
{
  return mAllowAddFeatures;
}

void QgsRelationReferenceWidget::setAllowAddFeatures( bool allowAddFeatures )
{
  mAllowAddFeatures = allowAddFeatures;
  updateAddEntryButton();
}

void QgsRelationReferenceWidget::featureIdentified( const QgsFeature& feature )
{
  if ( mReadOnlySelector )
  {
    QgsExpression expr( mReferencedLayer->displayExpression() );
    QgsExpressionContext context;
    context << QgsExpressionContextUtils::globalScope()
    << QgsExpressionContextUtils::projectScope()
    << QgsExpressionContextUtils::layerScope( mReferencedLayer );
    context.setFeature( feature );
    QString title = expr.evaluate( &context ).toString();
    if ( expr.hasEvalError() )
    {
      title = feature.attribute( mReferencedFieldIdx ).toString();
    }
    mLineEdit->setText( title );
    mForeignKey = feature.attribute( mReferencedFieldIdx );
    mFeature = feature;
  }
  else
  {
    mComboBox->setCurrentIndex( mComboBox->findData( feature.id(), QgsAttributeTableModel::FeatureIdRole ) );
    mFeature = feature;
  }

  mRemoveFKButton->setEnabled( mIsEditable );
  highlightFeature( feature );
  updateAttributeEditorFrame( feature );
  emit foreignKeyChanged( foreignKey() );

  unsetMapTool();
}

void QgsRelationReferenceWidget::unsetMapTool()
{
  // deactivate map tool if activated
  if ( mCanvas && mMapTool )
  {
    /* this will call mapToolDeactivated */
    mCanvas->unsetMapTool( mMapTool );
  }
}

void QgsRelationReferenceWidget::mapToolDeactivated()
{
  if ( mWindowWidget )
  {
    mWindowWidget->raise();
    mWindowWidget->activateWindow();
  }

  if ( mMessageBar && mMessageBarItem )
  {
    mMessageBar->popWidget( mMessageBarItem );
  }
  mMessageBarItem = nullptr;
}

void QgsRelationReferenceWidget::filterChanged()
{
  QVariant nullValue = QSettings().value( "qgis/nullValue", "NULL" );

  QStringList filters;
  QgsAttributeList attrs;

  QComboBox* scb = qobject_cast<QComboBox*>( sender() );

  Q_ASSERT( scb );

  if ( mChainFilters )
  {
    QComboBox* ccb = nullptr;
    Q_FOREACH ( QComboBox* cb, mFilterComboBoxes )
    {
      if ( !ccb )
      {
        if ( cb == scb )
          ccb = cb;

        continue;
      }

      if ( ccb->currentIndex() == 0 )
      {
        cb->setCurrentIndex( 0 );
        cb->setEnabled( false );
      }
      else
      {
        cb->blockSignals( true );
        cb->clear();
        cb->addItem( cb->property( "Field" ).toString() );

        // ccb = scb
        // cb = scb + 1
        Q_FOREACH ( const QString& txt, mFilterCache[ccb->property( "Field" ).toString()][ccb->currentText()] )
        {
          cb->addItem( txt );
        }

        cb->setEnabled( true );
        cb->blockSignals( false );

        ccb = cb;
      }
    }
  }

  Q_FOREACH ( QComboBox* cb, mFilterComboBoxes )
  {
    if ( cb->currentIndex() != 0 )
    {
      const QString fieldName = cb->property( "Field" ).toString();

      if ( cb->currentText() == nullValue.toString() )
      {
        filters << QString( "\"%1\" IS NULL" ).arg( fieldName );
      }
      else
      {
        if ( mReferencedLayer->fields().field( fieldName ).type() == QVariant::String )
        {
          filters << QString( "\"%1\" = '%2'" ).arg( fieldName, cb->currentText() );
        }
        else
        {
          filters << QString( "\"%1\" = %2" ).arg( fieldName, cb->currentText() );
        }
      }
      attrs << mReferencedLayer->fieldNameIndex( fieldName );
    }
  }

  QString filterExpression = filters.join( " AND " );

  QgsFeatureIterator it( mMasterModel->layerCache()->getFeatures( QgsFeatureRequest().setFilterExpression( filterExpression ).setSubsetOfAttributes( attrs ) ) );

  QgsFeature f;
  QgsFeatureIds featureIds;

  while ( it.nextFeature( f ) )
  {
    featureIds << f.id();
  }

  mFilterModel->setFilteredFeatures( featureIds );
}

void QgsRelationReferenceWidget::addEntry()
{
  QgsFeature f( mReferencedLayer->fields() );
  QgsAttributeMap attributes;

  // if custom text is in the combobox and the displayExpression is simply a field, use the current text for the new feature
  if ( mComboBox->itemText( mComboBox->currentIndex() ) != mComboBox->currentText() )
  {
    int fieldIdx = mReferencedLayer->fieldNameIndex( mReferencedLayer->displayExpression() );

    if ( fieldIdx != -1 )
    {
      attributes.insert( fieldIdx, mComboBox->currentText() );
    }
  }

  if ( mEditorContext.vectorLayerTools()->addFeature( mReferencedLayer, attributes, QgsGeometry(), &f ) )
  {
    int i = mComboBox->findData( f.id(), QgsAttributeTableModel::FeatureIdRole );
    mComboBox->setCurrentIndex( i );
    mAddEntryButton->setEnabled( false );
  }
}

void QgsRelationReferenceWidget::updateAddEntryButton()
{
  mAddEntryButton->setVisible( mAllowAddFeatures );
  mAddEntryButton->setEnabled( mReferencedLayer && mReferencedLayer->isEditable() );
}
