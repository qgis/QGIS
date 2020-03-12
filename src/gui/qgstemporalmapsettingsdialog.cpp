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
  QVBoxLayout *vl = new QVBoxLayout( );

  mTemporalMapSettingsWidget = new QgsTemporalMapSettingsWidget( this );

  vl->addWidget( mTemporalMapSettingsWidget, 1 );

  QDialogButtonBox *box = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  vl->addWidget( box );
  setLayout( vl );

  connect( box, &QDialogButtonBox::accepted, this, &QgsTemporalMapSettingsDialog::accept );
  connect( box, &QDialogButtonBox::rejected, this, &QgsTemporalMapSettingsDialog::reject );

  setWindowTitle( tr( "Temporal Map Settings" ) );
}

QgsTemporalMapSettingsWidget *QgsTemporalMapSettingsDialog::mapSettingsWidget()
{
  return mTemporalMapSettingsWidget;
}

///@endcond
