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
#include "qgslogger.h"

#include <QDir>
#include <QSettings>

QgsDatumTransformDialog::QgsDatumTransformDialog( const QString& layerName, const QList< QList< int > > &dt, QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mDt( dt )
    , mLayerName( layerName )
{
  setupUi( this );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  updateTitle();

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DatumTransformDialog/geometry" ).toByteArray() );
  mLabelSrcDescription->setText( "" );
  mLabelDstDescription->setText( "" );

  for ( int i = 0; i < 2; i++ )
  {
    mDatumTransformTreeWidget->setColumnWidth( i, settings.value( QString( "/Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTreeWidget->columnWidth( i ) ).toInt() );
  }

  load();

  mHideDeprecatedCheckBox->setChecked( settings.value( "/Windows/DatumTransformDialog/hideDeprecated", false ).toBool() );
  mRememberSelectionCheckBox->setChecked( settings.value( "/Windows/DatumTransformDialog/rememberSelection", false ).toBool() );
}

void QgsDatumTransformDialog::load()
{
  QgsDebugMsg( "Entered." );

  mDatumTransformTreeWidget->clear();

  QList< QList< int > >::const_iterator it = mDt.constBegin();
  for ( ; it != mDt.constEnd(); ++it )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    int score = 0;

    for ( int i = 0; i < 2 && i < it->size(); ++i )
    {
      int nr = it->at( i );
      item->setData( i, Qt::UserRole, nr );
      if ( nr == -1 )
        continue;

      item->setText( i, QgsCoordinateTransform::datumTransformString( nr ) );

      //Describe datums in a tooltip
      QString srcGeoProj, destGeoProj, remarks, scope;
      int epsgNr;
      bool preferred, deprecated;
      if ( !QgsCoordinateTransform::datumTransformCrsInfo( nr, epsgNr, srcGeoProj, destGeoProj, remarks, scope, preferred, deprecated ) )
        continue;

      QString toolTipString;
      if ( gridShiftTransformation( item->text( i ) ) )
      {
        toolTipString.append( QString( "<p><b>NTv2</b></p>" ) );
        if ( !deprecated )
          score += 2;
        if ( !testGridShiftFileAvailability( item, i ) )
        {
          score -= 10;
          item->setDisabled( true );
          continue;
        }
      }

      if ( epsgNr > 0 )
        toolTipString.append( QString( "<p><b>EPSG Transformations Code:</b> %1</p>" ).arg( epsgNr ) );

      toolTipString.append( QString( "<p><b>Source CRS:</b> %1</p><p><b>Destination CRS:</b> %2</p>" ).arg( srcGeoProj ).arg( destGeoProj ) );

      if ( !remarks.isEmpty() )
        toolTipString.append( QString( "<p><b>Remarks:</b> %1</p>" ).arg( remarks ) );
      if ( !scope.isEmpty() )
        toolTipString.append( QString( "<p><b>Scope:</b> %1</p>" ).arg( scope ) );
      if ( preferred )
      {
        toolTipString.append( "<p><b>Preferred transformation</b></p>" );
        score += 1;
      }
      if ( deprecated )
      {
        toolTipString.append( "<p><b>Deprecated transformation</b></p>" );
        score -= 2;
      }

      item->setToolTip( i, toolTipString );
    }

    item->setData( 2, Qt::UserRole, score);
    item->setText( 2, QString("%1").arg( score + 50 ) );  // Sort works on text only
    mDatumTransformTreeWidget->addTopLevelItem( item );
  }

  // Sort transformations by score
  mDatumTransformTreeWidget->sortItems( 2, Qt::DescendingOrder );
  mDatumTransformTreeWidget->setSortingEnabled( true );
  mDatumTransformTreeWidget->setSortingEnabled( false );
  mDatumTransformTreeWidget->setColumnHidden( 2, true );
  mDatumTransformTreeWidget->setCurrentItem(mDatumTransformTreeWidget->topLevelItem(0));

  updateStatus();
}

void QgsDatumTransformDialog::updateStatus()
{
  bool hideDeprecated;
  bool itemHidden;

  hideDeprecated = mHideDeprecatedCheckBox->isChecked();

  QTreeWidgetItemIterator it(mDatumTransformTreeWidget);
  while (*it) {
    itemHidden = hideDeprecated && (((*it)->data( 2, Qt::UserRole )).toInt() < 0);
    (*it)->setHidden( itemHidden );
    ++it;
  }

  if ( mDatumTransformTreeWidget->currentItem()->isHidden() )
  {
    mDatumTransformTreeWidget->setCurrentItem(mDatumTransformTreeWidget->topLevelItem(0));
  }

}
QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/DatumTransformDialog/geometry", saveGeometry() );
  settings.setValue( "/Windows/DatumTransformDialog/hideDeprecated", mHideDeprecatedCheckBox->isChecked() );
  settings.setValue( "/Windows/DatumTransformDialog/rememberSelection", mRememberSelectionCheckBox->isChecked() );

  for ( int i = 0; i < 2; i++ )
  {
    settings.setValue( QString( "/Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTreeWidget->columnWidth( i ) );
  }

  QApplication::restoreOverrideCursor();
}

void QgsDatumTransformDialog::setDatumTransformInfo( const QString& srcCRSauthId, const QString& destCRSauthId, int srcTransform, int destTransform)
{
  mSrcCRSauthId = srcCRSauthId;
  mDestCRSauthId = destCRSauthId;
  if ( srcTransform != -1 || destTransform != -1 )
  {
    QTreeWidgetItemIterator it(mDatumTransformTreeWidget);
    while (*it) {
      if ( (*it)->data(0, Qt::UserRole).toInt() == srcTransform && (*it)->data(1, Qt::UserRole).toInt() == destTransform )
      {
        mDatumTransformTreeWidget->setCurrentItem( (*it) );
        break;
      }
      ++it;
    }
  }

  updateTitle();
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
  return !itemText.isEmpty() && !itemText.contains( "towgs84", Qt::CaseInsensitive );
}

bool QgsDatumTransformDialog::testGridShiftFileAvailability( QTreeWidgetItem* item, int col ) const
{
  if ( !item )
  {
    return false;
  }

  QString itemText = item->text( col );
  if ( itemText.isEmpty() )
  {
    return false;
  }

  char* projLib = getenv( "PROJ_LIB" );
  if ( !projLib ) //no information about installation directory
  {
    item->setToolTip( col, tr( "The PROJ_LIB enviroment variable not set." ) );
    return false;
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
#if defined(Q_OS_WIN)
      if ( fileIt->compare( filename, Qt::CaseInsensitive ) == 0 )
#else
      if ( fileIt->compare( filename ) == 0 )
#endif //Q_OS_WIN
      {
        return true;
      }
    }
    item->setToolTip( col, tr( "File '%1' not found in directory '%2'" ).arg( filename ).arg( projDir.absolutePath() ) );
    return false; //not found in PROJ_LIB directory
  }
  item->setToolTip( col, tr( "The directory '%1' set to PROJ_LIB variable doesn't exist" ).arg( projDir.absolutePath() ) );
  return false;
}

void QgsDatumTransformDialog::on_mHideDeprecatedCheckBox_stateChanged( int )
{
  updateStatus();
}

void QgsDatumTransformDialog::on_mDatumTransformTreeWidget_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  mLabelSrcDescription->setText( current->toolTip( 0 ) );
  mLabelDstDescription->setText( current->toolTip( 1 ) );
}

void QgsDatumTransformDialog::updateTitle()
{
  mLabelLayer->setText( mLayerName );
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( mSrcCRSauthId );
  mLabelSrcCrs->setText( QString( "%1 - %2" ).arg( mSrcCRSauthId ).arg( crs.isValid() ? crs.description() : tr( "unknown" ) ) );
  crs.createFromString( mDestCRSauthId );
  mLabelDstCrs->setText( QString( "%1 - %2" ).arg( mDestCRSauthId ).arg( crs.isValid() ? crs.description() : tr( "unknown" ) ) );
}
