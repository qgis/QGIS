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

#include "qgsattributedialog.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgseditorwidgetfactory.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsrelreferenceconfigdlg.h"
#include "qgsvectorlayer.h"


QgsRelationReferenceWidget::QgsRelationReferenceWidget( QWidget* parent )
    : QWidget( parent )
    , mEditorContext( QgsAttributeEditorContext() )
    , mCanvas( NULL )
    , mMessageBar( NULL )
    , mHighlight( NULL )
    , mInitialValueAssigned( false )
    , mMapTool( NULL )
    , mMessageBarItem( NULL )
    , mRelationName( "" )
    , mReferencedAttributeDialog( NULL )
    , mReferencedLayer( NULL )
    , mReferencingLayer( NULL )
    , mWindowWidget( NULL )
    , mEmbedForm( false )
    , mReadOnlySelector( false )
    , mAllowMapIdentification( false )
{
  mTopLayout = new QVBoxLayout( this );
  mTopLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( mTopLayout );

  QHBoxLayout* editLayout = new QHBoxLayout();
  editLayout->setContentsMargins( 0, 0, 0, 0 );

  // combobox (for non-geometric relation)
  mComboBox = new QComboBox( this );
  editLayout->addWidget( mComboBox );

  // read-only line edit
  mLineEdit = new QLineEdit( this );
  mLineEdit->setReadOnly( true );
  editLayout->addWidget( mLineEdit );

  // open form button
  mOpenFormButton = new QToolButton( this );
  mOpenFormAction = new QAction( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ), tr( "Open related feature form" ), this );
  mOpenFormButton->addAction( mOpenFormAction );
  mOpenFormButton->setDefaultAction( mOpenFormAction );
  connect( mOpenFormButton, SIGNAL( triggered( QAction* ) ), this, SLOT( openForm() ) );
  editLayout->addWidget( mOpenFormButton );

  // highlight button
  mHighlightFeatureButton = new QToolButton( this );
  mHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionHighlightFeature.svg" ), tr( "Highlight feature" ), this );
  mScaleHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionScaleHighlightFeature.svg" ), tr( "Scale and highlight feature" ), this );
  mPanHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( "/mActionPanHighlightFeature.svg" ), tr( "Pan and highlight feature" ), this );
  mHighlightFeatureButton->addAction( mHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mScaleHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mPanHighlightFeatureAction );
  mHighlightFeatureButton->setDefaultAction( mHighlightFeatureAction );
  connect( mHighlightFeatureButton, SIGNAL( triggered( QAction* ) ), this, SLOT( highlightActionTriggered( QAction* ) ) );
  editLayout->addWidget( mHighlightFeatureButton );

  // map identification button
  mMapIdentificationButton = new QToolButton( this );
  mMapIdentificationAction = new QAction( QgsApplication::getThemeIcon( "/mActionMapIdentification.svg" ), tr( "Select on map" ), this );
  mMapIdentificationButton->addAction( mMapIdentificationAction );
  mMapIdentificationButton->setDefaultAction( mMapIdentificationAction );
  connect( mMapIdentificationButton, SIGNAL( triggered( QAction* ) ), this, SLOT( mapIdentification() ) );
  editLayout->addWidget( mMapIdentificationButton );

  // spacer
  editLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ) );

  // add line to top layout
  mTopLayout->addLayout( editLayout );

  // embed form
  mAttributeEditorFrame = new QgsCollapsibleGroupBox( this );
  mAttributeEditorFrame->setCollapsed( true );
  mAttributeEditorLayout = new QVBoxLayout( mAttributeEditorFrame );
  mAttributeEditorFrame->setLayout( mAttributeEditorLayout );
  mTopLayout->addWidget( mAttributeEditorFrame );

  // default mode is combobox, no geometric relation and no embed form
  mLineEdit->hide();
  mMapIdentificationButton->hide();
  mHighlightFeatureButton->hide();
  mAttributeEditorFrame->hide();
}

