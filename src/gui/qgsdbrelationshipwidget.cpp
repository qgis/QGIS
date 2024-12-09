/***************************************************************************
    qgsdbrelationshipwidget.cpp
    ------------------
    Date                 : November 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdbrelationshipwidget.h"
#include "moc_qgsdbrelationshipwidget.cpp"
#include "qgsgui.h"
#include "qgsdatabasetablemodel.h"
#include "qgsproviderregistry.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSortFilterProxyModel>

//
// QgsDbRelationWidget
//

QgsDbRelationWidget::QgsDbRelationWidget( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QWidget( parent )
  , mConnection( connection )
{
  setupUi( this );

  // takes ownership of connection
  mTableModel = new QgsDatabaseTableModel( connection, QString(), this );

  const QList<Qgis::RelationshipCardinality> cardinalities = mConnection->supportedRelationshipCardinalities();
  for ( Qgis::RelationshipCardinality cardinality :
        {
          Qgis::RelationshipCardinality::OneToMany,
          Qgis::RelationshipCardinality::ManyToOne,
          Qgis::RelationshipCardinality::OneToOne,
          Qgis::RelationshipCardinality::ManyToMany
        } )
  {
    if ( cardinalities.contains( cardinality ) )
      mCardinalityCombo->addItem( QgsRelation::cardinalityToDisplayString( cardinality ), QVariant::fromValue( cardinality ) );
  }

  const QList<Qgis::RelationshipStrength> strengths = mConnection->supportedRelationshipStrengths();
  for ( Qgis::RelationshipStrength strength :
        {
          Qgis::RelationshipStrength::Association,
          Qgis::RelationshipStrength::Composition
        } )
  {
    if ( strengths.contains( strength ) )
      mStrengthCombo->addItem( QgsRelation::strengthToDisplayString( strength ), QVariant::fromValue( strength ) );
  }

  const Qgis::RelationshipCapabilities capabilities = mConnection->supportedRelationshipCapabilities();
  if ( !( capabilities & Qgis::RelationshipCapability::ForwardPathLabel ) )
  {
    mForwardLabelLineEdit->hide();
    mForwardLabel->hide();
  }

  if ( !( capabilities & Qgis::RelationshipCapability::BackwardPathLabel ) )
  {
    mBackwardLabelLineEdit->hide();
    mReverseLabel->hide();
  }

  const QStringList relatedTableTypes = mConnection->relatedTableTypes();
  mRelatedTableTypeCombo->addItems( relatedTableTypes );

  mProxyModel = new QSortFilterProxyModel( mTableModel );
  mProxyModel->setSourceModel( mTableModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSortRole( Qt::DisplayRole );
  mProxyModel->sort( 0 );

  mLeftTableCombo->setModel( mProxyModel );
  mRightTableCombo->setModel( mProxyModel );

  connect( mNameEdit, &QLineEdit::textChanged, this, [=] {
    emit validityChanged( isValid() );
  } );
  connect( mLeftTableCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
    mLeftFieldsCombo->setFields( mConnection->fields( QString(), mLeftTableCombo->currentText() ) );
    emit validityChanged( isValid() );
  } );
  connect( mRightTableCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
    mRightFieldsCombo->setFields( mConnection->fields( QString(), mRightTableCombo->currentText() ) );
    emit validityChanged( isValid() );
  } );

  for ( QComboBox *combo :
        {
          mCardinalityCombo,
          qobject_cast<QComboBox *>( mLeftFieldsCombo ),
          qobject_cast<QComboBox *>( mRightFieldsCombo ),
          mStrengthCombo,
          mRelatedTableTypeCombo
        } )
  {
    connect( combo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int ) {
      emit validityChanged( isValid() );
    } );
  }

  mLeftFieldsCombo->setFields( mConnection->fields( QString(), mLeftTableCombo->currentText() ) );
  mRightFieldsCombo->setFields( mConnection->fields( QString(), mRightTableCombo->currentText() ) );
}

void QgsDbRelationWidget::setRelationship( const QgsWeakRelation &relationship )
{
  mRelation = relationship;
  mNameEdit->setText( mRelation.name() );
  mNameEdit->setEnabled( false );
  mStrengthCombo->setCurrentIndex( mStrengthCombo->findData( QVariant::fromValue( mRelation.strength() ) ) );

  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( mConnection->providerKey(), mRelation.referencedLayerSource() );
  mLeftTableCombo->setCurrentText( parts.value( QStringLiteral( "layerName" ) ).toString() );
  parts = QgsProviderRegistry::instance()->decodeUri( mConnection->providerKey(), mRelation.referencingLayerSource() );
  mRightTableCombo->setCurrentText( parts.value( QStringLiteral( "layerName" ) ).toString() );

  mCardinalityCombo->setCurrentIndex( mCardinalityCombo->findData( QVariant::fromValue( mRelation.cardinality() ) ) );
  mLeftFieldsCombo->setCurrentText( mRelation.referencedLayerFields().value( 0 ) );
  mRightFieldsCombo->setCurrentText( mRelation.referencingLayerFields().value( 0 ) );
  mForwardLabelLineEdit->setText( mRelation.forwardPathLabel() );
  mBackwardLabelLineEdit->setText( mRelation.backwardPathLabel() );
  mRelatedTableTypeCombo->setCurrentText( mRelation.relatedTableType() );
}

QgsWeakRelation QgsDbRelationWidget::relationship() const
{
  QgsWeakRelation result( mRelation.id().isEmpty() ? mNameEdit->text() : mRelation.id(), mRelation.name().isEmpty() ? mNameEdit->text() : mRelation.name(), mStrengthCombo->currentData().value<Qgis::RelationshipStrength>(), QString(), QString(), mConnection->tableUri( QString(), mRightTableCombo->currentText() ), mConnection->providerKey(), QString(), QString(), mConnection->tableUri( QString(), mLeftTableCombo->currentText() ), mConnection->providerKey() );
  result.setCardinality( mCardinalityCombo->currentData().value<Qgis::RelationshipCardinality>() );
  result.setReferencedLayerFields( { mLeftFieldsCombo->currentText() } );
  result.setReferencingLayerFields( { mRightFieldsCombo->currentText() } );
  result.setForwardPathLabel( mForwardLabelLineEdit->text() );
  result.setBackwardPathLabel( mBackwardLabelLineEdit->text() );

  result.setMappingTable( mRelation.mappingTable() );
  result.setMappingReferencedLayerFields( mRelation.mappingReferencedLayerFields() );
  result.setMappingReferencingLayerFields( mRelation.mappingReferencingLayerFields() );

  if ( mRelatedTableTypeCombo->currentIndex() >= 0 )
    result.setRelatedTableType( mRelatedTableTypeCombo->currentText() );
  return result;
}

bool QgsDbRelationWidget::isValid() const
{
  if ( mNameEdit->text().trimmed().isEmpty() )
    return false;

  if ( mLeftTableCombo->currentText().isEmpty() )
    return false;

  if ( mRightTableCombo->currentText().isEmpty() )
    return false;

  if ( mLeftTableCombo->currentText() == mRightTableCombo->currentText() )
    return false;

  if ( mLeftFieldsCombo->currentText().isEmpty() )
    return false;

  if ( mRightFieldsCombo->currentText().isEmpty() )
    return false;


  return true;
}

//
// QgsDbRelationDialog
//

QgsDbRelationDialog::QgsDbRelationDialog( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setObjectName( QStringLiteral( "QgsDbRelationDialog" ) );

  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsDbRelationWidget( connection );
  vLayout->addWidget( mWidget, 1 );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );

  setLayout( vLayout );
  connect( mWidget, &QgsDbRelationWidget::validityChanged, this, &QgsDbRelationDialog::validityChanged );
  validityChanged( mWidget->isValid() );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsDbRelationDialog::setRelationship( const QgsWeakRelation &relationship )
{
  mWidget->setRelationship( relationship );
}

QgsWeakRelation QgsDbRelationDialog::relationship() const
{
  return mWidget->relationship();
}

void QgsDbRelationDialog::accept()
{
  if ( !mWidget->isValid() )
    return;

  QDialog::accept();
}

void QgsDbRelationDialog::validityChanged( bool isValid )
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isValid );
}
