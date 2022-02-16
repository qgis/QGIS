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
#include <QCompleter>

#include "qgsattributeform.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributedialog.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgseditorwidgetfactory.h"
#include "qgsexpression.h"
#include "qgsfeaturelistmodel.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsrelationreferenceconfigdlg.h"
#include "qgsvectorlayer.h"
#include "qgsattributetablemodel.h"
#include "qgsmaptoolidentifyfeature.h"
#include "qgsmaptooldigitizefeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturelistcombobox.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeaturefiltermodel.h"
#include "qgsidentifymenu.h"
#include "qgsvectorlayerutils.h"


bool qVariantListIsNull( const QVariantList &list )
{
  if ( list.isEmpty() )
    return true;

  for ( int i = 0; i < list.size(); ++i )
  {
    if ( !list.at( i ).isNull() )
      return false;
  }
  return true;
}


QgsRelationReferenceWidget::QgsRelationReferenceWidget( QWidget *parent )
  : QWidget( parent )
{
  mTopLayout = new QVBoxLayout( this );
  mTopLayout->setContentsMargins( 0, 0, 0, 0 );

  setSizePolicy( sizePolicy().horizontalPolicy(), QSizePolicy::Fixed );

  setLayout( mTopLayout );

  QHBoxLayout *editLayout = new QHBoxLayout();
  editLayout->setContentsMargins( 0, 0, 0, 0 );
  editLayout->setSpacing( 2 );

  // Prepare the container and layout for the filter comboboxes
  mChooserContainer = new QWidget;
  editLayout->addWidget( mChooserContainer );
  QHBoxLayout *chooserLayout = new QHBoxLayout;
  chooserLayout->setContentsMargins( 0, 0, 0, 0 );
  mFilterLayout = new QHBoxLayout;
  mFilterLayout->setContentsMargins( 0, 0, 0, 0 );
  mFilterContainer = new QWidget;
  mFilterContainer->setLayout( mFilterLayout );
  mChooserContainer->setLayout( chooserLayout );
  chooserLayout->addWidget( mFilterContainer );

  mComboBox = new QgsFeatureListComboBox();
  mChooserContainer->layout()->addWidget( mComboBox );

  // open form button
  mOpenFormButton = new QToolButton();
  mOpenFormButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPropertyItem.svg" ) ) );
  mOpenFormButton->setText( tr( "Open Related Feature Form" ) );
  editLayout->addWidget( mOpenFormButton );

  mAddEntryButton = new QToolButton();
  mAddEntryButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAdd.svg" ) ) );
  mAddEntryButton->setText( tr( "Add New Entry" ) );
  editLayout->addWidget( mAddEntryButton );

  // highlight button
  mHighlightFeatureButton = new QToolButton( this );
  mHighlightFeatureButton->setPopupMode( QToolButton::MenuButtonPopup );
  mHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionHighlightFeature.svg" ) ), tr( "Highlight feature" ), this );
  mScaleHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionScaleHighlightFeature.svg" ) ), tr( "Scale and highlight feature" ), this );
  mPanHighlightFeatureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPanHighlightFeature.svg" ) ), tr( "Pan and highlight feature" ), this );
  mHighlightFeatureButton->addAction( mHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mScaleHighlightFeatureAction );
  mHighlightFeatureButton->addAction( mPanHighlightFeatureAction );
  mHighlightFeatureButton->setDefaultAction( mHighlightFeatureAction );
  editLayout->addWidget( mHighlightFeatureButton );

  // map identification button
  mMapIdentificationButton = new QToolButton( this );
  mMapIdentificationButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
  mMapIdentificationButton->setText( tr( "Select on Map" ) );
  mMapIdentificationButton->setCheckable( true );
  editLayout->addWidget( mMapIdentificationButton );

  // remove foreign key button
  mRemoveFKButton = new QToolButton( this );
  mRemoveFKButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemove.svg" ) ) );
  mRemoveFKButton->setText( tr( "No Selection" ) );
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
  mInvalidLabel = new QLabel( tr( "The relation is not valid. Please make sure your relation definitions are OK." ) );
  mInvalidLabel->setWordWrap( true );
  QFont font = mInvalidLabel->font();
  font.setItalic( true );
  mInvalidLabel->setStyleSheet( QStringLiteral( "QLabel { color: red; } " ) );
  mInvalidLabel->setFont( font );
  mTopLayout->addWidget( mInvalidLabel );

  // default mode is combobox, no geometric relation and no embed form
  mMapIdentificationButton->hide();
  mHighlightFeatureButton->hide();
  mAttributeEditorFrame->hide();
  mInvalidLabel->hide();
  mAddEntryButton->hide();

  // connect buttons
  connect( mOpenFormButton, &QAbstractButton::clicked, this, &QgsRelationReferenceWidget::openForm );
  connect( mHighlightFeatureButton, &QToolButton::triggered, this, &QgsRelationReferenceWidget::highlightActionTriggered );
  connect( mMapIdentificationButton, &QAbstractButton::clicked, this, &QgsRelationReferenceWidget::mapIdentification );
  connect( mRemoveFKButton, &QAbstractButton::clicked, this, &QgsRelationReferenceWidget::deleteForeignKeys );
  connect( mAddEntryButton, &QAbstractButton::clicked, this, &QgsRelationReferenceWidget::addEntry );
  connect( mComboBox, &QComboBox::editTextChanged, this, &QgsRelationReferenceWidget::updateAddEntryButton );
}

