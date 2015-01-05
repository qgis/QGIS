/***************************************************************************
    qgsrelationreferencewidget.cpp
     --------------------------------------
    Date                 : 20.4.2013
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


QgsRelationReferenceWidget::QgsRelationReferenceWidget( QWidget* parent )
    : QWidget( parent )
    , mEditorContext( QgsAttributeEditorContext() )
    , mCanvas( NULL )
    , mMessageBar( NULL )
    , mForeignKey( QVariant() )
    , mFeatureId( QgsFeatureId() )
    , mFkeyFieldIdx( -1 )
    , mAllowNull( true )
    , mHighlight( NULL )
    , mMapTool( NULL )
    , mMessageBarItem( NULL )
    , mRelationName( "" )
    , mReferencedAttributeForm( NULL )
    , mReferencedLayer( NULL )
    , mReferencingLayer( NULL )
    , mWindowWidget( NULL )
    , mShown( false )
    , mIsEditable( true )
    , mEmbedForm( false )
    , mReadOnlySelector( false )
    , mAllowMapIdentification( false )
    , mOpenFormButtonVisible( true )
{
  mTopLayout = new QVBoxLayout( this );
  mTopLayout->setContentsMargins( 0, 0, 0, 0 );
  mTopLayout->setAlignment( Qt::AlignTop );
  setLayout( mTopLayout );

  QHBoxLayout* editLayout = new QHBoxLayout();
  editLayout->setContentsMargins( 0, 0, 0, 0 );
  editLayout->setSpacing( 2 );

  // combobox (for non-geometric relation)
  mComboBox = new QComboBox( this );
  editLayout->addWidget( mComboBox );

  // read-only line edit
  mLineEdit = new QLineEdit( this );
  mLineEdit->setReadOnly( true );
  editLayout->addWidget( mLineEdit );

  // open form button
  mOpenFormButton = new QToolButton( this );
  mOpenFormButton->setIcon( QgsApplication::getThemeIcon( "/mActionPropertyItem.png" ) );
  mOpenFormButton->setText( tr( "Open related feature form" ) );
  editLayout->addWidget( mOpenFormButton );

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

  // spacer
  editLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );

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
}

QgsRelationReferenceWidget::~QgsRelationReferenceWidget()
{
  deleteHighlight();
  unsetMapTool();
  if ( mMapTool )
    delete mMapTool;
}

void QgsRelationReferenceWidget::setRelation( QgsRelation relation, bool allowNullValue )
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
    mFkeyFieldIdx = mReferencedLayer->fieldNameIndex( relation.fieldPairs().first().second );

    QgsAttributeEditorContext context( mEditorContext, relation, QgsAttributeEditorContext::Single, QgsAttributeEditorContext::Embed );

    if ( mEmbedForm )
    {
      mAttributeEditorFrame->setTitle( mReferencedLayer->name() );
      mReferencedAttributeForm = new QgsAttributeForm( relation.referencedLayer(), QgsFeature(), context, this );
      mReferencedAttributeForm->hideButtonBox();
      mAttributeEditorLayout->addWidget( mReferencedAttributeForm );
    }
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

  mComboBox->setEnabled( editable );
  mMapIdentificationButton->setEnabled( editable );
  mRemoveFKButton->setEnabled( editable );
  mIsEditable = editable;
}

void QgsRelationReferenceWidget::setForeignKey( const QVariant& value )
{
  if ( !value.isValid() || value.isNull() )
  {
    deleteForeignKey();
    return;
  }

  QgsFeature f;
  if ( !mReferencedLayer )
    return;

  // TODO: Rewrite using expression
  QgsFeatureIterator fit = mReferencedLayer->getFeatures( QgsFeatureRequest() );
  while ( fit.nextFeature( f ) )
  {
    if ( f.attribute( mFkeyFieldIdx ) == value )
    {
      break;
    }
  }

  if ( !f.isValid() )
  {
    deleteForeignKey();
    return;
  }

  mForeignKey = f.attribute( mFkeyFieldIdx );

  if ( mReadOnlySelector )
  {
    QgsExpression expr( mReferencedLayer->displayExpression() );
    QString title = expr.evaluate( &f ).toString();
    if ( expr.hasEvalError() )
    {
      title = f.attribute( mFkeyFieldIdx ).toString();
    }
    mLineEdit->setText( title );
    mFeatureId = f.id();
  }
  else
  {
    int i = mComboBox->findData( value );
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
  highlightFeature( f );
  updateAttributeEditorFrame( f );
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
    mFeatureId = QgsFeatureId();
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

QgsFeature QgsRelationReferenceWidget::referencedFeature()
{
  QgsFeature f;
  if ( mReferencedLayer )
  {
    QgsFeatureId fid;
    if ( mReadOnlySelector )
    {
      fid = mFeatureId;
    }
    else
    {
      fid = mComboBox->itemData( mComboBox->currentIndex() ).value<QgsFeatureId>();
    }
    mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( f );
  }
  return f;
}

QVariant QgsRelationReferenceWidget::foreignKey()
{
  if ( mReadOnlySelector )
  {
    return mForeignKey;
  }
  else
  {
    QVariant varFid = mComboBox->itemData( mComboBox->currentIndex() );
    if ( varFid.isNull() )
    {
      return QVariant();
    }
    else
    {
      return mFidFkMap.value( varFid.value<QgsFeatureId>() );
    }
  }
}

void QgsRelationReferenceWidget::setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar )
{
  mEditorContext = context;
  mCanvas = canvas;
  mMessageBar = messageBar;

  if ( mMapTool )
    delete mMapTool;
  mMapTool = new QgsMapToolIdentifyFeature( mCanvas );
  mMapTool->setAction( mMapIdentificationButton->defaultAction() );
}

void QgsRelationReferenceWidget::setEmbedForm( bool display )
{
  mAttributeEditorFrame->setVisible( display );
  mEmbedForm = display;
}

void QgsRelationReferenceWidget::setReadOnlySelector( bool readOnly )
{
  mComboBox->setHidden( readOnly );
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

void QgsRelationReferenceWidget::setOpenFormButtonVisible( bool openFormButtonVisible )
{
  mOpenFormButton->setVisible( openFormButtonVisible );
  mOpenFormButtonVisible = openFormButtonVisible;
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
    if ( mAllowNull )
    {
      const QString nullValue = QSettings().value( "qgis/nullValue", "NULL" ).toString();

      mComboBox->addItem( tr( "%1 (no selection)" ).arg( nullValue ), QVariant( QVariant::Int ) );
      mComboBox->setItemData( 0, QColor( Qt::gray ), Qt::ForegroundRole );
    }

    QgsExpression exp( mReferencedLayer->displayExpression() );

    QStringList attrs = exp.referencedColumns();
    attrs << mRelation.fieldPairs().first().second;

    QgsFeatureIterator fit = mReferencedLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( attrs, mReferencedLayer->pendingFields() ) );

    exp.prepare( mReferencedLayer->pendingFields() );

    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      QString txt = exp.evaluate( &f ).toString();
      mComboBox->addItem( txt, f.id() );

      if ( f.attribute( mFkeyFieldIdx ) == mForeignKey )
        mComboBox->setCurrentIndex( mComboBox->count() - 1 );

      mFidFkMap.insert( f.id(), f.attribute( mFkeyFieldIdx ) );
    }

    // Only connect after iterating, to have only one iterator on the referenced table at once
    connect( mComboBox, SIGNAL( activated( int ) ), this, SLOT( comboReferenceChanged( int ) ) );
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

  QgsGeometry* geom = f.geometry();
  if ( !geom )
  {
    return;
  }

  // scale or pan
  if ( canvasExtent == Scale )
  {
    QgsRectangle featBBox = geom->boundingBox();
    featBBox = mCanvas->mapSettings().layerToMapCoordinates( mReferencedLayer, featBBox );
    QgsRectangle extent = mCanvas->extent();
    if ( !extent.contains( featBBox ) )
    {
      extent.combineExtentWith( &featBBox );
      extent.scale( 1.1 );
      mCanvas->setExtent( extent );
      mCanvas->refresh();
    }
  }
  else if ( canvasExtent == Pan )
  {
    QgsPoint center = geom->centroid()->asPoint();
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
  mHighlight = NULL;
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

  connect( mMapTool, SIGNAL( featureIdentified( QgsFeature ) ), this, SLOT( featureIdentified( const QgsFeature ) ) );
  connect( mMapTool, SIGNAL( deactivated() ), this, SLOT( mapToolDeactivated() ) );

  if ( mMessageBar )
  {
    QString title = QString( "Relation %1 for %2." ).arg( mRelationName ).arg( mReferencingLayer->name() );
    QString msg = tr( "Identify a feature of %1 to be associated. Press <ESC> to cancel." ).arg( mReferencedLayer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    mMessageBar->pushItem( mMessageBarItem );
  }
}

void QgsRelationReferenceWidget::comboReferenceChanged( int index )
{
  QgsFeatureId fid = mComboBox->itemData( index ).value<QgsFeatureId>();
  QgsFeature feat;
  mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );
  highlightFeature( feat );
  updateAttributeEditorFrame( feat );
  emit foreignKeyChanged( mFidFkMap.value( fid ) );
}

void QgsRelationReferenceWidget::updateAttributeEditorFrame( const QgsFeature feature )
{
  // Check if we're running with an embedded frame we need to update
  if ( mAttributeEditorFrame )
  {
    if ( mReferencedAttributeForm )
    {
      mReferencedAttributeForm->setFeature( feature );
    }
  }
}

void QgsRelationReferenceWidget::featureIdentified( const QgsFeature& feature )
{
  if ( mReadOnlySelector )
  {
    QgsExpression expr( mReferencedLayer->displayExpression() );
    QString title = expr.evaluate( &feature ).toString();
    if ( expr.hasEvalError() )
    {
      title = feature.attribute( mFkeyFieldIdx ).toString();
    }
    mLineEdit->setText( title );
    mForeignKey = feature.attribute( mFkeyFieldIdx );
    mFeatureId = feature.id();
  }
  else
  {
    mComboBox->setCurrentIndex( mComboBox->findData( feature.attribute( mFkeyFieldIdx ) ) );
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
  mMessageBarItem = NULL;
}

