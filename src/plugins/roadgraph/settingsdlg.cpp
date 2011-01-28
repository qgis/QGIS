/***************************************************************************
 *   Copyright (C) 2010 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
//road-graph plugin includes
#include "settingsdlg.h"
#include <qgscontexthelp.h>

//qt includes
#include <qlabel.h>
#include <qcombobox.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <qdialogbuttonbox.h>
#include <qmessagebox.h>


// Qgis includes
#include "settings.h"

//standard includes

RgSettingsDlg::RgSettingsDlg( RgSettings *settings, QWidget* parent, Qt::WFlags fl )
    : mSettings( settings ), QDialog( parent, fl )
{
  // create base widgets;
  setWindowTitle( tr( "Road graph plugins settings" ) );
  QVBoxLayout *v = new QVBoxLayout( this );

  QHBoxLayout *h = new QHBoxLayout();
  QLabel *l = new QLabel( tr( "Plugins time unit:" ), this );
  h->addWidget( l );
  mcbPluginsTimeUnit = new QComboBox( this );
  h->addWidget( mcbPluginsTimeUnit );
  v->addLayout( h );

  h = new QHBoxLayout();
  l = new QLabel( tr( "Plugins distance unit:" ), this );
  h->addWidget( l );
  mcbPluginsDistanceUnit = new QComboBox( this );
  h->addWidget( mcbPluginsDistanceUnit );
  v->addLayout( h );

  /*
  h = new QHBoxLayout();
  l = new QLabel( tr("Select graph source:"), this);
  h->addWidget(l);
  mcbGraphDirector = new QComboBox( this );
  h->addWidget(mcbGraphDirector);
  v->addLayout(h);
  */

  mSettingsWidget = mSettings->getGui( this );
  v->addWidget( mSettingsWidget );

  QDialogButtonBox *bb = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  connect( bb, SIGNAL( accepted() ), this, SLOT( on_buttonBox_accepted() ) );
  connect( bb, SIGNAL( rejected() ), this, SLOT( on_buttonBox_rejected() ) );
  v->addWidget( bb );

  mcbPluginsTimeUnit->addItem( tr( "second" ), QVariant( "s" ) );
  mcbPluginsTimeUnit->addItem( tr( "hour" ), QVariant( "h" ) );
  mcbPluginsDistanceUnit->addItem( tr( "meter" ), QVariant( "m" ) );
  mcbPluginsDistanceUnit->addItem( tr( "kilometer" ), QVariant( "km" ) );

} // RgSettingsDlg::RgSettingsDlg()

RgSettingsDlg::~RgSettingsDlg()
{
}

void RgSettingsDlg::on_buttonBox_accepted()
{
  mSettings->setFromGui( mSettingsWidget );
  accept();
}

void RgSettingsDlg::on_buttonBox_rejected()
{
  reject();
}

void RgSettingsDlg::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( context_id );
}

QString RgSettingsDlg::timeUnitName()
{
  return mcbPluginsTimeUnit->itemData( mcbPluginsTimeUnit->currentIndex() ).toString();
}

void RgSettingsDlg::setTimeUnitName( const QString& name )
{
  int i = mcbPluginsTimeUnit->findData( QVariant( name ) );
  if ( i != -1 )
  {
    mcbPluginsTimeUnit->setCurrentIndex( i );
  }
}

QString RgSettingsDlg::distanceUnitName()
{
  return mcbPluginsDistanceUnit->itemData( mcbPluginsDistanceUnit->currentIndex() ).toString();
}

void RgSettingsDlg::setDistanceUnitName( const QString& name )
{
  int i = mcbPluginsDistanceUnit->findData( QVariant( name ) );
  if ( i != -1 )
  {
    mcbPluginsDistanceUnit->setCurrentIndex( i );
  }
}
