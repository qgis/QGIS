/***************************************************************************
                          qgsoptions.cpp
                    Set user options and preferences
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsoptions.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgisappstylesheet.h"
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsprojectionselectiondialog.h"
#include "qgscoordinatereferencesystem.h"
#include "qgstolerance.h"
#include "qgsscaleutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsauthconfigselect.h"
#include "qgsproject.h"
#include "qgsdualview.h"
#include "qgsrasterlayer.h"
#include "qgsrasterminmaxorigin.h"
#include "qgscontrastenhancement.h"

#include "qgsattributetablefiltermodel.h"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsrasterpyramidsoptionswidget.h"
#include "qgsdatumtransformtablewidget.h"
#include "qgsdialog.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscolordialog.h"
#include "qgsexpressioncontext.h"
#include "qgsunittypes.h"
#include "qgsclipboard.h"
#include "qgssettings.h"
#include "qgsoptionswidgetfactory.h"
#include "qgslocatorwidget.h"
#include "qgslocatoroptionswidget.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QLocale>
#include <QProcess>
#include <QToolBar>
#include <QScrollBar>
#include <QSize>
#include <QStyleFactory>
#include <QMessageBox>
#include <QNetworkDiskCache>

#include <limits>
#include <sqlite3.h>
#include "qgslogger.h"

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <gdal.h>
#include <geos_c.h>
#include <cpl_conv.h> // for setting gdal options

#include "qgsconfig.h"

/**
 * \class QgsOptions - Set user options and preferences
 * Constructor
 */
QgsOptions::QgsOptions( QWidget *parent, Qt::WindowFlags fl, const QList<QgsOptionsWidgetFactory *> &optionsFactories )
  : QgsOptionsDialogBase( QStringLiteral( "Options" ), parent, fl )

