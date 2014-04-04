/***************************************************************************
                         qgsdxfexportdialog.cpp
                         ----------------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfexportdialog.h"
#include "qgsmaplayer.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgis.h"
#include <QFileDialog>
#include <QPushButton>
#include <QSettings>

QgsDxfExportDialog::QgsDxfExportDialog( QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );

  mModel = new QgsMapLayerProxyModel( this );
  mModel->sourceLayerModel()->setItemsCheckable( true );
  mModel->setFilters( QgsMapLayerProxyModel::HasGeometry );
  mLayersListView->setModel( mModel );

  connect( mFileLineEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( setOkEnabled() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( saveSettings() ) );
  connect( mSelectAllButton, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
  connect( mUnSelectAllButton, SIGNAL( clicked() ), this, SLOT( unSelectAll() ) );

  //last dxf symbology mode
  QSettings s;
  mSymbologyModeComboBox->setCurrentIndex( s.value( "qgis/lastDxfSymbologyMode", "2" ).toInt() );
  //last symbol scale
  mSymbologyScaleLineEdit->setText( s.value( "qgis/lastSymbologyExportScale", "50000" ).toString() );
  mMapExtentCheckBox->setChecked( s.value( "qgis/lastDxfMapRectangle", "false" ).toBool() );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

QgsDxfExportDialog::~QgsDxfExportDialog()
{
}

void QgsDxfExportDialog::selectAll()
{
  mModel->sourceLayerModel()->checkAll( Qt::Checked );
}

void QgsDxfExportDialog::unSelectAll()
{
  mModel->sourceLayerModel()->checkAll( Qt::Unchecked );
}

QList<QgsMapLayer*> QgsDxfExportDialog::layers() const
{
  return mModel->sourceLayerModel()->layersChecked();
}

double QgsDxfExportDialog::symbologyScale() const
{
  double scale = mSymbologyScaleLineEdit->text().toDouble();
  if ( qgsDoubleNear( scale, 0.0 ) )
  {
    return 1.0;
  }
  return scale;
}

QString QgsDxfExportDialog::saveFile() const
{
  return mFileLineEdit->text();
}

QgsDxfExport::SymbologyExport QgsDxfExportDialog::symbologyMode() const
{
  return ( QgsDxfExport::SymbologyExport )mSymbologyModeComboBox->currentIndex();
}

void QgsDxfExportDialog::on_mFileSelectionButton_clicked()
{
  //get last dxf save directory
  QSettings s;
  QString lastSavePath = s.value( "qgis/lastDxfDir" ).toString();

  QString filePath = QFileDialog::getSaveFileName( 0, tr( "Export as DXF" ), lastSavePath, tr( "DXF files *.dxf *.DXF" ) );
  if ( !filePath.isEmpty() )
  {
    mFileLineEdit->setText( filePath );
  }
}

void QgsDxfExportDialog::setOkEnabled()
{
  QPushButton* btn = buttonBox->button( QDialogButtonBox::Ok );

  QString filePath = mFileLineEdit->text();
  if ( filePath.isEmpty() )
  {
    btn->setEnabled( false );
  }

  QFileInfo fi( filePath );
  btn->setEnabled( fi.absoluteDir().exists() );
}

bool QgsDxfExportDialog::exportMapExtent() const
{
  return mMapExtentCheckBox->isChecked();
}

void QgsDxfExportDialog::saveSettings()
{
  QSettings s;
  QFileInfo dxfFileInfo( mFileLineEdit->text() );
  s.setValue( "qgis/lastDxfDir", dxfFileInfo.absolutePath() );
  s.setValue( "qgis/lastDxfSymbologyMode", mSymbologyModeComboBox->currentIndex() );
  s.setValue( "qgis/lastSymbologyExportScale", mSymbologyScaleLineEdit->text() );
  s.setValue( "qgis/lastDxfMapRectangle", mMapExtentCheckBox->isChecked() );
}
