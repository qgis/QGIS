/***************************************************************************
                     qgsidentifyresults.cpp  -  description
                              -------------------
      begin                : Fri Oct 25 2002
      copyright            : (C) 2002 by Gary E.Sherman
      email                : sherman at mrcc dot com
      Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsactionmanager.h"
#include "qgsattributedialog.h"
#include "qgsdockwidget.h"
#include "qgseditorwidgetregistry.h"
#include "qgsfeatureaction.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"
#include "qgshighlight.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgsidentifyresultsdialog.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayeractionregistry.h"
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgswebview.h"
#include "qgswebframe.h"
#include "qgsstringutils.h"
#include "qgstreewidgetitem.h"
#include "qgsfiledownloaderdialog.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QCloseEvent>
#include <QLabel>
#include <QAction>
#include <QTreeWidgetItem>
#include <QPixmap>
#include <QMenu>
#include <QClipboard>
#include <QMenuBar>
#include <QPushButton>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QComboBox>
#include <QTextDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegExp>

//graph
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include "qgscolorramp.h" // for random colors

QgsIdentifyResultsWebView::QgsIdentifyResultsWebView( QWidget *parent ) : QgsWebView( parent )
{
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
#ifdef WITH_QTWEBKIT
  page()->setForwardUnsupportedContent( true );
#endif
  page()->setLinkDelegationPolicy( QWebPage::DontDelegateLinks );
  settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
  settings()->setAttribute( QWebSettings::PluginsEnabled, true );
#ifdef QGISDEBUG
  settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif
  connect( page(), &QWebPage::downloadRequested, this, &QgsIdentifyResultsWebView::downloadRequested );
  connect( page(), &QWebPage::unsupportedContent, this, &QgsIdentifyResultsWebView::unsupportedContent );
}


void QgsIdentifyResultsWebView::downloadRequested( const QNetworkRequest &request )
{
  handleDownload( request.url() );
}

void QgsIdentifyResultsWebView::unsupportedContent( QNetworkReply *reply )
{
  handleDownload( reply->url() );
}

void QgsIdentifyResultsWebView::handleDownload( QUrl url )
{
  if ( ! url.isValid() )
  {
    QMessageBox::warning( this, tr( "Invalid URL" ), tr( "The download URL is not valid: %1" ).arg( url.toString() ) );
  }
  else
  {
    const QString DOWNLOADER_LAST_DIR_KEY( QStringLiteral( "Qgis/fileDownloaderLastDir" ) );
    QgsSettings settings;
    // Try to get some information from the URL
    QFileInfo info( url.toString() );
    QString savePath = settings.value( DOWNLOADER_LAST_DIR_KEY ).toString();
    QString fileName = info.fileName().replace( QRegExp( "[^A-z0-9\\-_\\.]" ), QStringLiteral( "_" ) );
    if ( ! savePath.isEmpty() && ! fileName.isEmpty() )
    {
      savePath = QDir::cleanPath( savePath + QDir::separator() + fileName );
    }
    QString targetFile = QFileDialog::getSaveFileName( this,
                         tr( "Save As" ),
                         savePath,
                         info.suffix().isEmpty() ? QString() : "*." +  info.suffix()
                                                     );
    if ( ! targetFile.isEmpty() )
    {
      settings.setValue( DOWNLOADER_LAST_DIR_KEY, QFileInfo( targetFile ).dir().absolutePath() );
      // Start the download
      new QgsFileDownloaderDialog( url, targetFile );
    }
  }
}

void QgsIdentifyResultsWebView::print()
{
  QPrinter printer;
  QPrintDialog *dialog = new QPrintDialog( &printer );
  if ( dialog->exec() == QDialog::Accepted )
  {
    QgsWebView::print( &printer );
  }
}

void QgsIdentifyResultsWebView::contextMenuEvent( QContextMenuEvent *e )
{
  QMenu *menu = page()->createStandardContextMenu();
  if ( !menu )
    return;

  QAction *action = new QAction( tr( "Print" ), this );
  connect( action, &QAction::triggered, this, &QgsIdentifyResultsWebView::print );
  menu->addAction( action );
  menu->exec( e->globalPos() );
  delete menu;
}

QgsWebView *QgsIdentifyResultsWebView::createWindow( QWebPage::WebWindowType type )
{
  QDialog *d = new QDialog( this );
  QLayout *l = new QVBoxLayout( d );

  QgsWebView *wv = new QgsWebView( d );
  l->addWidget( wv );

  wv->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  wv->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
  wv->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
  wv->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
  wv->settings()->setAttribute( QWebSettings::PluginsEnabled, true );
#ifdef QGISDEBUG
  wv->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif

  d->setModal( type != QWebPage::WebBrowserWindow );
  d->show();

  return wv;
}

// QgsIdentifyResultsWebView size:
// ---------------
//
// 1) QTreeWidget is not able to scroll continuously through the content of large widgets
//    inserted into items via setItemWidget, it always jumps to the top of next
//    item and it is not able to scroll to the bottom of an inserted large
//    widget (until the QTreeWidget itself is large enough to show the whole
//    inserted widget).  => We have to keep the height of QgsIdentifyResultsWebView smaller
//    than the height of QTreeWidget so that a user can see it entire, even if
//    this height is smaller than QgsIdentifyResultsWebView content (i.e. QgsIdentifyResultsWebView scroolbar
//    is added). We make it even a bit smaller so that a user can see a bit of
//    context (items above/below) when scrolling which is more pleasant.
//
// 2) contentsSize() is 0,0 until a page is loaded. If there are no external
//    resources (like images) used, contentsSize() is available immediately
//    after setHtml(), otherwise the contentSize() is 0,0 until the page is
//    loaded and contentsSizeChanged () is emitted.
//
// 3) If QgsIdentifyResultsWebView is resized (on page load) after it was inserted into
//    QTreeWidget, the row does not reflect that change automatically and
//    consecutive resize of QTreeWidget will cause to shrink QgsIdentifyResultsWebView to the
//    original row height.  That is expected, Qt: "setItemWidget() should only
//    be used to display static content...  => we must not change QgsIdentifyResultsWebView
//    size after it was inserted to QTreeWidget

// TODO(?): Sometimes it may happen that if multiple QgsIdentifyResultsWebView are inserted to
// QTreeWidget for the first time, and both share the same external source
// (image) the layout gets somehow confused - wrong positions, overlapped (Qt
// bug?) until next QTreeWidget resize.

// TODO(?): if the results dialog is resized to smaller height, existing
// QgsIdentifyResultsWebView are not (and must not be!) resized and scrolling becomes a bit
// unpleasant until next identify. AFAIK it could only be solved using
// QItemDelegate.

// size hint according to content
QSize QgsIdentifyResultsWebView::sizeHint() const
{
  QSize s = page()->mainFrame()->contentsSize();
  QgsDebugMsg( QStringLiteral( "content size: %1 x %2" ).arg( s.width() ).arg( s.height() ) );
  int height = s.height();

  // parent is qt_scrollarea_viewport
  // parent is not available the first time - before results dialog was shown
  QWidget *widget = qobject_cast<QWidget *>( parent() );
  if ( widget )
  {
    // It can probably happen that parent is available but it does not have yet
    // correct size, see #9377.
    int max = widget->size().height() * 0.9;
    QgsDebugMsg( QStringLiteral( "parent widget height = %1 max height = %2" ).arg( widget->size().height() ).arg( max ) );
    height = std::min( height, max );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "parent not available" ) );
  }

  // Always keep some minimum size, e.g. if page is not yet loaded
  // or parent has wrong size
  height = std::max( height, 100 );

  s = QSize( size().width(), height );
  QgsDebugMsg( QStringLiteral( "size: %1 x %2" ).arg( s.width() ).arg( s.height() ) );
  return s;
}

QgsIdentifyResultsFeatureItem::QgsIdentifyResultsFeatureItem( const QgsFields &fields, const QgsFeature &feature, const QgsCoordinateReferenceSystem &crs, const QStringList &strings )
  : QTreeWidgetItem( strings )
  , mFields( fields )
  , mFeature( feature )
  , mCrs( crs )
{
}

void QgsIdentifyResultsWebViewItem::setHtml( const QString &html )
{
  mWebView->setHtml( html );
}

void QgsIdentifyResultsWebViewItem::setContent( const QByteArray &data, const QString &mimeType, const QUrl &baseUrl )
{
  mWebView->setContent( data, mimeType, baseUrl );
}

QgsIdentifyResultsWebViewItem::QgsIdentifyResultsWebViewItem( QTreeWidget *treeWidget )
{
  mWebView = new QgsIdentifyResultsWebView( treeWidget );
  mWebView->hide();
  setText( 0, tr( "Loading…" ) );
  connect( mWebView->page(), &QWebPage::loadFinished, this, &QgsIdentifyResultsWebViewItem::loadFinished );
}

void QgsIdentifyResultsWebViewItem::loadFinished( bool ok )
{
  Q_UNUSED( ok );

  mWebView->show();
  treeWidget()->setItemWidget( this, 0, mWebView );

  // Span columns to save some space, must be after setItemWidget() to take effect.
  setFirstColumnSpanned( true );

  disconnect( mWebView->page(), &QWebPage::loadFinished, this, &QgsIdentifyResultsWebViewItem::loadFinished );

}

// Tree hierarchy
//
// layer [userrole: QgsMapLayer]
//   feature: displayfield|displayvalue [userrole: fid, index in feature list]
//     derived attributes (if any) [userrole: "derived"]
//       name value
//     actions (if any) [userrole: "actions"]
//       edit [userrole: "edit"]
//       action [userrole: "action", idx]
//       action [userrole: "map_layer_action", QgsMapLayerAction]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//     displayname [userroles: fieldIdx, original name] displayvalue [userrole: original value]
//   feature
//     derived attributes (if any)
//       name value
//     actions (if any)
//       action
//     name value

QgsIdentifyResultsDialog::QgsIdentifyResultsDialog( QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mCanvas( canvas )
{
  setupUi( this );
  connect( cmbIdentifyMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsIdentifyResultsDialog::cmbIdentifyMode_currentIndexChanged );
  connect( cmbViewMode, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsIdentifyResultsDialog::cmbViewMode_currentIndexChanged );
  connect( mExpandNewAction, &QAction::triggered, this, &QgsIdentifyResultsDialog::mExpandNewAction_triggered );
  connect( cbxAutoFeatureForm, &QCheckBox::toggled, this, &QgsIdentifyResultsDialog::cbxAutoFeatureForm_toggled );
  connect( mExpandAction, &QAction::triggered, this, &QgsIdentifyResultsDialog::mExpandAction_triggered );
  connect( mCollapseAction, &QAction::triggered, this, &QgsIdentifyResultsDialog::mCollapseAction_triggered );
  connect( mActionCopy, &QAction::triggered, this, &QgsIdentifyResultsDialog::mActionCopy_triggered );

  mOpenFormAction->setDisabled( true );

  QgsSettings mySettings;
  mDock = new QgsDockWidget( tr( "Identify Results" ), QgisApp::instance() );
  mDock->setObjectName( QStringLiteral( "IdentifyResultsDock" ) );
  mDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  mDock->setWidget( this );
  if ( !QgisApp::instance()->restoreDockWidget( mDock ) )
    QgisApp::instance()->addDockWidget( Qt::RightDockWidgetArea, mDock );
  else
    QgisApp::instance()->panelMenu()->addAction( mDock->toggleViewAction() );

  int size = mySettings.value( QStringLiteral( "/qgis/iconSize" ), 16 ).toInt();
  if ( size > 32 )
  {
    size -= 16;
  }
  else if ( size == 32 )
  {
    size = 24;
  }
  else
  {
    size = 16;
  }
  mIdentifyToolbar->setIconSize( QSize( size, size ) );

  mExpandNewAction->setChecked( mySettings.value( QStringLiteral( "Map/identifyExpand" ), false ).toBool() );
  mActionCopy->setEnabled( false );
  lstResults->setColumnCount( 2 );
  lstResults->sortByColumn( -1 );
  setColumnText( 0, tr( "Feature" ) );
  setColumnText( 1, tr( "Value" ) );

  int width = mySettings.value( QStringLiteral( "Windows/Identify/columnWidth" ), "0" ).toInt();
  if ( width > 0 )
  {
    lstResults->setColumnWidth( 0, width );
  }
  width = mySettings.value( QStringLiteral( "Windows/Identify/columnWidthTable" ), "0" ).toInt();
  if ( width > 0 )
  {
    tblResults->setColumnWidth( 0, width );
  }

  // retrieve mode before on_cmbIdentifyMode_currentIndexChanged resets it on addItem
  QgsMapToolIdentify::IdentifyMode identifyMode = mySettings.enumValue( QStringLiteral( "Map/identifyMode" ), QgsMapToolIdentify::ActiveLayer );

  cmbIdentifyMode->addItem( tr( "Current layer" ), QgsMapToolIdentify::ActiveLayer );
  cmbIdentifyMode->addItem( tr( "Top down, stop at first" ), QgsMapToolIdentify::TopDownStopAtFirst );
  cmbIdentifyMode->addItem( tr( "Top down" ), QgsMapToolIdentify::TopDownAll );
  cmbIdentifyMode->addItem( tr( "Layer selection" ), QgsMapToolIdentify::LayerSelection );
  cmbIdentifyMode->setCurrentIndex( cmbIdentifyMode->findData( identifyMode ) );
  cbxAutoFeatureForm->setChecked( mySettings.value( QStringLiteral( "Map/identifyAutoFeatureForm" ), false ).toBool() );

  // view modes
  cmbViewMode->addItem( tr( "Tree" ), 0 );
  cmbViewMode->addItem( tr( "Table" ), 0 );
  cmbViewMode->addItem( tr( "Graph" ), 0 );

  // graph
  mPlot->setVisible( false );
  mPlot->setAutoFillBackground( false );
  mPlot->setAutoDelete( true );
  mPlot->insertLegend( new QwtLegend(), QwtPlot::TopLegend );
  QSizePolicy sizePolicy = QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  sizePolicy.setHorizontalStretch( 0 );
  sizePolicy.setVerticalStretch( 0 );
  sizePolicy.setHeightForWidth( mPlot->sizePolicy().hasHeightForWidth() );
  mPlot->setSizePolicy( sizePolicy );
  mPlot->updateGeometry();

  connect( lstResults, &QTreeWidget::itemExpanded,
           this, &QgsIdentifyResultsDialog::itemExpanded );

  connect( lstResults, &QTreeWidget::currentItemChanged,
           this, &QgsIdentifyResultsDialog::handleCurrentItemChanged );

  connect( lstResults, &QTreeWidget::itemClicked,
           this, &QgsIdentifyResultsDialog::itemClicked );

  connect( mActionPrint, &QAction::triggered, this, &QgsIdentifyResultsDialog::printCurrentItem );
  connect( mOpenFormAction, &QAction::triggered, this, &QgsIdentifyResultsDialog::featureForm );
  connect( mClearResultsAction, &QAction::triggered, this, &QgsIdentifyResultsDialog::clear );
  connect( mHelpToolButton, &QAbstractButton::clicked, this, &QgsIdentifyResultsDialog::showHelp );

  initSelectionModes();
}

QgsIdentifyResultsDialog::~QgsIdentifyResultsDialog()
{
  clearHighlights();

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Identify/columnWidth" ), lstResults->columnWidth( 0 ) );

  delete mActionPopup;
  qDeleteAll( mPlotCurves );
  mPlotCurves.clear();
}

void QgsIdentifyResultsDialog::initSelectionModes()
{
  mSelectModeButton = new QToolButton( mIdentifyToolbar );
  mSelectModeButton->setPopupMode( QToolButton::MenuButtonPopup );
  QList<QAction *> selectActions;
  selectActions << mActionSelectFeatures << mActionSelectPolygon
                << mActionSelectFreehand << mActionSelectRadius;

  QActionGroup *group = new QActionGroup( this );
  group->addAction( mActionSelectFeatures );
  group->addAction( mActionSelectPolygon );
  group->addAction( mActionSelectFreehand );
  group->addAction( mActionSelectRadius );

  mSelectModeButton->addActions( selectActions );
  mSelectModeButton->setDefaultAction( mActionSelectFeatures );

  mIdentifyToolbar->addWidget( mSelectModeButton );

  connect( mActionSelectFeatures, &QAction::triggered, this, &QgsIdentifyResultsDialog::setSelectionMode );
  connect( mActionSelectPolygon, &QAction::triggered, this, &QgsIdentifyResultsDialog::setSelectionMode );
  connect( mActionSelectFreehand, &QAction::triggered, this, &QgsIdentifyResultsDialog::setSelectionMode );
  connect( mActionSelectRadius, &QAction::triggered, this, &QgsIdentifyResultsDialog::setSelectionMode );
}

QTreeWidgetItem *QgsIdentifyResultsDialog::layerItem( QObject *object )
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = lstResults->topLevelItem( i );

    if ( item->data( 0, Qt::UserRole ).value<QObject *>() == object )
      return item;
  }

  return nullptr;
}

void QgsIdentifyResultsDialog::addFeature( const QgsMapToolIdentify::IdentifyResult &result )
{
  switch ( result.mLayer->type() )
  {
    case QgsMapLayer::VectorLayer:
      addFeature( qobject_cast<QgsVectorLayer *>( result.mLayer ), result.mFeature, result.mDerivedAttributes );
      break;

    case QgsMapLayer::RasterLayer:
      addFeature( qobject_cast<QgsRasterLayer *>( result.mLayer ), result.mLabel, result.mAttributes, result.mDerivedAttributes, result.mFields, result.mFeature, result.mParams );
      break;

    case QgsMapLayer::MeshLayer:
      addFeature( qobject_cast<QgsMeshLayer *>( result.mLayer ), result.mLabel, result.mAttributes, result.mDerivedAttributes );
      break;

    case QgsMapLayer::PluginLayer:
      break;
  }
}

void QgsIdentifyResultsDialog::addFeature( QgsVectorLayer *vlayer, const QgsFeature &f, const QMap<QString, QString> &derivedAttributes )
{
  QTreeWidgetItem *layItem = layerItem( vlayer );

  if ( !layItem )
  {
    layItem = new QTreeWidgetItem( QStringList() << vlayer->name() );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( vlayer ) ) );
    lstResults->addTopLevelItem( layItem );

    connect( vlayer, &QObject::destroyed, this, &QgsIdentifyResultsDialog::layerDestroyed );
    connect( vlayer, &QgsMapLayer::crsChanged, this, &QgsIdentifyResultsDialog::layerDestroyed );
    connect( vlayer, &QgsVectorLayer::featureDeleted, this, &QgsIdentifyResultsDialog::featureDeleted );
    connect( vlayer, &QgsVectorLayer::attributeValueChanged,
             this, &QgsIdentifyResultsDialog::attributeValueChanged );
    connect( vlayer, &QgsVectorLayer::editingStarted, this, &QgsIdentifyResultsDialog::editingToggled );
    connect( vlayer, &QgsVectorLayer::editingStopped, this, &QgsIdentifyResultsDialog::editingToggled );
  }

  QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( vlayer->fields(), f, vlayer->crs() );
  featItem->setData( 0, Qt::UserRole, FID_TO_STRING( f.id() ) );
  featItem->setData( 0, Qt::UserRole + 1, mFeatures.size() );
  mFeatures << f;
  layItem->addChild( featItem );

  if ( derivedAttributes.size() >= 0 )
  {
    QgsTreeWidgetItem *derivedItem = new QgsTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    derivedItem->setAlwaysOnTopPriority( 0 );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); ++it )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  //get valid QgsMapLayerActions for this layer
  QList< QgsMapLayerAction * > registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer );
  QList<QgsAction> actions = vlayer->actions()->actions( QStringLiteral( "Feature" ) );

  if ( !vlayer->fields().isEmpty() || !actions.isEmpty() || !registeredActions.isEmpty() )
  {
    QgsTreeWidgetItem *actionItem = new QgsTreeWidgetItem( QStringList() << tr( "(Actions)" ) );
    actionItem->setData( 0, Qt::UserRole, "actions" );
    actionItem->setAlwaysOnTopPriority( 1 );
    featItem->addChild( actionItem );

    if ( vlayer->fields().size() > 0 )
    {
      QTreeWidgetItem *editItem = new QTreeWidgetItem( QStringList() << QString() << ( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) ) );
      editItem->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) ) );
      editItem->setData( 0, Qt::UserRole, "edit" );
      actionItem->addChild( editItem );
    }

    Q_FOREACH ( const QgsAction &action, actions )
    {
      if ( !action.runable() )
        continue;

      if ( action.isEnabledOnlyWhenEditable() )
        continue;

      QTreeWidgetItem *twi = new QTreeWidgetItem( QStringList() << QString() << action.name() );
      twi->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
      twi->setData( 0, Qt::UserRole, "action" );
      twi->setData( 0, Qt::UserRole + 1, action.id() );
      actionItem->addChild( twi );
    }

    //add actions from QgsMapLayerActionRegistry
    for ( int i = 0; i < registeredActions.size(); i++ )
    {
      if ( registeredActions.at( i )->isEnabledOnlyWhenEditable() )
        continue;

      QgsMapLayerAction *action = registeredActions.at( i );
      QTreeWidgetItem *twi = new QTreeWidgetItem( QStringList() << QString() << action->text() );
      twi->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ) );
      twi->setData( 0, Qt::UserRole, "map_layer_action" );
      twi->setData( 0, Qt::UserRole + 1, qVariantFromValue( qobject_cast<QObject *>( action ) ) );
      actionItem->addChild( twi );

      connect( action, &QObject::destroyed, this, &QgsIdentifyResultsDialog::mapLayerActionDestroyed );
    }
  }

  const QgsFields &fields = vlayer->fields();
  QgsAttributes attrs = f.attributes();
  bool featureLabeled = false;

  for ( int i = 0; i < attrs.count(); ++i )
  {
    if ( i >= fields.count() )
      break;

    const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( vlayer, fields[i].name() );
    if ( setup.type() == QLatin1String( "Hidden" ) )
    {
      continue;
    }

    QString defVal;
    if ( fields.fieldOrigin( i ) == QgsFields::OriginProvider && vlayer->dataProvider() )
      defVal = vlayer->dataProvider()->defaultValueClause( fields.fieldOriginIndex( i ) );

    QString value = defVal == attrs.at( i ) ? defVal : fields.at( i ).displayString( attrs.at( i ) );
    QgsTreeWidgetItem *attrItem = new QgsTreeWidgetItem( QStringList() << QString::number( i ) << value );
    featItem->addChild( attrItem );

    attrItem->setData( 0, Qt::DisplayRole, vlayer->attributeDisplayName( i ) );
    attrItem->setToolTip( 0, vlayer->attributeDisplayName( i ) );
    attrItem->setData( 0, Qt::UserRole, fields.at( i ).name() );
    attrItem->setData( 0, Qt::UserRole + 1, i );

    attrItem->setData( 1, Qt::UserRole, value );

    value = representValue( vlayer, setup, fields.at( i ).name(), attrs.at( i ) );
    attrItem->setSortData( 1, value );
    attrItem->setToolTip( 1, value );
    bool foundLinks = false;
    QString links = QgsStringUtils::insertLinks( value, &foundLinks );
    if ( foundLinks )
    {
      QLabel *valueLabel = new QLabel( links );
      valueLabel->setOpenExternalLinks( true );
      attrItem->setData( 1, Qt::DisplayRole, QString() );
      attrItem->treeWidget()->setItemWidget( attrItem, 1, valueLabel );
    }
    else
    {
      attrItem->setData( 1, Qt::DisplayRole, value );
      attrItem->treeWidget()->setItemWidget( attrItem, 1, nullptr );
    }

    if ( fields.at( i ).name() == vlayer->displayField() )
    {
      featItem->setText( 0, attrItem->text( 0 ) );
      featItem->setToolTip( 0, attrItem->text( 0 ) );
      featItem->setText( 1, attrItem->text( 1 ) );
      featItem->setToolTip( 1, attrItem->text( 1 ) );
      featureLabeled = true;
    }
  }

  if ( !featureLabeled )
  {
    featItem->setText( 0, tr( "Title" ) );
    QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vlayer ) );
    context.setFeature( f );

    QString value = QgsExpression( vlayer->displayExpression() ).evaluate( &context ).toString();
    featItem->setText( 1, value );
    featItem->setToolTip( 1, value );
  }

  // table
  int j = tblResults->rowCount();
  for ( int i = 0; i < attrs.count(); ++i )
  {
    if ( i >= fields.count() )
      continue;

    QString value = fields.at( i ).displayString( attrs.at( i ) );
    const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( vlayer, fields.at( i ).name() );
    QString value2 = representValue( vlayer, setup, fields.at( i ).name(), attrs.at( i ) );

    tblResults->setRowCount( j + 1 );

    QgsDebugMsgLevel( QStringLiteral( "adding item #%1 / %2 / %3 / %4" ).arg( j ).arg( vlayer->name(), vlayer->attributeDisplayName( i ), value2 ), 4 );

    QTableWidgetItem *item = new QTableWidgetItem( vlayer->name() );
    item->setData( Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( vlayer ) ) );
    item->setData( Qt::UserRole + 1, vlayer->id() );
    tblResults->setItem( j, 0, item );

    item = new QTableWidgetItem( FID_TO_STRING( f.id() ) );
    item->setData( Qt::UserRole, FID_TO_STRING( f.id() ) );
    item->setData( Qt::UserRole + 1, mFeatures.size() );
    tblResults->setItem( j, 1, item );

    item = new QTableWidgetItem( QString::number( i ) );
    if ( fields.at( i ).name() == vlayer->displayField() )
      item->setData( Qt::DisplayRole, vlayer->attributeDisplayName( i ) + " *" );
    else
      item->setData( Qt::DisplayRole, vlayer->attributeDisplayName( i ) );
    item->setData( Qt::UserRole, fields.at( i ).name() );
    item->setData( Qt::UserRole + 1, i );
    tblResults->setItem( j, 2, item );

    item = new QTableWidgetItem( value );
    item->setData( Qt::UserRole, value );
    item->setData( Qt::DisplayRole, value2 );
    tblResults->setItem( j, 3, item );

    // highlight first item
    // if ( i==0 )
    // {
    //   QBrush b = tblResults->palette().brush( QPalette::AlternateBase );
    //   for ( int k = 0; k <= 3; k++)
    //  tblResults->item( j, k )->setBackground( b );
    // }

    tblResults->resizeRowToContents( j );
    j++;
  }
  //tblResults->resizeColumnToContents( 1 );

  highlightFeature( featItem );
}

void QgsIdentifyResultsDialog::mapLayerActionDestroyed()
{
  QTreeWidgetItemIterator it( lstResults );
  while ( *it )
  {
    if ( ( *it )->data( 0, Qt::UserRole ) == "map_layer_action" &&
         ( *it )->data( 0, Qt::UserRole + 1 ).value< QObject *>() == sender() )
      delete *it;
    else
      ++it;
  }
}

QgsIdentifyPlotCurve::QgsIdentifyPlotCurve( const QMap<QString, QString> &attributes,
    QwtPlot *plot, const QString &title, QColor color )
{
  mPlotCurve = new QwtPlotCurve( title );

  if ( color == QColor() )
  {
    color = QgsLimitedRandomColorRamp::randomColors( 1 ).at( 0 );
  }
  mPlotCurve->setSymbol( new QwtSymbol( QwtSymbol::Ellipse, QBrush( Qt::white ),
                                        QPen( color, 2 ), QSize( 9, 9 ) ) );
  mPlotCurve->setPen( QPen( color, 2 ) ); // needed for legend

  QVector<QPointF> myData;
  int i = 1;

  for ( QMap<QString, QString>::const_iterator it = attributes.begin();
        it != attributes.end(); ++it )
  {
    myData << QPointF( double( i++ ), it.value().toDouble() );
  }
  mPlotCurve->setSamples( myData );

  mPlotCurve->attach( plot );

  plot->setAxisMaxMinor( QwtPlot::xBottom, 0 );
  //mPlot->setAxisScale( QwtPlot::xBottom, 1, mPlotCurve->dataSize());
  //mPlot->setAxisScale( QwtPlot::yLeft, ymin, ymax );

  plot->replot();
  plot->setVisible( true );
}

QgsIdentifyPlotCurve::~QgsIdentifyPlotCurve()
{
  if ( mPlotCurve )
  {
    mPlotCurve->detach();
    delete mPlotCurve;
  }
}

QString QgsIdentifyResultsDialog::representValue( QgsVectorLayer *vlayer, const QgsEditorWidgetSetup &setup, const QString &fieldName, const QVariant &value )
{
  QVariant cache;
  QMap<QString, QVariant> &layerCaches = mWidgetCaches[vlayer->id()];

  QgsEditorWidgetFactory *factory = QgsGui::editorWidgetRegistry()->factory( setup.type() );
  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

  int idx = vlayer->fields().lookupField( fieldName );

  if ( !factory )
    return value.toString();

  if ( layerCaches.contains( fieldName ) )
  {
    cache = layerCaches[ fieldName ];
  }
  else
  {
    cache = fieldFormatter->createCache( vlayer, idx, setup.config() );
    layerCaches.insert( fieldName, cache );
  }

  return fieldFormatter->representValue( vlayer, idx, setup.config(), cache, value );
}

void QgsIdentifyResultsDialog::addFeature( QgsRasterLayer *layer,
    const QString &label,
    const QMap<QString, QString> &attributes,
    const QMap<QString, QString> &derivedAttributes,
    const QgsFields &fields,
    const QgsFeature &feature,
    const QMap<QString, QVariant> &params )
{
  QgsDebugMsg( QStringLiteral( "feature.isValid() = %1" ).arg( feature.isValid() ) );
  QTreeWidgetItem *layItem = layerItem( layer );

  QgsRaster::IdentifyFormat currentFormat = QgsRasterDataProvider::identifyFormatFromName( layer->customProperty( QStringLiteral( "identify/format" ) ).toString() );

  if ( !layItem )
  {
    layItem = new QTreeWidgetItem( QStringList() << QString::number( lstResults->topLevelItemCount() ) << layer->name() );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( layer ) ) );

    lstResults->addTopLevelItem( layItem );

    QComboBox *formatCombo = new QComboBox();

    // Add all supported formats, best first. HTML is considered the best because
    // it usually holds most information.
    int capabilities = layer->dataProvider()->capabilities();
    QList<QgsRaster::IdentifyFormat> formats;
    formats << QgsRaster::IdentifyFormatHtml
            << QgsRaster::IdentifyFormatFeature
            << QgsRaster::IdentifyFormatText
            << QgsRaster::IdentifyFormatValue;
    Q_FOREACH ( QgsRaster::IdentifyFormat f, formats )
    {
      if ( !( QgsRasterDataProvider::identifyFormatToCapability( f ) & capabilities ) )
        continue;
      formatCombo->addItem( QgsRasterDataProvider::identifyFormatLabel( f ), f );
      formatCombo->setItemData( formatCombo->count() - 1, qVariantFromValue( qobject_cast<QObject *>( layer ) ), Qt::UserRole + 1 );
      if ( currentFormat == f )
        formatCombo->setCurrentIndex( formatCombo->count() - 1 );
    }

    if ( formatCombo->count() > 1 )
    {
      // Add format combo box item
      // Space added before format to keep it first in ordered list: TODO better (user data)
      QTreeWidgetItem *formatItem = new QTreeWidgetItem( QStringList() << ' ' + tr( "Format" ) );
      layItem->addChild( formatItem );
      lstResults->setItemWidget( formatItem, 1, formatCombo );
      connect( formatCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ),
               this, static_cast<void ( QgsIdentifyResultsDialog::* )( int )>( &QgsIdentifyResultsDialog::formatChanged ) );
    }
    else
    {
      delete formatCombo;
    }

    connect( layer, &QObject::destroyed, this, &QgsIdentifyResultsDialog::layerDestroyed );
    connect( layer, &QgsMapLayer::crsChanged, this, &QgsIdentifyResultsDialog::layerDestroyed );
  }
  // Set/reset getFeatureInfoUrl (currently only available for Feature, so it may change if format changes)
  layItem->setData( 0, GetFeatureInfoUrlRole, params.value( QStringLiteral( "getFeatureInfoUrl" ) ) );

  QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( fields, feature, layer->crs(), QStringList() << label << QString() );
  layItem->addChild( featItem );

  // add feature attributes
  if ( feature.isValid() )
  {
    QgsDebugMsg( QStringLiteral( "fields size = %1 attributes size = %2" ).arg( fields.size() ).arg( feature.attributes().size() ) );
    QgsAttributes attrs = feature.attributes();
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( i >= fields.count() )
        continue;

      QTreeWidgetItem *attrItem = new QTreeWidgetItem( QStringList() << QString::number( i ) << attrs.at( i ).toString() );

      attrItem->setData( 0, Qt::DisplayRole, fields.at( i ).name() );

      QVariant value = attrs.at( i );
      attrItem->setData( 1, Qt::DisplayRole, value );
      featItem->addChild( attrItem );
    }
  }

  if ( currentFormat == QgsRaster::IdentifyFormatHtml || currentFormat == QgsRaster::IdentifyFormatText )
  {
    QgsIdentifyResultsWebViewItem *attrItem = new QgsIdentifyResultsWebViewItem( lstResults );
    featItem->addChild( attrItem ); // before setHtml()!
    if ( !attributes.isEmpty() )
    {
      attrItem->setContent( attributes.begin().value().toUtf8(), currentFormat == QgsRaster::IdentifyFormatHtml ? "text/html" : "text/plain; charset=utf-8" );
    }
    else
    {
      attrItem->setContent( tr( "No attributes." ).toUtf8(), QStringLiteral( "text/plain; charset=utf-8" ) );
    }
  }
  else
  {
    for ( QMap<QString, QString>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
    {
      featItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  if ( derivedAttributes.size() >= 0 )
  {
    QgsTreeWidgetItem *derivedItem = new QgsTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    derivedItem->setAlwaysOnTopPriority( 0 );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); ++it )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }

  // table
  int i = 0;
  int j = tblResults->rowCount();
  tblResults->setRowCount( j + attributes.count() );

  for ( QMap<QString, QString>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QgsDebugMsg( QStringLiteral( "adding item #%1 / %2 / %3 / %4" ).arg( j ).arg( layer->name(), it.key(), it.value() ) );
    QTableWidgetItem *item = new QTableWidgetItem( layer->name() );
    item->setData( Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( layer ) ) );
    item->setData( Qt::UserRole + 1, layer->id() );
    tblResults->setItem( j, 0, item );
    tblResults->setItem( j, 1, new QTableWidgetItem( QString::number( i + 1 ) ) );
    tblResults->setItem( j, 2, new QTableWidgetItem( it.key() ) );
    tblResults->setItem( j, 3, new QTableWidgetItem( it.value() ) );

    tblResults->resizeRowToContents( j );

    j++;
    i++;
  }
  //tblResults->resizeColumnToContents( 1 );

  // graph
  if ( !attributes.isEmpty() )
  {
    mPlotCurves.append( new QgsIdentifyPlotCurve( attributes, mPlot, layer->name() ) );
  }
}

void QgsIdentifyResultsDialog::addFeature( QgsMeshLayer *layer,
    const QString &label,
    const QMap< QString, QString > &attributes,
    const QMap< QString, QString > &derivedAttributes )
{
  QTreeWidgetItem *layItem = layerItem( layer );

  if ( !layItem )
  {
    layItem = new QTreeWidgetItem( QStringList() << QString::number( lstResults->topLevelItemCount() ) << layer->name() );
    layItem->setData( 0, Qt::UserRole, QVariant::fromValue( qobject_cast<QObject *>( layer ) ) );

    lstResults->addTopLevelItem( layItem );
    connect( layer, &QObject::destroyed, this, &QgsIdentifyResultsDialog::layerDestroyed );
    connect( layer, &QgsMapLayer::crsChanged, this, &QgsIdentifyResultsDialog::layerDestroyed );
  }

  QgsIdentifyResultsFeatureItem *featItem = new QgsIdentifyResultsFeatureItem( QgsFields(),
      QgsFeature(),
      layer->crs(),
      QStringList() << label << QString() );
  layItem->addChild( featItem );

  // attributes
  for ( QMap<QString, QString>::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    featItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
  }

  if ( derivedAttributes.size() >= 0 )
  {
    QgsTreeWidgetItem *derivedItem = new QgsTreeWidgetItem( QStringList() << tr( "(Derived)" ) );
    derivedItem->setData( 0, Qt::UserRole, "derived" );
    derivedItem->setAlwaysOnTopPriority( 0 );
    featItem->addChild( derivedItem );

    for ( QMap< QString, QString>::const_iterator it = derivedAttributes.begin(); it != derivedAttributes.end(); ++it )
    {
      derivedItem->addChild( new QTreeWidgetItem( QStringList() << it.key() << it.value() ) );
    }
  }
}

void QgsIdentifyResultsDialog::editingToggled()
{
  QTreeWidgetItem *layItem = layerItem( sender() );
  QgsVectorLayer *vlayer = vectorLayer( layItem );
  if ( !layItem || !vlayer )
    return;

  // iterate features
  int i;
  for ( i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    int j;
    for ( j = 0; j < featItem->childCount() && featItem->child( j )->data( 0, Qt::UserRole ).toString() != QLatin1String( "actions" ); j++ )
      QgsDebugMsg( QStringLiteral( "%1: skipped %2" ).arg( featItem->child( j )->data( 0, Qt::UserRole ).toString() ) );

    if ( j == featItem->childCount() || featItem->child( j )->childCount() < 1 )
      continue;

    QTreeWidgetItem *actions = featItem->child( j );

    for ( j = 0; i < actions->childCount() && actions->child( j )->data( 0, Qt::UserRole ).toString() != QLatin1String( "edit" ); j++ )
      ;

    if ( j == actions->childCount() )
      continue;

    QTreeWidgetItem *editItem = actions->child( j );
    mOpenFormAction->setToolTip( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) );
    editItem->setText( 1, mOpenFormAction->toolTip() );
  }
}

// Call to show the dialog box.
void QgsIdentifyResultsDialog::show()
{
  bool showFeatureForm = false;

  if ( lstResults->topLevelItemCount() > 0 )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( 0 );
    QTreeWidgetItem *featItem = layItem->child( 0 );

    if ( lstResults->topLevelItemCount() == 1 && layItem->childCount() == 1 )
    {
      lstResults->setCurrentItem( featItem );

      if ( QgsSettings().value( QStringLiteral( "/Map/identifyAutoFeatureForm" ), false ).toBool() )
      {
        QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( layItem->data( 0, Qt::UserRole ).value<QObject *>() );
        if ( layer )
        {
          // if this is the only feature and it's on a vector layer
          // show the form dialog instead of the results window
          showFeatureForm = true;
        }
      }
    }

    // expand first layer and feature
    featItem->setExpanded( true );
    layItem->setExpanded( true );
  }

  // expand all if enabled
  if ( mExpandNewAction->isChecked() )
  {
    lstResults->expandAll();
  }

  QDialog::show();

  // when the feature form is opened don't show and raise the identify result.
  // If it's not docked, the results would open after or possibly on top of the
  // feature form and stay open (on top the canvas) after the feature form is
  // closed.
  if ( showFeatureForm )
  {
    featureForm();
  }
  else
  {
    mDock->show();
    mDock->raise();
  }
}

void QgsIdentifyResultsDialog::itemClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column );
  if ( item->data( 0, Qt::UserRole ).toString() == QLatin1String( "edit" ) )
  {
    lstResults->setCurrentItem( item );
    featureForm();
  }
  else if ( item->data( 0, Qt::UserRole ).toString() == QLatin1String( "action" ) )
  {
    doAction( item, item->data( 0, Qt::UserRole + 1 ).toString() );
  }
  else if ( item->data( 0, Qt::UserRole ).toString() == QLatin1String( "map_layer_action" ) )
  {
    QgsMapLayerAction *action = item->data( 0, Qt::UserRole + 1 ).value<QgsMapLayerAction *>();
    doMapLayerAction( item, action );
  }
}

// Popup (create if necessary) a context menu that contains a list of
// actions that can be applied to the data in the identify results
// dialog box.

void QgsIdentifyResultsDialog::contextMenuEvent( QContextMenuEvent *event )
{

  // only handle context menu event if showing tree widget
  if ( stackedWidget->currentIndex() != 0 )
    return;

  QTreeWidgetItem *item = lstResults->itemAt( lstResults->viewport()->mapFrom( this, event->pos() ) );
  // if the user clicked below the end of the attribute list, just return
  if ( !item )
    return;

  QgsMapLayer *layer = vectorLayer( item );
  QgsVectorLayer *vlayer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );
  if ( !vlayer && !rlayer )
  {
    QgsDebugMsg( QStringLiteral( "Item does not belong to a layer." ) );
    return;
  }

  delete mActionPopup;
  mActionPopup = new QMenu();

  int idx = -1;
  // QTreeWidgetItem *featItem = featureItem( item );
  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( featItem )
  {
    if ( vlayer )
    {
      mActionPopup->addAction(
        QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) ),
        vlayer->isEditable() ? tr( "Edit Feature Form…" ) : tr( "View Feature Form…" ),
        this, &QgsIdentifyResultsDialog::featureForm );
    }

    if ( featItem->feature().isValid() )
    {
      mActionPopup->addAction( tr( "Zoom to Feature" ), this, &QgsIdentifyResultsDialog::zoomToFeature );
      mActionPopup->addAction( tr( "Copy Feature" ), this, &QgsIdentifyResultsDialog::copyFeature );
      mActionPopup->addAction( tr( "Toggle Feature Selection" ), this, &QgsIdentifyResultsDialog::toggleFeatureSelection );
    }

    mActionPopup->addAction( tr( "Copy Attribute Value" ), this, &QgsIdentifyResultsDialog::copyAttributeValue );
    mActionPopup->addAction( tr( "Copy Feature Attributes" ), this, &QgsIdentifyResultsDialog::copyFeatureAttributes );

    if ( item->parent() == featItem && item->childCount() == 0 )
    {
      idx = item->data( 0, Qt::UserRole + 1 ).toInt();
    }
  }

  if ( rlayer )
  {
    QTreeWidgetItem *layItem = layerItem( item );
    if ( layItem && !layItem->data( 0, GetFeatureInfoUrlRole ).toString().isEmpty() )
    {
      mActionPopup->addAction( tr( "Copy GetFeatureInfo request URL" ), this, &QgsIdentifyResultsDialog::copyGetFeatureInfoUrl );
    }
  }
  if ( !mActionPopup->children().isEmpty() )
  {
    mActionPopup->addSeparator();
  }

  mActionPopup->addAction( tr( "Clear Results" ), this, &QgsIdentifyResultsDialog::clear );
  mActionPopup->addAction( tr( "Clear Highlights" ), this, &QgsIdentifyResultsDialog::clearHighlights );
  mActionPopup->addAction( tr( "Highlight All" ), this, &QgsIdentifyResultsDialog::highlightAll );
  mActionPopup->addAction( tr( "Highlight Layer" ), this, [ = ] { highlightLayer(); } );
  if ( layer && QgsProject::instance()->layerIsEmbedded( layer->id() ).isEmpty() )
  {
    mActionPopup->addAction( tr( "Activate Layer" ), this, [ = ] { activateLayer(); } );
    mActionPopup->addAction( tr( "Layer Properties…" ), this, [ = ] { layerProperties(); } );
  }
  mActionPopup->addSeparator();
  mActionPopup->addAction( tr( "Expand All" ), this, &QgsIdentifyResultsDialog::expandAll );
  mActionPopup->addAction( tr( "Collapse All" ), this, &QgsIdentifyResultsDialog::collapseAll );
  mActionPopup->addSeparator();

  if ( featItem && vlayer )
  {
    QList<QgsAction> actions = vlayer->actions()->actions( QStringLiteral( "Field" ) );
    if ( !actions.isEmpty() )
    {
      mActionPopup->addSeparator();

      int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();

      Q_FOREACH ( const QgsAction &action, actions )
      {
        if ( !action.runable() )
          continue;

        if ( action.isEnabledOnlyWhenEditable() )
          continue;

        QgsFeatureAction *a = new QgsFeatureAction( action.name(), mFeatures[ featIdx ], vlayer, action.id(), idx, this );
        mActionPopup->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ), action.name(), a, &QgsFeatureAction::execute );
      }
    }
  }

  if ( featItem && vlayer )
  {
    //get valid QgsMapLayerActions for this layer
    QList< QgsMapLayerAction * > registeredActions = QgsGui::mapLayerActionRegistry()->mapLayerActions( vlayer );

    if ( !registeredActions.isEmpty() )
    {
      //add a separator between user defined and standard actions
      mActionPopup->addSeparator();

      int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();

      QList<QgsMapLayerAction *>::const_iterator actionIt;
      for ( actionIt = registeredActions.begin(); actionIt != registeredActions.end(); ++actionIt )
      {
        if ( ( *actionIt )->isEnabledOnlyWhenEditable() )
          continue;

        QgsIdentifyResultsDialogMapLayerAction *a = new QgsIdentifyResultsDialogMapLayerAction( ( *actionIt )->text(), this, ( *actionIt ), vlayer, &( mFeatures[ featIdx ] ) );
        mActionPopup->addAction( QgsApplication::getThemeIcon( QStringLiteral( "/mAction.svg" ) ), ( *actionIt )->text(), a, &QgsIdentifyResultsDialogMapLayerAction::execute );
      }
    }
  }

  mActionPopup->popup( event->globalPos() );
}

// Save the current window location (store in ~/.qt/qgisrc)
void QgsIdentifyResultsDialog::saveWindowLocation()
{
  QgsSettings settings;
  // first column width
  settings.setValue( QStringLiteral( "Windows/Identify/columnWidth" ), lstResults->columnWidth( 0 ) );
  settings.setValue( QStringLiteral( "Windows/Identify/columnWidthTable" ), tblResults->columnWidth( 0 ) );
}

void QgsIdentifyResultsDialog::setColumnText( int column, const QString &label )
{
  QTreeWidgetItem *header = lstResults->headerItem();
  header->setText( column, label );
}

void QgsIdentifyResultsDialog::expandColumnsToFit()
{
  lstResults->resizeColumnToContents( 0 );
  lstResults->resizeColumnToContents( 1 );
}

void QgsIdentifyResultsDialog::clear()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    disconnectLayer( lstResults->topLevelItem( i )->data( 0, Qt::UserRole ).value<QObject *>() );
  }

  lstResults->clear();
  lstResults->sortByColumn( -1 );
  clearHighlights();

  tblResults->clearContents();
  tblResults->setRowCount( 0 );

  mPlot->setVisible( false );
  Q_FOREACH ( QgsIdentifyPlotCurve *curve, mPlotCurves )
    delete curve;
  mPlotCurves.clear();

  mExpressionContextScope = QgsExpressionContextScope();

  // keep it visible but disabled, it can switch from disabled/enabled
  // after raster format change
  mActionPrint->setDisabled( true );
}

void QgsIdentifyResultsDialog::updateViewModes()
{
  // get # of identified vector and raster layers - there must be a better way involving caching
  int vectorCount = 0, rasterCount = 0;
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *item = lstResults->topLevelItem( i );
    if ( vectorLayer( item ) ) vectorCount++;
    else if ( rasterLayer( item ) ) rasterCount++;
  }

  lblViewMode->setEnabled( rasterCount > 0 );
  cmbViewMode->setEnabled( rasterCount > 0 );
  if ( rasterCount == 0 )
    cmbViewMode->setCurrentIndex( 0 );

}

void QgsIdentifyResultsDialog::clearHighlights()
{
  Q_FOREACH ( QgsHighlight *h, mHighlights )
  {
    delete h;
  }

  mHighlights.clear();
}

void QgsIdentifyResultsDialog::activate()
{
  Q_FOREACH ( QgsHighlight *h, mHighlights )
  {
    h->show();
  }

  if ( lstResults->topLevelItemCount() > 0 )
  {
    raise();
  }
}

void QgsIdentifyResultsDialog::deactivate()
{
  Q_FOREACH ( QgsHighlight *h, mHighlights )
  {
    h->hide();
  }
}

void QgsIdentifyResultsDialog::doAction( QTreeWidgetItem *item, const QString &action )
{
  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( featItem->parent()->data( 0, Qt::UserRole ).value<QObject *>() );
  if ( !layer )
    return;

  int idx = -1;
  if ( item->parent() == featItem )
  {
    QString fieldName = item->data( 0, Qt::DisplayRole ).toString();

    const QgsFields &fields = layer->fields();
    for ( int fldIdx = 0; fldIdx < fields.count(); ++fldIdx )
    {
      if ( fields.at( fldIdx ).name() == fieldName )
      {
        idx = fldIdx;
        break;
      }
    }
  }

  int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();
  layer->actions()->doAction( action, mFeatures[ featIdx ], idx, mExpressionContextScope );
}

void QgsIdentifyResultsDialog::doMapLayerAction( QTreeWidgetItem *item, QgsMapLayerAction *action )
{
  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( featItem->parent()->data( 0, Qt::UserRole ).value<QObject *>() );
  if ( !layer )
    return;

  if ( !action )
    return;

  int featIdx = featItem->data( 0, Qt::UserRole + 1 ).toInt();
  action->triggerForFeature( layer, &mFeatures[ featIdx ] );
}

QTreeWidgetItem *QgsIdentifyResultsDialog::featureItem( QTreeWidgetItem *item )
{
  if ( !item )
    return nullptr;

  QTreeWidgetItem *featItem = nullptr;
  if ( item->parent() )
  {
    if ( item->parent()->parent() )
    {
      if ( item->parent()->parent()->parent() )
      {
        // derived or action attribute item
        featItem = item->parent()->parent();
      }
      else
      {
        // attribute item
        featItem = item->parent();
      }
    }
    else
    {
      // feature item
      featItem = item;
    }
  }
  else
  {
    // top level layer item, return feature item if only one
#if 0
    if ( item->childCount() > 1 )
      return 0;
    featItem = item->child( 0 );
#endif

    int count = 0;

    for ( int i = 0; i < item->childCount(); i++ )
    {
      QgsIdentifyResultsFeatureItem *fi = dynamic_cast<QgsIdentifyResultsFeatureItem *>( item->child( i ) );
      if ( fi )
      {
        count++;
        if ( !featItem )
          featItem = fi;
      }
    }

    if ( count != 1 )
      return nullptr;
  }

  return featItem;
}

QTreeWidgetItem *QgsIdentifyResultsDialog::layerItem( QTreeWidgetItem *item )
{
  if ( item && item->parent() )
  {
    item = featureItem( item )->parent();
  }

  return item;
}

QgsMapLayer *QgsIdentifyResultsDialog::layer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return nullptr;
  return qobject_cast<QgsMapLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QgsVectorLayer *QgsIdentifyResultsDialog::vectorLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return nullptr;
  return qobject_cast<QgsVectorLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QgsRasterLayer *QgsIdentifyResultsDialog::rasterLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return nullptr;
  return qobject_cast<QgsRasterLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QgsMeshLayer *QgsIdentifyResultsDialog::meshLayer( QTreeWidgetItem *item )
{
  item = layerItem( item );
  if ( !item )
    return nullptr;
  return qobject_cast<QgsMeshLayer *>( item->data( 0, Qt::UserRole ).value<QObject *>() );
}

QTreeWidgetItem *QgsIdentifyResultsDialog::retrieveAttributes( QTreeWidgetItem *item, QgsAttributeMap &attributes, int &idx )
{
  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return nullptr;

  idx = -1;

  attributes.clear();
  for ( int i = 0; i < featItem->childCount(); i++ )
  {
    QTreeWidgetItem *item = featItem->child( i );
    if ( item->childCount() > 0 )
      continue;
    if ( item == lstResults->currentItem() )
      idx = item->data( 0, Qt::UserRole + 1 ).toInt();
    attributes.insert( item->data( 0, Qt::UserRole + 1 ).toInt(), item->data( 1, Qt::DisplayRole ) );
  }

  return featItem;
}

void QgsIdentifyResultsDialog::itemExpanded( QTreeWidgetItem *item )
{
  Q_UNUSED( item );
  // column width is now stored in settings
  //expandColumnsToFit();
}

void QgsIdentifyResultsDialog::handleCurrentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  Q_UNUSED( previous );

  mActionPrint->setEnabled( false );

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( current ) );
  mActionCopy->setEnabled( featItem && featItem->feature().isValid() );
  mOpenFormAction->setEnabled( featItem && featItem->feature().isValid() );

  QgsVectorLayer *vlayer = vectorLayer( current );
  if ( vlayer )
  {
    mOpenFormAction->setToolTip( vlayer->isEditable() ? tr( "Edit feature form" ) : tr( "View feature form" ) );
  }

  if ( !current )
  {
    emit selectedFeatureChanged( nullptr, 0 );
    return;
  }

  // An item may be printed if a child is QgsIdentifyResultsWebViewItem
  for ( int i = 0; i < current->childCount(); i++ )
  {
    QgsIdentifyResultsWebViewItem *wv = dynamic_cast<QgsIdentifyResultsWebViewItem *>( current->child( i ) );
    if ( wv )
    {
      mActionPrint->setEnabled( true );
      break;
    }
  }

  QTreeWidgetItem *layItem = layerItem( current );
  if ( current == layItem )
  {
    highlightLayer( layItem );
  }
  else
  {
    clearHighlights();
    highlightFeature( current );
  }
}

void QgsIdentifyResultsDialog::layerDestroyed()
{
  QObject *senderObject = sender();

  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    if ( layItem->data( 0, Qt::UserRole ).value<QObject *>() == senderObject )
    {
      for ( int j = 0; j < layItem->childCount(); j++ )
      {
        delete mHighlights.take( layItem->child( j ) );
      }
    }
  }

  disconnectLayer( senderObject );
  delete layerItem( senderObject );

  // remove items, starting from last
  for ( int i = tblResults->rowCount() - 1; i >= 0; i-- )
  {
    QgsDebugMsg( QStringLiteral( "item %1 / %2" ).arg( i ).arg( tblResults->rowCount() ) );
    QTableWidgetItem *layItem = tblResults->item( i, 0 );
    if ( layItem && layItem->data( Qt::UserRole ).value<QObject *>() == senderObject )
    {
      QgsDebugMsg( QStringLiteral( "removing row %1" ).arg( i ) );
      tblResults->removeRow( i );
    }
  }
}

void QgsIdentifyResultsDialog::disconnectLayer( QObject *layer )
{
  if ( !layer )
    return;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( vlayer )
  {
    disconnect( vlayer, &QgsVectorLayer::featureDeleted, this, &QgsIdentifyResultsDialog::featureDeleted );
    disconnect( vlayer, &QgsVectorLayer::attributeValueChanged,
                this, &QgsIdentifyResultsDialog::attributeValueChanged );
    disconnect( vlayer, &QgsVectorLayer::editingStarted, this, &QgsIdentifyResultsDialog::editingToggled );
    disconnect( vlayer, &QgsVectorLayer::editingStopped, this, &QgsIdentifyResultsDialog::editingToggled );
  }

  disconnect( layer, &QObject::destroyed, this, &QgsIdentifyResultsDialog::layerDestroyed );
}

void QgsIdentifyResultsDialog::featureDeleted( QgsFeatureId fid )
{
  QTreeWidgetItem *layItem = layerItem( sender() );

  if ( !layItem )
    return;

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    if ( featItem && STRING_TO_FID( featItem->data( 0, Qt::UserRole ) ) == fid )
    {
      delete mHighlights.take( featItem );
      delete featItem;
      break;
    }
  }

  if ( layItem->childCount() == 0 )
  {
    delete layItem;
  }

  for ( int i = tblResults->rowCount() - 1; i >= 0; i-- )
  {
    QgsDebugMsg( QStringLiteral( "item %1 / %2" ).arg( i ).arg( tblResults->rowCount() ) );
    QTableWidgetItem *layItem = tblResults->item( i, 0 );
    QTableWidgetItem *featItem = tblResults->item( i, 1 );
    if ( layItem && layItem->data( Qt::UserRole ).value<QObject *>() == sender() &&
         featItem && STRING_TO_FID( featItem->data( Qt::UserRole ) ) == fid )
    {
      QgsDebugMsg( QStringLiteral( "removing row %1" ).arg( i ) );
      tblResults->removeRow( i );
    }
  }
}

void QgsIdentifyResultsDialog::attributeValueChanged( QgsFeatureId fid, int idx, const QVariant &val )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( sender() );
  QTreeWidgetItem *layItem = layerItem( sender() );

  if ( !layItem )
    return;

  if ( idx >= vlayer->fields().size() )
    return;

  QgsField fld = vlayer->fields().at( idx );

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *featItem = layItem->child( i );

    if ( featItem && STRING_TO_FID( featItem->data( 0, Qt::UserRole ) ) == fid )
    {
      QString value( fld.displayString( val ) );

      if ( fld.name() == vlayer->displayField() )
        featItem->setData( 1, Qt::DisplayRole, value );

      for ( int j = 0; j < featItem->childCount(); j++ )
      {
        QTreeWidgetItem *item = featItem->child( j );
        if ( item->childCount() > 0 )
          continue;

        if ( item->data( 0, Qt::UserRole + 1 ).toInt() == idx )
        {
          const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( vlayer, fld.name() );
          value = representValue( vlayer, setup, fld.name(), val );

          QgsTreeWidgetItem *treeItem = static_cast< QgsTreeWidgetItem * >( item );
          treeItem->setSortData( 1, value );
          treeItem->setToolTip( 1, value );

          bool foundLinks = false;
          QString links = QgsStringUtils::insertLinks( value, &foundLinks );
          if ( foundLinks )
          {
            QLabel *valueLabel = new QLabel( links );
            valueLabel->setOpenExternalLinks( true );
            treeItem->setData( 1, Qt::DisplayRole, QString() );
            treeItem->treeWidget()->setItemWidget( item, 1, valueLabel );
          }
          else
          {
            treeItem->setData( 1, Qt::DisplayRole, value );
            treeItem->treeWidget()->setItemWidget( item, 1, nullptr );
          }
          return;
        }
      }
    }
  }
}

void QgsIdentifyResultsDialog::highlightFeature( QTreeWidgetItem *item )
{
  QgsMapLayer *layer = nullptr;
  QgsVectorLayer *vlayer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );

  layer = vlayer ? static_cast<QgsMapLayer *>( vlayer ) : static_cast<QgsMapLayer *>( rlayer );

  if ( !layer ) return;

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( !featItem )
    return;

  if ( mHighlights.contains( featItem ) )
    return;

  if ( !featItem->feature().hasGeometry() || featItem->feature().geometry().wkbType() == QgsWkbTypes::Unknown )
    return;

  QgsHighlight *highlight = nullptr;
  if ( vlayer )
  {
    highlight = new QgsHighlight( mCanvas, featItem->feature(), vlayer );
  }
  else
  {
    highlight = new QgsHighlight( mCanvas, featItem->feature().geometry(), layer );
    highlight->setWidth( 2 );
  }

  QgsSettings settings;
  QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int alpha = settings.value( QStringLiteral( "Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  double buffer = settings.value( QStringLiteral( "Map/highlight/buffer" ), Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  double minWidth = settings.value( QStringLiteral( "Map/highlight/minWidth" ), Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();

  highlight->setColor( color ); // sets also fill with default alpha
  color.setAlpha( alpha );
  highlight->setFillColor( color ); // sets fill with alpha
  highlight->setBuffer( buffer );
  highlight->setMinWidth( minWidth );
  highlight->show();
  mHighlights.insert( featItem, highlight );
}

void QgsIdentifyResultsDialog::zoomToFeature()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsVectorLayer *vlayer = vectorLayer( item );
  QgsRasterLayer *rlayer = rasterLayer( item );
  if ( !vlayer && !rlayer )
    return;

  QgsMapLayer *layer = nullptr;
  if ( vlayer )
    layer = vlayer;
  else
    layer = rlayer;

  QgsIdentifyResultsFeatureItem *featItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( item ) );
  if ( !featItem )
    return;

  QgsFeature feat = featItem->feature();
  if ( !feat.hasGeometry() )
    return;

  // TODO: verify CRS for raster WMS features
  QgsRectangle rect = mCanvas->mapSettings().layerExtentToOutputExtent( layer, feat.geometry().boundingBox() );

  if ( rect.isEmpty() )
  {
    QgsPointXY c = rect.center();
    rect = mCanvas->extent();
    rect.scale( 0.5, &c );
  }

  mCanvas->setExtent( rect );
  mCanvas->refresh();
}

void QgsIdentifyResultsDialog::featureForm()
{
  QTreeWidgetItem *item = lstResults->currentItem();

  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( !vlayer )
    return;

  QTreeWidgetItem *featItem = featureItem( item );
  if ( !featItem )
    return;

  QgsFeatureId fid = STRING_TO_FID( featItem->data( 0, Qt::UserRole ) );

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( f ) )
    return;

  QString actionId = featItem->data( 0, Qt::UserRole + 1 ).toString();

  QgsFeatureAction action( tr( "Attributes changed" ), f, vlayer, actionId, -1, this );
  if ( vlayer->isEditable() )
  {
    action.editFeature( false );
  }
  else
  {
    action.viewFeatureForm( mHighlights.take( featItem ) );
  }
}

void QgsIdentifyResultsDialog::highlightAll()
{
  for ( int i = 0; i < lstResults->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *layItem = lstResults->topLevelItem( i );

    for ( int j = 0; j < layItem->childCount(); j++ )
    {
      highlightFeature( layItem->child( j ) );
    }
  }
}

void QgsIdentifyResultsDialog::highlightLayer()
{
  highlightLayer( lstResults->currentItem() );
}

void QgsIdentifyResultsDialog::highlightLayer( QTreeWidgetItem *item )
{
  QTreeWidgetItem *layItem = layerItem( item );
  if ( !layItem )
    return;

  clearHighlights();

  for ( int i = 0; i < layItem->childCount(); i++ )
  {
    highlightFeature( layItem->child( i ) );
  }
}

void QgsIdentifyResultsDialog::layerProperties()
{
  layerProperties( lstResults->currentItem() );
}

void QgsIdentifyResultsDialog::activateLayer()
{
  connect( this, static_cast<void ( QgsIdentifyResultsDialog::* )( QgsMapLayer * )>( &QgsIdentifyResultsDialog::activateLayer )
           , QgisApp::instance(), &QgisApp::setActiveLayer );
  emit activateLayer( layer( lstResults->currentItem() ) );
  disconnect( this, static_cast<void ( QgsIdentifyResultsDialog::* )( QgsMapLayer * )>( &QgsIdentifyResultsDialog::activateLayer ),
              QgisApp::instance(), &QgisApp::setActiveLayer );
}

void QgsIdentifyResultsDialog::layerProperties( QTreeWidgetItem *item )
{
  QgsVectorLayer *vlayer = vectorLayer( item );
  if ( !vlayer )
    return;

  QgisApp::instance()->showLayerProperties( vlayer );
}

void QgsIdentifyResultsDialog::expandAll()
{
  lstResults->expandAll();
}

void QgsIdentifyResultsDialog::collapseAll()
{
  lstResults->collapseAll();
}

void QgsIdentifyResultsDialog::copyAttributeValue()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString text = lstResults->currentItem()->data( 1, Qt::DisplayRole ).toString();
  QgsDebugMsg( QStringLiteral( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}

void QgsIdentifyResultsDialog::copyFeatureAttributes()
{
  QClipboard *clipboard = QApplication::clipboard();
  QString text;

  QgsVectorLayer *vlayer = vectorLayer( lstResults->currentItem() );
  QgsRasterLayer *rlayer = rasterLayer( lstResults->currentItem() );
  if ( !vlayer && !rlayer )
  {
    return;
  }

  if ( vlayer )
  {
    int idx;
    QgsAttributeMap attributes;
    retrieveAttributes( lstResults->currentItem(), attributes, idx );

    const QgsFields &fields = vlayer->fields();

    for ( QgsAttributeMap::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
    {
      int attrIdx = it.key();
      if ( attrIdx < 0 || attrIdx >= fields.count() )
        continue;

      text += QStringLiteral( "%1: %2\n" ).arg( fields.at( attrIdx ).name(), it.value().toString() );
    }
  }
  else if ( rlayer )
  {
    QTreeWidgetItem *featItem = featureItem( lstResults->currentItem() );
    if ( !featItem )
      return;

    for ( int i = 0; i < featItem->childCount(); i++ )
    {
      QTreeWidgetItem *item = featItem->child( i );
      if ( item->childCount() > 0 )
        continue;
      text += QStringLiteral( "%1: %2\n" ).arg( item->data( 0, Qt::DisplayRole ).toString(), item->data( 1, Qt::DisplayRole ).toString() );
    }
  }

  QgsDebugMsg( QStringLiteral( "set clipboard: %1" ).arg( text ) );
  clipboard->setText( text );
}

void QgsIdentifyResultsDialog::copyGetFeatureInfoUrl()
{
  QClipboard *clipboard = QApplication::clipboard();
  QTreeWidgetItem *item = lstResults->currentItem();
  QTreeWidgetItem *layItem = layerItem( item );
  if ( !layItem )
    return;
  clipboard->setText( layItem->data( 0, GetFeatureInfoUrlRole ).toString() );
}

void QgsIdentifyResultsDialog::printCurrentItem()
{
  QTreeWidgetItem *item = lstResults->currentItem();
  if ( !item )
    return;

  // There should only be one HTML item / result
  QgsIdentifyResultsWebViewItem *wv = nullptr;
  for ( int i = 0; i < item->childCount() && !wv; i++ )
  {
    wv = dynamic_cast<QgsIdentifyResultsWebViewItem *>( item->child( i ) );
  }

  if ( !wv )
  {
    QMessageBox::warning( this, tr( "Print HTML Response" ), tr( "Cannot print this item." ) );
    return;
  }

  wv->webView()->print();
}

void QgsIdentifyResultsDialog::cmbIdentifyMode_currentIndexChanged( int index )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Map/identifyMode" ), cmbIdentifyMode->itemData( index ).toInt() );
}

void QgsIdentifyResultsDialog::cmbViewMode_currentIndexChanged( int index )
{
  stackedWidget->setCurrentIndex( index );
}

void QgsIdentifyResultsDialog::cbxAutoFeatureForm_toggled( bool checked )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Map/identifyAutoFeatureForm" ), checked );
}

void QgsIdentifyResultsDialog::mExpandNewAction_triggered( bool checked )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Map/identifyExpand" ), checked );
}

void QgsIdentifyResultsDialog::mActionCopy_triggered( bool checked )
{
  Q_UNUSED( checked );
  copyFeature();
}

void QgsIdentifyResultsDialog::copyFeature()
{

  QgsIdentifyResultsFeatureItem *item = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( lstResults->selectedItems().value( 0 ) ) );

  if ( !item ) // should not happen
  {
    QgsDebugMsg( QStringLiteral( "Selected item is not feature" ) );
    return;
  }

  QgsFeatureStore featureStore( item->fields(), item->crs() );
  QgsFeature f( item->feature() );
  featureStore.addFeature( f );
  emit copyToClipboard( featureStore );
}

void QgsIdentifyResultsDialog::toggleFeatureSelection()
{

  QgsIdentifyResultsFeatureItem *item = dynamic_cast<QgsIdentifyResultsFeatureItem *>( featureItem( lstResults->selectedItems().value( 0 ) ) );

  if ( !item ) // should not happen
  {
    QgsDebugMsg( QStringLiteral( "Selected item is not feature" ) );
    return;
  }

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer( item ) );
  if ( !vl )
    return;

  if ( vl->selectedFeatureIds().contains( item->feature().id() ) )
    vl->deselect( item->feature().id() );
  else
    vl->select( item->feature().id() );
}

void QgsIdentifyResultsDialog::formatChanged( int index )
{
  QComboBox *combo = qobject_cast<QComboBox *>( sender() );
  if ( !combo )
  {
    QgsDebugMsg( QStringLiteral( "sender is not QComboBox" ) );
    return;
  }

  QgsRaster::IdentifyFormat format = ( QgsRaster::IdentifyFormat ) combo->itemData( index, Qt::UserRole ).toInt();
  QgsDebugMsg( QStringLiteral( "format = %1" ).arg( format ) );
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( combo->itemData( index, Qt::UserRole + 1 ).value<QObject *>() );
  if ( !layer )
  {
    QgsDebugMsg( QStringLiteral( "cannot get raster layer" ) );
    return;
  }

  // Store selected identify format in layer
  layer->setCustomProperty( QStringLiteral( "identify/format" ), QgsRasterDataProvider::identifyFormatName( format ) );

  // remove all children of that layer from results, except the first (format)
  QTreeWidgetItem *layItem = layerItem( layer );
  if ( !layItem )
  {
    QgsDebugMsg( QStringLiteral( "cannot get layer item" ) );
    return;
  }

  for ( int i = layItem->childCount() - 1; i > 0; i-- )
  {
    QTreeWidgetItem *child = layItem->child( i );
    QgsDebugMsg( QStringLiteral( "remove %1:0x%2" ).arg( i ).arg( ( qint64 ) child, 0, 16 ) );
    layItem->removeChild( child );
    QgsDebugMsg( QStringLiteral( "removed %1:0x%2" ).arg( i ).arg( ( qint64 ) child, 0, 16 ) );
  }

  // let know QgsMapToolIdentify that format changed
  emit formatChanged( layer );

  // Expand
  layItem->setExpanded( true );
  for ( int i = 1; i < layItem->childCount(); i++ )
  {
    QTreeWidgetItem *subItem = layItem->child( i );
    subItem->setExpanded( true );
    for ( int j = 0; j < subItem->childCount(); j++ )
    {
      subItem->child( j )->setExpanded( true );
    }
  }
}

/*
 * QgsIdentifyResultsDialogMapLayerAction
 */