{
  setupUi( this );
  connect( cbxProjectDefaultNew, &QCheckBox::toggled, this, &QgsOptions::cbxProjectDefaultNew_toggled );
  connect( leProjectGlobalCrs, &QgsProjectionSelectionWidget::crsChanged, this, &QgsOptions::leProjectGlobalCrs_crsChanged );
  connect( leLayerGlobalCrs, &QgsProjectionSelectionWidget::crsChanged, this, &QgsOptions::leLayerGlobalCrs_crsChanged );
  connect( lstGdalDrivers, &QTreeWidget::itemDoubleClicked, this, &QgsOptions::lstGdalDrivers_itemDoubleClicked );
  connect( mProjectOnLaunchCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOptions::mProjectOnLaunchCmbBx_currentIndexChanged );
  connect( spinFontSize, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsOptions::spinFontSize_valueChanged );
  connect( mFontFamilyRadioQt, &QRadioButton::released, this, &QgsOptions::mFontFamilyRadioQt_released );
  connect( mFontFamilyRadioCustom, &QRadioButton::released, this, &QgsOptions::mFontFamilyRadioCustom_released );
  connect( mFontFamilyComboBox, &QFontComboBox::currentFontChanged, this, &QgsOptions::mFontFamilyComboBox_currentFontChanged );
  connect( mProxyTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOptions::mProxyTypeComboBox_currentIndexChanged );
  connect( mCustomVariablesChkBx, &QCheckBox::toggled, this, &QgsOptions::mCustomVariablesChkBx_toggled );
  connect( mCurrentVariablesQGISChxBx, &QCheckBox::toggled, this, &QgsOptions::mCurrentVariablesQGISChxBx_toggled );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOptions::showHelp );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  // stylesheet setup
  mStyleSheetBuilder = QgisApp::instance()->styleSheetBuilder();
  mStyleSheetNewOpts = mStyleSheetBuilder->defaultOptions();
  mStyleSheetOldOpts = QMap<QString, QVariant>( mStyleSheetNewOpts );

  connect( mFontFamilyRadioCustom, &QAbstractButton::toggled, mFontFamilyComboBox, &QWidget::setEnabled );

  connect( cmbIconSize, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::activated ), this, &QgsOptions::iconSizeChanged );
  connect( cmbIconSize, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::highlighted ), this, &QgsOptions::iconSizeChanged );
  connect( cmbIconSize, &QComboBox::editTextChanged, this, &QgsOptions::iconSizeChanged );

  connect( this, &QDialog::accepted, this, &QgsOptions::saveOptions );
  connect( this, &QDialog::rejected, this, &QgsOptions::rejectOptions );

  QStringList styles = QStyleFactory::keys();
  QStringList filteredStyles = styles;
  for ( int i = filteredStyles.count() - 1; i >= 0; --i )
  {
    // filter out the broken adwaita styles - see note in main.cpp
    if ( filteredStyles.at( i ).contains( QStringLiteral( "adwaita" ), Qt::CaseInsensitive ) )
    {
      filteredStyles.removeAt( i );
    }
  }
  if ( filteredStyles.isEmpty() )
  {
    //oops - none left!.. have to let user use a broken style
    filteredStyles = styles;
  }

  cmbStyle->addItems( filteredStyles );

  QStringList themes = QgsApplication::uiThemes().keys();
  cmbUITheme->addItems( themes );

  connect( cmbUITheme, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsOptions::uiThemeChanged );

  mIdentifyHighlightColorButton->setColorDialogTitle( tr( "Identify Highlight Color" ) );
  mIdentifyHighlightColorButton->setAllowOpacity( true );
  mIdentifyHighlightColorButton->setContext( QStringLiteral( "gui" ) );
  mIdentifyHighlightColorButton->setDefaultColor( Qgis::DEFAULT_HIGHLIGHT_COLOR );

  mSettings = new QgsSettings();

  double identifyValue = mSettings->value( QStringLiteral( "/Map/searchRadiusMM" ), Qgis::DEFAULT_SEARCH_RADIUS_MM ).toDouble();
  QgsDebugMsg( QString( "Standard Identify radius setting read from settings file: %1" ).arg( identifyValue ) );
  if ( identifyValue <= 0.0 )
    identifyValue = Qgis::DEFAULT_SEARCH_RADIUS_MM;
  spinBoxIdentifyValue->setMinimum( 0.0 );
  spinBoxIdentifyValue->setValue( identifyValue );
  QColor highlightColor = QColor( mSettings->value( QStringLiteral( "/Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int highlightAlpha = mSettings->value( QStringLiteral( "/Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  highlightColor.setAlpha( highlightAlpha );
  mIdentifyHighlightColorButton->setColor( highlightColor );
  double highlightBuffer = mSettings->value( QStringLiteral( "/Map/highlight/buffer" ), Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  mIdentifyHighlightBufferSpinBox->setValue( highlightBuffer );
  double highlightMinWidth = mSettings->value( QStringLiteral( "/Map/highlight/minWidth" ), Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();
  mIdentifyHighlightMinWidthSpinBox->setValue( highlightMinWidth );

  // custom environment variables
  bool useCustomVars = mSettings->value( QStringLiteral( "qgis/customEnvVarsUse" ), QVariant( false ) ).toBool();
  mCustomVariablesChkBx->setChecked( useCustomVars );
  if ( !useCustomVars )
  {
    mAddCustomVarBtn->setEnabled( false );
    mRemoveCustomVarBtn->setEnabled( false );
    mCustomVariablesTable->setEnabled( false );
  }
  const QStringList customVarsList = mSettings->value( QStringLiteral( "qgis/customEnvVars" ) ).toStringList();
  for ( const QString &varStr : customVarsList )
  {
    int pos = varStr.indexOf( QLatin1Char( '|' ) );
    if ( pos == -1 )
      continue;
    QString varStrApply = varStr.left( pos );
    QString varStrNameValue = varStr.mid( pos + 1 );
    pos = varStrNameValue.indexOf( QLatin1Char( '=' ) );
    if ( pos == -1 )
      continue;
    QString varStrName = varStrNameValue.left( pos );
    QString varStrValue = varStrNameValue.mid( pos + 1 );

    addCustomEnvVarRow( varStrName, varStrValue, varStrApply );
  }
  QFontMetrics fmCustomVar( mCustomVariablesTable->horizontalHeader()->font() );
  int fmCustomVarH = fmCustomVar.height() + 8;
  mCustomVariablesTable->horizontalHeader()->setFixedHeight( fmCustomVarH );

  mCustomVariablesTable->setColumnWidth( 0, 120 );
  if ( mCustomVariablesTable->rowCount() > 0 )
  {
    mCustomVariablesTable->resizeColumnToContents( 1 );
  }
  else
  {
    mCustomVariablesTable->setColumnWidth( 1, 120 );
  }

  // current environment variables
  mCurrentVariablesTable->horizontalHeader()->setFixedHeight( fmCustomVarH );
  QMap<QString, QString> sysVarsMap = QgsApplication::systemEnvVars();
  const QStringList currentVarsList = QProcess::systemEnvironment();

  for ( const QString &varStr : currentVarsList )
  {
    int pos = varStr.indexOf( QLatin1Char( '=' ) );
    if ( pos == -1 )
      continue;
    QStringList varStrItms;
    QString varStrName = varStr.left( pos );
    QString varStrValue = varStr.mid( pos + 1 );
    varStrItms << varStrName << varStrValue;

    // check if different than system variable
    QString sysVarVal;
    bool sysVarMissing = !sysVarsMap.contains( varStrName );
    if ( sysVarMissing )
      sysVarVal = tr( "not present" );

    if ( !sysVarMissing && sysVarsMap.value( varStrName ) != varStrValue )
      sysVarVal = sysVarsMap.value( varStrName );

    if ( !sysVarVal.isEmpty() )
      sysVarVal = tr( "System value: %1" ).arg( sysVarVal );

    int rowCnt = mCurrentVariablesTable->rowCount();
    mCurrentVariablesTable->insertRow( rowCnt );

    QFont fItm;
    for ( int i = 0; i < varStrItms.size(); ++i )
    {
      QTableWidgetItem *varNameItm = new QTableWidgetItem( varStrItms.at( i ) );
      varNameItm->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable
                            | Qt::ItemIsEditable | Qt::ItemIsDragEnabled );
      fItm = varNameItm->font();
      if ( !sysVarVal.isEmpty() )
      {
        fItm.setBold( true );
        varNameItm->setFont( fItm );
        varNameItm->setToolTip( sysVarVal );
      }
      mCurrentVariablesTable->setItem( rowCnt, i, varNameItm );
    }
    fItm.setBold( true );
    QFontMetrics fmRow( fItm );
    mCurrentVariablesTable->setRowHeight( rowCnt, fmRow.height() + 6 );
  }
  if ( mCurrentVariablesTable->rowCount() > 0 )
    mCurrentVariablesTable->resizeColumnToContents( 0 );

  //local directories to search when loading c++ plugins
  const QStringList pluginsPathList = mSettings->value( QStringLiteral( "plugins/searchPathsForPlugins" ) ).toStringList();
  for ( const QString &path : pluginsPathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListPluginPaths );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListPluginPaths->addItem( newItem );
  }
  connect( mBtnAddPluginPath, &QAbstractButton::clicked, this, &QgsOptions::addPluginPath );
  connect( mBtnRemovePluginPath, &QAbstractButton::clicked, this, &QgsOptions::removePluginPath );

  //local directories to search when looking for an SVG with a given basename
  const QStringList svgPathList = QgsApplication::svgPaths();
  for ( const QString &path : svgPathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListSVGPaths );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListSVGPaths->addItem( newItem );
  }
  connect( mBtnAddSVGPath, &QAbstractButton::clicked, this, &QgsOptions::addSVGPath );
  connect( mBtnRemoveSVGPath, &QAbstractButton::clicked, this, &QgsOptions::removeSVGPath );

  //local directories to search when looking for a composer templates
  const QStringList composerTemplatePathList = QgsApplication::composerTemplatePaths();
  for ( const QString &path : composerTemplatePathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListComposerTemplatePaths );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListComposerTemplatePaths->addItem( newItem );
  }
  connect( mBtnAddTemplatePath, &QAbstractButton::clicked, this, &QgsOptions::addTemplatePath );
  connect( mBtnRemoveTemplatePath, &QAbstractButton::clicked, this, &QgsOptions::removeTemplatePath );

  //paths hidden from browser
  const QStringList hiddenPathList = mSettings->value( QStringLiteral( "/browser/hiddenPaths" ) ).toStringList();
  for ( const QString &path : hiddenPathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListHiddenBrowserPaths );
    newItem->setText( path );
    mListHiddenBrowserPaths->addItem( newItem );
  }
  connect( mBtnRemoveHiddenPath, &QAbstractButton::clicked, this, &QgsOptions::removeHiddenPath );

  //locations of the QGIS help
  const QStringList helpPathList = mSettings->value( QStringLiteral( "help/helpSearchPath" ), "https://docs.qgis.org/$qgis_short_version/$qgis_locale/docs/user_manual/" ).toStringList();
  for ( const QString &path : helpPathList )
  {
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText( 0, path );
    item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
    mHelpPathTreeWidget->addTopLevelItem( item );
  }
  connect( mBtnAddHelpPath, &QAbstractButton::clicked, this, &QgsOptions::addHelpPath );
  connect( mBtnRemoveHelpPath, &QAbstractButton::clicked, this, &QgsOptions::removeHelpPath );
  connect( mBtnMoveHelpUp, &QAbstractButton::clicked, this, &QgsOptions::moveHelpPathUp );
  connect( mBtnMoveHelpDown, &QAbstractButton::clicked, this, &QgsOptions::moveHelpPathDown );

  //Network timeout
  mNetworkTimeoutSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), 60000 ).toInt() );
  leUserAgent->setText( mSettings->value( QStringLiteral( "/qgis/networkAndProxy/userAgent" ), "Mozilla/5.0" ).toString() );

  // WMS capabilities expiry time
  mDefaultCapabilitiesExpirySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultCapabilitiesExpiry" ), 24 ).toInt() );

  // WMS/WMS-C tile expiry time
  mDefaultTileExpirySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultTileExpiry" ), 24 ).toInt() );

  // WMS/WMS-C default max retry in case of tile request errors
  mDefaultTileMaxRetrySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultTileMaxRetry" ), 3 ).toInt() );

  // Proxy stored authentication configurations
  mAuthSettings->setDataprovider( QStringLiteral( "proxy" ) );
  QString authcfg = mSettings->value( QStringLiteral( "proxy/authcfg" ) ).toString();
  mAuthSettings->setConfigId( authcfg );
  mAuthSettings->setWarningText( mAuthSettings->formattedWarning( QgsAuthSettingsWidget::UserSettings ) );

  //Web proxy settings
  grpProxy->setChecked( mSettings->value( QStringLiteral( "proxy/proxyEnabled" ), false ).toBool() );
  leProxyHost->setText( mSettings->value( QStringLiteral( "proxy/proxyHost" ), QString() ).toString() );
  leProxyPort->setText( mSettings->value( QStringLiteral( "proxy/proxyPort" ), QString() ).toString() );

  mAuthSettings->setPassword( mSettings->value( QStringLiteral( "proxy/proxyPassword" ), QString() ).toString() );
  mAuthSettings->setUsername( mSettings->value( QStringLiteral( "proxy/proxyUser" ), QString() ).toString() );

  //available proxy types
  mProxyTypeComboBox->insertItem( 0, QStringLiteral( "DefaultProxy" ) );
  mProxyTypeComboBox->insertItem( 1, QStringLiteral( "Socks5Proxy" ) );
  mProxyTypeComboBox->insertItem( 2, QStringLiteral( "HttpProxy" ) );
  mProxyTypeComboBox->insertItem( 3, QStringLiteral( "HttpCachingProxy" ) );
  mProxyTypeComboBox->insertItem( 4, QStringLiteral( "FtpCachingProxy" ) );
  QString settingProxyType = mSettings->value( QStringLiteral( "proxy/proxyType" ), QStringLiteral( "DefaultProxy" ) ).toString();
  mProxyTypeComboBox->setCurrentIndex( mProxyTypeComboBox->findText( settingProxyType ) );

  //URLs excluded not going through proxies
  const QStringList excludedUrlPathList = mSettings->value( QStringLiteral( "proxy/proxyExcludedUrls" ) ).toStringList();
  for ( const QString &path : excludedUrlPathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mExcludeUrlListWidget );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mExcludeUrlListWidget->addItem( newItem );
  }
  connect( mAddUrlPushButton, &QAbstractButton::clicked, this, &QgsOptions::addExcludedUrl );
  connect( mRemoveUrlPushButton, &QAbstractButton::clicked, this, &QgsOptions::removeExcludedUrl );

  // cache settings
  mCacheDirectory->setText( mSettings->value( QStringLiteral( "cache/directory" ) ).toString() );
  mCacheDirectory->setPlaceholderText( QDir( QgsApplication::qgisSettingsDirPath() ).canonicalPath() + QDir::separator() + QStringLiteral( "cache" ) );
  mCacheSize->setMinimum( 0 );
  mCacheSize->setMaximum( std::numeric_limits<int>::max() );
  mCacheSize->setSingleStep( 1024 );
  qint64 cacheSize = mSettings->value( QStringLiteral( "cache/size" ), 50 * 1024 * 1024 ).toULongLong();
  mCacheSize->setValue( ( int )( cacheSize / 1024 ) );
  connect( mBrowseCacheDirectory, &QAbstractButton::clicked, this, &QgsOptions::browseCacheDirectory );
  connect( mClearCache, &QAbstractButton::clicked, this, &QgsOptions::clearCache );

  // Access (auth) cache settings
  mAutoClearAccessCache->setChecked( mSettings->value( QStringLiteral( "clear_auth_cache_on_errors" ), true, QgsSettings::Section::Auth ).toBool( ) );
  connect( mClearAccessCache, &QAbstractButton::clicked, this, &QgsOptions::clearAccessCache );

  connect( mAutoClearAccessCache, &QCheckBox::clicked, this, [ = ]( bool checked )
  {
    mSettings->setValue( QStringLiteral( "clear_auth_cache_on_errors" ), checked, QgsSettings::Section::Auth );
  } );

  //wms search server
  leWmsSearch->setText( mSettings->value( QStringLiteral( "/qgis/WMSSearchUrl" ), "http://geopole.org/wms/search?search=%1&type=rss" ).toString() );

  // set the attribute table default filter
  cmbAttrTableBehavior->clear();
  cmbAttrTableBehavior->addItem( tr( "Show all features" ), QgsAttributeTableFilterModel::ShowAll );
  cmbAttrTableBehavior->addItem( tr( "Show selected features" ), QgsAttributeTableFilterModel::ShowSelected );
  cmbAttrTableBehavior->addItem( tr( "Show features visible on map" ), QgsAttributeTableFilterModel::ShowVisible );
  cmbAttrTableBehavior->setCurrentIndex( cmbAttrTableBehavior->findData( mSettings->value( QStringLiteral( "/qgis/attributeTableBehavior" ), QgsAttributeTableFilterModel::ShowAll ).toInt() ) );

  mAttrTableViewComboBox->clear();
  mAttrTableViewComboBox->addItem( tr( "Remember last view" ), -1 );
  mAttrTableViewComboBox->addItem( tr( "Table view" ), QgsDualView::AttributeTable );
  mAttrTableViewComboBox->addItem( tr( "Form view" ), QgsDualView::AttributeEditor );
  mAttrTableViewComboBox->setCurrentIndex( mAttrTableViewComboBox->findData( mSettings->value( QStringLiteral( "/qgis/attributeTableView" ), -1 ).toInt() ) );

  spinBoxAttrTableRowCache->setValue( mSettings->value( QStringLiteral( "/qgis/attributeTableRowCache" ), 10000 ).toInt() );
  spinBoxAttrTableRowCache->setSpecialValueText( tr( "All" ) );

  // set the prompt for raster sublayers
  // 0 = Always -> always ask (if there are existing sublayers)
  // 1 = If needed -> ask if layer has no bands, but has sublayers
  // 2 = Never -> never prompt, will not load anything
  // 3 = Load all -> never prompt, but load all sublayers
  cmbPromptRasterSublayers->clear();
  cmbPromptRasterSublayers->addItem( tr( "Always" ) );
  cmbPromptRasterSublayers->addItem( tr( "If needed" ) ); //this means, prompt if there are sublayers but no band in the main dataset
  cmbPromptRasterSublayers->addItem( tr( "Never" ) );
  cmbPromptRasterSublayers->addItem( tr( "Load all" ) );
  cmbPromptRasterSublayers->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/promptForRasterSublayers" ), 0 ).toInt() );

  // Scan for valid items in the browser dock
  cmbScanItemsInBrowser->clear();
  cmbScanItemsInBrowser->addItem( tr( "Check file contents" ), "contents" ); // 0
  cmbScanItemsInBrowser->addItem( tr( "Check extension" ), "extension" );    // 1
  int index = cmbScanItemsInBrowser->findData( mSettings->value( QStringLiteral( "/qgis/scanItemsInBrowser2" ), QString() ) );
  if ( index == -1 ) index = 1;
  cmbScanItemsInBrowser->setCurrentIndex( index );

  // Scan for contents of compressed files (.zip) in browser dock
  cmbScanZipInBrowser->clear();
  cmbScanZipInBrowser->addItem( tr( "No" ), QVariant( "no" ) );
  // cmbScanZipInBrowser->addItem( tr( "Passthru" ) );     // 1 - removed
  cmbScanZipInBrowser->addItem( tr( "Basic scan" ), QVariant( "basic" ) );
  cmbScanZipInBrowser->addItem( tr( "Full scan" ), QVariant( "full" ) );
  index = cmbScanZipInBrowser->findData( mSettings->value( QStringLiteral( "/qgis/scanZipInBrowser2" ), QString() ) );
  if ( index == -1 ) index = 1;
  cmbScanZipInBrowser->setCurrentIndex( index );

  // log rendering events, for userspace debugging
  mLogCanvasRefreshChkBx->setChecked( mSettings->value( QStringLiteral( "/Map/logCanvasRefreshEvent" ), false ).toBool() );

  //set the default projection behavior radio buttongs
  if ( mSettings->value( QStringLiteral( "/Projections/defaultBehavior" ), "prompt" ).toString() == QLatin1String( "prompt" ) )
  {
    radPromptForProjection->setChecked( true );
  }
  else if ( mSettings->value( QStringLiteral( "/Projections/defaultBehavior" ), "prompt" ).toString() == QLatin1String( "useProject" ) )
  {
    radUseProjectProjection->setChecked( true );
  }
  else //useGlobal
  {
    radUseGlobalProjection->setChecked( true );
  }
  QString myLayerDefaultCrs = mSettings->value( QStringLiteral( "/Projections/layerDefaultCrs" ), GEO_EPSG_CRS_AUTHID ).toString();
  mLayerDefaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( myLayerDefaultCrs );
  leLayerGlobalCrs->setCrs( mLayerDefaultCrs );

  QString myDefaultCrs = mSettings->value( QStringLiteral( "/Projections/projectDefaultCrs" ), GEO_EPSG_CRS_AUTHID ).toString();
  mDefaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( myDefaultCrs );
  leProjectGlobalCrs->setCrs( mDefaultCrs );
  leProjectGlobalCrs->setOptionVisible( QgsProjectionSelectionWidget::DefaultCrs, false );

  mShowDatumTransformDialogCheckBox->setChecked( mSettings->value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool() );

  // Datum transforms
  QgsCoordinateTransformContext context;
  context.readSettings();
  mDefaultDatumTransformTableWidget->setTransformContext( context );


  // Set the units for measuring
  mDistanceUnitsComboBox->addItem( tr( "Meters" ), QgsUnitTypes::DistanceMeters );
  mDistanceUnitsComboBox->addItem( tr( "Kilometers" ), QgsUnitTypes::DistanceKilometers );
  mDistanceUnitsComboBox->addItem( tr( "Feet" ), QgsUnitTypes::DistanceFeet );
  mDistanceUnitsComboBox->addItem( tr( "Yards" ), QgsUnitTypes::DistanceYards );
  mDistanceUnitsComboBox->addItem( tr( "Miles" ), QgsUnitTypes::DistanceMiles );
  mDistanceUnitsComboBox->addItem( tr( "Nautical miles" ), QgsUnitTypes::DistanceNauticalMiles );
  mDistanceUnitsComboBox->addItem( tr( "Degrees" ), QgsUnitTypes::DistanceDegrees );
  mDistanceUnitsComboBox->addItem( tr( "Map units" ), QgsUnitTypes::DistanceUnknownUnit );

  bool ok = false;
  QgsUnitTypes::DistanceUnit distanceUnits = QgsUnitTypes::decodeDistanceUnit( mSettings->value( QStringLiteral( "/qgis/measure/displayunits" ) ).toString(), &ok );
  if ( !ok )
    distanceUnits = QgsUnitTypes::DistanceMeters;
  mDistanceUnitsComboBox->setCurrentIndex( mDistanceUnitsComboBox->findData( distanceUnits ) );

  mAreaUnitsComboBox->addItem( tr( "Square meters" ), QgsUnitTypes::AreaSquareMeters );
  mAreaUnitsComboBox->addItem( tr( "Square kilometers" ), QgsUnitTypes::AreaSquareKilometers );
  mAreaUnitsComboBox->addItem( tr( "Square feet" ), QgsUnitTypes::AreaSquareFeet );
  mAreaUnitsComboBox->addItem( tr( "Square yards" ), QgsUnitTypes::AreaSquareYards );
  mAreaUnitsComboBox->addItem( tr( "Square miles" ), QgsUnitTypes::AreaSquareMiles );
  mAreaUnitsComboBox->addItem( tr( "Hectares" ), QgsUnitTypes::AreaHectares );
  mAreaUnitsComboBox->addItem( tr( "Acres" ), QgsUnitTypes::AreaAcres );
  mAreaUnitsComboBox->addItem( tr( "Square nautical miles" ), QgsUnitTypes::AreaSquareNauticalMiles );
  mAreaUnitsComboBox->addItem( tr( "Square degrees" ), QgsUnitTypes::AreaSquareDegrees );
  mAreaUnitsComboBox->addItem( tr( "Map units" ), QgsUnitTypes::AreaUnknownUnit );

  QgsUnitTypes::AreaUnit areaUnits = QgsUnitTypes::decodeAreaUnit( mSettings->value( QStringLiteral( "/qgis/measure/areaunits" ) ).toString(), &ok );
  if ( !ok )
    areaUnits = QgsUnitTypes::AreaSquareMeters;
  mAreaUnitsComboBox->setCurrentIndex( mAreaUnitsComboBox->findData( areaUnits ) );

  mAngleUnitsComboBox->addItem( tr( "Degrees" ), QgsUnitTypes::AngleDegrees );
  mAngleUnitsComboBox->addItem( tr( "Radians" ), QgsUnitTypes::AngleRadians );
  mAngleUnitsComboBox->addItem( tr( "Gon/gradians" ), QgsUnitTypes::AngleGon );
  mAngleUnitsComboBox->addItem( tr( "Minutes of arc" ), QgsUnitTypes::AngleMinutesOfArc );
  mAngleUnitsComboBox->addItem( tr( "Seconds of arc" ), QgsUnitTypes::AngleSecondsOfArc );
  mAngleUnitsComboBox->addItem( tr( "Turns/revolutions" ), QgsUnitTypes::AngleTurn );

  QgsUnitTypes::AngleUnit unit = QgsUnitTypes::decodeAngleUnit( mSettings->value( QStringLiteral( "/qgis/measure/angleunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleDegrees ) ).toString() );
  mAngleUnitsComboBox->setCurrentIndex( mAngleUnitsComboBox->findData( unit ) );

  // set decimal places of the measure tool
  int decimalPlaces = mSettings->value( QStringLiteral( "/qgis/measure/decimalplaces" ), "3" ).toInt();
  mDecimalPlacesSpinBox->setRange( 0, 12 );
  mDecimalPlacesSpinBox->setValue( decimalPlaces );

  // set if base unit of measure tool should be changed
  bool baseUnit = mSettings->value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
  if ( baseUnit )
  {
    mKeepBaseUnitCheckBox->setChecked( true );
  }
  else
  {
    mKeepBaseUnitCheckBox->setChecked( false );
  }

  cmbIconSize->setCurrentIndex( cmbIconSize->findText( mSettings->value( QStringLiteral( "/IconSize" ), QGIS_ICON_SIZE ).toString() ) );

  // set font size and family
  spinFontSize->blockSignals( true );
  mFontFamilyRadioQt->blockSignals( true );
  mFontFamilyRadioCustom->blockSignals( true );
  mFontFamilyComboBox->blockSignals( true );

  spinFontSize->setValue( mStyleSheetOldOpts.value( QStringLiteral( "fontPointSize" ) ).toInt() );
  QString fontFamily = mStyleSheetOldOpts.value( QStringLiteral( "fontFamily" ) ).toString();
  bool isQtDefault = ( fontFamily == mStyleSheetBuilder->defaultFont().family() );
  mFontFamilyRadioQt->setChecked( isQtDefault );
  mFontFamilyRadioCustom->setChecked( !isQtDefault );
  mFontFamilyComboBox->setEnabled( !isQtDefault );
  if ( !isQtDefault )
  {
    QFont *tempFont = new QFont( fontFamily );
    // is exact family match returned from system?
    if ( tempFont->family() == fontFamily )
    {
      mFontFamilyComboBox->setCurrentFont( *tempFont );
    }
    delete tempFont;
  }

  spinFontSize->blockSignals( false );
  mFontFamilyRadioQt->blockSignals( false );
  mFontFamilyRadioCustom->blockSignals( false );
  mFontFamilyComboBox->blockSignals( false );

  // custom group boxes
  mCustomGroupBoxChkBx->setChecked( mStyleSheetOldOpts.value( QStringLiteral( "groupBoxCustom" ) ).toBool() );
  connect( mCustomGroupBoxChkBx, &QAbstractButton::clicked, this, &QgsOptions::useCustomGroupBox );

  mMessageTimeoutSpnBx->setValue( mSettings->value( QStringLiteral( "/qgis/messageTimeout" ), 5 ).toInt() );

  QString name = mSettings->value( QStringLiteral( "/qgis/style" ) ).toString();
  cmbStyle->setCurrentIndex( cmbStyle->findText( name, Qt::MatchFixedString ) );

  QString theme = QgsApplication::themeName();
  cmbUITheme->setCurrentIndex( cmbUITheme->findText( theme, Qt::MatchFixedString ) );

  mNativeColorDialogsChkBx->setChecked( mSettings->value( QStringLiteral( "/qgis/native_color_dialogs" ), false ).toBool() );
  mLiveColorDialogsChkBx->setChecked( mSettings->value( QStringLiteral( "/qgis/live_color_dialogs" ), false ).toBool() );

  //set the state of the checkboxes
  //Changed to default to true as of QGIS 1.7
  chkAntiAliasing->setChecked( mSettings->value( QStringLiteral( "/qgis/enable_anti_aliasing" ), true ).toBool() );
  chkUseRenderCaching->setChecked( mSettings->value( QStringLiteral( "/qgis/enable_render_caching" ), true ).toBool() );
  chkParallelRendering->setChecked( mSettings->value( QStringLiteral( "/qgis/parallel_rendering" ), true ).toBool() );
  spinMapUpdateInterval->setValue( mSettings->value( QStringLiteral( "/qgis/map_update_interval" ), 250 ).toInt() );
  chkMaxThreads->setChecked( QgsApplication::maxThreads() != -1 );
  spinMaxThreads->setEnabled( chkMaxThreads->isChecked() );
  spinMaxThreads->setRange( 1, QThread::idealThreadCount() );
  spinMaxThreads->setValue( QgsApplication::maxThreads() );

  // Default simplify drawing configuration
  mSimplifyDrawingGroupBox->setChecked( mSettings->value( QStringLiteral( "/qgis/simplifyDrawingHints" ), ( int )QgsVectorSimplifyMethod::GeometrySimplification ).toInt() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/simplifyDrawingTol" ), Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD ).toFloat() );
  mSimplifyDrawingAtProvider->setChecked( !mSettings->value( QStringLiteral( "/qgis/simplifyLocal" ), true ).toBool() );

  //segmentation tolerance type
  mToleranceTypeComboBox->addItem( tr( "Maximum angle" ), 0 );
  mToleranceTypeComboBox->addItem( tr( "Maximum difference" ), 1 );
  int toleranceType = mSettings->value( QStringLiteral( "/qgis/segmentationToleranceType" ), "0" ).toInt();
  int toleranceTypeIndex = mToleranceTypeComboBox->findData( toleranceType );
  if ( toleranceTypeIndex != -1 )
  {
    mToleranceTypeComboBox->setCurrentIndex( toleranceTypeIndex );
  }

  double tolerance = mSettings->value( QStringLiteral( "/qgis/segmentationTolerance" ), "0.01745" ).toDouble();
  if ( toleranceType == 0 )
  {
    tolerance = tolerance * 180.0 / M_PI; //value shown to the user is degree, not rad
  }
  mSegmentationToleranceSpinBox->setValue( tolerance );

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  myScalesList.append( QStringLiteral( "1:1" ) );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( mSettings->value( QStringLiteral( "/qgis/simplifyMaxScale" ), 1 ).toFloat() );

  // Magnifier
  double magnifierMin = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MIN;
  double magnifierMax = 100 * QgsGuiUtils::CANVAS_MAGNIFICATION_MAX;
  double magnifierVal = 100 * mSettings->value( QStringLiteral( "/qgis/magnifier_factor_default" ), 1.0 ).toDouble();
  doubleSpinBoxMagnifierDefault->setRange( magnifierMin, magnifierMax );
  doubleSpinBoxMagnifierDefault->setSingleStep( 50 );
  doubleSpinBoxMagnifierDefault->setDecimals( 0 );
  doubleSpinBoxMagnifierDefault->setSuffix( QStringLiteral( "%" ) );
  doubleSpinBoxMagnifierDefault->setValue( magnifierVal );

  // Default local simplification algorithm
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), ( int )QgsVectorSimplifyMethod::Distance );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), ( int )QgsVectorSimplifyMethod::SnapToGrid );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), ( int )QgsVectorSimplifyMethod::Visvalingam );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData( mSettings->value( QStringLiteral( "/qgis/simplifyAlgorithm" ), 0 ).toInt() ) );

  // Slightly awkard here at the settings value is true to use QImage,
  // but the checkbox is true to use QPixmap
  chkAddedVisibility->setChecked( mSettings->value( QStringLiteral( "/qgis/new_layers_visible" ), true ).toBool() );
  cbxLegendClassifiers->setChecked( mSettings->value( QStringLiteral( "/qgis/showLegendClassifiers" ), false ).toBool() );
  cbxHideSplash->setChecked( mSettings->value( QStringLiteral( "/qgis/hideSplash" ), false ).toBool() );
  mDataSourceManagerNonModal->setChecked( mSettings->value( QStringLiteral( "/qgis/dataSourceManagerNonModal" ), false ).toBool() );
  cbxCheckVersion->setChecked( mSettings->value( QStringLiteral( "/qgis/checkVersion" ), true ).toBool() );
  cbxAttributeTableDocked->setChecked( mSettings->value( QStringLiteral( "/qgis/dockAttributeTable" ), false ).toBool() );
  cbxAddPostgisDC->setChecked( mSettings->value( QStringLiteral( "/qgis/addPostgisDC" ), false ).toBool() );
  cbxAddOracleDC->setChecked( mSettings->value( QStringLiteral( "/qgis/addOracleDC" ), false ).toBool() );
  cbxCompileExpressions->setChecked( mSettings->value( QStringLiteral( "/qgis/compileExpressions" ), true ).toBool() );

  mComboCopyFeatureFormat->addItem( tr( "Plain text, no geometry" ), QgsClipboard::AttributesOnly );
  mComboCopyFeatureFormat->addItem( tr( "Plain text, WKT geometry" ), QgsClipboard::AttributesWithWKT );
  mComboCopyFeatureFormat->addItem( tr( "GeoJSON" ), QgsClipboard::GeoJSON );
  if ( mSettings->contains( QStringLiteral( "/qgis/copyFeatureFormat" ) ) )
    mComboCopyFeatureFormat->setCurrentIndex( mComboCopyFeatureFormat->findData( mSettings->value( QStringLiteral( "/qgis/copyFeatureFormat" ), true ).toInt() ) );
  else
    mComboCopyFeatureFormat->setCurrentIndex( mComboCopyFeatureFormat->findData( mSettings->value( QStringLiteral( "/qgis/copyGeometryAsWKT" ), true ).toBool() ?
        QgsClipboard::AttributesWithWKT : QgsClipboard::AttributesOnly ) );
  leNullValue->setText( QgsApplication::nullRepresentation() );
  cbxIgnoreShapeEncoding->setChecked( mSettings->value( QStringLiteral( "/qgis/ignoreShapeEncoding" ), true ).toBool() );

  cmbLegendDoubleClickAction->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/legendDoubleClickAction" ), 0 ).toInt() );

  // WMS getLegendGraphic setting
  mLegendGraphicResolutionSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultLegendGraphicResolution" ), 0 ).toInt() );

  //
  // Raster properties
  //
  spnRed->setValue( mSettings->value( QStringLiteral( "/Raster/defaultRedBand" ), 1 ).toInt() );
  spnGreen->setValue( mSettings->value( QStringLiteral( "/Raster/defaultGreenBand" ), 2 ).toInt() );
  spnBlue->setValue( mSettings->value( QStringLiteral( "/Raster/defaultBlueBand" ), 3 ).toInt() );

  initContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, QStringLiteral( "singleBand" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::SINGLE_BAND_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM ) );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ),
                           QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM ) );

  initMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, QStringLiteral( "singleBand" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::SINGLE_BAND_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS ) );
  initMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ),
                    QgsRasterMinMaxOrigin::limitsString( QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS ) );

  spnThreeBandStdDev->setValue( mSettings->value( QStringLiteral( "/Raster/defaultStandardDeviation" ), QgsRasterMinMaxOrigin::DEFAULT_STDDEV_FACTOR ).toDouble() );

  mRasterCumulativeCutLowerDoubleSpinBox->setValue( 100.0 * mSettings->value( QStringLiteral( "/Raster/cumulativeCutLower" ), QString::number( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_LOWER ) ).toDouble() );
  mRasterCumulativeCutUpperDoubleSpinBox->setValue( 100.0 * mSettings->value( QStringLiteral( "/Raster/cumulativeCutUpper" ), QString::number( QgsRasterMinMaxOrigin::CUMULATIVE_CUT_UPPER ) ).toDouble() );

  //set the color for selections
  int myRed = mSettings->value( QStringLiteral( "/qgis/default_selection_color_red" ), 255 ).toInt();
  int myGreen = mSettings->value( QStringLiteral( "/qgis/default_selection_color_green" ), 255 ).toInt();
  int myBlue = mSettings->value( QStringLiteral( "/qgis/default_selection_color_blue" ), 0 ).toInt();
  int myAlpha = mSettings->value( QStringLiteral( "/qgis/default_selection_color_alpha" ), 255 ).toInt();
  pbnSelectionColor->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  pbnSelectionColor->setColorDialogTitle( tr( "Set Selection Color" ) );
  pbnSelectionColor->setAllowOpacity( true );
  pbnSelectionColor->setContext( QStringLiteral( "gui" ) );
  pbnSelectionColor->setDefaultColor( QColor( 255, 255, 0, 255 ) );

  //set the default color for canvas background
  myRed = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_red" ), 255 ).toInt();
  myGreen = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_green" ), 255 ).toInt();
  myBlue = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_blue" ), 255 ).toInt();
  pbnCanvasColor->setColor( QColor( myRed, myGreen, myBlue ) );
  pbnCanvasColor->setColorDialogTitle( tr( "Set Canvas Color" ) );
  pbnCanvasColor->setContext( QStringLiteral( "gui" ) );
  pbnCanvasColor->setDefaultColor( Qt::white );

  // set the default color for the measure tool
  myRed = mSettings->value( QStringLiteral( "/qgis/default_measure_color_red" ), 222 ).toInt();
  myGreen = mSettings->value( QStringLiteral( "/qgis/default_measure_color_green" ), 155 ).toInt();
  myBlue = mSettings->value( QStringLiteral( "/qgis/default_measure_color_blue" ), 67 ).toInt();
  pbnMeasureColor->setColor( QColor( myRed, myGreen, myBlue ) );
  pbnMeasureColor->setColorDialogTitle( tr( "Set Measuring Tool Color" ) );
  pbnMeasureColor->setContext( QStringLiteral( "gui" ) );
  pbnMeasureColor->setDefaultColor( QColor( 222, 155, 67 ) );

  int projOpen = mSettings->value( QStringLiteral( "/qgis/projOpenAtLaunch" ), 0 ).toInt();
  mProjectOnLaunchCmbBx->setCurrentIndex( projOpen );
  mProjectOnLaunchLineEdit->setText( mSettings->value( QStringLiteral( "/qgis/projOpenAtLaunchPath" ) ).toString() );
  mProjectOnLaunchLineEdit->setEnabled( projOpen == 2 );
  mProjectOnLaunchPushBtn->setEnabled( projOpen == 2 );
  connect( mProjectOnLaunchPushBtn, &QAbstractButton::pressed, this, &QgsOptions::selectProjectOnLaunch );

  chbAskToSaveProjectChanges->setChecked( mSettings->value( QStringLiteral( "qgis/askToSaveProjectChanges" ), QVariant( true ) ).toBool() );
  mLayerDeleteConfirmationChkBx->setChecked( mSettings->value( QStringLiteral( "qgis/askToDeleteLayers" ), true ).toBool() );
  chbWarnOldProjectVersion->setChecked( mSettings->value( QStringLiteral( "/qgis/warnOldProjectVersion" ), QVariant( true ) ).toBool() );
  cmbEnableMacros->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/enableMacros" ), 1 ).toInt() );

  // templates
  cbxProjectDefaultNew->setChecked( mSettings->value( QStringLiteral( "/qgis/newProjectDefault" ), QVariant( false ) ).toBool() );
  QString templateDirName = mSettings->value( QStringLiteral( "/qgis/projectTemplateDir" ),
                            QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
  // make dir if it doesn't exist - should just be called once
  QDir templateDir;
  if ( ! templateDir.exists( templateDirName ) )
  {
    templateDir.mkdir( templateDirName );
  }
  leTemplateFolder->setText( templateDirName );
  connect( pbnProjectDefaultSetCurrent, &QAbstractButton::clicked, this, &QgsOptions::setCurrentProjectDefault );
  connect( pbnProjectDefaultReset, &QAbstractButton::clicked, this, &QgsOptions::resetProjectDefault );
  connect( pbnTemplateFolderBrowse, &QAbstractButton::pressed, this, &QgsOptions::browseTemplateFolder );
  connect( pbnTemplateFolderReset, &QAbstractButton::pressed, this, &QgsOptions::resetTemplateFolder );

  setZoomFactorValue();

  // predefined scales for scale combobox
  QString scalePaths = mSettings->value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString();
  if ( !scalePaths.isEmpty() )
  {
    const QStringList ScalesList = scalePaths.split( ',' );
    for ( const QString &scale : ScalesList )
    {
      addScaleToScaleList( scale );
    }
  }
  connect( mListGlobalScales, &QListWidget::itemChanged, this, &QgsOptions::scaleItemChanged );
  connect( pbnAddScale, &QAbstractButton::clicked, this, &QgsOptions::addScale );
  connect( pbnRemoveScale, &QAbstractButton::clicked, this, &QgsOptions::removeScale );
  connect( pbnImportScales, &QAbstractButton::clicked, this, &QgsOptions::exportScales );
  connect( pbnImportScales, &QAbstractButton::clicked, this, &QgsOptions::importScales );
  connect( pbnDefaultScaleValues, &QAbstractButton::clicked, this, &QgsOptions::restoreDefaultScaleValues );

  //
  // Color palette
  //
  connect( mButtonAddColor, &QAbstractButton::clicked, this, &QgsOptions::addColor );
  connect( mButtonRemoveColor, &QAbstractButton::clicked, mTreeCustomColors, &QgsColorSchemeList::removeSelection );
  connect( mButtonCopyColors, &QAbstractButton::clicked, mTreeCustomColors, &QgsColorSchemeList::copyColors );
  connect( mButtonPasteColors, &QAbstractButton::clicked, mTreeCustomColors, &QgsColorSchemeList::pasteColors );
  connect( mButtonImportColors, &QAbstractButton::clicked, mTreeCustomColors, &QgsColorSchemeList::showImportColorsDialog );
  connect( mButtonExportColors, &QAbstractButton::clicked, mTreeCustomColors, &QgsColorSchemeList::showExportColorsDialog );

  //find custom color scheme from registry
  QList<QgsCustomColorScheme *> customSchemes;
  QgsApplication::colorSchemeRegistry()->schemes( customSchemes );
  if ( customSchemes.length() > 0 )
  {
    mTreeCustomColors->setScheme( customSchemes.at( 0 ) );
  }

  //
  // Composer settings
  //

  //default composer font
  mComposerFontComboBox->blockSignals( true );

  QString composerFontFamily = mSettings->value( QStringLiteral( "/Composer/defaultFont" ) ).toString();

  QFont *tempComposerFont = new QFont( composerFontFamily );
  // is exact family match returned from system?
  if ( tempComposerFont->family() == composerFontFamily )
  {
    mComposerFontComboBox->setCurrentFont( *tempComposerFont );
  }
  delete tempComposerFont;

  mComposerFontComboBox->blockSignals( false );

  //default composer grid color
  int gridRed, gridGreen, gridBlue, gridAlpha;
  gridRed = mSettings->value( QStringLiteral( "/Composer/gridRed" ), 190 ).toInt();
  gridGreen = mSettings->value( QStringLiteral( "/Composer/gridGreen" ), 190 ).toInt();
  gridBlue = mSettings->value( QStringLiteral( "/Composer/gridBlue" ), 190 ).toInt();
  gridAlpha = mSettings->value( QStringLiteral( "/Composer/gridAlpha" ), 100 ).toInt();
  QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );
  mGridColorButton->setColor( gridColor );
  mGridColorButton->setColorDialogTitle( tr( "Select Grid Color" ) );
  mGridColorButton->setAllowOpacity( true );
  mGridColorButton->setContext( QStringLiteral( "gui" ) );
  mGridColorButton->setDefaultColor( QColor( 190, 190, 190, 100 ) );

  //default composer grid style
  QString gridStyleString;
  gridStyleString = mSettings->value( QStringLiteral( "/Composer/gridStyle" ), "Dots" ).toString();
  mGridStyleComboBox->insertItem( 0, tr( "Solid" ) );
  mGridStyleComboBox->insertItem( 1, tr( "Dots" ) );
  mGridStyleComboBox->insertItem( 2, tr( "Crosses" ) );
  if ( gridStyleString == QLatin1String( "Solid" ) )
  {
    mGridStyleComboBox->setCurrentIndex( 0 );
  }
  else if ( gridStyleString == QLatin1String( "Crosses" ) )
  {
    mGridStyleComboBox->setCurrentIndex( 2 );
  }
  else
  {
    //default grid is dots
    mGridStyleComboBox->setCurrentIndex( 1 );
  }

  //grid and guide defaults
  mGridResolutionSpinBox->setValue( mSettings->value( QStringLiteral( "/Composer/defaultSnapGridResolution" ), 10.0 ).toDouble() );
  mSnapToleranceSpinBox->setValue( mSettings->value( QStringLiteral( "/Composer/defaultSnapTolerancePixels" ), 5 ).toInt() );
  mOffsetXSpinBox->setValue( mSettings->value( QStringLiteral( "/Composer/defaultSnapGridOffsetX" ), 0 ).toDouble() );
  mOffsetYSpinBox->setValue( mSettings->value( QStringLiteral( "/Composer/defaultSnapGridOffsetY" ), 0 ).toDouble() );

  //
  // Locale settings
  //
  QString mySystemLocale = QLocale::system().name();
  lblSystemLocale->setText( tr( "Detected active locale on your system: %1" ).arg( mySystemLocale ) );
  QString myUserLocale = mSettings->value( QStringLiteral( "locale/userLocale" ), QString() ).toString();
  QStringList myI18nList = i18nList();
  Q_FOREACH ( const QString &l, myI18nList )
  {
    // QTBUG-57802: eo locale is improperly handled
    QString displayName = l.startsWith( QLatin1String( "eo" ) ) ? QLocale::languageToString( QLocale::Esperanto ) : QLocale( l ).nativeLanguageName();
    cboLocale->addItem( QIcon( QString( ":/images/flags/%1.svg" ).arg( l ) ), displayName, l );
  }
  cboLocale->setCurrentIndex( cboLocale->findData( myUserLocale ) );
  bool myLocaleOverrideFlag = mSettings->value( QStringLiteral( "locale/overrideFlag" ), false ).toBool();
  grpLocale->setChecked( myLocaleOverrideFlag );

  //set elements in digitizing tab
  mLineWidthSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/line_width" ), 1 ).toInt() );
  myRed = mSettings->value( QStringLiteral( "/qgis/digitizing/line_color_red" ), 255 ).toInt();
  myGreen = mSettings->value( QStringLiteral( "/qgis/digitizing/line_color_green" ), 0 ).toInt();
  myBlue = mSettings->value( QStringLiteral( "/qgis/digitizing/line_color_blue" ), 0 ).toInt();
  myAlpha = mSettings->value( QStringLiteral( "/qgis/digitizing/line_color_alpha" ), 200 ).toInt();
  mLineColorToolButton->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  mLineColorToolButton->setAllowOpacity( true );
  mLineColorToolButton->setContext( QStringLiteral( "gui" ) );
  mLineColorToolButton->setDefaultColor( QColor( 255, 0, 0, 200 ) );

  myRed = mSettings->value( QStringLiteral( "/qgis/digitizing/fill_color_red" ), 255 ).toInt();
  myGreen = mSettings->value( QStringLiteral( "/qgis/digitizing/fill_color_green" ), 0 ).toInt();
  myBlue = mSettings->value( QStringLiteral( "/qgis/digitizing/fill_color_blue" ), 0 ).toInt();
  myAlpha = mSettings->value( QStringLiteral( "/qgis/digitizing/fill_color_alpha" ), 30 ).toInt();
  mFillColorToolButton->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  mFillColorToolButton->setAllowOpacity( true );
  mFillColorToolButton->setContext( QStringLiteral( "gui" ) );
  mFillColorToolButton->setDefaultColor( QColor( 255, 0, 0, 30 ) );

  mLineGhostCheckBox->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/line_ghost" ), false ).toBool() );

  mDefaultZValueSpinBox->setValue(
    mSettings->value( QStringLiteral( "/qgis/digitizing/default_z_value" ), Qgis::DEFAULT_Z_COORDINATE ).toDouble()
  );

  //default snap mode
  mSnappingEnabledDefault->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ),  false ).toBool() );
  mDefaultSnapModeComboBox->addItem( tr( "Vertex" ), QgsSnappingConfig::Vertex );
  mDefaultSnapModeComboBox->addItem( tr( "Vertex and segment" ), QgsSnappingConfig::VertexAndSegment );
  mDefaultSnapModeComboBox->addItem( tr( "Segment" ), QgsSnappingConfig::Segment );
  mDefaultSnapModeComboBox->setCurrentIndex( mDefaultSnapModeComboBox->findData( mSettings->value( QStringLiteral( "/qgis/digitizing/default_snap_type" ), QgsSnappingConfig::Vertex ).toInt() ) );
  mDefaultSnappingToleranceSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance" ), 0 ).toDouble() );
  mSearchRadiusVertexEditSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit" ), 10 ).toDouble() );
  int defSnapUnits = mSettings->value( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance_unit" ), QgsTolerance::ProjectUnits ).toInt();
  if ( defSnapUnits == QgsTolerance::ProjectUnits || defSnapUnits == QgsTolerance::LayerUnits )
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "pixels" ) );
  }
  mDefaultSnappingToleranceComboBox->setCurrentIndex( index );
  int defRadiusUnits = mSettings->value( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit_unit" ), QgsTolerance::Pixels ).toInt();
  if ( defRadiusUnits == QgsTolerance::ProjectUnits || defRadiusUnits == QgsTolerance::LayerUnits )
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "pixels" ) );
  }
  mSearchRadiusVertexEditComboBox->setCurrentIndex( index );

  mSnappingMarkerColorButton->setColor( mSettings->value( QStringLiteral( "/qgis/digitizing/snap_color" ), QColor( Qt::magenta ) ).value<QColor>() );
  mSnappingTooltipsCheckbox->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/snap_tooltip" ), false ).toBool() );

  //vertex marker
  mMarkersOnlyForSelectedCheckBox->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/marker_only_for_selected" ), true ).toBool() );

  mMarkerStyleComboBox->addItem( tr( "Semi transparent circle" ) );
  mMarkerStyleComboBox->addItem( tr( "Cross" ) );
  mMarkerStyleComboBox->addItem( tr( "None" ) );

  mValidateGeometries->clear();
  mValidateGeometries->addItem( tr( "Off" ) );
  mValidateGeometries->addItem( tr( "QGIS" ) );
  mValidateGeometries->addItem( tr( "GEOS" ) );

  QString markerStyle = mSettings->value( QStringLiteral( "/qgis/digitizing/marker_style" ), "Cross" ).toString();
  if ( markerStyle == QLatin1String( "SemiTransparentCircle" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Semi transparent circle" ) ) );
  }
  else if ( markerStyle == QLatin1String( "Cross" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Cross" ) ) );
  }
  else if ( markerStyle == QLatin1String( "None" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "None" ) ) );
  }
  mMarkerSizeSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/marker_size" ), 3 ).toInt() );

  chkReuseLastValues->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/reuseLastValues" ), false ).toBool() );
  chkDisableAttributeValuesDlg->setChecked( mSettings->value( QStringLiteral( "/qgis/digitizing/disable_enter_attribute_values_dialog" ), false ).toBool() );
  mValidateGeometries->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/digitizing/validate_geometries" ), 1 ).toInt() );

  mSnappingMainDialogComboBox->clear();
  mSnappingMainDialogComboBox->addItem( tr( "Dialog" ), "dialog" );
  mSnappingMainDialogComboBox->addItem( tr( "Dock" ), "dock" );
  mSnappingMainDialogComboBox->setCurrentIndex( mSnappingMainDialogComboBox->findData( mSettings->value( QStringLiteral( "/qgis/mainSnappingWidgetMode" ), "dialog" ).toString() ) );

  mOffsetJoinStyleComboBox->addItem( tr( "Round" ), 0 );
  mOffsetJoinStyleComboBox->addItem( tr( "Miter" ), 1 );
  mOffsetJoinStyleComboBox->addItem( tr( "Bevel" ), 2 );
  mOffsetJoinStyleComboBox->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/digitizing/offset_join_style" ), 0 ).toInt() );
  mOffsetQuadSegSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), 8 ).toInt() );
  mCurveOffsetMiterLimitComboBox->setValue( mSettings->value( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), 5.0 ).toDouble() );

  // load gdal driver list only when gdal tab is first opened
  mLoadedGdalDriverList = false;

  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 0 );
  connect( mAddCustomVarBtn, &QAbstractButton::clicked, this, &QgsOptions::addCustomVariable );
  connect( mRemoveCustomVarBtn, &QAbstractButton::clicked, this, &QgsOptions::removeCustomVariable );

  // locator
  mLocatorOptionsWidget = new QgsLocatorOptionsWidget( QgisApp::instance()->locatorWidget(), this );
  QVBoxLayout *locatorLayout = new QVBoxLayout();
  locatorLayout->addWidget( mLocatorOptionsWidget );
  mOptionsLocatorGroupBox->setLayout( locatorLayout );

  mAdvancedSettingsEditor->setSettingsObject( mSettings );

  Q_FOREACH ( QgsOptionsWidgetFactory *factory, optionsFactories )
  {
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon( factory->icon() );
    item->setText( factory->title() );
    item->setToolTip( factory->title() );

    mOptionsListWidget->addItem( item );

    QgsOptionsPageWidget *page = factory->createWidget( this );
    if ( !page )
      continue;

    mAdditionalOptionWidgets << page;
    mOptionsStackedWidget->addWidget( page );
  }

  connect( pbnEditCreateOptions, &QAbstractButton::pressed, this, &QgsOptions::editCreateOptions );
  connect( pbnEditPyramidsOptions, &QAbstractButton::pressed, this, &QgsOptions::editPyramidsOptions );

  // restore window and widget geometry/state
  connect( mRestoreDefaultWindowStateBtn, &QAbstractButton::clicked, this, &QgsOptions::restoreDefaultWindowState );

  restoreOptionsBaseUi();
}


