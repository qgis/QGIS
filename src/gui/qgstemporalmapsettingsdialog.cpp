/***************************************************************************
                         qgstemporalmapsettingsdialog.cpp
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalmapsettingsdialog.h"
#include "qgsgui.h"
#include "qgstemporalmapsettingswidget.h"

///@cond PRIVATE

QgsTemporalMapSettingsDialog::QgsTemporalMapSettingsDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  connect( this, &QDialog::accepted, this, &QgsTemporalMapSettingsDialog::accept );
  connect( this, &QDialog::rejected, this, &QgsTemporalMapSettingsDialog::reject );

  mTemporalMapSettingsWidget = new QgsTemporalMapSettingsWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( frame );
  layout->addWidget( mTemporalMapSettingsWidget );

  setWindowTitle( tr( "Temporal Map Settings" ) );
}

QgsTemporalMapSettingsWidget *QgsTemporalMapSettingsDialog::mapSettingsWidget()
{
  return mTemporalMapSettingsWidget;
}

///@endcond
