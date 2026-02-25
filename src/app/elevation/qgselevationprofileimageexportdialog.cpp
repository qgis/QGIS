/***************************************************************************
                          qgselevationprofileimageexportdialog.h
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationprofileimageexportdialog.h"

#include "qgselevationprofileexportsettingswidget.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsplot.h"

#include <QString>

#include "moc_qgselevationprofileimageexportdialog.cpp"

using namespace Qt::StringLiterals;

QgsElevationProfileImageExportDialog::QgsElevationProfileImageExportDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  mProfileSettingsWidget = new QgsElevationProfileExportSettingsWidget();
  scrollAreaLayout->addWidget( mProfileSettingsWidget );
  scrollAreaLayout->addStretch( 1 );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [] {
    QgsHelp::openHelp( u"map_views/elevation_profile.html#export-elevation-profile"_s );
  } );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsElevationProfileImageExportDialog::setPlotSettings( const Qgs2DXyPlot &plot )
{
  mProfileSettingsWidget->setPlotSettings( plot );
}

void QgsElevationProfileImageExportDialog::updatePlotSettings( Qgs2DXyPlot &plot )
{
  mProfileSettingsWidget->updatePlotSettings( plot );
}

void QgsElevationProfileImageExportDialog::setImageSize( const QSize &size )
{
  mWidthSpinBox->setValue( size.width() );
  mHeightSpinBox->setValue( size.height() );
}

QSize QgsElevationProfileImageExportDialog::imageSize() const
{
  return QSize( mWidthSpinBox->value(), mHeightSpinBox->value() );
}