QgsOptions::~QgsOptions()
{
  delete mSettings;
}

void QgsOptions::setCurrentPage( const QString &pageWidgetName )
{
  //find the page with a matching widget name
  for ( int idx = 0; idx < mOptionsStackedWidget->count(); ++idx )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( idx );
    if ( currentPage->objectName() == pageWidgetName )
    {
      //found the page, set it as current
      mOptionsStackedWidget->setCurrentIndex( idx );
      return;
    }
  }
}

void QgsOptions::mProxyTypeComboBox_currentIndexChanged( int idx )
{
  frameManualProxy->setEnabled( idx != 0 );
}

void QgsOptions::cbxProjectDefaultNew_toggled( bool checked )
{
  if ( checked )
  {
    QString fileName = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_default.qgs" );
    if ( ! QFile::exists( fileName ) )
    {
      QMessageBox::information( nullptr, tr( "Save default project" ), tr( "You must set a default project" ) );
      cbxProjectDefaultNew->setChecked( false );
    }
  }
}

void QgsOptions::setCurrentProjectDefault()
{
  QString fileName = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_default.qgs" );
  if ( QgsProject::instance()->write( fileName ) )
  {
    QMessageBox::information( nullptr, tr( "Save default project" ), tr( "Current project saved as default" ) );
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Save default project" ), tr( "Error saving current project as default" ) );
  }
}

