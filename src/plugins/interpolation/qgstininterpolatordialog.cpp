/***************************************************************************
                              qgstininterpolatordialog.cpp
                              ----------------------------
  begin                : March 29, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstininterpolatordialog.h"
#include "qgstininterpolator.h"
#include <QFileDialog>
#include <QSettings>

QgsTINInterpolatorDialog::QgsTINInterpolatorDialog( QWidget* parent, QgisInterface* iface ): QgsInterpolatorDialog( parent, iface )
{
  setupUi( this );

  //don't export triangulation by default
  mExportTriangulationCheckBox->setCheckState( Qt::Unchecked );
  mTriangulationFileEdit->setEnabled( false );
  mTriangulationFileButton->setEnabled( false );

  //enter available interpolation methods
  mInterpolationComboBox->insertItem( 0, tr( "Linear interpolation" ) );
  //mInterpolationComboBox->insertItem(1, tr("Clough-Toucher interpolation")); //to come...
}

QgsTINInterpolatorDialog::~QgsTINInterpolatorDialog()
{

}

QgsInterpolator* QgsTINInterpolatorDialog::createInterpolator() const
{
  QgsTINInterpolator* theInterpolator = new QgsTINInterpolator( mInputData, true );
  if ( mExportTriangulationCheckBox->checkState() == Qt::Checked )
  {
    theInterpolator->setExportTriangulationToFile( true );
    theInterpolator->setTriangulationFilePath( mTriangulationFileEdit->text() );
  }
  else
  {
    theInterpolator->setExportTriangulationToFile( false );
  }
  return theInterpolator;
}

void QgsTINInterpolatorDialog::on_mExportTriangulationCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked )
  {
    mTriangulationFileEdit->setEnabled( true );
    mTriangulationFileButton->setEnabled( true );
  }
  else
  {
    mTriangulationFileEdit->setEnabled( false );
    mTriangulationFileButton->setEnabled( false );
  }
}

void QgsTINInterpolatorDialog::on_mTriangulationFileButton_clicked()
{
  QSettings s;
  //read last triangulation directory
  QString lastTriangulationDir = s.value( "/Interpolation/lastTriangulationDir", "" ).toString();
  QString filename = QFileDialog::getSaveFileName( 0, tr( "Save triangulation to file" ), lastTriangulationDir, "*shp" );
  if ( !filename.isEmpty() )
  {
    mTriangulationFileEdit->setText( filename );

    //and save triangulation directory
    QFileInfo triangulationFileInfo( filename );
    QDir fileDir = triangulationFileInfo.absoluteDir();
    if ( fileDir.exists() )
    {
      s.setValue( "/Interpolation/lastTriangulationDir", triangulationFileInfo.absolutePath() );
    }
  }
}
