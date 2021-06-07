/***************************************************************************
    qgsprofilerwidgetfactory.cpp
    -------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
