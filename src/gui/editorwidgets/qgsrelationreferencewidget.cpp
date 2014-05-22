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
#include <QToolButton>

#include "qgsattributedialog.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgseditorwidgetfactory.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsrelreferenceconfigdlg.h"
#include "qgsvectorlayer.h"


QgsRelationReferenceWidget::QgsRelationReferenceWidget( QWidget* parent )
    : QWidget( parent )
    , mReferencedLayer( NULL )
    , mInitialValueAssigned( false )
    , mAttributeDialog( NULL )
    , mEditorContext( QgsAttributeEditorContext() )
{
  mLayout = new QGridLayout( this );
  setLayout( mLayout );
  mComboBox = new QComboBox( this );
  mLayout->addWidget( mComboBox, 0, 0, 1, 1 );

  // action button
  QToolButton* attributeEditorButton = new QToolButton( this );
  mShowFormAction = new QAction( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ), tr( "Open Form" ), this );
  attributeEditorButton->addAction( mShowFormAction );
  attributeEditorButton->setDefaultAction( mShowFormAction );
  connect( attributeEditorButton, SIGNAL( triggered( QAction* ) ), this, SLOT( buttonTriggered( QAction* ) ) );
  mLayout->addWidget( attributeEditorButton, 0, 1, 1, 1 );

  // embed form
  mAttributeEditorFrame = new QgsCollapsibleGroupBox( this );
  mAttributeEditorFrame->setCollapsed( true );
  mAttributeEditorLayout = new QVBoxLayout( mAttributeEditorFrame );
  mAttributeEditorFrame->setLayout( mAttributeEditorLayout );
  mLayout->addWidget( mAttributeEditorFrame, 1, 0, 1, 3 );

  mLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ), 0, 2, 1, 1 );
}

void QgsRelationReferenceWidget::displayEmbedForm( bool display )
{
  mAttributeEditorFrame->setVisible( display );
}

void QgsRelationReferenceWidget::setRelation( QgsRelation relation, bool allowNullValue )
{
  if ( relation.isValid() )
  {
    if ( allowNullValue )
    {
      mComboBox->addItem( tr( "(no selection)" ), QVariant( field().type() ) );
    }

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
    mLayout->addWidget( lbl, 1, 0, 1, 3 );
  }
}

void QgsRelationReferenceWidget::setRelationEditable( bool editable )
{
  mComboBox->setEnabled( editable );
}

void QgsRelationReferenceWidget::setRelatedFeature( const QVariant& value )
{
  QgsFeatureId fid = mFidFkMap.key( value );
  int oldIdx = mComboBox->currentIndex();
  mComboBox->setCurrentIndex( mComboBox->findData( fid ) );

  if ( !mInitialValueAssigned )
  {
    // In case the default-selected item (first) is the actual item
    // then no referenceChanged event was triggered automatically:
    // Do it!
    if ( oldIdx == mComboBox->currentIndex() )
      referenceChanged( mComboBox->currentIndex() );
    mInitialValueAssigned = true;
  }
}

QVariant QgsRelationReferenceWidget::relatedFeature()
{
  QVariant varFid = mComboBox->itemData( mComboBox->currentIndex() );
  if ( varFid.isNull() )
  {
    return QVariant( field.type() );
  }
  else
  {
    return mFidFkMap.value( varFid.value<QgsFeatureId>() );
  }
}

void QgsRelationReferenceWidget::setEditorContext( QgsAttributeEditorContext context )
{
  mEditorContext = context;
}

void QgsRelationReferenceWidget::buttonTriggered( QAction* action )
{
  if ( action == mShowFormAction )
  {
    openForm();
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
  mAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, this, true, mEditorContext );
  mAttributeDialog->exec();
  delete mAttributeDialog;
}

void QgsRelationReferenceWidget::referenceChanged( int index )
{
  QgsFeatureId fid = mComboBox->itemData( index ).value<QgsFeatureId>();

  emit relatedFeatureChanged( mFidFkMap.value( fid ) );

  // Check if we're running with an embedded frame we need to update
  if ( mAttributeEditorFrame )
  {
    QgsFeature feat;

    mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

    if ( feat.isValid() )
    {
      // Backup old dialog and delete only after creating the new dialog, so we can "hot-swap" the contained QgsFeature
      QgsAttributeDialog* oldDialog = mAttributeDialog;

      if ( mAttributeDialog )
      {
        mAttributeEditorLayout->removeWidget( mAttributeDialog );
      }

      // TODO: Get a proper QgsDistanceArea thingie
      mAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, mAttributeEditorFrame, false, mEditorContext );
      QWidget* attrDialog = mAttributeDialog;
      attrDialog->setWindowFlags( Qt::Widget ); // Embed instead of opening as window
      mAttributeEditorLayout->addWidget( attrDialog );
      attrDialog->show();

      delete oldDialog;
    }
  }
}
