/***************************************************************************
                          qgsaddtaborgroup.h
        Add a tab or a group for the tab and group display of fields
                             -------------------
    begin                : 2012-07-30
    copyright            : (C) 2012 by Denis Rouzaud
    email                : denis dot rouzaud at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsaddtaborgroup.h"

#include <QTreeWidgetItem>
#include <QComboBox>
#include <QRadioButton>

QgsAddTabOrGroup::QgsAddTabOrGroup( QgsVectorLayer *lyr, const QList < TabPair >& tabList, QWidget * parent )
    : QDialog( parent )
    , mLayer( lyr )
    , mTabs( tabList )
{
  setupUi( this );

  mTabButton->setChecked( true );
  mTabList->setEnabled( false );
  if ( !mTabs.isEmpty() )
  {
    int i = 0;
    Q_FOREACH ( const TabPair& tab, mTabs )
    {
      mTabList->addItem( tab.first, i );
      ++i;
    }
  }
  else
  {
    mGroupButton->setEnabled( false );
  }

  connect( mTabButton, SIGNAL( toggled( bool ) ), this, SLOT( on_mTabButton_toggled( bool ) ) );
  connect( mGroupButton, SIGNAL( toggled( bool ) ), this, SLOT( on_mGroupButton_toggled( bool ) ) );

  mColumnCountSpinBox->setValue( QSettings().value( "/qgis/attributeForm/defaultTabColumnCount", 1 ).toInt() );

  setWindowTitle( tr( "Add tab or group for %1" ).arg( mLayer->name() ) );
} // QgsVectorLayerProperties ctor

QgsAddTabOrGroup::~QgsAddTabOrGroup()
{
}

QString QgsAddTabOrGroup::name()
{
  return mName->text();
}

QTreeWidgetItem* QgsAddTabOrGroup::tab()
{
  TabPair tab = mTabs.at( mTabList->itemData( mTabList->currentIndex() ).toInt() );
  return tab.second;
}

int QgsAddTabOrGroup::columnCount() const
{
  return mColumnCountSpinBox->value();
}

bool QgsAddTabOrGroup::tabButtonIsChecked()
{
  return mTabButton->isChecked();
}

void QgsAddTabOrGroup::accept()
{
  if ( mColumnCountSpinBox->value() > 0 )
  {
    if ( mGroupButton->isChecked() )
    {
      QSettings().setValue( "/qgis/attributeForm/defaultGroupColumnCount", mColumnCountSpinBox->value() );
    }
    else
    {
      QSettings().setValue( "/qgis/attributeForm/defaultTabColumnCount", mColumnCountSpinBox->value() );
    }
  }

  QDialog::accept();
}

void QgsAddTabOrGroup::on_mGroupButton_toggled( bool checked )
{
  mTabList->setEnabled( checked );

  if ( checked )
  {
    mColumnCountSpinBox->setValue( QSettings().value( "/qgis/attributeForm/defaultGroupColumnCount", 1 ).toInt() );
  }
}

void QgsAddTabOrGroup::on_mTabButton_toggled( bool checked )
{
  mTabList->setEnabled( !checked );
  if ( checked )
    mColumnCountSpinBox->setValue( QSettings().value( "/qgis/attributeForm/defaultTabColumnCount", 1 ).toInt() );
}
