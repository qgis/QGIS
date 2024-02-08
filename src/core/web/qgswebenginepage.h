/***************************************************************************
                          qgswebenginepage.h
                             -------------------
    begin                : December 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBENGINEPAGE_H
#define QGSWEBENGINEPAGE_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QObject>
#include <QUrl>
#include <QPageLayout>
#include <QSize>
#include <memory>

SIP_IF_MODULE( HAVE_WEBENGINE_SIP )

class QWebEnginePage;
class QPainter;

/**
 * \ingroup core
 * \brief A wrapper around the QWebEnginePage class, adding extra functionality.
 *
 * \warning This class is only available on QGIS builds with WebEngine support enabled.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsWebEnginePage : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsWebEnginePage, with the specified \a parent widget.
     */
    QgsWebEnginePage( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsWebEnginePage() override;

    /**
     * Returns a reference to the QWebEnginePage.
     *
     * \note Not available in Python bindings
     */
    QWebEnginePage *page() SIP_SKIP;

    /**
     * Sets the content of the web page to \a data. If the \a mimeType argument is empty, it is assumed that the content is text/plain,charset=US-ASCII
     *
     * The \a baseUrl is optional and used to resolve relative URLs in the document, such as referenced images or stylesheets.
     *
     * If \a blocking is TRUE then the call will block while the HTML is loaded. Otherwise the html is loaded immediately; external objects are loaded asynchronously.
     *
     * \warning Setting \a blocking to TRUE involves running the event loop on the current thread. Take care when calling from the main thread as incorrect use will result in crashes.
     *
     * \returns TRUE if loading was successful
     */
    bool setContent( const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl(), bool blocking = false );

    /**
     * Sets the content of this page to \a html.
     *
     * The \a baseUrl is optional and used to resolve relative URLs in the document, such as referenced images or stylesheets.
     *
     * If \a blocking is TRUE then the call will block while the HTML is loaded. Otherwise the html is loaded immediately; external objects are loaded asynchronously.
     *
     * \note This function works only for HTML, for other mime types (such as XHTML and SVG) setContent() should be used instead.
     *
     * \warning Setting \a blocking to TRUE involves running the event loop on the current thread. Take care when calling from the main thread as incorrect use will result in crashes.
     *
     * \returns TRUE if loading was successful
     */
    bool setHtml( const QString &html, const QUrl &baseUrl = QUrl(), bool blocking = false );

    /**
     * Sets the \a url of the web page to be displayed.
     *
     * Setting this property clears the page and loads the URL.
     *
     * If \a blocking is TRUE then the call will block while the HTML is loaded. Otherwise the html is loaded immediately; external objects are loaded asynchronously.
     *
     * \warning Setting \a blocking to TRUE involves running the event loop on the current thread. Take care when calling from the main thread as incorrect use will result in crashes.
     *
     * \returns TRUE if loading was successful
     */
    bool setUrl( const QUrl &url, bool blocking = false );

    /**
     * Returns the size of the page document, in pixels.
     *
     * \warning If the page content was NOT loaded using a blocking method, then this method involves running the event loop on the current thread. Take care when calling from the main thread as incorrect use will result in crashes.
     */
    QSize documentSize() const;

    /**
     * Renders the web page contents to a \a painter. Content will be rendered as vector objects.
     *
     * The \a painterRect argument specifies the target rectangle for the page in \a painter coordinates.
     *
     * \warning This method involves running the event loop on the current thread. Take care when calling from the main thread as incorrect use will result in crashes.
     * \warning This method requires a QGIS build with PDF4Qt library support.
     *
     * \returns TRUE if rendering was successful
     *
     * \throws QgsNotSupportedException on QGIS builds without PDF4Qt library support.
     */
    bool render( QPainter *painter, const QRectF &painterRect ) SIP_THROW( QgsNotSupportedException );

  signals:

    /**
     * This signal is emitted when the page starts loading content.
     */
    void loadStarted();

    /**
     * This signal is emitted when the global \a progress status changes.
     *
     * The current value is provided by \a progress and scales from 0 to 100.
     * It accumulates changes from all the child frames.
     */
    void loadProgress( int progress );

    /**
     * This signal is emitted when the page finishes loading content.
     *
     * This signal is independent of script execution or page rendering.
     *
     * \a ok will indicate whether the load was successful or any error occurred.
     */
    void loadFinished( bool ok );

  private:

    void handlePostBlockingLoadOperations();

    std::unique_ptr< QWebEnginePage > mPage;
    mutable QSize mCachedSize;
};

#endif // QGSWEBENGINEPAGE_H
