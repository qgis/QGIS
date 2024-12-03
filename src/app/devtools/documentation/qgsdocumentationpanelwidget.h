/***************************************************************************
    qgsdocumentationpanelwidget.h
    -------------------------
    begin                : October 2024
    copyright            : (C) 2024 by Yoann Quenach de Quivillic
    email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDOCUMENTATIONPANELWIDGET_H
#define QGSDOCUMENTATIONPANELWIDGET_H

#include "qgsdevtoolwidget.h"
#include "qgsconfig.h"
#include "ui_qgsdocumentationpanelbase.h"

#ifdef HAVE_WEBENGINE
class QWebEngineView;
#else
class QgsWebView;
#endif

/**
 * \ingroup app
 * \class QgsDocumentationPanelWidget
 * \brief A panel widget showing the API documentation.
 *
 * \since QGIS 3.42
 */
class QgsDocumentationPanelWidget : public QgsDevToolWidget, private Ui::QgsDocumentationPanelBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDocumentationPanelWidget.
     */
    QgsDocumentationPanelWidget( QWidget *parent );


    void showUrl( const QUrl &url );

  private:
#ifdef HAVE_WEBENGINE
    QWebEngineView *mWebView = nullptr;
#else
    QgsWebView *mWebView = nullptr;
#endif
};


#endif // QGSDOCUMENTATIONPANELWIDGET_H