QgsRelationReferenceWidget::~QgsRelationReferenceWidget()
{
  deleteHighlight();
  unsetMapTool();
}

void QgsRelationReferenceWidget::setRelation( const QgsRelation &relation, bool allowNullValue )
{
  mAllowNull = allowNullValue;
  mRemoveFKButton->setVisible( allowNullValue && mReadOnlySelector );

  if ( relation.isValid() )
  {
    mReferencedLayerId = relation.referencedLayerId();
    mReferencedLayerName = relation.referencedLayer()->name();
    setReferencedLayerDataSource( relation.referencedLayer()->publicSource() );
    mReferencedLayerProviderKey = relation.referencedLayer()->providerType();
    mInvalidLabel->hide();

    mRelation = relation;
    mReferencingLayer = relation.referencingLayer();
    mReferencedLayer = relation.referencedLayer();

    const QList<QgsRelation::FieldPair> fieldPairs = relation.fieldPairs();
    for ( const QgsRelation::FieldPair &fieldPair : fieldPairs )
    {
      mReferencedFields << fieldPair.referencedField();
    }
    if ( mComboBox )
    {
      mComboBox->setAllowNull( mAllowNull );
      mComboBox->setSourceLayer( mReferencedLayer );
      mComboBox->setIdentifierFields( mReferencedFields );
      mComboBox->setFilterExpression( mFilterExpression );
    }
    mAttributeEditorFrame->setObjectName( QStringLiteral( "referencing/" ) + relation.name() );

    if ( mEmbedForm )
    {
      QgsAttributeEditorContext context( mEditorContext, relation, QgsAttributeEditorContext::Single, QgsAttributeEditorContext::Embed );
      mAttributeEditorFrame->setTitle( mReferencedLayer->name() );
      mReferencedAttributeForm = new QgsAttributeForm( relation.referencedLayer(), QgsFeature(), context, this );
      mAttributeEditorLayout->addWidget( mReferencedAttributeForm );
    }

    connect( mReferencedLayer, &QgsVectorLayer::editingStarted, this, &QgsRelationReferenceWidget::updateAddEntryButton );
    connect( mReferencedLayer, &QgsVectorLayer::editingStopped, this, &QgsRelationReferenceWidget::updateAddEntryButton );
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
  {
    unsetMapTool();
  }

  mFilterContainer->setEnabled( editable );
  mComboBox->setEnabled( editable && !mReadOnlySelector );
  mComboBox->setEditable( true );
  mMapIdentificationButton->setEnabled( editable );
  mRemoveFKButton->setEnabled( editable );
  mIsEditable = editable;
}

void QgsRelationReferenceWidget::setForeignKey( const QVariant &value )
{
  setForeignKeys( QVariantList() << value );
}

void QgsRelationReferenceWidget::setForeignKeys( const QVariantList &values )
{
  if ( values.isEmpty() )
  {
    return;
  }
  if ( qVariantListIsNull( values ) )
  {
    deleteForeignKeys();
    return;
  }

  if ( !mReferencedLayer )
    return;

  mComboBox->setIdentifierValues( values );

  if ( mEmbedForm || mChainFilters )
  {
    QgsFeatureRequest request = mComboBox->currentFeatureRequest();
    mReferencedLayer->getFeatures( request ).nextFeature( mFeature );
  }
  if ( mChainFilters )
  {
    QVariant nullValue = QgsApplication::nullRepresentation();
    const int count = std::min( mFilterComboBoxes.size(), mFilterFields.size() );
    for ( int i = 0; i < count; i++ )
    {
      QVariant v = mFeature.attribute( mFilterFields[i] );
      QString f = v.isNull() ? nullValue.toString() : v.toString();
      mFilterComboBoxes.at( i )->setCurrentIndex( mFilterComboBoxes.at( i )->findText( f ) );
    }
  }

  mRemoveFKButton->setEnabled( mIsEditable );
  highlightFeature( mFeature ); // TODO : make this async
  updateAttributeEditorFrame( mFeature );

  emitForeignKeysChanged( foreignKeys() );
}

void QgsRelationReferenceWidget::deleteForeignKeys()
{
  // deactivate filter comboboxes
  if ( mChainFilters && !mFilterComboBoxes.isEmpty() )
  {
    QComboBox *cb = mFilterComboBoxes.first();
    cb->setCurrentIndex( 0 );
    disableChainedComboBoxes( cb );
  }

  mComboBox->setIdentifierValuesToNull();
  mRemoveFKButton->setEnabled( false );
  updateAttributeEditorFrame( QgsFeature() );

  emitForeignKeysChanged( foreignKeys() );
}

QgsFeature QgsRelationReferenceWidget::referencedFeature() const
{
  QgsFeature f;
  if ( mReferencedLayer )
  {
    mReferencedLayer->getFeatures( mComboBox->currentFeatureRequest() ).nextFeature( f );
  }
  return f;
}

void QgsRelationReferenceWidget::showIndeterminateState()
{
  whileBlocking( mComboBox )->setIdentifierValuesToNull();
  mRemoveFKButton->setEnabled( false );
  updateAttributeEditorFrame( QgsFeature() );
}

QVariant QgsRelationReferenceWidget::foreignKey() const
{
  QVariantList fkeys;
  if ( fkeys.isEmpty() )
    return QVariant( QVariant::Int );
  else
    return fkeys.at( 0 );
}

QVariantList QgsRelationReferenceWidget::foreignKeys() const
{
  return mComboBox->identifierValues();
}

void QgsRelationReferenceWidget::setEditorContext( const QgsAttributeEditorContext &context, QgsMapCanvas *canvas, QgsMessageBar *messageBar )
{
  mEditorContext = context;
  mCanvas = canvas;
  mMessageBar = messageBar;

  mMapToolIdentify.reset( new QgsMapToolIdentifyFeature( mCanvas ) );
  mMapToolIdentify->setButton( mMapIdentificationButton );

  if ( mEditorContext.cadDockWidget() )
  {
    mMapToolDigitize.reset( new QgsMapToolDigitizeFeature( mCanvas, mEditorContext.cadDockWidget() ) );
    mMapToolDigitize->setButton( mAddEntryButton );
    updateAddEntryButton();
  }
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
  mComboBox->setEnabled( !readOnly );
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

void QgsRelationReferenceWidget::setFilterFields( const QStringList &filterFields )
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

void QgsRelationReferenceWidget::setFilterExpression( const QString &expression )
{
  mFilterExpression = expression;
}

void QgsRelationReferenceWidget::showEvent( QShowEvent *e )
{
  Q_UNUSED( e )

  mShown = true;
  if ( !mInitialized )
    init();
}

void QgsRelationReferenceWidget::init()
{
  if ( mReferencedLayer )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QSet<QString> requestedAttrs;

    if ( !mFilterFields.isEmpty() )
    {
      for ( const QString &fieldName : std::as_const( mFilterFields ) )
      {
        int idx = mReferencedLayer->fields().lookupField( fieldName );

        if ( idx == -1 )
          continue;

        QComboBox *cb = new QComboBox();
        cb->setProperty( "Field", fieldName );
        cb->setProperty( "FieldAlias", mReferencedLayer->attributeDisplayName( idx ) );
        mFilterComboBoxes << cb;
        QVariantList uniqueValues = qgis::setToList( mReferencedLayer->uniqueValues( idx ) );
        cb->addItem( mReferencedLayer->attributeDisplayName( idx ) );
        QVariant nullValue = QgsApplication::nullRepresentation();
        cb->addItem( nullValue.toString(), QVariant( mReferencedLayer->fields().at( idx ).type() ) );

        std::sort( uniqueValues.begin(), uniqueValues.end(), qgsVariantLessThan );
        const auto constUniqueValues = uniqueValues;
        for ( const QVariant &v : constUniqueValues )
        {
          cb->addItem( v.toString(), v );
        }

        connect( cb, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRelationReferenceWidget::filterChanged );

        // Request this attribute for caching
        requestedAttrs << fieldName;

        mFilterLayout->addWidget( cb );
      }

      if ( mChainFilters )
      {
        QVariant nullValue = QgsApplication::nullRepresentation();

        QgsFeature ft;
        QgsFeatureIterator fit = mFilterExpression.isEmpty()
                                 ? mReferencedLayer->getFeatures()
                                 : mReferencedLayer->getFeatures( mFilterExpression );
        while ( fit.nextFeature( ft ) )
        {
          const int count = std::min( mFilterComboBoxes.count(), mFilterFields.count() );
          for ( int i = 0; i < count - 1; i++ )
          {
            QVariant cv = ft.attribute( mFilterFields.at( i ) );
            QVariant nv = ft.attribute( mFilterFields.at( i + 1 ) );
            QString cf = cv.isNull() ? nullValue.toString() : cv.toString();
            QString nf = nv.isNull() ? nullValue.toString() : nv.toString();
            mFilterCache[mFilterFields[i]][cf] << nf;
          }
        }

        if ( !mFilterComboBoxes.isEmpty() )
        {
          QComboBox *cb = mFilterComboBoxes.first();
          cb->setCurrentIndex( 0 );
          disableChainedComboBoxes( cb );
        }
      }
    }
    else
    {
      mFilterContainer->hide();
    }

    mComboBox->setSourceLayer( mReferencedLayer );
    mComboBox->setDisplayExpression( mReferencedLayer->displayExpression() );
    mComboBox->setAllowNull( mAllowNull );
    mComboBox->setIdentifierFields( mReferencedFields );

    if ( ! mFilterExpression.isEmpty() )
      mComboBox->setFilterExpression( mFilterExpression );

    QVariant nullValue = QgsApplication::nullRepresentation();

    if ( mChainFilters && mFeature.isValid() )
    {
      for ( int i = 0; i < mFilterFields.size(); i++ )
      {
        QVariant v = mFeature.attribute( mFilterFields[i] );
        QString f = v.isNull() ? nullValue.toString() : v.toString();
        mFilterComboBoxes.at( i )->setCurrentIndex( mFilterComboBoxes.at( i )->findText( f ) );
      }
    }

    // Only connect after iterating, to have only one iterator on the referenced table at once
    connect( mComboBox, &QgsFeatureListComboBox::currentFeatureChanged, this, &QgsRelationReferenceWidget::comboReferenceChanged );

    QApplication::restoreOverrideCursor();

    mInitialized = true;
  }
}

void QgsRelationReferenceWidget::highlightActionTriggered( QAction *action )
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
  QgsAttributeDialog *attributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, this, true, context );
  attributeDialog->show();
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

  if ( !f.hasGeometry() )
  {
    return;
  }

  QgsGeometry geom = f.geometry();

  // scale or pan
  if ( canvasExtent == Scale )
  {
    QgsRectangle featBBox = geom.boundingBox();
    featBBox = mCanvas->mapSettings().layerToMapCoordinates( mReferencedLayer, featBBox );
    QgsRectangle extent = mCanvas->extent();
    if ( !extent.contains( featBBox ) )
    {
      extent.combineExtentWith( featBBox );
      extent.scale( 1.1 );
      mCanvas->setExtent( extent, true );
      mCanvas->refresh();
    }
  }
  else if ( canvasExtent == Pan )
  {
    QgsGeometry centroid = geom.centroid();
    QgsPointXY center = centroid.asPoint();
    center = mCanvas->mapSettings().layerToMapCoordinates( mReferencedLayer, center );
    mCanvas->zoomByFactor( 1.0, &center ); // refresh is done in this method
  }

  // highlight
  deleteHighlight();
  mHighlight = new QgsHighlight( mCanvas, f, mReferencedLayer );
  QgsIdentifyMenu::styleHighlight( mHighlight );
  mHighlight->show();

  QTimer *timer = new QTimer( this );
  timer->setSingleShot( true );
  connect( timer, &QTimer::timeout, this, &QgsRelationReferenceWidget::deleteHighlight );
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

  const QgsVectorLayerTools *tools = mEditorContext.vectorLayerTools();
  if ( !tools )
    return;
  if ( !mCanvas )
    return;

  mMapToolIdentify->setLayer( mReferencedLayer );
  setMapTool( mMapToolIdentify );

  connect( mMapToolIdentify, qOverload<const QgsFeature &>( &QgsMapToolIdentifyFeature::featureIdentified ), this, &QgsRelationReferenceWidget::featureIdentified );

  if ( mMessageBar )
  {
    QString title = tr( "Relation %1 for %2." ).arg( mRelation.name(), mReferencingLayer->name() );
    QString msg = tr( "Identify a feature of %1 to be associated. Press &lt;ESC&gt; to cancel." ).arg( mReferencedLayer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    mMessageBar->pushItem( mMessageBarItem );
  }
}

