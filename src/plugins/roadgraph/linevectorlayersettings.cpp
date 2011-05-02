/***************************************************************************
 *   Copyright (C) 2009 by Sergey Yakushev                                 *
 *   yakushevs <at> list.ru                                                *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

/**
 * \file linevectorlayersettings.cpp
 * \brief implementation of RgLineVectorLayerSettings
 */

#include "linevectorlayersettings.h"
#include "linevectorlayerwidget.h"

// Qgis includes
#include <qgsproject.h>

// QT includes
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>

//standard includes

RgLineVectorLayerSettings::RgLineVectorLayerSettings()
{
  mLayer = "";
  mDirection = "";
  mDefaultDirection = Both;
  mSpeed = "";
  mDefaultSpeed = 40;
}
RgLineVectorLayerSettings::~RgLineVectorLayerSettings()
{

}
bool RgLineVectorLayerSettings::test()
{
  // implement me

  // check default speed
  if ( mDefaultSpeed <= 0.0 )
  {
    return false;
  }
  if ( mLayer == "" )
  {
    return false;
  }
  // implement me

  return true;
} // RgLineVectorLayerSettings::test()

void RgLineVectorLayerSettings::read( const QgsProject *project )
{
  int dd          = project->readNumEntry( "roadgraphplugin", "/defaultDirection" );
  mDirection    = project->readEntry( "roadgraphplugin", "/directionField" );
  mFirstPointToLastPointDirectionVal =
    project->readEntry( "roadgraphplugin", "/FirstPointToLastPointDirectionVal" );
  mLastPointToFirstPointDirectionVal =
    project->readEntry( "roadgraphplugin", "/LastPointToFirstPointDirectionVal" );
  mBothDirectionVal = project->readEntry( "roadgraphplugin", "/BothDirectionVal" );
  mSpeed        = project->readEntry( "roadgraphplugin", "/speedField" );
  mDefaultSpeed = project->readDoubleEntry( "roadgraphplugin", "/defaultSpeed" );
  mLayer        = project->readEntry( "roadgraphplugin", "/layer" );
  mSpeedUnitName = project->readEntry( "roadgraphplugin", "/speedUnitName" );

  if ( dd == 1 )
  {
    mDefaultDirection = FirstPointToLastPoint;
  }
  else if ( dd == 2 )
  {
    mDefaultDirection = LastPointToFirstPoint;
  }
  else if ( dd == 3 )
  {
    mDefaultDirection = Both;
  }

} // RgLineVectorLayerSettings::read( const QgsProject *project )

void RgLineVectorLayerSettings::write( QgsProject *project )
{
  project->writeEntry( "roadgraphplugin", "/defaultDirection", mDefaultDirection );
  project->writeEntry( "roadgraphplugin", "/directionField",   mDirection );
  project->writeEntry( "roadgraphplugin", "/FirstPointToLastPointDirectionVal",
                       mFirstPointToLastPointDirectionVal );
  project->writeEntry( "roadgraphplugin", "/LastPointToFirstPointDirectionVal",
                       mLastPointToFirstPointDirectionVal );
  project->writeEntry( "roadgraphplugin", "/BothDirectionVal", mBothDirectionVal );
  project->writeEntry( "roadgraphplugin", "/speedField",   mSpeed );
  project->writeEntry( "roadgraphplugin", "/defaultSpeed", mDefaultSpeed );
  project->writeEntry( "roadgraphplugin", "/layer",        mLayer );
  project->writeEntry( "roadgraphplugin", "/speedUnitName",    mSpeedUnitName );
} // RgLineVectorLayerSettings::write( QgsProject *project )

QWidget* RgLineVectorLayerSettings::getGui( QWidget *parent )
{
  return new RgLineVectorLayerSettingsWidget( this, parent );
}

void RgLineVectorLayerSettings::setFromGui( QWidget *myGui )
{
  RgLineVectorLayerSettingsWidget* w = dynamic_cast<RgLineVectorLayerSettingsWidget*>( myGui );
  if ( w == NULL )
    return;

  mFirstPointToLastPointDirectionVal  = w->mleFirstPointToLastPointDirection->text();
  mLastPointToFirstPointDirectionVal  = w->mleLastPointToFirstPointDirection->text();
  mBothDirectionVal                   = w->mleBothDirection->text();
  mDirection                          = w->mcbDirection->currentText();
  mLayer                              = w->mcbLayers->currentText();

  if ( w->mcbDirectionDefault->currentIndex() == 0 )
  {
    mDefaultDirection = Both;
  }
  else if ( w->mcbDirectionDefault->currentIndex() == 1 )
  {
    mDefaultDirection = FirstPointToLastPoint;
  }
  else if ( w->mcbDirectionDefault->currentIndex() == 2 )
  {
    mDefaultDirection = LastPointToFirstPoint;
  }

  mSpeed = w->mcbSpeed->currentText();
  mDefaultSpeed = w->msbSpeedDefault->value();

  if ( w->mcbUnitOfSpeed->currentIndex() == 0 )
  {
    mSpeedUnitName = "m/s";
  }
  else if ( w->mcbUnitOfSpeed->currentIndex() == 1 )
  {
    mSpeedUnitName = "km/h";
  }
}