void QgsIdentifyResultsDialogMapLayerAction::execute()
{
  mAction->triggerForFeature( mLayer, mFeature );
}

void QgsIdentifyResultsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#identify" ) );
}

void QgsIdentifyResultsDialog::setSelectionMode()
{
  QObject *obj = sender();
  QgsMapToolSelectionHandler::SelectionMode oldMode = mSelectionMode;
  if ( obj == mActionSelectFeatures )
  {
    mSelectModeButton->setDefaultAction( mActionSelectFeatures );
    mSelectionMode = QgsMapToolSelectionHandler::SelectSimple;
  }
  else if ( obj == mActionSelectPolygon )
  {
    mSelectModeButton->setDefaultAction( mActionSelectPolygon );
    mSelectionMode = QgsMapToolSelectionHandler::SelectPolygon;
  }
  else if ( obj == mActionSelectFreehand )
  {
    mSelectModeButton->setDefaultAction( mActionSelectFreehand );
    mSelectionMode = QgsMapToolSelectionHandler::SelectFreehand;
  }
  else if ( obj == mActionSelectRadius )
  {
    mSelectModeButton->setDefaultAction( mActionSelectRadius );
    mSelectionMode = QgsMapToolSelectionHandler::SelectRadius;
  }

  if ( oldMode != mSelectionMode )
    emit selectionModeChanged();
}

QgsMapToolSelectionHandler::SelectionMode QgsIdentifyResultsDialog::selectionMode() const
{
  return mSelectionMode;
}

void QgsIdentifyResultsDialog::setExpressionContextScope( const QgsExpressionContextScope &scope )
{
  mExpressionContextScope = scope;
}

QgsExpressionContextScope QgsIdentifyResultsDialog::expressionContextScope() const
{
  return mExpressionContextScope;
}
