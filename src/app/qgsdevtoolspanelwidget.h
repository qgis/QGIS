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

class QgsDevToolWidgetFactory;

class APP_EXPORT QgsDevToolsPanelWidget : public QWidget, private Ui::QgsDevToolsWidgetBase
{
    Q_OBJECT
  public:

    QgsDevToolsPanelWidget( const QList<QgsDevToolWidgetFactory *> &factories, QWidget *parent = nullptr );
    ~QgsDevToolsPanelWidget() override;

    void addToolFactory( QgsDevToolWidgetFactory *factory );

    void removeToolFactory( QgsDevToolWidgetFactory *factory );

  private slots:

    void setCurrentTool( int row );

  private:

    QMap< QgsDevToolWidgetFactory *, int> mFactoryPages;
};

#endif // QGSDEVTOOLSPANELWIDGET_H
