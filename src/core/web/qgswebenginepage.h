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
#include <memory>

SIP_IF_MODULE( HAVE_WEBENGINE_SIP )

class QWebEnginePage;

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
     * The html is loaded immediately; external objects are loaded asynchronously.
     */
    void setContent( const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl() );

    /**
     * Sets the content of this page to \a html.
     *
     * The \a baseUrl is optional and used to resolve relative URLs in the document, such as referenced images or stylesheets.
     *
     * The html is loaded immediately; external objects are loaded asynchronously.
     *
     * \note This function works only for HTML, for other mime types (such as XHTML and SVG) setContent() should be used instead.
     */
    void setHtml( const QString &html, const QUrl &baseUrl = QUrl() );

    /**
     * Sets the \a url of the web page to be displayed.
     *
     * Setting this property clears the page and loads the URL.
     */
    void setUrl( const QUrl &url );

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

    std::unique_ptr< QWebEnginePage > mPage;
};

#endif // QGSWEBENGINEPAGE_H
