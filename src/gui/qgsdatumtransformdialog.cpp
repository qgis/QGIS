/***************************************************************************
                         qgsdatumtransformdialog.cpp
                         ---------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatumtransformdialog.h"
#include "qgscoordinatetransform.h"
#include <QDir>

QgsDatumTransformDialog::QgsDatumTransformDialog( const QString& layerName, const QList< QList< int > >& dt, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  setWindowTitle( tr( "Select datum transformations for layer" ) + " " + layerName );
  QList< QList< int > >::const_iterator it = dt.constBegin();
  for ( ; it != dt.constEnd(); ++it )
  {
    QTreeWidgetItem* item = new QTreeWidgetItem();
    bool itemDisabled = false;

    for ( int i = 0; i < 2; ++i )
    {
      if ( i >= it->size() )
      {
        break;
      }

      int nr = it->at( i );
      item->setData( i, Qt::UserRole, nr );
      if ( nr != -1 )
      {
        item->setText( i, QgsCoordinateTransform::datumTransformString( nr ) );
      }

      if ( gridShiftTransformation( item->text( i ) ) && !testGridShiftFileAvailability( item, i ) )
      {
        itemDisabled = true;
      }
    }
    item->setDisabled( itemDisabled );
    mDatumTransformTreeWidget->addTopLevelItem( item );
  }
}

QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
}

QgsDatumTransformDialog::QgsDatumTransformDialog(): QDialog()
{
  setupUi( this );
}

QList< int > QgsDatumTransformDialog::selectedDatumTransform()
{
  QList<int> list;
  QTreeWidgetItem * item = mDatumTransformTreeWidget->currentItem();
  if ( item )
  {
    for ( int i = 0; i < 2; ++i )
    {
      int transformNr = item->data( i, Qt::UserRole ).toInt();
      list << transformNr;
    }
  }
  return list;
}

bool QgsDatumTransformDialog::rememberSelection() const
{
  return mRememberSelectionCheckBox->isChecked();
}

bool QgsDatumTransformDialog::gridShiftTransformation( const QString& itemText ) const
{
  return ( !itemText.isEmpty() && !itemText.contains( "towgs84", Qt::CaseInsensitive ) );
}

bool QgsDatumTransformDialog::testGridShiftFileAvailability( QTreeWidgetItem* item, int col ) const
{
  if ( !item )
  {
    return true;
  }

  QString itemText = item->text( col );
  if ( itemText.isEmpty() )
  {
    return true;
  }

  char* projLib = getenv( "PROJ_LIB" );
  if ( !projLib ) //no information about installation directory
  {
    return true;
  }

  QStringList itemEqualSplit = itemText.split( "=" );
  QString filename;
  for ( int i = 1; i < itemEqualSplit.size(); ++i )
  {
    if ( i > 1 )
    {
      filename.append( "=" );
    }
    filename.append( itemEqualSplit.at( i ) );
  }

  QDir projDir( projLib );
  if ( projDir.exists() )
  {
    //look if filename in directory
    QStringList fileList = projDir.entryList();
    QStringList::const_iterator fileIt = fileList.constBegin();
    for ( ; fileIt != fileList.constEnd(); ++fileIt )
    {
      if ( *fileIt == filename )
      {
        return true;
      }
    }
    item->setToolTip( col, tr( "File '%1' not found in directory '%2'" ).arg( filename ).arg( projDir.absolutePath() ) );
    return false; //not found in PROJ_LIB directory
  }
  return true;
}