void QgsRelationReferenceWidget::comboReferenceChanged()
{
  mReferencedLayer->getFeatures( mComboBox->currentFeatureRequest() ).nextFeature( mFeature );
  highlightFeature( mFeature );
  updateAttributeEditorFrame( mFeature );

  emitForeignKeysChanged( mComboBox->identifierValues() );
}

void QgsRelationReferenceWidget::updateAttributeEditorFrame( const QgsFeature &feature )
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

QgsRelation QgsRelationReferenceWidget::relation() const
{
  return mRelation;
}

void QgsRelationReferenceWidget::featureIdentified( const QgsFeature &feature )
{
  mComboBox->setCurrentFeature( feature );
  mFeature = feature;

  mRemoveFKButton->setEnabled( mIsEditable );
  highlightFeature( feature );
  updateAttributeEditorFrame( feature );
  emitForeignKeysChanged( foreignKeys(), true );

  unsetMapTool();
}

void QgsRelationReferenceWidget::setMapTool( QgsMapTool *mapTool )
{
  mCurrentMapTool = mapTool;
  mCanvas->setMapTool( mapTool );

  mWindowWidget = window();

  mCanvas->window()->raise();
  mCanvas->activateWindow();
  mCanvas->setFocus();
  connect( mapTool, &QgsMapTool::deactivated, this, &QgsRelationReferenceWidget::mapToolDeactivated );
}

