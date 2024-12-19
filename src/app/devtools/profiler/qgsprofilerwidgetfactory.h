/***************************************************************************
    qgsprofilerwidgetfactory.h
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
#ifndef QGSPROFILERWIDGETFACTORY_H
#define QGSPROFILERWIDGETFACTORY_H

#include "qgsdevtoolwidgetfactory.h"

class QgsRuntimeProfiler;

class QgsProfilerWidgetFactory : public QgsDevToolWidgetFactory
{
  public:
    QgsProfilerWidgetFactory( QgsRuntimeProfiler *profiler );
    QgsDevToolWidget *createWidget( QWidget *parent = nullptr ) const override;

  private:
    QgsRuntimeProfiler *mProfiler = nullptr;
};


#endif // QGSPROFILERWIDGETFACTORY_H
