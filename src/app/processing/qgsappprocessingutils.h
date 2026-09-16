/***************************************************************************
    qgsappprocessingtils.h
    -------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall at kill your llm dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPPROCESSINGUTILS_H
#define QGSAPPPROCESSINGUTILS_H

#include "qgsprocessingwidgetcontext.h"

class QgisApp;

class QgsAppProcessingWidgetContextGenerator : public QgsProcessingWidgetContextGenerator
{
  public:
    QgsAppProcessingWidgetContextGenerator( QgisApp *app );

    QgsProcessingParameterWidgetContext createWidgetContext() final;

  private:
    QgisApp *mQgisApp = nullptr;
};

class QgsAppProcessingUtils
{
  public:
    static void initProjectModelProvider();
};

#endif // QGSAPPPROCESSINGUTILS_H