void QgsRelationReferenceWidget::unsetMapTool()
{
  // deactivate map tools if activated
  if ( mCurrentMapTool )
  {
    /* this will call mapToolDeactivated */
    mCanvas->unsetMapTool( mCurrentMapTool );

    if ( mCurrentMapTool == mMapToolDigitize )
    {
      disconnect( mCanvas, &QgsMapCanvas::keyPressed, this, &QgsRelationReferenceWidget::onKeyPressed );
      disconnect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationReferenceWidget::entryAdded );
    }
    else
    {
      disconnect( mMapToolIdentify, qOverload<const QgsFeature &>( &QgsMapToolIdentifyFeature::featureIdentified ), this, &QgsRelationReferenceWidget::featureIdentified );
    }
  }
}

void QgsRelationReferenceWidget::onKeyPressed( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    unsetMapTool();
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
  QVariant nullValue = QgsApplication::nullRepresentation();

  QMap<QString, QString> filters;
  QgsAttributeList attrs;

  QComboBox *scb = qobject_cast<QComboBox *>( sender() );

  Q_ASSERT( scb );

  QgsFeature f;
  QgsFeatureIds featureIds;
  QString filterExpression = mFilterExpression;

  // wrap the expression with parentheses as it might contain `OR`
  if ( !filterExpression.isEmpty() )
    filterExpression = QStringLiteral( " ( %1 ) " ).arg( filterExpression );

  // comboboxes have to be disabled before building filters
  if ( mChainFilters )
    disableChainedComboBoxes( scb );

  // build filters
  const auto constMFilterComboBoxes = mFilterComboBoxes;
  for ( QComboBox *cb : constMFilterComboBoxes )
  {
    if ( cb->currentIndex() != 0 )
    {
      const QString fieldName = cb->property( "Field" ).toString();

      if ( cb->currentText() == nullValue.toString() )
      {
        filters[fieldName] = QStringLiteral( "\"%1\" IS NULL" ).arg( fieldName );
      }
      else
      {
        filters[fieldName] = QgsExpression::createFieldEqualityExpression( fieldName, cb->currentText() );
      }
      attrs << mReferencedLayer->fields().lookupField( fieldName );
    }
  }

  if ( mChainFilters )
  {
    QComboBox *ccb = nullptr;
    const auto constMFilterComboBoxes = mFilterComboBoxes;
    for ( QComboBox *cb : constMFilterComboBoxes )
    {
      if ( !ccb )
      {
        if ( cb == scb )
          ccb = cb;

        continue;
      }

      if ( ccb->currentIndex() != 0 )
      {
        const QString fieldName = cb->property( "Field" ).toString();

        cb->blockSignals( true );
        cb->clear();
        cb->addItem( cb->property( "FieldAlias" ).toString() );

        // ccb = scb
        // cb = scb + 1
        QStringList texts;
        const auto txts { mFilterCache[ccb->property( "Field" ).toString()][ccb->currentText()] };
        for ( const QString &txt : txts )
        {
          QMap<QString, QString> filtersAttrs = filters;
          filtersAttrs[fieldName] = QgsExpression::createFieldEqualityExpression( fieldName, txt );
          QgsAttributeList subset = attrs;

          QString expression = filterExpression;
          if ( ! filterExpression.isEmpty() && ! filtersAttrs.values().isEmpty() )
            expression += QLatin1String( " AND " );

          expression += filtersAttrs.isEmpty() ? QString() : QStringLiteral( " ( " );
          expression += filtersAttrs.values().join( QLatin1String( " AND " ) );
          expression += filtersAttrs.isEmpty() ? QString() : QStringLiteral( " ) " );

          subset << mReferencedLayer->fields().lookupField( fieldName );

          QgsFeatureIterator it( mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterExpression( expression ).setSubsetOfAttributes( subset ) ) );

          bool found = false;
          while ( it.nextFeature( f ) )
          {
            if ( !featureIds.contains( f.id() ) )
              featureIds << f.id();

            found = true;
          }

          // item is only provided if at least 1 feature exists
          if ( found )
            texts << txt;
        }

        texts.sort();
        cb->addItems( texts );

        cb->setEnabled( true );
        cb->blockSignals( false );

        ccb = cb;
      }
    }
  }

  if ( ! filterExpression.isEmpty() && ! filters.values().isEmpty() )
    filterExpression += QLatin1String( " AND " );

  filterExpression += filters.isEmpty() ? QString() : QStringLiteral( " ( " );
  filterExpression += filters.values().join( QLatin1String( " AND " ) );
  filterExpression += filters.isEmpty() ? QString() : QStringLiteral( " ) " );

  mComboBox->setFilterExpression( filterExpression );
}

