/***************************************************************************
    qgsqueryloggerwidgetfactory.h
    -------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSQUERYLOGGERWIDGETFACTORY_H
#define QGSQUERYLOGGERWIDGETFACTORY_H

#include "qgsdevtoolwidgetfactory.h"

class QgsAppQueryLogger;

class QgsDatabaseQueryLoggerWidgetFactory: public QgsDevToolWidgetFactory
{
  public:

    QgsDatabaseQueryLoggerWidgetFactory( QgsAppQueryLogger *logger );
    QgsDevToolWidget *createWidget( QWidget *parent = nullptr ) const override;

  private:

    QgsAppQueryLogger *mLogger = nullptr;
};


#endif // QGSQUERYLOGGERWIDGETFACTORY_H
