/***************************************************************************
    qgsappgpstools.cpp
    ---------------------
    begin                : May 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgisapp.h"
#include "qgsappgpstools.h"
#include "moc_qgsappgpstools.cpp"
#include "qgsappgpsdigitizing.h"

QgsAppGpsTools::QgsAppGpsTools()
{
}

void QgsAppGpsTools::setGpsPanelConnection( QgsGpsConnection *connection )
{
  QgisApp::instance()->setGpsPanelConnection( connection );
}

void QgsAppGpsTools::createFeatureFromGpsTrack()
{
  QgisApp::instance()->gpsDigitizing()->createFeature();
}

void QgsAppGpsTools::setGpsTrackLineSymbol( QgsLineSymbol *symbol )
{
  if ( symbol )
  {
    QgsAppGpsDigitizing::setGpsTrackLineSymbol( symbol );
    QgisApp::instance()->gpsDigitizing()->updateTrackAppearance();
  }
}
