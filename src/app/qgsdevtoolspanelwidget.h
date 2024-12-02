/***************************************************************************
    qgsdevtoolspanelwidget.h
    ---------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDEVTOOLSPANELWIDGET_H
#define QGSDEVTOOLSPANELWIDGET_H

#include "ui_qgsdevtoolswidgetbase.h"
#include "qgis_app.h"
#include "qgssettingstree.h"

class QgsDevToolWidgetFactory;
class QgsDevToolWidget;
class QgsDocumentationPanelWidget;
class APP_EXPORT QgsDevToolsPanelWidget : public QWidget, private Ui::QgsDevToolsWidgetBase
{
    Q_OBJECT
  public:
    static inline QgsSettingsTreeNode *sTreeDevTools = QgsSettingsTree::sTreeApp->createChildNode( QStringLiteral( "devtools" ) );
    static const QgsSettingsEntryString *settingLastActiveTab;

    QgsDevToolsPanelWidget( const QList<QgsDevToolWidgetFactory *> &factories, QWidget *parent = nullptr );
    ~QgsDevToolsPanelWidget() override;

    void addToolWidget( QgsDevToolWidget *widget );
    void addToolFactory( QgsDevToolWidgetFactory *factory );

    void removeToolFactory( QgsDevToolWidgetFactory *factory );

    void setActiveTab( const QString &title );

    void showApiDocumentation(
      Qgis::DocumentationApi api = Qgis::DocumentationApi::PyQgis,
      Qgis::DocumentationBrowser browser = Qgis::DocumentationBrowser::DeveloperToolsPanel,
      const QString &object = QString(),
      const QString &module = QString()
    );

    void showUrl( const QUrl &url );

  private slots:

    void setCurrentTool( int row );

  private:
    QMap<QgsDevToolWidgetFactory *, int> mFactoryPages;
    QgsDocumentationPanelWidget *mDocumentationPanel = nullptr;
};

#endif // QGSDEVTOOLSPANELWIDGET_H