void QgsRelationReferenceWidget::addEntry()
{
  if ( !mReferencedLayer )
    return;

  const QgsVectorLayerTools *tools = mEditorContext.vectorLayerTools();
  if ( !tools )
    return;
  if ( !mCanvas )
    return;

  // no geometry, skip the digitizing
  if ( mReferencedLayer->geometryType() == QgsWkbTypes::UnknownGeometry || mReferencedLayer->geometryType() == QgsWkbTypes::NullGeometry )
  {
    QgsFeature f( mReferencedLayer->fields() );
    entryAdded( f );
    return;
  }

  mMapToolDigitize->setLayer( mReferencedLayer );
  setMapTool( mMapToolDigitize );

  connect( mMapToolDigitize, &QgsMapToolDigitizeFeature::digitizingCompleted, this, &QgsRelationReferenceWidget::entryAdded );
  connect( mCanvas, &QgsMapCanvas::keyPressed, this, &QgsRelationReferenceWidget::onKeyPressed );

  if ( mMessageBar )
  {
    QString title = tr( "Relation %1 for %2." ).arg( mRelation.name(), mReferencingLayer->name() );

    QString displayString = QgsVectorLayerUtils::getFeatureDisplayString( mReferencingLayer, mFormFeature );
    QString msg = tr( "Link feature to %1 \"%2\" : Digitize the geometry for the new feature on layer %3. Press &lt;ESC&gt; to cancel." )
                  .arg( mReferencingLayer->name(), displayString, mReferencedLayer->name() );
    mMessageBarItem = QgsMessageBar::createMessage( title, msg, this );
    mMessageBar->pushItem( mMessageBarItem );
  }

}

