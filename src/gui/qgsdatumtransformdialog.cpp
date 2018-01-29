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
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsproject.h"

#include <QDir>
#include <QPushButton>

QgsDatumTransformDialog::QgsDatumTransformDialog( const QgsCoordinateReferenceSystem &sourceCrs,
    const QgsCoordinateReferenceSystem &destinationCrs,
    QPair<int, int> selectedDatumTransforms,
    QWidget *parent,
    Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );

  mDatumTransformTableWidget->setColumnCount( 2 );
  QStringList headers;
  headers << tr( "Source transform" ) << tr( "Destination transform" ) ;
  mDatumTransformTableWidget->setHorizontalHeaderLabels( headers );

  mSourceProjectionSelectionWidget->setCrs( sourceCrs );
  mDestinationProjectionSelectionWidget->setCrs( destinationCrs );

  connect( mHideDeprecatedCheckBox, &QCheckBox::stateChanged, this, &QgsDatumTransformDialog::mHideDeprecatedCheckBox_stateChanged );
  connect( mDatumTransformTableWidget, &QTableWidget::currentItemChanged, this, &QgsDatumTransformDialog::tableCurrentItemChanged );

  connect( mSourceProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setSourceCrs );
  connect( mDestinationProjectionSelectionWidget, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDatumTransformDialog::setDestinationCrs );

  //get list of datum transforms
  mSourceCrs = sourceCrs;
  mDestinationCrs = destinationCrs;
  mDatumTransforms = QgsDatumTransform::datumTransformations( sourceCrs, destinationCrs );

  QApplication::setOverrideCursor( Qt::ArrowCursor );

  setOKButtonEnabled();

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/DatumTransformDialog/geometry" ) ).toByteArray() );
  mHideDeprecatedCheckBox->setChecked( settings.value( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), false ).toBool() );

  mLabelSrcDescription->clear();
  mLabelDstDescription->clear();

  load( selectedDatumTransforms );
}

void QgsDatumTransformDialog::load( const QPair<int, int> &selectedDatumTransforms )
{
  mDatumTransformTableWidget->setRowCount( 0 );

  int row = 0;

  for ( const QgsDatumTransform::TransformPair &transform : qgis::as_const( mDatumTransforms ) )
  {
    bool itemDisabled = false;
    bool itemHidden = false;

    if ( transform.sourceTransformId == -1 && transform.destinationTransformId == -1 )
      continue;

    for ( int i = 0; i < 2; ++i )
    {
      QTableWidgetItem *item = new QTableWidgetItem();
      int nr = i == 0 ? transform.sourceTransformId : transform.destinationTransformId;
      item->setData( Qt::UserRole, nr );

      item->setText( QgsDatumTransform::datumTransformToProj( nr ) );

      //Describe datums in a tooltip
      QgsDatumTransform::TransformInfo info = QgsDatumTransform::datumTransformInfo( nr );
      if ( info.datumTransformId == -1 )
        continue;

      if ( mHideDeprecatedCheckBox->isChecked() && info.deprecated )
      {
        itemHidden = true;
      }

      QString toolTipString;
      if ( gridShiftTransformation( item->text() ) )
      {
        toolTipString.append( QStringLiteral( "<p><b>NTv2</b></p>" ) );
      }

      if ( info.epsgCode > 0 )
        toolTipString.append( QStringLiteral( "<p><b>EPSG Transformations Code:</b> %1</p>" ).arg( info.epsgCode ) );

      toolTipString.append( QStringLiteral( "<p><b>Source CRS:</b> %1</p><p><b>Destination CRS:</b> %2</p>" ).arg( info.sourceCrsDescription, info.destinationCrsDescription ) );

      if ( !info.remarks.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Remarks:</b> %1</p>" ).arg( info.remarks ) );
      if ( !info.scope.isEmpty() )
        toolTipString.append( QStringLiteral( "<p><b>Scope:</b> %1</p>" ).arg( info.scope ) );
      if ( info.preferred )
        toolTipString.append( "<p><b>Preferred transformation</b></p>" );
      if ( info.deprecated )
        toolTipString.append( "<p><b>Deprecated transformation</b></p>" );

      item->setToolTip( toolTipString );

      if ( gridShiftTransformation( item->text() ) && !testGridShiftFileAvailability( item ) )
      {
        itemDisabled = true;
      }

      if ( !itemHidden )
      {
        if ( itemDisabled )
        {
          item->setFlags( Qt::NoItemFlags );
        }
        mDatumTransformTableWidget->setRowCount( row + 1 );
        mDatumTransformTableWidget->setItem( row, i, item );
      }
      else
      {
        delete item;
      }
    }

    if ( transform.sourceTransformId == selectedDatumTransforms.first &&
         transform.destinationTransformId == selectedDatumTransforms.second )
    {
      mDatumTransformTableWidget->selectRow( row );
    }

    row++;
  }

  mDatumTransformTableWidget->resizeColumnsToContents();

  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setOKButtonEnabled()
{
  int row = mDatumTransformTableWidget->currentRow();
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mSourceCrs.isValid() && mDestinationCrs.isValid() && row >= 0 );
}

QgsDatumTransformDialog::~QgsDatumTransformDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/hideDeprecated" ), mHideDeprecatedCheckBox->isChecked() );

  for ( int i = 0; i < 2; i++ )
  {
    settings.setValue( QStringLiteral( "Windows/DatumTransformDialog/columnWidths/%1" ).arg( i ), mDatumTransformTableWidget->columnWidth( i ) );
  }

  QApplication::restoreOverrideCursor();
}

