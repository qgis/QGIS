/***************************************************************************
                         qgsnewvectorlayerdialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
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
/* $Id$ */

#include "qgsnewvectorlayerdialog.h"
#include "qgsapplication.h"
#include "qgisapp.h" // <- for theme icons
#include "qgslogger.h"
#include <QPushButton>

QgsNewVectorLayerDialog::QgsNewVectorLayerDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  mAddAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  mRemoveAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mTypeBox->addItem( tr( "Text data" ), "String" );
  mTypeBox->addItem( tr( "Whole number" ), "Integer" );
  mTypeBox->addItem( tr( "Decimal number" ), "Real" );

  mWidth->setValidator( new QIntValidator( 1, 255, this ) );
  mPrecision->setValidator( new QIntValidator( 0, 20, this ) );

  mPointRadioButton->setChecked( true );
  mFileFormatComboBox->addItem( tr( "ESRI Shapefile" ), "ESRI Shapefile" );
  /* Disabled until provider properly supports editing the created file formats */
  //mFileFormatComboBox->addItem( tr( "Comma Separated Value" ), "Comma Separated Value" );
  //mFileFormatComboBox->addItem(tr( "GML"), "GML" );
  //mFileFormatComboBox->addItem(tr( "Mapinfo File" ), "Mapinfo File" );
  if ( mFileFormatComboBox->count() == 1 )
  {
    mFileFormatComboBox->setVisible( false );
    mFileFormatLabel->setVisible( false );
  }
  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );
}

QgsNewVectorLayerDialog::~QgsNewVectorLayerDialog()
{
}

void QgsNewVectorLayerDialog::on_mTypeBox_currentIndexChanged( int index )
{
  mPrecision->setEnabled( index == 2 );  // Real
}

QGis::WkbType QgsNewVectorLayerDialog::selectedType() const
{
  if ( mPointRadioButton->isChecked() )
  {
    return QGis::WKBPoint;
  }
  else if ( mLineRadioButton->isChecked() )
  {
    return QGis::WKBLineString;
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    return QGis::WKBPolygon;
  }
  return QGis::WKBUnknown;
}

void QgsNewVectorLayerDialog::on_mAddAttributeButton_clicked()
{
  QString myName = mNameEdit->text();
  QString myWidth = mWidth->text();
  QString myPrecision = mPrecision->text();
  //use userrole to avoid translated type string
  QString myType = mTypeBox->itemData( mTypeBox->currentIndex(), Qt::UserRole ).toString();
  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << myWidth << myPrecision ) );
  if ( mAttributeView->topLevelItemCount() > 0 )
  {
    mOkButton->setEnabled( true );
  }
  mNameEdit->clear();
}

void QgsNewVectorLayerDialog::on_mRemoveAttributeButton_clicked()
{
  delete( mAttributeView->currentItem() );
  if ( mAttributeView->topLevelItemCount() == 0 )
  {
    mOkButton->setEnabled( false );
  }
}

void QgsNewVectorLayerDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}

void QgsNewVectorLayerDialog::attributes( std::list<std::pair<QString, QString> >& at ) const
{
  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QTreeWidgetItem *item = *it;
    QString type = QString( "%1;%2;%3" ).arg( item->text( 1 ) ).arg( item->text( 2 ) ).arg( item->text( 3 ) );
    at.push_back( std::make_pair( item->text( 0 ), type ) );
    QgsDebugMsg( QString( "appending %1//%2" ).arg( item->text( 0 ) ).arg( type ) );
    ++it;
  }
}

QString QgsNewVectorLayerDialog::selectedFileFormat() const
{
  //use userrole to avoid translated type string
  QString myType = mFileFormatComboBox->itemData( mFileFormatComboBox->currentIndex(), Qt::UserRole ).toString();
  return myType;
}
