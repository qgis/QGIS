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

#include "qgsprocessingguiregistry.h"
#include "qgsprocessingwidgetcontext.h"

#include <QObject>

class QgisApp;

class QgsAppProcessingWidgetContextGenerator : public QgsProcessingWidgetContextGenerator
{
  public:
    QgsAppProcessingWidgetContextGenerator( QgisApp *app );

    QgsProcessingParameterWidgetContext createWidgetContext() final;

  private:
    QgisApp *mQgisApp = nullptr;
};

class QgsAppProcessingContextFactory : public QgsProcessingContextFactory
{
  public:
    QgsAppProcessingContextFactory( QgisApp *app );

    QgsProcessingContext *createContext( QgsProcessingFeedback *feedback = nullptr ) final;

    QgsExpressionContext createExpressionContext() const;

  private:
    QgisApp *mQgisApp = nullptr;
};

class QgsAppProcessingUtils : public QObject
{
    Q_OBJECT
  public:
    QgsAppProcessingUtils( QgisApp *app );

    void registerActions();

    void openModelDesigner();

    static void initProjectModelProvider();

  private slots:

    void updateModels();

  private:

    /**
     * Returns the older menu titles used in processing settings for menu configuration.
     * This is a quasi-stable string, as changing it would break user menu configuration.
     */
    QString legacyMenuTitle( Qgis::ProcessingMenu menu );

    QMenu *parentMenu( Qgis::ProcessingMenu menu );

    //! Finds the matching Processing menu, creating it if required.
    QMenu *processingToolMenu( Qgis::ProcessingMenu menu );

    QgisApp *mQgisApp = nullptr;
};

#endif // QGSAPPPROCESSINGUTILS_H
