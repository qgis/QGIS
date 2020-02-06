/***************************************************************************
                         qgsaddattrdialog.h  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaddattrdialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgslogger.h"

#include <QMessageBox>

QgsAddAttrDialog::QgsAddAttrDialog( QgsVectorLayer *vlayer, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mIsShapeFile( vlayer && vlayer->providerType() == QLatin1String( "ogr" ) && vlayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
{
  setupUi( this );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAddAttrDialog::mTypeBox_currentIndexChanged );
  connect( mLength, &QSpinBox::editingFinished, this, &QgsAddAttrDialog::mLength_editingFinished );

  if ( !vlayer )
    return;

  //fill data types into the combo box
  const QList< QgsVectorDataProvider::NativeType > &typelist = vlayer->dataProvider()->nativeTypes();

  for ( int i = 0; i < typelist.size(); i++ )
  {
    QgsDebugMsg( QStringLiteral( "name:%1 type:%2 typeName:%3 length:%4-%5 prec:%6-%7" )
                 .arg( typelist[i].mTypeDesc )
                 .arg( typelist[i].mType )
                 .arg( typelist[i].mTypeName )
                 .arg( typelist[i].mMinLen ).arg( typelist[i].mMaxLen )
                 .arg( typelist[i].mMinPrec ).arg( typelist[i].mMaxPrec ) );

    mTypeBox->addItem( typelist[i].mTypeDesc );
    mTypeBox->setItemData( i, static_cast<int>( typelist[i].mType ), Qt::UserRole );
    mTypeBox->setItemData( i, typelist[i].mTypeName, Qt::UserRole + 1 );
    mTypeBox->setItemData( i, typelist[i].mMinLen, Qt::UserRole + 2 );
    mTypeBox->setItemData( i, typelist[i].mMaxLen, Qt::UserRole + 3 );
    mTypeBox->setItemData( i, typelist[i].mMinPrec, Qt::UserRole + 4 );
    mTypeBox->setItemData( i, typelist[i].mMaxPrec, Qt::UserRole + 5 );
  }

  mTypeBox_currentIndexChanged( 0 );

  if ( mIsShapeFile )
    mNameEdit->setMaxLength( 10 );

  mNameEdit->setFocus();
}

void QgsAddAttrDialog::mTypeBox_currentIndexChanged( int idx )
{
  mTypeName->setText( mTypeBox->itemData( idx, Qt::UserRole + 1 ).toString() );

  mLength->setMinimum( mTypeBox->itemData( idx, Qt::UserRole + 2 ).toInt() );
  mLength->setMaximum( mTypeBox->itemData( idx, Qt::UserRole + 3 ).toInt() );
  mLength->setVisible( mLength->minimum() < mLength->maximum() );
  mLengthLabel->setVisible( mLength->minimum() < mLength->maximum() );
  if ( mLength->value() < mLength->minimum() )
    mLength->setValue( mLength->minimum() );
  if ( mLength->value() > mLength->maximum() )
    mLength->setValue( mLength->maximum() );
  setPrecisionMinMax();
}

void QgsAddAttrDialog::mLength_editingFinished()
{
  setPrecisionMinMax();
}

void QgsAddAttrDialog::setPrecisionMinMax()
{
  int idx = mTypeBox->currentIndex();
  int minPrecType = mTypeBox->itemData( idx, Qt::UserRole + 4 ).toInt();
  int maxPrecType = mTypeBox->itemData( idx, Qt::UserRole + 5 ).toInt();
  mPrec->setVisible( minPrecType < maxPrecType );
  mPrecLabel->setVisible( minPrecType < maxPrecType );
  mPrec->setMinimum( minPrecType );
  mPrec->setMaximum( std::max( minPrecType, std::min( maxPrecType, mLength->value() ) ) );
}

void QgsAddAttrDialog::accept()
{
  if ( mIsShapeFile && mNameEdit->text().compare( QLatin1String( "shape" ), Qt::CaseInsensitive ) == 0 )
  {
    QMessageBox::warning( this, tr( "Add Field" ),
                          tr( "Invalid field name. This field name is reserved and cannot be used." ) );
    return;
  }
  if ( mNameEdit->text().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Add Field" ),
                          tr( "No name specified. Please specify a name to create a new field." ) );
    return;
  }

  QDialog::accept();
}

QgsField QgsAddAttrDialog::field() const
{

  QgsDebugMsg( QStringLiteral( "idx:%1 name:%2 type:%3 typeName:%4 length:%5 prec:%6 comment:%7" )
               .arg( mTypeBox->currentIndex() )
               .arg( mNameEdit->text() )
               .arg( mTypeBox->currentData( Qt::UserRole ).toInt() )
               .arg( mTypeBox->currentData( Qt::UserRole + 1 ).toString() )
               .arg( mLength->value() )
               .arg( mPrec->value() )
               .arg( mCommentEdit->text() ) );

  return QgsField(
           mNameEdit->text(),
           ( QVariant::Type ) mTypeBox->currentData( Qt::UserRole ).toInt(),
           mTypeBox->currentData( Qt::UserRole + 1 ).toString(),
           mLength->value(),
           mPrec->value(),
           mCommentEdit->text(),
           static_cast<QVariant::Type>( mTypeBox->currentData( Qt::UserRole ).toInt() ) == QVariant::Map ? QVariant::String : QVariant::Invalid
         );
}
