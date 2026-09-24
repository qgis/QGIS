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
class QToolBar;

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

    /**
     * Returns the toolbar for algorithms, creating it if it does not yet exist.
     */
    QToolBar *algorithmsToolBar();

    /**
     * Checks the definitions of the default menus, pushing message log warnings when they are invalid
     */
    void validateDefaultAlgorithmActions();

    /**
     * Creates algorithm actions for all algorithms shown in menus and toolbars, and populates those menus
     * and toolbars accordingly.
     *
     * Any existing algorithm actions will be deleted.
     */
    QList< QAction * > createAlgorithmActions();

    /**
     * Adds some predefined actions to default toolbars, such as the "select by..." algorithms to the selection toolbar.
     */
    void addAlgorithmsToDefaultToolbars();

  private slots:

    void updateModels();

  private:
    QList< QAction * > createAlgorithmActionsForProvider( const QgsProcessingProvider *provider );

    QAction *createActionForAlgorithm( const QgsProcessingAlgorithm *algorithm, const QString &iconPath = QString() );

    /**
     *  Returns the menu title to use for a Processing menu. This is not a "stable" string, and is used for
     *  GUI display only.
     */
    QString menuTitle( Qgis::ProcessingMenu menu );

    /**
     * Returns the older menu titles used in processing settings for menu configuration.
     * This is a quasi-stable string, as changing it would break user menu configuration.
     */
    QString legacyMenuTitle( Qgis::ProcessingMenu menu );

    std::optional< Qgis::ProcessingMenu > menuForLegacySettingValue( const QString &settingValue );

    QMenu *parentMenu( Qgis::ProcessingMenu menu );

    //! Finds the matching Processing menu, creating it if required.
    QMenu *processingToolMenu( Qgis::ProcessingMenu menu );

    static QString algorithmActionText( const QgsProcessingAlgorithm *algorithm );

    QgisApp *mQgisApp = nullptr;
    QList< QAction * > mAlgorithmActions;
    QToolBar *mAlgorithmsToolbar = nullptr;
};

#endif // QGSAPPPROCESSINGUTILS_H