void QgsRelationReferenceWidget::entryAdded( const QgsFeature &feat )
{
  QgsFeature f( feat );
  QgsAttributeMap attributes;

  // if custom text is in the combobox and the displayExpression is simply a field, use the current text for the new feature
  if ( mComboBox->itemText( mComboBox->currentIndex() ) != mComboBox->currentText() )
  {
    int fieldIdx = mReferencedLayer->fields().lookupField( mReferencedLayer->displayExpression() );

    if ( fieldIdx != -1 )
    {
      attributes.insert( fieldIdx, mComboBox->currentText() );
    }
  }

  if ( mEditorContext.vectorLayerTools()->addFeature( mReferencedLayer, attributes, f.geometry(), &f, this, false, true ) )
  {
    QVariantList attrs;
    for ( const QString &fieldName : std::as_const( mReferencedFields ) )
      attrs << f.attribute( fieldName );

    setForeignKeys( attrs );

    mAddEntryButton->setEnabled( false );
  }

  unsetMapTool();
}

void QgsRelationReferenceWidget::updateAddEntryButton()
{
  mAddEntryButton->setVisible( mAllowAddFeatures && mMapToolDigitize );
  mAddEntryButton->setEnabled( mReferencedLayer && mReferencedLayer->isEditable() );
}

