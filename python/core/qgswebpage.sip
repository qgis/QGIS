/***************************************************************************
  qgswebpage.sip - QgsWebPage
 ---------------------
 begin                : May 2016
 copyright            : (C) 2016 by Jürgen Fischer
 email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

%If (!WebKit)
class QWebSettings : QObject
{
%TypeHeaderCode
#include "qgswebpage.h"
%End
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
    explicit QWebSettings( QObject* parent = 0 );

    void setUserStyleSheetUrl( const QUrl& );

    void setAttribute( WebAttribute, bool on );
};

class QWebPage : QObject
{
%TypeHeaderCode
#include "qgswebpage.h"
%End
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

    explicit QWebPage( QObject* parent = 0 );

    ~QWebPage();

    QPalette palette() const;

    void setPalette( const QPalette& palette );

    void setViewportSize( const QSize & size ) const;

    void setLinkDelegationPolicy( LinkDelegationPolicy linkDelegationPolicy );

    void setNetworkAccessManager( QNetworkAccessManager* networkAccessManager );

    // QWebFrame* mainFrame() const;

    QWebSettings* settings() const;

    QSize viewportSize() const;

    QMenu *createStandardContextMenu() /Factory/;

   protected:
     virtual void javaScriptConsoleMessage( const QString& , int, const QString& );
};
%End

class QgsWebPage : QWebPage
{
%TypeHeaderCode
#include "qgswebpage.h"
%End
  public:

    /** Constructor for QgsWebPage.
     * @param parent parent object
     */
    QgsWebPage( QObject* parent = 0 );

    /** Sets an identifier for the QgsWebPage. The page's identifier is included in messages written to the
     * log, and should be set to a user-friendly string so that users can identify which QgsWebPage has
     * logged the message.
     * @param identifier identifier string
     * @see identifier()
     */
    void setIdentifier( const QString& identifier );

    /** Returns the QgsWebPage's identifier. The page's identifier is included in messages written to the
     * log so that users can identify which QgsWebPage has logged the message.
     * @see setIdentifier()
     */
    QString identifier() const;

  protected:
    virtual void javaScriptConsoleMessage( const QString& message, int lineNumber, const QString& );
};
