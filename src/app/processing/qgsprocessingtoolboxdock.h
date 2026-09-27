/***************************************************************************
    qgsprocessingtoolboxdock.h
    ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGTOOLBOXDOCK_H
#define QGSPROCESSINGTOOLBOXDOCK_H

#include "ui_qgsprocessingtoolboxdockwidgetbase.h"

#include "qgsdockwidget.h"

class QgsProcessingProvider;

class QgsProcessingToolboxDockWidget : public QgsDockWidget, private Ui::QgsProcessingToolboxDockWidgetBase
{
    Q_OBJECT
  public:
    QgsProcessingToolboxDockWidget( QWidget *parent );

    void initializeActions();

    void setInPlaceEditMode( bool enabled );

  private slots:

    void setFilterString( const QString &string );
    void showPopupMenu( const QPoint &pos );
    void executeCurrentAlgorithm();
    void executeCurrentAlgorithmAsBatchProcess();
    void openSettings( const QString &url );

    void providerAdded( const QString &id );
    void providerRemoved( const QString &id );

    void activeLayerChanged( QgsMapLayer *layer );

  private:
    bool hasDisabledProviders();
    void addProviderActions( QgsProcessingProvider *provider );

    bool mTipWasClosed = false;
    bool mInPlaceMode = false;
};

#endif // QGSPROCESSINGTOOLBOXDOCK_H
