/***************************************************************************
  qgsrelationadddlg.cpp - QgsRelationAddDlg
  ---------------------------------

 begin                : 4.10.2013
 copyright            : (C) 2013 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "qgsrelationadddlg.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayercombobox.h"
#include "qgsfieldcombobox.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsapplication.h"



QgsFieldPairWidget::QgsFieldPairWidget( int index, QWidget *parent )
  : QWidget( parent )
  , mIndex( index )
  , mEnabled( index == 0 )
{
  mLayout = new QHBoxLayout();
  mLayout->setMargin( 0 );

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );
  mAddButton->setMinimumWidth( 30 );
  mLayout->addWidget( mAddButton, 0, Qt::AlignLeft );

  mRemoveButton = new QToolButton( this );
  mRemoveButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyRemove.svg" ) ) );
  mRemoveButton->setMinimumWidth( 30 );
  mLayout->addWidget( mRemoveButton, 0, Qt::AlignLeft );

  mSpacerItem = new QSpacerItem( 30, 30, QSizePolicy::Minimum, QSizePolicy::Maximum );
  mLayout->addSpacerItem( mSpacerItem );

  mReferencedFieldCombobox = new QgsFieldComboBox( this );
  connect( mReferencedFieldCombobox, &QgsFieldComboBox::fieldChanged, this, &QgsFieldPairWidget::configChanged );
  mLayout->addWidget( mReferencedFieldCombobox, 1 );

  mReferencingFieldCombobox = new QgsFieldComboBox( this );
  connect( mReferencingFieldCombobox, &QgsFieldComboBox::fieldChanged, this, &QgsFieldPairWidget::configChanged );
  mLayout->addWidget( mReferencingFieldCombobox, 1 );

  setLayout( mLayout );
  updateWidgetVisibility();

  connect( mAddButton, &QToolButton::clicked, this, &QgsFieldPairWidget::changeEnable );
  connect( mRemoveButton, &QToolButton::clicked, this, &QgsFieldPairWidget::changeEnable );
  connect( mAddButton, &QToolButton::clicked, this, &QgsFieldPairWidget::pairEnabled );
  connect( mRemoveButton, &QToolButton::clicked, this, &QgsFieldPairWidget::pairDisabled );
}

void QgsFieldPairWidget::setReferencingLayer( QgsMapLayer *layer )
{
  mReferencingFieldCombobox->setLayer( layer );
}

void QgsFieldPairWidget::setReferencedLayer( QgsMapLayer *layer )
{
  mReferencedFieldCombobox->setLayer( layer );
}

QString QgsFieldPairWidget::referencingField() const
{
  return mReferencingFieldCombobox->currentField();
}

QString QgsFieldPairWidget::referencedField() const
{
  return mReferencedFieldCombobox->currentField();
}

bool QgsFieldPairWidget::isPairEnabled() const
{
  return mEnabled;
}

void QgsFieldPairWidget::updateWidgetVisibility()
{
  mAddButton->setVisible( !mEnabled );
  mRemoveButton->setVisible( mIndex > 0 && mEnabled );
  mReferencingFieldCombobox->setVisible( mEnabled );
  mReferencedFieldCombobox->setVisible( mEnabled );
  int spacerSize = 0;
  if ( !mRemoveButton->isVisible() && !mAddButton->isVisible() )
    spacerSize = mRemoveButton->minimumWidth() + mLayout->spacing();
  mSpacerItem->changeSize( spacerSize, spacerSize );
}

void QgsFieldPairWidget::changeEnable()
{
  mEnabled = !mEnabled || ( mIndex == 0 );
  updateWidgetVisibility();
  emit configChanged();
}

////////////////////
// QgsRelationAddDlg

QgsRelationAddDlg::QgsRelationAddDlg( QWidget *parent )
  : QDialog( parent )
{
  QGridLayout *layout = new QGridLayout(); // column 0 is kept free for alignmenent purpose

  // row 0: name
  // col 1
  QLabel *nameLabel = new QLabel( tr( "Name" ), this );
  layout->addWidget( nameLabel, 0, 1 );
  // col 2
  mNameLineEdit = new QLineEdit( this );
  layout->addWidget( mNameLineEdit, 0, 2 );

  // row 1: labels
  // col 1
  QLabel *referencedLabel = new QLabel( tr( "Referenced layer (parent)" ), this );
  layout->addWidget( referencedLabel, 1, 1 );
  // col 2
  QLabel *referencingLabel = new QLabel( tr( "Referencing layer (child)" ), this );
  layout->addWidget( referencingLabel, 1, 2 );

  // row 2: layer comboboxes
  // col 1
  mReferencedLayerCombobox = new QgsMapLayerComboBox( this );
  mReferencedLayerCombobox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  layout->addWidget( mReferencedLayerCombobox, 2, 1 );
  // col 2
  mReferencingLayerCombobox = new QgsMapLayerComboBox( this );
  mReferencingLayerCombobox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  layout->addWidget( mReferencingLayerCombobox, 2, 2 );

  // row 3: field pairs layout
  mFieldPairsLayout = new QVBoxLayout();
  layout->addLayout( mFieldPairsLayout, 3, 0, 1, 3 );

  // row 4: id
  // row 1
  QLabel *idLabel = new QLabel( tr( "Id" ), this );
  layout->addWidget( idLabel, 4, 1 );
  // col 2
  mIdLineEdit = new QLineEdit( this );
  mIdLineEdit->setPlaceholderText( tr( "[Generated automatically]" ) );
  layout->addWidget( mIdLineEdit, 4, 2 );

  // row 5: strength
  QLabel *strengthLabel = new QLabel( tr( "Relationship strength" ), this );
  layout->addWidget( strengthLabel, 5, 1 );
  // col 2
  mStrengthCombobox = new QComboBox( this );
  mStrengthCombobox->addItem( "Association", QVariant::fromValue( QgsRelation::RelationStrength::Association ) );
  mStrengthCombobox->addItem( "Composition", QVariant::fromValue( QgsRelation::RelationStrength::Composition ) );
  mStrengthCombobox->setToolTip( QStringLiteral( "On composition, the child features will be duplicated too.\n"
                                 "Duplications are made by the feature duplication action.\n"
                                 "The default actions are activated in the Action section of the layer properties." ) );
  layout->addWidget( mStrengthCombobox, 5, 2 );

  // row 6: button box
  mButtonBox = new QDialogButtonBox( this );
  mButtonBox->setOrientation( Qt::Horizontal );
  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRelationAddDlg::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsRelationAddDlg::reject );
  layout->addWidget( mButtonBox, 7, 1, 1, 2 );

  // set layout
  layout->setColumnMinimumWidth( 0, 30 );
  layout->setColumnStretch( 1, 1 );
  layout->setColumnStretch( 2, 1 );
  layout->setRowStretch( 6, 2 );
  setLayout( layout );

  addFieldPairWidget();
  addFieldPairWidget();

  checkDefinitionValid();

  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
}

void QgsRelationAddDlg::addFieldPairWidget()
{
  int index = mFieldPairWidgets.count();
  QgsFieldPairWidget *w = new QgsFieldPairWidget( index, this );
  w->setReferencingLayer( mReferencingLayerCombobox->currentLayer() );
  w->setReferencedLayer( mReferencedLayerCombobox->currentLayer() );
  mFieldPairsLayout->addWidget( w );
  mFieldPairWidgets << w;

  connect( w, &QgsFieldPairWidget::configChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( w, &QgsFieldPairWidget::pairDisabled, this, [ = ]() {emit fieldPairRemoved( w );} );
  connect( w, &QgsFieldPairWidget::pairEnabled, this, &QgsRelationAddDlg::addFieldPairWidget );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, w, &QgsFieldPairWidget::setReferencingLayer );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, w, &QgsFieldPairWidget::setReferencedLayer );
}

void QgsRelationAddDlg::fieldPairRemoved( QgsFieldPairWidget *fieldPairWidget )
{
  // remove widget only if not the first one, last one should always be disabled
  int index = mFieldPairWidgets.indexOf( fieldPairWidget );
  if ( index > 0 && index < mFieldPairWidgets.count() )
  {
    mFieldPairWidgets.removeAt( index );
    fieldPairWidget->deleteLater();
  }
}

QString QgsRelationAddDlg::referencingLayerId()
{
  return mReferencingLayerCombobox->currentLayer()->id();
}

QString QgsRelationAddDlg::referencedLayerId()
{
  return mReferencedLayerCombobox->currentLayer()->id();
}

QList< QPair< QString, QString > > QgsRelationAddDlg::references()
{
  QList< QPair< QString, QString > > references;
  for ( int i = 0; i < mFieldPairWidgets.count(); i++ )
  {
    QString referencingField = mFieldPairWidgets.at( i )->referencingField();
    QString referencedField = mFieldPairWidgets.at( i )->referencedField();
    references << qMakePair( referencingField, referencedField );
  }
  return references;
}

QString QgsRelationAddDlg::relationId()
{
  return mIdLineEdit->text();
}

QString QgsRelationAddDlg::relationName()
{
  return mNameLineEdit->text();
}

QgsRelation::RelationStrength QgsRelationAddDlg::relationStrength()
{
  return mStrengthCombobox->currentData().value<QgsRelation::RelationStrength>();
}

void QgsRelationAddDlg::checkDefinitionValid()
{
  bool valid = mReferencingLayerCombobox->currentLayer() && mReferencedLayerCombobox->currentLayer();
  for ( const QgsFieldPairWidget *fieldPairWidget : qgis::as_const( mFieldPairWidgets ) )
  {
    if ( !fieldPairWidget->isPairEnabled() )
      continue;
    valid &= !fieldPairWidget->referencingField().isNull() && !fieldPairWidget->referencedField().isNull();
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( valid );
}