void QgsOptions::resetProjectDefault()
{
  QString fileName = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_default.qgs" );
  if ( QFile::exists( fileName ) )
  {
    QFile::remove( fileName );
  }
  cbxProjectDefaultNew->setChecked( false );
}

void QgsOptions::browseTemplateFolder()
{
  QString newDir = QFileDialog::getExistingDirectory( nullptr, tr( "Choose a directory to store project template files" ),
                   leTemplateFolder->text() );
  if ( ! newDir.isNull() )
  {
    leTemplateFolder->setText( newDir );
  }
}

void QgsOptions::resetTemplateFolder()
{
  leTemplateFolder->setText( QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_templates" ) );
}

void QgsOptions::iconSizeChanged( const QString &iconSize )
{
  QgisApp::instance()->setIconSizes( iconSize.toInt() );
}


void QgsOptions::uiThemeChanged( const QString &theme )
{
  if ( theme == QgsApplication::themeName() )
    return;

  QgsApplication::setUITheme( theme );
}

void QgsOptions::mProjectOnLaunchCmbBx_currentIndexChanged( int indx )
{
  bool specific = ( indx == 2 );
  mProjectOnLaunchLineEdit->setEnabled( specific );
  mProjectOnLaunchPushBtn->setEnabled( specific );
}

void QgsOptions::selectProjectOnLaunch()
{
  // Retrieve last used project dir from persistent settings
  QgsSettings settings;
  QString lastUsedDir = mSettings->value( QStringLiteral( "/UI/lastProjectDir" ), QDir::homePath() ).toString();
  QString projPath = QFileDialog::getOpenFileName( this,
                     tr( "Choose project file to open at launch" ),
                     lastUsedDir,
                     tr( "QGIS files" ) + " (*.qgs *.QGS)" );
  if ( !projPath.isNull() )
  {
    mProjectOnLaunchLineEdit->setText( projPath );
  }
}

void QgsOptions::saveOptions()
{
  QgsSettings settings;

  mSettings->setValue( QStringLiteral( "UI/UITheme" ), cmbUITheme->currentText() );

  // custom environment variables
  mSettings->setValue( QStringLiteral( "qgis/customEnvVarsUse" ), QVariant( mCustomVariablesChkBx->isChecked() ) );
  QStringList customVars;
  for ( int i = 0; i < mCustomVariablesTable->rowCount(); ++i )
  {
    if ( mCustomVariablesTable->item( i, 1 )->text().isEmpty() )
      continue;
    QComboBox *varApplyCmbBx = qobject_cast<QComboBox *>( mCustomVariablesTable->cellWidget( i, 0 ) );
    QString customVar = varApplyCmbBx->currentData().toString();
    customVar += '|';
    customVar += mCustomVariablesTable->item( i, 1 )->text();
    customVar += '=';
    customVar += mCustomVariablesTable->item( i, 2 )->text();
    customVars << customVar;
  }
  mSettings->setValue( QStringLiteral( "qgis/customEnvVars" ), QVariant( customVars ) );

  //search directories for user plugins
  QStringList pathsList;
  for ( int i = 0; i < mListPluginPaths->count(); ++i )
  {
    pathsList << mListPluginPaths->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "plugins/searchPathsForPlugins" ), pathsList );

  //search directories for svgs
  pathsList.clear();
  for ( int i = 0; i < mListSVGPaths->count(); ++i )
  {
    pathsList << mListSVGPaths->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "svg/searchPathsForSVG" ), pathsList );

  pathsList.clear();
  for ( int i = 0; i < mListComposerTemplatePaths->count(); ++i )
  {
    pathsList << mListComposerTemplatePaths->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "composer/searchPathsForTemplates" ), pathsList );

  pathsList.clear();
  for ( int i = 0; i < mListHiddenBrowserPaths->count(); ++i )
  {
    pathsList << mListHiddenBrowserPaths->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "/browser/hiddenPaths" ), pathsList );

  //QGIS help locations
  QStringList helpPaths;
  for ( int i = 0; i < mHelpPathTreeWidget->topLevelItemCount(); ++i )
  {
    if ( QTreeWidgetItem *item = mHelpPathTreeWidget->topLevelItem( i ) )
    {
      helpPaths << item->text( 0 );
    }
  }
  mSettings->setValue( QStringLiteral( "help/helpSearchPath" ), helpPaths );

  //Network timeout
  mSettings->setValue( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), mNetworkTimeoutSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/networkAndProxy/userAgent" ), leUserAgent->text() );

  // WMS capabiltiies expiry time
  mSettings->setValue( QStringLiteral( "/qgis/defaultCapabilitiesExpiry" ), mDefaultCapabilitiesExpirySpinBox->value() );

  // WMS/WMS-C tile expiry time
  mSettings->setValue( QStringLiteral( "/qgis/defaultTileExpiry" ), mDefaultTileExpirySpinBox->value() );

  // WMS/WMS-C default max retry in case of tile request errors
  mSettings->setValue( QStringLiteral( "/qgis/defaultTileMaxRetry" ), mDefaultTileMaxRetrySpinBox->value() );

  // Proxy stored authentication configurations
  mSettings->setValue( QStringLiteral( "proxy/authcfg" ), mAuthSettings->configId( ) );

  //Web proxy settings
  mSettings->setValue( QStringLiteral( "proxy/proxyEnabled" ), grpProxy->isChecked() );
  mSettings->setValue( QStringLiteral( "proxy/proxyHost" ), leProxyHost->text() );
  mSettings->setValue( QStringLiteral( "proxy/proxyPort" ), leProxyPort->text() );
  mSettings->setValue( QStringLiteral( "proxy/proxyUser" ),  mAuthSettings->username() );
  mSettings->setValue( QStringLiteral( "proxy/proxyPassword" ), mAuthSettings->password() );
  mSettings->setValue( QStringLiteral( "proxy/proxyType" ), mProxyTypeComboBox->currentText() );

  if ( !mCacheDirectory->text().isEmpty() )
    mSettings->setValue( QStringLiteral( "cache/directory" ), mCacheDirectory->text() );
  else
    mSettings->remove( QStringLiteral( "cache/directory" ) );

  mSettings->setValue( QStringLiteral( "cache/size" ), QVariant::fromValue( mCacheSize->value() * 1024L ) );

  //url to exclude from proxys
  QString proxyExcludeString;
  for ( int i = 0; i < mExcludeUrlListWidget->count(); ++i )
  {
    if ( i != 0 )
    {
      proxyExcludeString += '|';
    }
    proxyExcludeString += mExcludeUrlListWidget->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "proxy/proxyExcludedUrls" ), proxyExcludeString );

  QgisApp::instance()->namUpdate();

  //wms search url
  mSettings->setValue( QStringLiteral( "/qgis/WMSSearchUrl" ), leWmsSearch->text() );

  //general settings
  mSettings->setValue( QStringLiteral( "/Map/searchRadiusMM" ), spinBoxIdentifyValue->value() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/color" ), mIdentifyHighlightColorButton->color().name() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/colorAlpha" ), mIdentifyHighlightColorButton->color().alpha() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/buffer" ), mIdentifyHighlightBufferSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/minWidth" ), mIdentifyHighlightMinWidthSpinBox->value() );

  bool showLegendClassifiers = mSettings->value( QStringLiteral( "/qgis/showLegendClassifiers" ), false ).toBool();
  mSettings->setValue( QStringLiteral( "/qgis/showLegendClassifiers" ), cbxLegendClassifiers->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/hideSplash" ), cbxHideSplash->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/dataSourceManagerNonModal" ), mDataSourceManagerNonModal->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/checkVersion" ), cbxCheckVersion->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/dockAttributeTable" ), cbxAttributeTableDocked->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/attributeTableBehavior" ), cmbAttrTableBehavior->currentData() );
  mSettings->setValue( QStringLiteral( "/qgis/attributeTableView" ), mAttrTableViewComboBox->currentData() );
  mSettings->setValue( QStringLiteral( "/qgis/attributeTableRowCache" ), spinBoxAttrTableRowCache->value() );
  mSettings->setValue( QStringLiteral( "/qgis/promptForRasterSublayers" ), cmbPromptRasterSublayers->currentIndex() );
  mSettings->setValue( QStringLiteral( "/qgis/scanItemsInBrowser2" ),
                       cmbScanItemsInBrowser->currentData().toString() );
  mSettings->setValue( QStringLiteral( "/qgis/scanZipInBrowser2" ),
                       cmbScanZipInBrowser->currentData().toString() );
  mSettings->setValue( QStringLiteral( "/qgis/ignoreShapeEncoding" ), cbxIgnoreShapeEncoding->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/mainSnappingWidgetMode" ), mSnappingMainDialogComboBox->currentData() );

  mSettings->setValue( QStringLiteral( "/qgis/addPostgisDC" ), cbxAddPostgisDC->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/addOracleDC" ), cbxAddOracleDC->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/compileExpressions" ), cbxCompileExpressions->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/defaultLegendGraphicResolution" ), mLegendGraphicResolutionSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/copyFeatureFormat" ), mComboCopyFeatureFormat->currentData().toInt() );

  mSettings->setValue( QStringLiteral( "/qgis/new_layers_visible" ), chkAddedVisibility->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/enable_anti_aliasing" ), chkAntiAliasing->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/enable_render_caching" ), chkUseRenderCaching->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/parallel_rendering" ), chkParallelRendering->isChecked() );
  int maxThreads = chkMaxThreads->isChecked() ? spinMaxThreads->value() : -1;
  QgsApplication::setMaxThreads( maxThreads );
  mSettings->setValue( QStringLiteral( "/qgis/max_threads" ), maxThreads );

  mSettings->setValue( QStringLiteral( "/qgis/map_update_interval" ), spinMapUpdateInterval->value() );
  mSettings->setValue( QStringLiteral( "/qgis/legendDoubleClickAction" ), cmbLegendDoubleClickAction->currentIndex() );

  // Default simplify drawing configuration
  QgsVectorSimplifyMethod::SimplifyHints simplifyHints = QgsVectorSimplifyMethod::NoSimplification;
  if ( mSimplifyDrawingGroupBox->isChecked() )
  {
    simplifyHints |= QgsVectorSimplifyMethod::GeometrySimplification;
    if ( mSimplifyDrawingSpinBox->value() > 1 ) simplifyHints |= QgsVectorSimplifyMethod::AntialiasingSimplification;
  }
  mSettings->setValue( QStringLiteral( "/qgis/simplifyDrawingHints" ), ( int ) simplifyHints );
  mSettings->setValue( QStringLiteral( "/qgis/simplifyAlgorithm" ), mSimplifyAlgorithmComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/simplifyDrawingTol" ), mSimplifyDrawingSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/simplifyLocal" ), !mSimplifyDrawingAtProvider->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/simplifyMaxScale" ), mSimplifyMaximumScaleComboBox->scale() );

  // magnification
  mSettings->setValue( QStringLiteral( "/qgis/magnifier_factor_default" ), doubleSpinBoxMagnifierDefault->value() / 100 );

  //curve segmentation
  int segmentationType = mToleranceTypeComboBox->currentData().toInt();
  mSettings->setValue( QStringLiteral( "/qgis/segmentationToleranceType" ), segmentationType );
  double segmentationTolerance = mSegmentationToleranceSpinBox->value();
  if ( segmentationType == 0 )
  {
    segmentationTolerance = segmentationTolerance / 180.0 * M_PI; //user sets angle tolerance in degrees, internal classes need value in rad
  }
  mSettings->setValue( QStringLiteral( "/qgis/segmentationTolerance" ), segmentationTolerance );

  // project
  mSettings->setValue( QStringLiteral( "/qgis/projOpenAtLaunch" ), mProjectOnLaunchCmbBx->currentIndex() );
  mSettings->setValue( QStringLiteral( "/qgis/projOpenAtLaunchPath" ), mProjectOnLaunchLineEdit->text() );

  mSettings->setValue( QStringLiteral( "/qgis/askToSaveProjectChanges" ), chbAskToSaveProjectChanges->isChecked() );
  mSettings->setValue( QStringLiteral( "qgis/askToDeleteLayers" ), mLayerDeleteConfirmationChkBx->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/warnOldProjectVersion" ), chbWarnOldProjectVersion->isChecked() );
  if ( ( mSettings->value( QStringLiteral( "/qgis/projectTemplateDir" ) ).toString() != leTemplateFolder->text() ) ||
       ( mSettings->value( QStringLiteral( "/qgis/newProjectDefault" ) ).toBool() != cbxProjectDefaultNew->isChecked() ) )
  {
    mSettings->setValue( QStringLiteral( "/qgis/newProjectDefault" ), cbxProjectDefaultNew->isChecked() );
    mSettings->setValue( QStringLiteral( "/qgis/projectTemplateDir" ), leTemplateFolder->text() );
    QgisApp::instance()->updateProjectFromTemplates();
  }
  mSettings->setValue( QStringLiteral( "/qgis/enableMacros" ), cmbEnableMacros->currentIndex() );

  QgsApplication::setNullRepresentation( leNullValue->text() );
  mSettings->setValue( QStringLiteral( "/qgis/style" ), cmbStyle->currentText() );
  mSettings->setValue( QStringLiteral( "/IconSize" ), cmbIconSize->currentText() );

  mSettings->setValue( QStringLiteral( "/qgis/messageTimeout" ), mMessageTimeoutSpnBx->value() );

  mSettings->setValue( QStringLiteral( "/qgis/native_color_dialogs" ), mNativeColorDialogsChkBx->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/live_color_dialogs" ), mLiveColorDialogsChkBx->isChecked() );

  // rasters settings
  mSettings->setValue( QStringLiteral( "/Raster/defaultRedBand" ), spnRed->value() );
  mSettings->setValue( QStringLiteral( "/Raster/defaultGreenBand" ), spnGreen->value() );
  mSettings->setValue( QStringLiteral( "/Raster/defaultBlueBand" ), spnBlue->value() );

  saveContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, QStringLiteral( "singleBand" ) );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ) );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ) );

  saveMinMaxLimits( cboxContrastEnhancementLimitsSingleBand, QStringLiteral( "singleBand" ) );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandSingleByte, QStringLiteral( "multiBandSingleByte" ) );
  saveMinMaxLimits( cboxContrastEnhancementLimitsMultiBandMultiByte, QStringLiteral( "multiBandMultiByte" ) );

  mSettings->setValue( QStringLiteral( "/Raster/defaultStandardDeviation" ), spnThreeBandStdDev->value() );

  mSettings->setValue( QStringLiteral( "/Raster/cumulativeCutLower" ), mRasterCumulativeCutLowerDoubleSpinBox->value() / 100.0 );
  mSettings->setValue( QStringLiteral( "/Raster/cumulativeCutUpper" ), mRasterCumulativeCutUpperDoubleSpinBox->value() / 100.0 );

  // log rendering events, for userspace debugging
  mSettings->setValue( QStringLiteral( "/Map/logCanvasRefreshEvent" ), mLogCanvasRefreshChkBx->isChecked() );

  //check behavior so default projection when new layer is added with no
  //projection defined...
  if ( radPromptForProjection->isChecked() )
  {
    mSettings->setValue( QStringLiteral( "/Projections/defaultBehavior" ), "prompt" );
  }
  else if ( radUseProjectProjection->isChecked() )
  {
    mSettings->setValue( QStringLiteral( "/Projections/defaultBehavior" ), "useProject" );
  }
  else //assumes radUseGlobalProjection is checked
  {
    mSettings->setValue( QStringLiteral( "/Projections/defaultBehavior" ), "useGlobal" );
  }

  mSettings->setValue( QStringLiteral( "/Projections/layerDefaultCrs" ), mLayerDefaultCrs.authid() );
  mSettings->setValue( QStringLiteral( "/Projections/projectDefaultCrs" ), mDefaultCrs.authid() );

  mSettings->setValue( QStringLiteral( "/Projections/showDatumTransformDialog" ), mShowDatumTransformDialogCheckBox->isChecked() );

  //measurement settings

  QgsUnitTypes::DistanceUnit distanceUnit = static_cast< QgsUnitTypes::DistanceUnit >( mDistanceUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/displayunits" ), QgsUnitTypes::encodeUnit( distanceUnit ) );

  QgsUnitTypes::AreaUnit areaUnit = static_cast< QgsUnitTypes::AreaUnit >( mAreaUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/areaunits" ), QgsUnitTypes::encodeUnit( areaUnit ) );

  QgsUnitTypes::AngleUnit angleUnit = static_cast< QgsUnitTypes::AngleUnit >( mAngleUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/angleunits" ), QgsUnitTypes::encodeUnit( angleUnit ) );

  int decimalPlaces = mDecimalPlacesSpinBox->value();
  mSettings->setValue( QStringLiteral( "/qgis/measure/decimalplaces" ), decimalPlaces );

  bool baseUnit = mKeepBaseUnitCheckBox->isChecked();
  mSettings->setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), baseUnit );

  //set the color for selections
  QColor myColor = pbnSelectionColor->color();
  mSettings->setValue( QStringLiteral( "/qgis/default_selection_color_red" ), myColor.red() );
  mSettings->setValue( QStringLiteral( "/qgis/default_selection_color_green" ), myColor.green() );
  mSettings->setValue( QStringLiteral( "/qgis/default_selection_color_blue" ), myColor.blue() );
  mSettings->setValue( QStringLiteral( "/qgis/default_selection_color_alpha" ), myColor.alpha() );

  //set the default color for canvas background
  myColor = pbnCanvasColor->color();
  mSettings->setValue( QStringLiteral( "/qgis/default_canvas_color_red" ), myColor.red() );
  mSettings->setValue( QStringLiteral( "/qgis/default_canvas_color_green" ), myColor.green() );
  mSettings->setValue( QStringLiteral( "/qgis/default_canvas_color_blue" ), myColor.blue() );

  //set the default color for the measure tool
  myColor = pbnMeasureColor->color();
  mSettings->setValue( QStringLiteral( "/qgis/default_measure_color_red" ), myColor.red() );
  mSettings->setValue( QStringLiteral( "/qgis/default_measure_color_green" ), myColor.green() );
  mSettings->setValue( QStringLiteral( "/qgis/default_measure_color_blue" ), myColor.blue() );

  mSettings->setValue( QStringLiteral( "/qgis/zoom_factor" ), zoomFactorValue() );

  //digitizing
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/line_width" ), mLineWidthSpinBox->value() );
  QColor digitizingColor = mLineColorToolButton->color();
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/line_color_red" ), digitizingColor.red() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/line_color_green" ), digitizingColor.green() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/line_color_blue" ), digitizingColor.blue() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/line_color_alpha" ), digitizingColor.alpha() );

  digitizingColor = mFillColorToolButton->color();
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/fill_color_red" ), digitizingColor.red() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/fill_color_green" ), digitizingColor.green() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/fill_color_blue" ), digitizingColor.blue() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/fill_color_alpha" ), digitizingColor.alpha() );

  settings.setValue( QStringLiteral( "/qgis/digitizing/line_ghost" ), mLineGhostCheckBox->isChecked() );

  mSettings->setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), mDefaultZValueSpinBox->value() );

  //default snap mode
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ), mSnappingEnabledDefault->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/default_snap_type" ), mDefaultSnapModeComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance" ), mDefaultSnappingToleranceSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit" ), mSearchRadiusVertexEditSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/default_snapping_tolerance_unit" ),
                       ( mDefaultSnappingToleranceComboBox->currentIndex() == 0 ? QgsTolerance::ProjectUnits : QgsTolerance::Pixels ) );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/search_radius_vertex_edit_unit" ),
                       ( mSearchRadiusVertexEditComboBox->currentIndex()  == 0 ? QgsTolerance::ProjectUnits : QgsTolerance::Pixels ) );

  mSettings->setValue( QStringLiteral( "/qgis/digitizing/snap_color" ), mSnappingMarkerColorButton->color() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/snap_tooltip" ), mSnappingTooltipsCheckbox->isChecked() );

  mSettings->setValue( QStringLiteral( "/qgis/digitizing/marker_only_for_selected" ), mMarkersOnlyForSelectedCheckBox->isChecked() );

  QString markerComboText = mMarkerStyleComboBox->currentText();
  if ( markerComboText == tr( "Semi transparent circle" ) )
  {
    mSettings->setValue( QStringLiteral( "/qgis/digitizing/marker_style" ), "SemiTransparentCircle" );
  }
  else if ( markerComboText == tr( "Cross" ) )
  {
    mSettings->setValue( QStringLiteral( "/qgis/digitizing/marker_style" ), "Cross" );
  }
  else if ( markerComboText == tr( "None" ) )
  {
    mSettings->setValue( QStringLiteral( "/qgis/digitizing/marker_style" ), "None" );
  }
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/marker_size" ), ( mMarkerSizeSpinBox->value() ) );

  mSettings->setValue( QStringLiteral( "/qgis/digitizing/reuseLastValues" ), chkReuseLastValues->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/disable_enter_attribute_values_dialog" ), chkDisableAttributeValuesDlg->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/validate_geometries" ), mValidateGeometries->currentIndex() );

  mSettings->setValue( QStringLiteral( "/qgis/digitizing/offset_join_style" ), mOffsetJoinStyleComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/offset_quad_seg" ), mOffsetQuadSegSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/digitizing/offset_miter_limit" ), mCurveOffsetMiterLimitComboBox->value() );

  // default scale list
  QString myPaths;
  for ( int i = 0; i < mListGlobalScales->count(); ++i )
  {
    if ( i != 0 )
    {
      myPaths += ',';
    }
    myPaths += mListGlobalScales->item( i )->text();
  }
  mSettings->setValue( QStringLiteral( "Map/scales" ), myPaths );

  //
  // Color palette
  //
  if ( mTreeCustomColors->isDirty() )
  {
    mTreeCustomColors->saveColorsToScheme();
  }

  //
  // Composer settings
  //

  //default font
  QString composerFont = mComposerFontComboBox->currentFont().family();
  mSettings->setValue( QStringLiteral( "/Composer/defaultFont" ), composerFont );

  //grid color
  mSettings->setValue( QStringLiteral( "/Composer/gridRed" ), mGridColorButton->color().red() );
  mSettings->setValue( QStringLiteral( "/Composer/gridGreen" ), mGridColorButton->color().green() );
  mSettings->setValue( QStringLiteral( "/Composer/gridBlue" ), mGridColorButton->color().blue() );
  mSettings->setValue( QStringLiteral( "/Composer/gridAlpha" ), mGridColorButton->color().alpha() );

  //grid style
  if ( mGridStyleComboBox->currentText() == tr( "Solid" ) )
  {
    mSettings->setValue( QStringLiteral( "/Composer/gridStyle" ), "Solid" );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Dots" ) )
  {
    mSettings->setValue( QStringLiteral( "/Composer/gridStyle" ), "Dots" );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Crosses" ) )
  {
    mSettings->setValue( QStringLiteral( "/Composer/gridStyle" ), "Crosses" );
  }

  //grid and guide defaults
  mSettings->setValue( QStringLiteral( "/Composer/defaultSnapGridResolution" ), mGridResolutionSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/Composer/defaultSnapTolerancePixels" ), mSnapToleranceSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/Composer/defaultSnapGridOffsetX" ), mOffsetXSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/Composer/defaultSnapGridOffsetY" ), mOffsetYSpinBox->value() );

  //
  // Locale settings
  //
  mSettings->setValue( QStringLiteral( "locale/userLocale" ), cboLocale->currentData().toString() );
  mSettings->setValue( QStringLiteral( "locale/overrideFlag" ), grpLocale->isChecked() );

  // Gdal skip driver list
  if ( mLoadedGdalDriverList )
    saveGdalDriverList();

  // refresh symbology for any legend items, only if needed
  if ( showLegendClassifiers != cbxLegendClassifiers->isChecked() )
  {
    // TODO[MD] QgisApp::instance()->legend()->updateLegendItemSymbologies();
  }

  //save variables
  QgsExpressionContextUtils::setGlobalVariables( mVariableEditor->variablesInActiveScope() );

  // save app stylesheet last (in case reset becomes necessary)
  if ( mStyleSheetNewOpts != mStyleSheetOldOpts )
  {
    mStyleSheetBuilder->saveToSettings( mStyleSheetNewOpts );
  }

  mDefaultDatumTransformTableWidget->transformContext().writeSettings();

  mLocatorOptionsWidget->commitChanges();

  Q_FOREACH ( QgsOptionsPageWidget *widget, mAdditionalOptionWidgets )
  {
    widget->apply();
  }
}

void QgsOptions::rejectOptions()
{
  // don't reset stylesheet if we don't have to
  if ( mStyleSheetNewOpts != mStyleSheetOldOpts )
  {
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetOldOpts );
  }
}

