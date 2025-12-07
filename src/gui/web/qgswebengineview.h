/***************************************************************************
                          qgswebengineview.cpp
                             -------------------
    begin                : December 2025
    copyright            : (C) 2025 by Jean-Baptiste Peter
    email                : jbpeter at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBENGINEVIEW_H
#define QGSWEBENGINEVIEW_H

#include "qgis_sip.h"
#include "qgis_gui.h"
#include <QWidget>

SIP_IF_MODULE( HAVE_WEBENGINE_SIP )

class QWebEngineView;
class QUrl;
class QDragEnterEvent;
class QDropEvent;
class QDialog;

/**
 * \ingroup gui
 * \brief A wrapper around the QWebEngineView class.
 * \warning This class is only available on QGIS builds with WebEngine support enabled.
 */
class GUI_EXPORT QgsWebEngineView : public QWidget
{
    Q_OBJECT

  public:
    /**
   * Constructor for QgsWebEngineView.
   */
    QgsWebEngineView( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsWebEngineView() override;

    /**
   * Sets the URL for the web engine view to load.
   */
    void setUrl( const QUrl &url );

    /**
   * Sets whether the web engine view accepts drops.
   */
    void setAcceptDrops( bool accept );

    /**
   * Sets the context menu policy for the web engine view.
   */
    void setContextMenuPolicy( Qt::ContextMenuPolicy policy );

    /**
   * Opens a debug view window connected to this webview.
   * The debug view provides developer tools for inspecting and debugging
   * the web content loaded in this view.
   */
    void openDebugView();

  protected:
    /**
     * Called for drag enter events.
     * Can be reimplemented in Python to handle custom drag enter behavior.
     */
    virtual void dragEnterEvent( QDragEnterEvent *event ) override;

    /**
     * Called for drop events.
     * Can be reimplemented in Python to handle custom drop behavior.
     */
    virtual void dropEvent( QDropEvent *event ) override;

    /**
     * Event filter to intercept events from the underlying QWebEngineView.
     */
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private:
    std::unique_ptr< QWebEngineView > mView;
    std::unique_ptr< QWebEngineView > mDebugView;
};

#endif // QGSWEBENGINEVIEW_H