int QgsDatumTransformDialog::availableTransformationCount()
{
  return mDatumTransforms.count();
}


QPair<QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int> > QgsDatumTransformDialog::selectedDatumTransforms()
{
  int row = mDatumTransformTableWidget->currentRow();
  QPair< QPair<QgsCoordinateReferenceSystem, int>, QPair<QgsCoordinateReferenceSystem, int > > sdt;
  sdt.first.first = mSourceCrs;
  sdt.second.first = mDestinationCrs;

  if ( row >= 0 )
  {
    QTableWidgetItem *srcItem = mDatumTransformTableWidget->item( row, 0 );
    sdt.first.second = srcItem ? srcItem->data( Qt::UserRole ).toInt() : -1;
    QTableWidgetItem *destItem = mDatumTransformTableWidget->item( row, 1 );
    sdt.second.second = destItem ? destItem->data( Qt::UserRole ).toInt() : -1;
  }
  else
  {
    sdt.first.second = -1;
    sdt.second.second = -1;
  }
  return sdt;
}

bool QgsDatumTransformDialog::gridShiftTransformation( const QString &itemText ) const
{
  return !itemText.isEmpty() && !itemText.contains( QLatin1String( "towgs84" ), Qt::CaseInsensitive );
}

bool QgsDatumTransformDialog::testGridShiftFileAvailability( QTableWidgetItem *item ) const
{
  if ( !item )
  {
    return true;
  }

  QString itemText = item->text();
  if ( itemText.isEmpty() )
  {
    return true;
  }

  char *projLib = getenv( "PROJ_LIB" );
  if ( !projLib ) //no information about installation directory
  {
    return true;
  }

  QStringList itemEqualSplit = itemText.split( '=' );
  QString filename;
  for ( int i = 1; i < itemEqualSplit.size(); ++i )
  {
    if ( i > 1 )
    {
      filename.append( '=' );
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
    item->setToolTip( tr( "File '%1' not found in directory '%2'" ).arg( filename, projDir.absolutePath() ) );
    return false; //not found in PROJ_LIB directory
  }
  return true;
}

void QgsDatumTransformDialog::mHideDeprecatedCheckBox_stateChanged( int )
{
  load();
}

void QgsDatumTransformDialog::tableCurrentItemChanged( QTableWidgetItem *, QTableWidgetItem * )
{
  int row = mDatumTransformTableWidget->currentRow();
  if ( row < 0 )
    return;

  QTableWidgetItem *srcItem = mDatumTransformTableWidget->item( row, 0 );
  mLabelSrcDescription->setText( srcItem ? srcItem->toolTip() : QString() );
  QTableWidgetItem *destItem = mDatumTransformTableWidget->item( row, 1 );
  mLabelDstDescription->setText( destItem ? destItem->toolTip() : QString() );

  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  mSourceCrs = sourceCrs;
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  load();
  setOKButtonEnabled();
}

void QgsDatumTransformDialog::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  mDestinationCrs = destinationCrs;
  mDatumTransforms = QgsDatumTransform::datumTransformations( mSourceCrs, mDestinationCrs );
  load();
  setOKButtonEnabled();
}
