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

QgsAddTabOrGroup::QgsAddTabOrGroup( QgsVectorLayer *lyr, QList < TabPair > tabList, QWidget * parent )
    : QDialog( parent )
    , mLayer( lyr )
    , mTabs( tabList )
{
  setupUi( this );

  mTabButton->setChecked( true );
  mTabList->setEnabled( false );
  if ( mTabs.size() > 0 )
  {
    int i = 0;
    foreach ( TabPair tab, mTabs )
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

bool QgsAddTabOrGroup::tabButtonIsChecked()
{
  return mTabButton->isChecked();
}

void QgsAddTabOrGroup::on_mGroupButton_toggled( bool checked )
{
  mTabList->setEnabled( checked );
}

void QgsAddTabOrGroup::on_mTabButton_toggled( bool checked )
{
  mTabList->setEnabled( !checked );
}