void QgsRelationReferenceWidget::disableChainedComboBoxes( const QComboBox *scb )
{
  QComboBox *ccb = nullptr;
  const auto constMFilterComboBoxes = mFilterComboBoxes;
  for ( QComboBox *cb : constMFilterComboBoxes )
  {
    if ( !ccb )
    {
      if ( cb == scb )
      {
        ccb = cb;
      }

      continue;
    }

    cb->setCurrentIndex( 0 );
    if ( ccb->currentIndex() == 0 )
    {
      cb->setEnabled( false );
    }

    ccb = cb;
  }
}

void QgsRelationReferenceWidget::emitForeignKeysChanged( const QVariantList &foreignKeys, bool force )
{
  if ( foreignKeys == mForeignKeys && force == false && qVariantListIsNull( foreignKeys ) == qVariantListIsNull( mForeignKeys ) )
    return;

  mForeignKeys = foreignKeys;
  Q_NOWARN_DEPRECATED_PUSH
  emit foreignKeyChanged( foreignKeys.at( 0 ) );
  Q_NOWARN_DEPRECATED_POP
  emit foreignKeysChanged( foreignKeys );
}

QString QgsRelationReferenceWidget::referencedLayerName() const
{
  return mReferencedLayerName;
}

void QgsRelationReferenceWidget::setReferencedLayerName( const QString &relationLayerName )
{
  mReferencedLayerName = relationLayerName;
}

QString QgsRelationReferenceWidget::referencedLayerId() const
{
  return mReferencedLayerId;
}

void QgsRelationReferenceWidget::setReferencedLayerId( const QString &relationLayerId )
{
  mReferencedLayerId = relationLayerId;
}

QString QgsRelationReferenceWidget::referencedLayerProviderKey() const
{
  return mReferencedLayerProviderKey;
}

void QgsRelationReferenceWidget::setReferencedLayerProviderKey( const QString &relationProviderKey )
{
  mReferencedLayerProviderKey = relationProviderKey;
}

QString QgsRelationReferenceWidget::referencedLayerDataSource() const
{
  return mReferencedLayerDataSource;
}

void QgsRelationReferenceWidget::setReferencedLayerDataSource( const QString &relationDataSource )
{
  const QgsPathResolver resolver { QgsProject::instance()->pathResolver() };
  mReferencedLayerDataSource = resolver.writePath( relationDataSource );
}

void QgsRelationReferenceWidget::setFormFeature( const QgsFeature &formFeature )
{
  mFormFeature = formFeature;
}