void QgsOptions::spinFontSize_valueChanged( int fontSize )
{
  mStyleSheetNewOpts.insert( QStringLiteral( "fontPointSize" ), QVariant( fontSize ) );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
}

void QgsOptions::mFontFamilyRadioQt_released()
{
  if ( mStyleSheetNewOpts.value( QStringLiteral( "fontFamily" ) ).toString() != mStyleSheetBuilder->defaultFont().family() )
  {
    mStyleSheetNewOpts.insert( QStringLiteral( "fontFamily" ), QVariant( mStyleSheetBuilder->defaultFont().family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::mFontFamilyRadioCustom_released()
{
  if ( mFontFamilyComboBox->currentFont().family() != mStyleSheetBuilder->defaultFont().family() )
  {
    mStyleSheetNewOpts.insert( QStringLiteral( "fontFamily" ), QVariant( mFontFamilyComboBox->currentFont().family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::mFontFamilyComboBox_currentFontChanged( const QFont &font )
{
  if ( mFontFamilyRadioCustom->isChecked()
       && mStyleSheetNewOpts.value( QStringLiteral( "fontFamily" ) ).toString() != font.family() )
  {
    mStyleSheetNewOpts.insert( QStringLiteral( "fontFamily" ), QVariant( font.family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::useCustomGroupBox( bool chkd )
{
  mStyleSheetNewOpts.insert( QStringLiteral( "groupBoxCustom" ), QVariant( chkd ) );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
}

void QgsOptions::leProjectGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mDefaultCrs = crs;
}

void QgsOptions::leLayerGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mLayerDefaultCrs = crs;
}

void QgsOptions::lstGdalDrivers_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column );
  // edit driver if driver supports write
  if ( item && ( cmbEditCreateOptions->findText( item->text( 0 ) ) != -1 ) )
  {
    editGdalDriver( item->text( 0 ) );
  }
}

void QgsOptions::editCreateOptions()
{
  editGdalDriver( cmbEditCreateOptions->currentText() );
}

void QgsOptions::editPyramidsOptions()
{
  editGdalDriver( QStringLiteral( "_pyramids" ) );
}

void QgsOptions::editGdalDriver( const QString &driverName )
{
  if ( driverName.isEmpty() )
    return;

  QgsDialog dlg( this, nullptr, QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  QVBoxLayout *layout = dlg.layout();
  QString title = tr( "Create Options - %1 Driver" ).arg( driverName );
  if ( driverName == QLatin1String( "_pyramids" ) )
    title = tr( "Create Options - pyramids" );
  dlg.setWindowTitle( title );
  if ( driverName == QLatin1String( "_pyramids" ) )
  {
    QgsRasterPyramidsOptionsWidget *optionsWidget =
      new QgsRasterPyramidsOptionsWidget( &dlg, QStringLiteral( "gdal" ) );
    layout->addWidget( optionsWidget );
    dlg.resize( 400, 400 );
    if ( dlg.exec() == QDialog::Accepted )
      optionsWidget->apply();
  }
  else
  {
    QgsRasterFormatSaveOptionsWidget *optionsWidget =
      new QgsRasterFormatSaveOptionsWidget( &dlg, driverName,
                                            QgsRasterFormatSaveOptionsWidget::Full, QStringLiteral( "gdal" ) );
    layout->addWidget( optionsWidget );
    if ( dlg.exec() == QDialog::Accepted )
      optionsWidget->apply();
  }

}

// Return state of the visibility flag for newly added layers. If

bool QgsOptions::newVisible()
{
  return chkAddedVisibility->isChecked();
}

QStringList QgsOptions::i18nList()
{
  QStringList myList;
  myList << QStringLiteral( "en_US" ); //there is no qm file for this so we add it manually
  QString myI18nPath = QgsApplication::i18nPath();
  QDir myDir( myI18nPath, QStringLiteral( "qgis*.qm" ) );
  QStringList myFileList = myDir.entryList();
  QStringListIterator myIterator( myFileList );
  while ( myIterator.hasNext() )
  {
    QString myFileName = myIterator.next();

    // Ignore the 'en' translation file, already added as 'en_US'.
    if ( myFileName.compare( QLatin1String( "qgis_en.qm" ) ) == 0 ) continue;

    myList << myFileName.remove( QStringLiteral( "qgis_" ) ).remove( QStringLiteral( ".qm" ) );
  }
  return myList;
}

void QgsOptions::restoreDefaultWindowState()
{
  // richard
  if ( QMessageBox::warning( this, tr( "Restore UI Defaults" ), tr( "Are you sure to reset the UI to default (needs restart)?" ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
    return;
  mSettings->setValue( QStringLiteral( "/qgis/restoreDefaultWindowState" ), true );
}

void QgsOptions::mCustomVariablesChkBx_toggled( bool chkd )
{
  mAddCustomVarBtn->setEnabled( chkd );
  mRemoveCustomVarBtn->setEnabled( chkd );
  mCustomVariablesTable->setEnabled( chkd );
}

void QgsOptions::addCustomEnvVarRow( const QString &varName, const QString &varVal, const QString &varApply )
{
  int rowCnt = mCustomVariablesTable->rowCount();
  mCustomVariablesTable->insertRow( rowCnt );

  QComboBox *varApplyCmbBx = new QComboBox( this );
  varApplyCmbBx->addItem( tr( "Overwrite" ), QVariant( "overwrite" ) );
  varApplyCmbBx->addItem( tr( "If Undefined" ), QVariant( "undefined" ) );
  varApplyCmbBx->addItem( tr( "Unset" ), QVariant( "unset" ) );
  varApplyCmbBx->addItem( tr( "Prepend" ), QVariant( "prepend" ) );
  varApplyCmbBx->addItem( tr( "Append" ), QVariant( "append" ) );
  varApplyCmbBx->setCurrentIndex( varApply.isEmpty() ? 0 : varApplyCmbBx->findData( QVariant( varApply ) ) );

  QFont cbf = varApplyCmbBx->font();
  QFontMetrics cbfm = QFontMetrics( cbf );
  cbf.setPointSize( cbf.pointSize() - 2 );
  varApplyCmbBx->setFont( cbf );
  mCustomVariablesTable->setCellWidget( rowCnt, 0, varApplyCmbBx );

  Qt::ItemFlags itmFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                           | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

  QTableWidgetItem *varNameItm = new QTableWidgetItem( varName );
  varNameItm->setFlags( itmFlags );
  mCustomVariablesTable->setItem( rowCnt, 1, varNameItm );

  QTableWidgetItem *varValueItm = new QTableWidgetItem( varVal );
  varNameItm->setFlags( itmFlags );
  mCustomVariablesTable->setItem( rowCnt, 2, varValueItm );

  mCustomVariablesTable->setRowHeight( rowCnt, cbfm.height() + 8 );
}

void QgsOptions::addCustomVariable()
{
  addCustomEnvVarRow( QString(), QString() );
  mCustomVariablesTable->setFocus();
  mCustomVariablesTable->setCurrentCell( mCustomVariablesTable->rowCount() - 1, 1 );
  mCustomVariablesTable->edit( mCustomVariablesTable->currentIndex() );
}

void QgsOptions::removeCustomVariable()
{
  mCustomVariablesTable->removeRow( mCustomVariablesTable->currentRow() );
}

void QgsOptions::mCurrentVariablesQGISChxBx_toggled( bool qgisSpecific )
{
  for ( int i = mCurrentVariablesTable->rowCount() - 1; i >= 0; --i )
  {
    if ( qgisSpecific )
    {
      QString itmTxt = mCurrentVariablesTable->item( i, 0 )->text();
      if ( !itmTxt.startsWith( QLatin1String( "QGIS" ), Qt::CaseInsensitive ) )
        mCurrentVariablesTable->hideRow( i );
    }
    else
    {
      mCurrentVariablesTable->showRow( i );
    }
  }
  if ( mCurrentVariablesTable->rowCount() > 0 )
  {
    mCurrentVariablesTable->sortByColumn( 0, Qt::AscendingOrder );
    mCurrentVariablesTable->resizeColumnToContents( 0 );
  }
}

void QgsOptions::addPluginPath()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListPluginPaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListPluginPaths->addItem( newItem );
    mListPluginPaths->setCurrentItem( newItem );
  }
}

void QgsOptions::removePluginPath()
{
  int currentRow = mListPluginPaths->currentRow();
  QListWidgetItem *itemToRemove = mListPluginPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::addHelpPath()
{
  QTreeWidgetItem *item = new QTreeWidgetItem();
  item->setText( 0, QStringLiteral( "HELP_LOCATION" ) );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mHelpPathTreeWidget->addTopLevelItem( item );
  mHelpPathTreeWidget->setCurrentItem( item );
}

void QgsOptions::removeHelpPath()
{
  QList<QTreeWidgetItem *> items = mHelpPathTreeWidget->selectedItems();
  for ( int i = 0; i < items.size(); ++i )
  {
    int idx = mHelpPathTreeWidget->indexOfTopLevelItem( items.at( i ) );
    if ( idx >= 0 )
    {
      delete mHelpPathTreeWidget->takeTopLevelItem( idx );
    }
  }
}

void QgsOptions::moveHelpPathUp()
{
  QList<QTreeWidgetItem *> selectedItems = mHelpPathTreeWidget->selectedItems();
  QList<QTreeWidgetItem *>::iterator itemIt = selectedItems.begin();
  for ( ; itemIt != selectedItems.end(); ++itemIt )
  {
    int currentIndex = mHelpPathTreeWidget->indexOfTopLevelItem( *itemIt );
    if ( currentIndex > 0 )
    {
      mHelpPathTreeWidget->takeTopLevelItem( currentIndex );
      mHelpPathTreeWidget->insertTopLevelItem( currentIndex - 1, *itemIt );
      mHelpPathTreeWidget->setCurrentItem( *itemIt );
    }
  }
}

void QgsOptions::moveHelpPathDown()
{
  QList<QTreeWidgetItem *> selectedItems = mHelpPathTreeWidget->selectedItems();
  QList<QTreeWidgetItem *>::iterator itemIt = selectedItems.begin();
  for ( ; itemIt != selectedItems.end(); ++itemIt )
  {
    int currentIndex = mHelpPathTreeWidget->indexOfTopLevelItem( *itemIt );
    if ( currentIndex <  mHelpPathTreeWidget->topLevelItemCount() - 1 )
    {
      mHelpPathTreeWidget->takeTopLevelItem( currentIndex );
      mHelpPathTreeWidget->insertTopLevelItem( currentIndex + 1, *itemIt );
      mHelpPathTreeWidget->setCurrentItem( *itemIt );
    }
  }
}

void QgsOptions::addTemplatePath()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListComposerTemplatePaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListComposerTemplatePaths->addItem( newItem );
    mListComposerTemplatePaths->setCurrentItem( newItem );
  }
}

void QgsOptions::removeTemplatePath()
{
  int currentRow = mListComposerTemplatePaths->currentRow();
  QListWidgetItem *itemToRemove = mListComposerTemplatePaths->takeItem( currentRow );
  delete itemToRemove;
}


void QgsOptions::addSVGPath()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListSVGPaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListSVGPaths->addItem( newItem );
    mListSVGPaths->setCurrentItem( newItem );
  }
}

void QgsOptions::removeHiddenPath()
{
  int currentRow = mListHiddenBrowserPaths->currentRow();
  QListWidgetItem *itemToRemove = mListHiddenBrowserPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::removeSVGPath()
{
  int currentRow = mListSVGPaths->currentRow();
  QListWidgetItem *itemToRemove = mListSVGPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::addExcludedUrl()
{
  QListWidgetItem *newItem = new QListWidgetItem( mExcludeUrlListWidget );
  newItem->setText( QStringLiteral( "URL" ) );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mExcludeUrlListWidget->addItem( newItem );
  mExcludeUrlListWidget->setCurrentItem( newItem );
}

void QgsOptions::removeExcludedUrl()
{
  int currentRow = mExcludeUrlListWidget->currentRow();
  QListWidgetItem *itemToRemove = mExcludeUrlListWidget->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::browseCacheDirectory()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( mCacheDirectory->text() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( !myDir.isEmpty() )
  {
    mCacheDirectory->setText( QDir::toNativeSeparators( myDir ) );
  }

}

void QgsOptions::clearCache()
{
  QgsNetworkAccessManager::instance()->cache()->clear();
  QMessageBox::information( this, tr( "Cache cleared" ), tr( "Content cache has been cleared" ) );
}

void QgsOptions::clearAccessCache()
{
  QgsNetworkAccessManager::instance()->clearAccessCache();
  QMessageBox::information( this, tr( "Cache cleared" ), tr( "Connection authentication cache has been cleared" ) );
}

void QgsOptions::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  Q_UNUSED( index );
  // load gdal driver list when gdal tab is first opened
  if ( mOptionsStackedWidget->currentWidget()->objectName() == QLatin1String( "mOptionsPageGDAL" )
       && ! mLoadedGdalDriverList )
  {
    loadGdalDriverList();
  }
}

void QgsOptions::loadGdalDriverList()
{
  QStringList mySkippedDrivers = QgsApplication::skippedGdalDrivers();
  GDALDriverH myGdalDriver; // current driver
  QString myGdalDriverDescription;
  QStringList myDrivers;
  QStringList myGdalWriteDrivers;
  QMap<QString, QString> myDriversFlags, myDriversExt, myDriversLongName;

  // make sure we save list when accept()
  mLoadedGdalDriverList = true;

  // allow retrieving metadata from all drivers, they will be skipped again when saving
  CPLSetConfigOption( "GDAL_SKIP", "" );
  GDALAllRegister();

  int myGdalDriverCount = GDALGetDriverCount();
  for ( int i = 0; i < myGdalDriverCount; ++i )
  {
    myGdalDriver = GDALGetDriver( i );

    Q_CHECK_PTR( myGdalDriver );

    if ( !myGdalDriver )
    {
      QgsLogger::warning( "unable to get driver " + QString::number( i ) );
      continue;
    }

    // in GDAL 2.0 vector and mixed drivers are returned by GDALGetDriver, so filter out non-raster drivers
    // TODO add same UI for vector drivers
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_RASTER, nullptr ) ) != QLatin1String( "YES" ) )
      continue;

    myGdalDriverDescription = GDALGetDescription( myGdalDriver );
    myDrivers << myGdalDriverDescription;

    QgsDebugMsg( QString( "driver #%1 - %2" ).arg( i ).arg( myGdalDriverDescription ) );

    // get driver R/W flags, taken from GDALGeneralCmdLineProcessor()
    const char *pszRWFlag, *pszVirtualIO;
    if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_CREATE, nullptr ) )
    {
      myGdalWriteDrivers << myGdalDriverDescription;
      pszRWFlag = "rw+";
    }
    else if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_CREATECOPY,
                                   nullptr ) )
      pszRWFlag = "rw";
    else
      pszRWFlag = "ro";
    if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_VIRTUALIO, nullptr ) )
      pszVirtualIO = "v";
    else
      pszVirtualIO = "";
    myDriversFlags[myGdalDriverDescription] = QStringLiteral( "%1%2" ).arg( pszRWFlag, pszVirtualIO );

    // get driver extensions and long name
    // the gdal provider can override/add extensions but there is no interface to query this
    // aside from parsing QgsRasterLayer::buildSupportedRasterFileFilter()
    myDriversExt[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_EXTENSION", "" ) ).toLower();
    myDriversLongName[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_LONGNAME", "" ) );

  }
  // restore GDAL_SKIP just in case
  CPLSetConfigOption( "GDAL_SKIP", mySkippedDrivers.join( QStringLiteral( " " ) ).toUtf8() );

  myDrivers.removeDuplicates();
  // myDrivers.sort();
  // sort list case insensitive - no existing function for this!
  QMap<QString, QString> strMap;
  Q_FOREACH ( const QString &str, myDrivers )
    strMap.insert( str.toLower(), str );
  myDrivers = strMap.values();

  Q_FOREACH ( const QString &myName, myDrivers )
  {
    QTreeWidgetItem *mypItem = new QTreeWidgetItem( QStringList( myName ) );
    if ( mySkippedDrivers.contains( myName ) )
    {
      mypItem->setCheckState( 0, Qt::Unchecked );
    }
    else
    {
      mypItem->setCheckState( 0, Qt::Checked );
    }

    // add driver metadata
    mypItem->setText( 1, myDriversExt[myName] );
    QString myFlags = myDriversFlags[myName];
    mypItem->setText( 2, myFlags );
    mypItem->setText( 3, myDriversLongName[myName] );
    lstGdalDrivers->addTopLevelItem( mypItem );
  }
  // adjust column width
  for ( int i = 0; i < 4; i++ )
  {
    lstGdalDrivers->resizeColumnToContents( i );
    lstGdalDrivers->setColumnWidth( i, lstGdalDrivers->columnWidth( i ) + 5 );
  }

  // populate cmbEditCreateOptions with gdal write drivers - sorted, GTiff first
  strMap.clear();
  Q_FOREACH ( const QString &str, myGdalWriteDrivers )
    strMap.insert( str.toLower(), str );
  myGdalWriteDrivers = strMap.values();
  myGdalWriteDrivers.removeAll( QStringLiteral( "Gtiff" ) );
  myGdalWriteDrivers.prepend( QStringLiteral( "GTiff" ) );
  cmbEditCreateOptions->clear();
  Q_FOREACH ( const QString &myName, myGdalWriteDrivers )
  {
    cmbEditCreateOptions->addItem( myName );
  }

}

void QgsOptions::saveGdalDriverList()
{
  for ( int i = 0; i < lstGdalDrivers->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem *mypItem = lstGdalDrivers->topLevelItem( i );
    if ( mypItem->checkState( 0 ) == Qt::Unchecked )
    {
      QgsApplication::skipGdalDriver( mypItem->text( 0 ) );
    }
    else
    {
      QgsApplication::restoreGdalDriver( mypItem->text( 0 ) );
    }
  }
  mSettings->setValue( QStringLiteral( "gdal/skipList" ), QgsApplication::skippedGdalDrivers().join( QStringLiteral( " " ) ) );
}

void QgsOptions::addScale()
{
  int myScale = QInputDialog::getInt(
                  this,
                  tr( "Enter scale" ),
                  tr( "Scale denominator" ),
                  -1,
                  1
                );

  if ( myScale != -1 )
  {
    QListWidgetItem *newItem = addScaleToScaleList( QStringLiteral( "1:%1" ).arg( myScale ) );
    mListGlobalScales->setCurrentItem( newItem );
  }
}

void QgsOptions::removeScale()
{
  int currentRow = mListGlobalScales->currentRow();
  QListWidgetItem *itemToRemove = mListGlobalScales->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::restoreDefaultScaleValues()
{
  mListGlobalScales->clear();

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  Q_FOREACH ( const QString &scale, myScalesList )
  {
    addScaleToScaleList( scale );
  }
}

void QgsOptions::importScales()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load scales" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QString msg;
  QStringList myScales;
  if ( !QgsScaleUtils::loadScaleList( fileName, myScales, msg ) )
  {
    QgsDebugMsg( msg );
  }

  Q_FOREACH ( const QString &scale, myScales )
  {
    addScaleToScaleList( scale );
  }
}

void QgsOptions::exportScales()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save scales" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  if ( !fileName.endsWith( QLatin1String( ".xml" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".xml" );
  }

  QStringList myScales;
  myScales.reserve( mListGlobalScales->count() );
  for ( int i = 0; i < mListGlobalScales->count(); ++i )
  {
    myScales.append( mListGlobalScales->item( i )->text() );
  }

  QString msg;
  if ( !QgsScaleUtils::saveScaleList( fileName, myScales, msg ) )
  {
    QgsDebugMsg( msg );
  }
}