QgsRelationReferenceWidget::~QgsRelationReferenceWidget()
{
  deleteHighlight();
  delete mMapTool;
}

void QgsRelationReferenceWidget::setRelation( QgsRelation relation, bool allowNullValue )
{
  if ( relation.isValid() )
  {
    if ( allowNullValue )
    {
      const QString nullValue = QSettings().value( "qgis/nullValue", "NULL" ).toString();
      mComboBox->addItem( nullValue );
      mComboBox->setItemData( mComboBox->count() - 1, Qt::gray, Qt::ForegroundRole );
    }

    mReferencingLayer = relation.referencingLayer();
    mRelationName = relation.name();
    mReferencedLayer = relation.referencedLayer();
    int refFieldIdx = mReferencedLayer->fieldNameIndex( relation.fieldPairs().first().second );

    QgsFeatureIterator fit = mReferencedLayer->getFeatures( QgsFeatureRequest() );

    QgsExpression exp( mReferencedLayer->displayExpression() );
    exp.prepare( mReferencedLayer->pendingFields() );

    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      QString txt = exp.evaluate( &f ).toString();

      mComboBox->addItem( txt, f.id() );

      mFidFkMap.insert( f.id(), f.attribute( refFieldIdx ) );
    }


    // Only connect after iterating, to have only one iterator on the referenced table at once
    connect( mComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( referenceChanged( int ) ) );
  }
  else
  {
    QLabel* lbl = new QLabel( tr( "The relation is not valid. Please make sure your relation definitions are ok." ) );
    QFont font = lbl->font();
    font.setItalic( true );
    lbl->setStyleSheet( "QLabel { color: red; } " );
    lbl->setFont( font );
    mTopLayout->addWidget( lbl, 1, 0 );
  }
}

void QgsRelationReferenceWidget::setRelationEditable( bool editable )
{
  mLineEdit->setEnabled( editable );
  mComboBox->setEnabled( editable );
  mMapIdentificationButton->setEnabled( editable );
}

void QgsRelationReferenceWidget::setRelatedFeature( const QVariant& value )
{
  const QgsFeatureId fid = mFidFkMap.key( value );
  if ( mReferencedLayer )
    setRelatedFeature( fid );
}

void QgsRelationReferenceWidget::setRelatedFeature( const QgsFeatureId& fid )
{
  int oldIdx = mComboBox->currentIndex();
  int newIdx = mComboBox->findData( fid );
  mComboBox->setCurrentIndex( newIdx );

  if ( !mInitialValueAssigned )
  {
    // In case the default-selected item (first) is the actual item
    // then no referenceChanged event was triggered automatically:
    // Do it!
    if ( oldIdx == mComboBox->currentIndex() )
      referenceChanged( mComboBox->currentIndex() );
    mInitialValueAssigned = true;
  }

  // update line edit
  mLineEdit->setText( mFidFkMap.value( fid ).toString() );
}

void QgsRelationReferenceWidget::mapToolDeactivated()
{
  if ( mWindowWidget )
  {
    mWindowWidget->show();
  }

  if ( mMessageBar && mMessageBarItem )
  {
    mMessageBar->popWidget( mMessageBarItem );
  }
  mMessageBarItem = NULL;
}

QVariant QgsRelationReferenceWidget::relatedFeature()
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

void QgsRelationReferenceWidget::setEditorContext( QgsAttributeEditorContext context, QgsMapCanvas* canvas, QgsMessageBar* messageBar )
{
  mEditorContext = context;
  mCanvas = canvas;
  mMessageBar = messageBar;
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
  mReadOnlySelector = readOnly;
}

void QgsRelationReferenceWidget::setAllowMapIdentification( bool allowMapIdentification )
{
  mHighlightFeatureButton->setVisible( allowMapIdentification );
  mMapIdentificationButton->setVisible( allowMapIdentification );
  mAllowMapIdentification = allowMapIdentification;
}

