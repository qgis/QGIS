/***************************************************************************
                         qgsgeomtypedialog.cpp  -  description
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

#include "qgsgeomtypedialog.h"
#include "qgsapplication.h"
#include "qgisapp.h" // <- for theme icons
#include <QPushButton>

QgsGeomTypeDialog::QgsGeomTypeDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  mAddAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  mRemoveAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mTypeBox->addItem( tr( "Real" ), "Real" );
  mTypeBox->addItem( tr( "Integer" ), "Integer" );;
  mTypeBox->addItem( tr( "String" ), "String" );

  mPointRadioButton->setChecked( true );
  mFileFormatComboBox->addItem( "ESRI Shapefile" );
  /*mFileFormatComboBox->addItem("Comma Separated Value");
  mFileFormatComboBox->addItem("GML");
  mFileFormatComboBox->addItem("Mapinfo File");*/
  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );
}

QgsGeomTypeDialog::~QgsGeomTypeDialog()
{
}

QGis::WkbType QgsGeomTypeDialog::selectedType() const
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

void QgsGeomTypeDialog::on_mAddAttributeButton_clicked()
{
  QString myName = mNameEdit->text();
  //use userrole to avoid translated type string
  QString myType = mTypeBox->itemData( mTypeBox->currentIndex(), Qt::UserRole ).toString();
  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType ) );
  if ( mAttributeView->topLevelItemCount() > 0 )
  {
    mOkButton->setEnabled( true );
  }
  mNameEdit->clear();
}

void QgsGeomTypeDialog::on_mRemoveAttributeButton_clicked()
{
  delete( mAttributeView->currentItem() );
  if ( mAttributeView->topLevelItemCount() == 0 )
  {
    mOkButton->setEnabled( false );
  }
}

void QgsGeomTypeDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}

void QgsGeomTypeDialog::attributes( std::list<std::pair<QString, QString> >& at ) const
{
  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QTreeWidgetItem *item = *it;
    at.push_back( std::make_pair( item->text( 0 ), item->text( 1 ) ) );
#ifdef QGISDEBUG
    qWarning( "appending %s//%s", item->text( 0 ).toLocal8Bit().constData(), item->text( 1 ).toLocal8Bit().constData() );
#endif
    ++it;
  }
}

QString QgsGeomTypeDialog::selectedFileFormat() const
{
  return mFileFormatComboBox->currentText();
}
