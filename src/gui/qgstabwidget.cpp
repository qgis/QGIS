/***************************************************************************
  qgstabwidget.cpp - QgsTabWidget

 ---------------------
 begin                : 8.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstabwidget.h"

#include "qgslogger.h"

QgsTabWidget::QgsTabWidget( QWidget *parent )
  : QTabWidget( parent )
{
}

void QgsTabWidget::hideTab( QWidget *tab )
{
  QgsDebugMsg( "Hide" );
  TabInformation &info = mTabs[ realTabIndex( tab )];
  if ( info.visible )
  {
    mSetTabVisibleFlag = true;
    removeTab( info.sourceIndex );
    info.visible = false;
    mSetTabVisibleFlag = false;
  }
}

void QgsTabWidget::showTab( QWidget *tab )
{
  QgsDebugMsg( "Show" );
  TabInformation &info = mTabs[ realTabIndex( tab )];
  if ( ! info.visible )
  {
    mSetTabVisibleFlag = true;
    insertTab( info.sourceIndex + 1, info.widget, info.label );
    info.visible = true;
    mSetTabVisibleFlag = false;
  }
}

void QgsTabWidget::setTabVisible( QWidget *tab, bool visible )
{
  if ( visible )
    showTab( tab );
  else
    hideTab( tab );
}

int QgsTabWidget::realTabIndex( QWidget *widget )
{
  int realIndex = 0;
  Q_FOREACH ( const TabInformation &info, mTabs )
  {
    if ( info.widget == widget )
      return realIndex;
    ++realIndex;
  }
  return -1;
}

void QgsTabWidget::tabInserted( int index )
{
  if ( !mSetTabVisibleFlag )
  {
    QWidget *newWidget = widget( index );

    if ( index == 0 )
    {
      mTabs.insert( 0, TabInformation( newWidget, tabText( index ) ) );
    }
    else
    {
      bool inserted = false;
      QList<TabInformation>::iterator it;

      for ( it = mTabs.begin(); it != mTabs.end(); ++it )
      {
        if ( it->sourceIndex == index )
        {
          mTabs.insert( it, TabInformation( newWidget, tabText( index ) ) );
          inserted = true;
          break;
        }
      }

      if ( !inserted )
      {
        mTabs.append( TabInformation( newWidget, tabText( index ) ) );
      }
    }
  }

  synchronizeIndexes();
}

void QgsTabWidget::tabRemoved( int index )
{
  if ( !mSetTabVisibleFlag )
  {
    QList<TabInformation>::iterator it;

    for ( it = mTabs.begin(); it != mTabs.end(); ++it )
    {
      if ( it->sourceIndex == index )
      {
        mTabs.removeOne( *it );
        break;
      }
    }
  }

  synchronizeIndexes();
}

void QgsTabWidget::synchronizeIndexes()
{
  QgsDebugMsg( "---------" );
  int i = -1;
  QWidget *nextWidget = widget( 0 );

  QList<TabInformation>::iterator it;

  for ( it = mTabs.begin(); it != mTabs.end(); ++it )
  {
    if ( it->widget == nextWidget )
    {
      i++;
      nextWidget = widget( i + 1 );
    }
    it->sourceIndex = i;
    QgsDebugMsg( QStringLiteral( "Tab %1 (%2): %3" ).arg( it->sourceIndex ).arg( it->label ).arg( i ) );
  }
}

QgsTabWidget::TabInformation QgsTabWidget::tabInfo( QWidget *widget )
{
  Q_FOREACH ( const TabInformation &info, mTabs )
  {
    if ( info.widget == widget )
      return info;
  }
  return TabInformation();
}

bool QgsTabWidget::TabInformation::operator ==( const QgsTabWidget::TabInformation &other )
{
  return other.widget == widget && other.sourceIndex == sourceIndex;
}