void QgsRelationReferenceWidget::highlightActionTriggered( QAction* action )
{
  if ( action == mHighlightFeatureAction )
  {
    highlightFeature();
  }
  else if ( action == mScaleHighlightFeatureAction )
  {
    highlightFeature( Scale );
  }
  else if ( action == mPanHighlightFeatureAction )
  {
    highlightFeature( Pan );
  }
}

void QgsRelationReferenceWidget::openForm()
{
  QgsFeatureId fid = mComboBox->itemData( mComboBox->currentIndex() ).value<QgsFeatureId>();

  QgsFeature feat;

  if ( !mReferencedLayer )
    return;

  mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

  if ( !feat.isValid() )
    return;

  // TODO: Get a proper QgsDistanceArea thingie
  mReferencedAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, this, true, mEditorContext );
  mReferencedAttributeDialog->exec();
  delete mReferencedAttributeDialog;
}

void QgsRelationReferenceWidget::highlightFeature( CanvasExtent canvasExtent )
{
  QgsFeatureId fid = mComboBox->itemData( mComboBox->currentIndex() ).value<QgsFeatureId>();

  QgsFeature feat;

  if ( !mReferencedLayer )
    return;

  mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

  if ( !feat.isValid() )
    return;

  if ( !mCanvas )
    return;

  QgsGeometry* geom = feat.geometry();
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
  mHighlight = new QgsHighlight( mCanvas, feat, mReferencedLayer );
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
  if ( !mReferencedLayer )
    return;

  QgsVectorLayerTools* tools = mEditorContext.vectorLayerTools();
  if ( !tools )
    return;
  if ( !mCanvas )
    return;

  mMapTool = new QgsMapToolIdentifyFeature( mReferencedLayer, mCanvas );
  mCanvas->setMapTool( mMapTool );
  mWindowWidget = window();
  mWindowWidget->hide();
  connect( mMapTool, SIGNAL( featureIdentified( QgsFeatureId ) ), this, SLOT( featureIdentified( QgsFeatureId ) ) );
  connect( mMapTool, SIGNAL( deactivated() ), this, SLOT( mapToolDeactivated() ) );

  if ( mMessageBar )
  {
    QString title = QString( "Relation %1 for %2." ).arg( mRelationName ).arg( mReferencingLayer->name() );
    QString msg = tr( "identify a feature of %1 to be associated. Press <ESC> to cancel." ).arg( mReferencedLayer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg );
    mMessageBar->pushItem( mMessageBarItem );
  }
}

void QgsRelationReferenceWidget::referenceChanged( int index )
{
  QgsFeatureId fid = mComboBox->itemData( index ).value<QgsFeatureId>();

  highlightFeature();

  emit relatedFeatureChanged( mFidFkMap.value( fid ) );

  // Check if we're running with an embedded frame we need to update
  if ( mAttributeEditorFrame )
  {
    QgsFeature feat;

    mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

    if ( feat.isValid() )
    {
      if ( mReferencedAttributeDialog )
      {
        mAttributeEditorLayout->removeWidget( mReferencedAttributeDialog );
      }

      // TODO: Get a proper QgsDistanceArea thingie
      mReferencedAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, mAttributeEditorFrame, false, mEditorContext );
      QWidget* attrDialog = mReferencedAttributeDialog;
      attrDialog->setWindowFlags( Qt::Widget ); // Embed instead of opening as window
      mAttributeEditorLayout->addWidget( attrDialog );
      attrDialog->show();
    }
  }
}

void QgsRelationReferenceWidget::featureIdentified( const QgsFeatureId& fid )
{
  setRelatedFeature( fid );

  // deactivate map tool if activate
  if ( mCanvas && mMapTool )
  {
    mCanvas->unsetMapTool( mMapTool );
  }

  if ( mWindowWidget )
    mWindowWidget->show();
}