void QgsOptions::initContrastEnhancement( QComboBox *cbox, const QString &name, const QString &defaultVal )
{
  QgsSettings settings;

  //add items to the color enhanceContrast combo boxes
  cbox->addItem( tr( "No Stretch" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::NoEnhancement ) );
  cbox->addItem( tr( "Stretch To MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchToMinimumMaximum ) );
  cbox->addItem( tr( "Stretch And Clip To MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::StretchAndClipToMinimumMaximum ) );
  cbox->addItem( tr( "Clip To MinMax" ),
                 QgsContrastEnhancement::contrastEnhancementAlgorithmString( QgsContrastEnhancement::ClipToMinimumMaximum ) );

  QString contrastEnhancement = mSettings->value( "/Raster/defaultContrastEnhancementAlgorithm/" + name, defaultVal ).toString();
  cbox->setCurrentIndex( cbox->findData( contrastEnhancement ) );
}

void QgsOptions::saveContrastEnhancement( QComboBox *cbox, const QString &name )
{
  QgsSettings settings;
  QString value = cbox->currentData().toString();
  mSettings->setValue( "/Raster/defaultContrastEnhancementAlgorithm/" + name, value );
}

void QgsOptions::initMinMaxLimits( QComboBox *cbox, const QString &name, const QString &defaultVal )
{
  QgsSettings settings;

  //add items to the color limitsContrast combo boxes
  cbox->addItem( tr( "Cumulative pixel count cut" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::CumulativeCut ) );
  cbox->addItem( tr( "Minimum / maximum" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::MinMax ) );
  cbox->addItem( tr( "Mean +/- standard deviation" ),
                 QgsRasterMinMaxOrigin::limitsString( QgsRasterMinMaxOrigin::StdDev ) );

  QString contrastLimits = mSettings->value( "/Raster/defaultContrastEnhancementLimits/" + name, defaultVal ).toString();
  cbox->setCurrentIndex( cbox->findData( contrastLimits ) );
}

void QgsOptions::saveMinMaxLimits( QComboBox *cbox, const QString &name )
{
  QgsSettings settings;
  QString value = cbox->currentData().toString();
  mSettings->setValue( "/Raster/defaultContrastEnhancementLimits/" + name, value );
}

void QgsOptions::addColor()
{
  QColor newColor = QgsColorDialog::getColor( QColor(), this->parentWidget(), tr( "Select color" ), true );
  if ( !newColor.isValid() )
  {
    return;
  }
  activateWindow();

  mTreeCustomColors->addColor( newColor, QgsSymbolLayerUtils::colorToName( newColor ) );
}

QListWidgetItem *QgsOptions::addScaleToScaleList( const QString &newScale )
{
  QListWidgetItem *newItem = new QListWidgetItem( newScale );
  addScaleToScaleList( newItem );
  return newItem;
}

void QgsOptions::addScaleToScaleList( QListWidgetItem *newItem )
{
  // If the new scale already exists, delete it.
  QListWidgetItem *duplicateItem = mListGlobalScales->findItems( newItem->text(), Qt::MatchExactly ).value( 0 );
  delete duplicateItem;

  int newDenominator = newItem->text().split( ':' ).value( 1 ).toInt();
  int i;
  for ( i = 0; i < mListGlobalScales->count(); i++ )
  {
    int denominator = mListGlobalScales->item( i )->text().split( ':' ).value( 1 ).toInt();
    if ( newDenominator > denominator )
      break;
  }

  newItem->setData( Qt::UserRole, newItem->text() );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mListGlobalScales->insertItem( i, newItem );
}

void QgsOptions::scaleItemChanged( QListWidgetItem *changedScaleItem )
{
  // Check if the new value is valid, restore the old value if not.
  QRegExp regExp( "1:0*[1-9]\\d*" );
  if ( regExp.exactMatch( changedScaleItem->text() ) )
  {
    //Remove leading zeroes from the denominator
    regExp.setPattern( QStringLiteral( "1:0*" ) );
    changedScaleItem->setText( changedScaleItem->text().replace( regExp, QStringLiteral( "1:" ) ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Invalid scale" ), tr( "The text you entered is not a valid scale." ) );
    changedScaleItem->setText( changedScaleItem->data( Qt::UserRole ).toString() );
  }

  // Take the changed item out of the list and re-add it. This keeps things ordered and creates correct meta-data for the changed item.
  int row = mListGlobalScales->row( changedScaleItem );
  mListGlobalScales->takeItem( row );
  addScaleToScaleList( changedScaleItem );
  mListGlobalScales->setCurrentItem( changedScaleItem );
}

double QgsOptions::zoomFactorValue()
{
  // Get the decimal value for zoom factor. This function is needed because the zoom factor spin box is shown as a percent value.
  // The minimum zoom factor value is 1.01
  if ( spinZoomFactor->value() == spinZoomFactor->minimum() )
    return 1.01;
  else
    return spinZoomFactor->value() / 100.0;
}

void QgsOptions::setZoomFactorValue()
{
  // Set the percent value for zoom factor spin box. This function is for converting the decimal zoom factor value in the qgis setting to the percent zoom factor value.
  if ( mSettings->value( QStringLiteral( "/qgis/zoom_factor" ), 2 ) <= 1.01 )
  {
    spinZoomFactor->setValue( spinZoomFactor->minimum() );
  }
  else
  {
    int percentValue = mSettings->value( QStringLiteral( "/qgis/zoom_factor" ), 2 ).toDouble() * 100;
    spinZoomFactor->setValue( percentValue );
  }
}

void QgsOptions::showHelp()
{
  QWidget *activeTab = mOptionsStackedWidget->currentWidget();
  QString link;

  // give first priority to created pages which have specified a help key
  for ( const QgsOptionsPageWidget *widget : qgis::as_const( mAdditionalOptionWidgets ) )
  {
    if ( widget == activeTab )
    {
      link = widget->helpKey();
      break;
    }
  }

  if ( link.isEmpty() )
  {
    link = QStringLiteral( "introduction/qgis_configuration.html" );

    if ( activeTab == mOptionsPageAuth )
    {
      link = QStringLiteral( "auth_system/index.html" );
    }
    else if ( activeTab == mOptionsPageVariables )
    {
      link = QStringLiteral( "introduction/general_tools.html#variables" );
    }
    else if ( activeTab == mOptionsPageCRS )
    {
      link = QStringLiteral( "working_with_projections/working_with_projections.html" );
    }
  }
  QgsHelp::openHelp( link );
}
