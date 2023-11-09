/***************************************************************************
  qgsloadrasterattributetabledialog.cpp - QgsLoadRasterAttributeTableDialog

 ---------------------
 begin                : 21.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsloadrasterattributetabledialog.h"
#include "qgsrasterattributetable.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include <QMessageBox>
#include <QPushButton>

QgsLoadRasterAttributeTableDialog::QgsLoadRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, QWidget *parent )
  : QDialog( parent )
  , mRasterLayer( rasterLayer )
{

  setupUi( this );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  connect( mDbfPathWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & )
  {
    updateButtons();
  } );

  mRasterBand->setLayer( mRasterLayer );

  mDbfPathWidget->setFilter( QStringLiteral( "VAT DBF Files (*.vat.dbf)" ) );

  updateButtons();

  QgsGui::enableAutoGeometryRestore( this );
}

QString QgsLoadRasterAttributeTableDialog::filePath() const
{
  return mDbfPathWidget->filePath();
}

int QgsLoadRasterAttributeTableDialog::rasterBand()
{
  return  mRasterBand->currentBand();
}

void QgsLoadRasterAttributeTableDialog::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

bool QgsLoadRasterAttributeTableDialog::openWhenDone() const
{
  return mOpenRat->isChecked();
}

void QgsLoadRasterAttributeTableDialog::setOpenWhenDoneVisible( bool visible )
{
  if ( ! visible )
  {
    mOpenRat->setChecked( false );
  }
  mOpenRat->setVisible( visible );
}

void QgsLoadRasterAttributeTableDialog::accept()
{
  bool success { false };

  if ( rasterBand() < 1 )
  {
    notify( tr( "Invalid Raster Band" ),
            tr( "The selected raster band %1 is not valid." ).arg( rasterBand() ),
            Qgis::MessageLevel::Critical );
  }
  else
  {
    std::unique_ptr<QgsRasterAttributeTable> rat = std::make_unique<QgsRasterAttributeTable>( );

    QString errorMessage;
    success = rat->readFromFile( filePath(), &errorMessage );

    if ( ! success )
    {
      notify( tr( "Error Loading Raster Attribute Table " ),
              tr( "The raster attribute table could not be loaded.\n%1" ).arg( errorMessage ),
              Qgis::MessageLevel::Critical );
    }
    else
    {
      if ( ! rat->isValid( &errorMessage ) )
      {
        switch ( QMessageBox::warning( nullptr, tr( "Invalid Raster Attribute Table" ), tr( "The raster attribute table is not valid:\n%1\nLoad anyway?" ), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel ) )
        {
          case QMessageBox::Cancel:
            return;
          case QMessageBox::Yes:
            success = true;
            break;
          case QMessageBox::No:
          default:
            success = false;
            break;
        }
      }

      if ( mRasterLayer->attributeTable( rasterBand() ) && ! mRasterLayer->attributeTable( rasterBand() )->filePath().isEmpty() )
      {
        switch ( QMessageBox::warning( nullptr, tr( "Confirm Attribute Table Replacement" ), tr( "Raster band %1 already has an associated attribute table loaded from '%2'. Are you sure you want to replace the existing raster attribute table?" ).arg( QString::number( rasterBand() ),  mRasterLayer->attributeTable( rasterBand() )->filePath() ), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel ) )
        {
          case QMessageBox::Cancel:
            return;
          case QMessageBox::Yes:
            success = true;
            break;
          case QMessageBox::No:
          default:
            success = false;
            break;
        }
      }

      if ( success )
      {
        mRasterLayer->dataProvider()->setAttributeTable( rasterBand(), rat.release() );
        notify( tr( "Raster Attribute Table Loaded" ),
                tr( "The new raster attribute table was successfully loaded." ),
                Qgis::MessageLevel::Success );
      }
    }
  }

  QDialog::accept();
}

void QgsLoadRasterAttributeTableDialog::notify( const QString &title, const QString &message, Qgis::MessageLevel level )
{
  if ( mMessageBar )
  {
    mMessageBar->pushMessage( message, level );
  }
  else
  {
    switch ( level )
    {
      case Qgis::MessageLevel::Info:
      case Qgis::MessageLevel::Success:
      case Qgis::MessageLevel::NoLevel:
      {
        QMessageBox::information( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Warning:
      {
        QMessageBox::warning( nullptr, title, message );
        break;
      }
      case Qgis::MessageLevel::Critical:
      {
        QMessageBox::critical( nullptr, title, message );
        break;
      }
    }
  }
}

void QgsLoadRasterAttributeTableDialog::updateButtons()
{
  const bool isValidPath { ! mDbfPathWidget->filePath().isEmpty() &&QFile::exists( mDbfPathWidget->filePath() ) };
  mButtonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isValidPath );
}
