/***************************************************************************
    qgsprofilerwidgetfactory.cpp
    -------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprofilerwidgetfactory.h"
#include "qgsprofilerpanelwidget.h"
#include "qgsapplication.h"

QgsProfilerWidgetFactory::QgsProfilerWidgetFactory( QgsRuntimeProfiler *profiler )
  : QgsDevToolWidgetFactory( QObject::tr( "Profiler" ), QgsApplication::getThemeIcon( QStringLiteral( "mIconStopwatch.svg" ) ) )
  , mProfiler( profiler )
{
}

QgsDevToolWidget *QgsProfilerWidgetFactory::createWidget( QWidget *parent ) const
{
  return new QgsProfilerPanelWidget( mProfiler, parent );
}
