/***************************************************************************

               ----------------------------------------------------
              date                 : 19.5.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWEBPAGE_H
#define QGSWEBPAGE_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsmessagelog.h"
#include <QObject>

#ifdef WITH_QTWEBKIT
#include <QWebPage>
#else

#include "qgswebframe.h"

#include <QMenu>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QTextBrowser>


/**
 * \ingroup core
 * \brief A collection of stubs to mimic the API of a QWebSettings on systems
 * where QtWebkit is not available.
 */
class CORE_EXPORT QWebSettings : public QObject
{
/// @cond NOT_STABLE_API
    Q_OBJECT

  public:

    enum WebAttribute
    {
      AutoLoadImages,
      JavascriptEnabled,
      JavaEnabled,
      PluginsEnabled,
      PrivateBrowsingEnabled,
      JavascriptCanOpenWindows,
      JavascriptCanAccessClipboard,
      DeveloperExtrasEnabled,
      LinksIncludedInFocusChain,
      ZoomTextOnly,
      PrintElementBackgrounds,
      OfflineStorageDatabaseEnabled,
      OfflineWebApplicationCacheEnabled,
      LocalStorageEnabled,
      LocalContentCanAccessRemoteUrls,
      DnsPrefetchEnabled,
      XSSAuditingEnabled,
      AcceleratedCompositingEnabled,
      SpatialNavigationEnabled,
      LocalContentCanAccessFileUrls,
      TiledBackingStoreEnabled,
      FrameFlatteningEnabled,
      SiteSpecificQuirksEnabled,
      JavascriptCanCloseWindows,
      WebGLEnabled,
      CSSRegionsEnabled,
      HyperlinkAuditingEnabled,
      CSSGridLayoutEnabled,
      ScrollAnimatorEnabled,
      CaretBrowsingEnabled,
      NotificationsEnabled
    };
    explicit QWebSettings( QObject *parent = nullptr )
      : QObject( parent )
    {
    }

    void setUserStyleSheetUrl( const QUrl & )
    {
    }

    void setAttribute( WebAttribute, bool )
    {
    }
/// @endcond
};

/**
 * \ingroup core
 * \brief A collection of stubs to mimic the API of a QWebPage on systems
 * where QtWebkit is not available.
 */
class CORE_EXPORT QWebPage : public QObject
{
/// @cond NOT_STABLE_API
    Q_OBJECT

  public:

    enum LinkDelegationPolicy
    {
      DontDelegateLinks,
      DelegateExternalLinks,
      DelegateAllLinks
    };

    enum WebWindowType
    {
      WebBrowserWindow,
      WebModalDialog
    };

    explicit QWebPage( QObject *parent = nullptr )
      : QObject( parent )
      , mSettings( new QWebSettings() )
      , mFrame( new QWebFrame() )
    {
      connect( mFrame, &QWebFrame::loadFinished, this, &QWebPage::loadFinished );
    }

    ~QWebPage()
    {
      delete mFrame;
      delete mSettings;
    }

    QPalette palette() const
    {
      return QPalette();
    }

    void setPalette( const QPalette &palette )
    {
      Q_UNUSED( palette )
    }

    void setViewportSize( const QSize &size ) const
    {
      Q_UNUSED( size )
    }

    void setLinkDelegationPolicy( LinkDelegationPolicy linkDelegationPolicy )
    {
      if ( !parent() )
        return;

      QTextBrowser *tb = qobject_cast<QTextBrowser *>( parent() );
      if ( !tb )
        return;

      tb->setOpenExternalLinks( linkDelegationPolicy != DontDelegateLinks );
    }

    void setNetworkAccessManager( QNetworkAccessManager *networkAccessManager )
    {
      Q_UNUSED( networkAccessManager )
    }

    QWebFrame *mainFrame() const
    {
      return mFrame;
    }

    QWebSettings *settings() const
    {
      return mSettings;
    }

    QSize viewportSize() const
    {
      return QSize();
    }

    QMenu *createStandardContextMenu()
    {
      return new QMenu();
    }

  signals:

    void loadFinished( bool ok );

    void downloadRequested( const QNetworkRequest &request );

    void unsupportedContent( QNetworkReply *reply );

  public slots:

  protected:

    virtual void javaScriptConsoleMessage( const QString &, int, const QString & ) {}

  private:
    QWebSettings *mSettings = nullptr;
    QWebFrame *mFrame = nullptr;
/// @endcond
};
#endif

/**
 * \ingroup core
 * \class QgsWebPage
 * \brief QWebPage subclass which redirects JavaScript errors and console output to the QGIS message log.
 * \note Not available in Python bindings
 */
class CORE_EXPORT QgsWebPage : public QWebPage
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsWebPage.
     * \param parent parent object
     */
    explicit QgsWebPage( QObject *parent = nullptr )
      : QWebPage( parent )
    {}

    /**
     * Sets an identifier for the QgsWebPage. The page's identifier is included in messages written to the
     * log, and should be set to a user-friendly string so that users can identify which QgsWebPage has
     * logged the message.
     * \param identifier identifier string
     * \see identifier()
     */
    void setIdentifier( const QString &identifier ) { mIdentifier = identifier; }

    /**
     * Returns the QgsWebPage's identifier. The page's identifier is included in messages written to the
     * log so that users can identify which QgsWebPage has logged the message.
     * \see setIdentifier()
     */
    QString identifier() const { return mIdentifier; }

  protected:

    void javaScriptConsoleMessage( const QString &message, int lineNumber, const QString & ) override
    {
      if ( mIdentifier.isEmpty() )
        QgsMessageLog::logMessage( tr( "Line %1: %2" ).arg( lineNumber ).arg( message ), tr( "JavaScript" ) );
      else
        QgsMessageLog::logMessage( tr( "%1 (line %2): %3" ).arg( mIdentifier ).arg( lineNumber ).arg( message ), tr( "JavaScript" ) );
    }

  private:

    QString mIdentifier;

};

#endif // QGSWEBPAGE_H

