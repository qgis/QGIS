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

#include "qgsattributedialog.h"
#include "qgscollapsiblegroupbox.h"
#include "qgseditorwidgetfactory.h"
#include "qgsexpression.h"
#include "qgsfield.h"
#include "qgsproject.h"
#include "qgsrelreferenceconfigdlg.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

QgsRelationReferenceWidget::QgsRelationReferenceWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QgsAttributeEditorContext context, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mInitialValueAssigned( false )
    , mComboBox( NULL )
    , mAttributeEditorFrame( NULL )
    , mAttributeEditorLayout( NULL )
    , mAttributeEditorButton( NULL )
    , mReferencedLayer( NULL )
    , mAttributeDialog( NULL )
    , mEditorContext( context )
{
}

QWidget* QgsRelationReferenceWidget::createWidget( QWidget* parent )
{
  return new QWidget( parent );
}

void QgsRelationReferenceWidget::initWidget( QWidget* editor )
{
  QGridLayout* layout = new QGridLayout( editor );
  editor->setLayout( layout );

  mComboBox = new QComboBox( editor );
  layout->addWidget( mComboBox, 0, 0, 1, 1 );

  if ( config( "ShowForm", true ).toBool() )
  {
    mAttributeEditorFrame = new QgsCollapsibleGroupBox( editor );
    mAttributeEditorLayout = new QVBoxLayout( mAttributeEditorFrame );
    mAttributeEditorFrame->setLayout( mAttributeEditorLayout );

    layout->addWidget( mAttributeEditorFrame, 1, 0, 1, 3 );
  }
  else
  {
    mAttributeEditorButton = new QPushButton( tr( "Open Form" ) );

    layout->addWidget( mAttributeEditorButton, 0, 1, 1, 1 );

    connect( mAttributeEditorButton, SIGNAL( clicked() ), this, SLOT( openForm() ) );
  }

  layout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Expanding ), 0, 2, 1, 1 );

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config( "Relation" ).toString() );

  if ( relation.isValid() )
  {
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

    if ( config( "AllowNULL" ).toBool() )
    {
      mComboBox->addItem( "[NULL]" );
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
    layout->addWidget( lbl, 1, 0, 1, 3 );
  }
}

QVariant QgsRelationReferenceWidget::value()
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

void QgsRelationReferenceWidget::setValue( const QVariant& value )
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

void QgsRelationReferenceWidget::setEnabled( bool enabled )
{
  mComboBox->setEnabled( enabled );
}

void QgsRelationReferenceWidget::referenceChanged( int index )
{
  QgsFeatureId fid = mComboBox->itemData( index ).value<QgsFeatureId>();

  emit valueChanged( mFidFkMap.value( fid ) );

  // Check if we're running with an embedded frame we need to update
  if ( mAttributeEditorFrame )
  {
    QgsFeature feat;

    mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

    if ( feat.isValid() )
    {
      // Backup old dialog and delete only after creating the new dialog, so we can "hot-swap" the contained QgsFeature
      QgsAttributeDialog* oldDialog = mAttributeDialog;

      if ( mAttributeDialog && mAttributeDialog->dialog() )
      {
        mAttributeEditorLayout->removeWidget( mAttributeDialog->dialog() );
      }

      // TODO: Get a proper QgsDistanceArea thingie
      mAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, mAttributeEditorFrame, false, mEditorContext );
      QWidget* attrDialog = mAttributeDialog->dialog();
      attrDialog->setWindowFlags( Qt::Widget ); // Embed instead of opening as window
      mAttributeEditorLayout->addWidget( attrDialog );
      attrDialog->show();

      delete oldDialog;
    }
  }
}

void QgsRelationReferenceWidget::openForm()
{
  QgsFeatureId fid = mComboBox->itemData( mComboBox->currentIndex() ).value<QgsFeatureId>();

  QgsFeature feat;

  mReferencedLayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( feat );

  if ( !feat.isValid() )
    return;

  // TODO: Get a proper QgsDistanceArea thingie
  mAttributeDialog = new QgsAttributeDialog( mReferencedLayer, new QgsFeature( feat ), true, widget(), true, mEditorContext );
  mAttributeDialog->exec();
  delete mAttributeDialog;
}
