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
#include "qgsmaprenderer.h"
#include "qgsgenericprojectionselector.h"
#include "qgscoordinatereferencesystem.h"
#include "qgstolerance.h"
#include "qgsscaleutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsdualview.h"
#include "qgscrscache.h"

#include "qgsattributetablefiltermodel.h"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsrasterpyramidsoptionswidget.h"
#include "qgsdialog.h"
#include "qgscomposer.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerv2utils.h"
#include "qgscolordialog.h"
#include "qgsexpressioncontext.h"
#include "qgsunittypes.h"
#include "qgsclipboard.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QSettings>
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

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>
#include <geos_c.h>
#include <cpl_conv.h> // for setting gdal options

#include "qgsconfig.h"

/**
 * \class QgsOptions - Set user options and preferences
 * Constructor
 */
QgsOptions::QgsOptions( QWidget *parent, Qt::WindowFlags fl )
    : QgsOptionsDialogBase( "Options", parent, fl )
    , mSettings( nullptr )
{
  setupUi( this );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  // stylesheet setup
  mStyleSheetBuilder = QgisApp::instance()->styleSheetBuilder();
  mStyleSheetNewOpts = mStyleSheetBuilder->defaultOptions();
  mStyleSheetOldOpts = QMap<QString, QVariant>( mStyleSheetNewOpts );

  connect( mFontFamilyRadioCustom, SIGNAL( toggled( bool ) ), mFontFamilyComboBox, SLOT( setEnabled( bool ) ) );

  connect( cmbIconSize, SIGNAL( activated( const QString& ) ), this, SLOT( iconSizeChanged( const QString& ) ) );
  connect( cmbIconSize, SIGNAL( highlighted( const QString& ) ), this, SLOT( iconSizeChanged( const QString& ) ) );
  connect( cmbIconSize, SIGNAL( editTextChanged( const QString& ) ), this, SLOT( iconSizeChanged( const QString& ) ) );

  connect( this, SIGNAL( accepted() ), this, SLOT( saveOptions() ) );
  connect( this, SIGNAL( rejected() ), this, SLOT( rejectOptions() ) );

  QStringList styles = QStyleFactory::keys();
  cmbStyle->addItems( styles );

  QStringList themes = QgsApplication::uiThemes().keys();
  cmbUITheme->addItems( themes );

  connect( cmbUITheme, SIGNAL( currentIndexChanged( const QString& ) ), this, SLOT( uiThemeChanged( const QString& ) ) );

  mIdentifyHighlightColorButton->setColorDialogTitle( tr( "Identify highlight color" ) );
  mIdentifyHighlightColorButton->setAllowAlpha( true );
  mIdentifyHighlightColorButton->setContext( "gui" );
  mIdentifyHighlightColorButton->setDefaultColor( QGis::DEFAULT_HIGHLIGHT_COLOR );

  mSettings = new QSettings();

  double identifyValue = mSettings->value( "/Map/searchRadiusMM", QGis::DEFAULT_SEARCH_RADIUS_MM ).toDouble();
  QgsDebugMsg( QString( "Standard Identify radius setting read from settings file: %1" ).arg( identifyValue ) );
  if ( identifyValue <= 0.0 )
    identifyValue = QGis::DEFAULT_SEARCH_RADIUS_MM;
  spinBoxIdentifyValue->setMinimum( 0.0 );
  spinBoxIdentifyValue->setValue( identifyValue );
  QColor highlightColor = QColor( mSettings->value( "/Map/highlight/color", QGis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int highlightAlpha = mSettings->value( "/Map/highlight/colorAlpha", QGis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  highlightColor.setAlpha( highlightAlpha );
  mIdentifyHighlightColorButton->setColor( highlightColor );
  double highlightBuffer = mSettings->value( "/Map/highlight/buffer", QGis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  mIdentifyHighlightBufferSpinBox->setValue( highlightBuffer );
  double highlightMinWidth = mSettings->value( "/Map/highlight/minWidth", QGis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();
  mIdentifyHighlightMinWidthSpinBox->setValue( highlightMinWidth );

  // custom environment variables
  bool useCustomVars = mSettings->value( "qgis/customEnvVarsUse", QVariant( false ) ).toBool();
  mCustomVariablesChkBx->setChecked( useCustomVars );
  if ( !useCustomVars )
  {
    mAddCustomVarBtn->setEnabled( false );
    mRemoveCustomVarBtn->setEnabled( false );
    mCustomVariablesTable->setEnabled( false );
  }
  QStringList customVarsList = mSettings->value( "qgis/customEnvVars", "" ).toStringList();
  Q_FOREACH ( const QString &varStr, customVarsList )
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
  QStringList currentVarsList = QProcess::systemEnvironment();

  Q_FOREACH ( const QString &varStr, currentVarsList )
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
      QTableWidgetItem* varNameItm = new QTableWidgetItem( varStrItms.at( i ) );
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
  QString myPaths = mSettings->value( "plugins/searchPathsForPlugins", "" ).toString();
  if ( !myPaths.isEmpty() )
  {
    QStringList myPathList = myPaths.split( '|' );
    QStringList::const_iterator pathIt = myPathList.constBegin();
    for ( ; pathIt != myPathList.constEnd(); ++pathIt )
    {
      QListWidgetItem* newItem = new QListWidgetItem( mListPluginPaths );
      newItem->setText( *pathIt );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mListPluginPaths->addItem( newItem );
    }
  }

  //local directories to search when looking for an SVG with a given basename
  QStringList svgPaths = QgsApplication::svgPaths();
  if ( !svgPaths.isEmpty() )
  {
    Q_FOREACH ( const QString& path, svgPaths )
    {
      QListWidgetItem* newItem = new QListWidgetItem( mListSVGPaths );
      newItem->setText( path );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mListSVGPaths->addItem( newItem );
    }
  }

  QStringList templatePaths = QgsApplication::composerTemplatePaths();
  if ( !templatePaths.isEmpty() )
  {
    Q_FOREACH ( const QString& path, templatePaths )
    {
      QListWidgetItem* newItem = new QListWidgetItem( mListComposerTemplatePaths );
      newItem->setText( path );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mListComposerTemplatePaths->addItem( newItem );
    }
  }

  QStringList hiddenItems = mSettings->value( "/browser/hiddenPaths",
                            QStringList() ).toStringList();
  QStringList::const_iterator pathIt = hiddenItems.constBegin();
  for ( ; pathIt != hiddenItems.constEnd(); ++pathIt )
  {
    QListWidgetItem* newItem = new QListWidgetItem( mListHiddenBrowserPaths );
    newItem->setText( *pathIt );
    mListHiddenBrowserPaths->addItem( newItem );
  }

  //Network timeout
  mNetworkTimeoutSpinBox->setValue( mSettings->value( "/qgis/networkAndProxy/networkTimeout", "60000" ).toInt() );
  leUserAgent->setText( mSettings->value( "/qgis/networkAndProxy/userAgent", "Mozilla/5.0" ).toString() );

  // WMS capabilities expiry time
  mDefaultCapabilitiesExpirySpinBox->setValue( mSettings->value( "/qgis/defaultCapabilitiesExpiry", "24" ).toInt() );

  // WMS/WMS-C tile expiry time
  mDefaultTileExpirySpinBox->setValue( mSettings->value( "/qgis/defaultTileExpiry", "24" ).toInt() );

  // WMS/WMS-C default max retry in case of tile request errors
  mDefaultTileMaxRetrySpinBox->setValue( mSettings->value( "/qgis/defaultTileMaxRetry", "3" ).toInt() );

  //Web proxy settings
  grpProxy->setChecked( mSettings->value( "proxy/proxyEnabled", "0" ).toBool() );
  leProxyHost->setText( mSettings->value( "proxy/proxyHost", "" ).toString() );
  leProxyPort->setText( mSettings->value( "proxy/proxyPort", "" ).toString() );
  leProxyUser->setText( mSettings->value( "proxy/proxyUser", "" ).toString() );
  leProxyPassword->setText( mSettings->value( "proxy/proxyPassword", "" ).toString() );

  //available proxy types
  mProxyTypeComboBox->insertItem( 0, "DefaultProxy" );
  mProxyTypeComboBox->insertItem( 1, "Socks5Proxy" );
  mProxyTypeComboBox->insertItem( 2, "HttpProxy" );
  mProxyTypeComboBox->insertItem( 3, "HttpCachingProxy" );
  mProxyTypeComboBox->insertItem( 4, "FtpCachingProxy" );
  QString settingProxyType = mSettings->value( "proxy/proxyType", "DefaultProxy" ).toString();
  mProxyTypeComboBox->setCurrentIndex( mProxyTypeComboBox->findText( settingProxyType ) );

  //URLs excluded not going through proxies
  QString proxyExcludedURLs = mSettings->value( "proxy/proxyExcludedUrls", "" ).toString();
  if ( !proxyExcludedURLs.isEmpty() )
  {
    QStringList splitUrls = proxyExcludedURLs.split( '|' );
    QStringList::const_iterator urlIt = splitUrls.constBegin();
    for ( ; urlIt != splitUrls.constEnd(); ++urlIt )
    {
      QListWidgetItem* newItem = new QListWidgetItem( mExcludeUrlListWidget );
      newItem->setText( *urlIt );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mExcludeUrlListWidget->addItem( newItem );
    }
  }

  // cache settings
  mCacheDirectory->setText( mSettings->value( "cache/directory" ).toString() );
  mCacheDirectory->setPlaceholderText( QDir( QgsApplication::qgisSettingsDirPath() ).canonicalPath() + QDir::separator() + "cache" );
  mCacheSize->setMinimum( 0 );
  mCacheSize->setMaximum( std::numeric_limits<int>::max() );
  mCacheSize->setSingleStep( 1024 );
  mCacheSize->setValue( mSettings->value( "cache/size" ).toInt() / 1024 );

  //wms search server
  leWmsSearch->setText( mSettings->value( "/qgis/WMSSearchUrl", "http://geopole.org/wms/search?search=%1&type=rss" ).toString() );

  // set the attribute table default filter
  cmbAttrTableBehaviour->clear();
  cmbAttrTableBehaviour->addItem( tr( "Show all features" ), QgsAttributeTableFilterModel::ShowAll );
  cmbAttrTableBehaviour->addItem( tr( "Show selected features" ), QgsAttributeTableFilterModel::ShowSelected );
  cmbAttrTableBehaviour->addItem( tr( "Show features visible on map" ), QgsAttributeTableFilterModel::ShowVisible );
  cmbAttrTableBehaviour->setCurrentIndex( cmbAttrTableBehaviour->findData( mSettings->value( "/qgis/attributeTableBehaviour", QgsAttributeTableFilterModel::ShowAll ).toInt() ) );

  mAttrTableViewComboBox->clear();
  mAttrTableViewComboBox->addItem( tr( "Remember last view" ), -1 );
  mAttrTableViewComboBox->addItem( tr( "Table view" ), QgsDualView::AttributeTable );
  mAttrTableViewComboBox->addItem( tr( "Form view" ), QgsDualView::AttributeEditor );
  mAttrTableViewComboBox->setCurrentIndex( mAttrTableViewComboBox->findData( mSettings->value( "/qgis/attributeTableView", -1 ).toInt() ) );

  spinBoxAttrTableRowCache->setValue( mSettings->value( "/qgis/attributeTableRowCache", 10000 ).toInt() );
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
  cmbPromptRasterSublayers->setCurrentIndex( mSettings->value( "/qgis/promptForRasterSublayers", 0 ).toInt() );

  // Scan for valid items in the browser dock
  cmbScanItemsInBrowser->clear();
  cmbScanItemsInBrowser->addItem( tr( "Check file contents" ), "contents" ); // 0
  cmbScanItemsInBrowser->addItem( tr( "Check extension" ), "extension" );    // 1
  int index = cmbScanItemsInBrowser->findData( mSettings->value( "/qgis/scanItemsInBrowser2", "" ) );
  if ( index == -1 ) index = 1;
  cmbScanItemsInBrowser->setCurrentIndex( index );

  // Scan for contents of compressed files (.zip) in browser dock
  cmbScanZipInBrowser->clear();
  cmbScanZipInBrowser->addItem( tr( "No" ), QVariant( "no" ) );
  // cmbScanZipInBrowser->addItem( tr( "Passthru" ) );     // 1 - removed
  cmbScanZipInBrowser->addItem( tr( "Basic scan" ), QVariant( "basic" ) );
  cmbScanZipInBrowser->addItem( tr( "Full scan" ), QVariant( "full" ) );
  index = cmbScanZipInBrowser->findData( mSettings->value( "/qgis/scanZipInBrowser2", "" ) );
  if ( index == -1 ) index = 1;
  cmbScanZipInBrowser->setCurrentIndex( index );

  // log rendering events, for userspace debugging
  mLogCanvasRefreshChkBx->setChecked( mSettings->value( "/Map/logCanvasRefreshEvent", false ).toBool() );

  //set the default projection behaviour radio buttongs
  if ( mSettings->value( "/Projections/defaultBehaviour", "prompt" ).toString() == "prompt" )
  {
    radPromptForProjection->setChecked( true );
  }
  else if ( mSettings->value( "/Projections/defaultBehaviour", "prompt" ).toString() == "useProject" )
  {
    radUseProjectProjection->setChecked( true );
  }
  else //useGlobal
  {
    radUseGlobalProjection->setChecked( true );
  }
  QString myLayerDefaultCrs = mSettings->value( "/Projections/layerDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString();
  mLayerDefaultCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( myLayerDefaultCrs );
  leLayerGlobalCrs->setCrs( mLayerDefaultCrs );

  //on the fly CRS transformation settings
  //it would be logical to have single settings value but originaly the radio buttons were checkboxes
  if ( mSettings->value( "/Projections/otfTransformAutoEnable", true ).toBool() )
  {
    radOtfAuto->setChecked( true );
  }
  else if ( mSettings->value( "/Projections/otfTransformEnabled", false ).toBool() )
  {
    radOtfTransform->setChecked( true );
  }
  else
  {
    radOtfNone->setChecked( true ); // default
  }

  QString myDefaultCrs = mSettings->value( "/Projections/projectDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString();
  mDefaultCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( myDefaultCrs );
  leProjectGlobalCrs->setCrs( mDefaultCrs );
  leProjectGlobalCrs->setOptionVisible( QgsProjectionSelectionWidget::DefaultCrs, false );

  //default datum transformations
  mSettings->beginGroup( "/Projections" );

  chkShowDatumTransformDialog->setChecked( mSettings->value( "showDatumTransformDialog", false ).toBool() );

  QStringList projectionKeys = mSettings->allKeys();

  //collect src and dest entries that belong together
  QMap< QPair< QString, QString >, QPair< int, int > > transforms;
  QStringList::const_iterator pkeyIt = projectionKeys.constBegin();
  for ( ; pkeyIt != projectionKeys.constEnd(); ++pkeyIt )
  {
    if ( pkeyIt->contains( "srcTransform" ) || pkeyIt->contains( "destTransform" ) )
    {
      QStringList split = pkeyIt->split( '/' );
      QString srcAuthId, destAuthId;
      if ( ! split.isEmpty() )
      {
        srcAuthId = split.at( 0 );
      }
      if ( split.size() > 1 )
      {
        destAuthId = split.at( 1 ).split( '_' ).at( 0 );
      }

      if ( pkeyIt->contains( "srcTransform" ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].first = mSettings->value( *pkeyIt ).toInt();
      }
      else if ( pkeyIt->contains( "destTransform" ) )
      {
        transforms[ qMakePair( srcAuthId, destAuthId )].second = mSettings->value( *pkeyIt ).toInt();
      }
    }
  }
  mSettings->endGroup();

  QMap< QPair< QString, QString >, QPair< int, int > >::const_iterator transformIt = transforms.constBegin();
  for ( ; transformIt != transforms.constEnd(); ++transformIt )
  {
    const QPair< int, int >& v = transformIt.value();
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText( 0, transformIt.key().first );
    item->setText( 1, transformIt.key().second );
    item->setText( 2, QString::number( v.first ) );
    item->setText( 3, QString::number( v.second ) );
    mDefaultDatumTransformTreeWidget->addTopLevelItem( item );
  }

  // Set the units for measuring
  mDistanceUnitsComboBox->addItem( tr( "Meters" ), QGis::Meters );
  mDistanceUnitsComboBox->addItem( tr( "Kilometers" ), QGis::Kilometers );
  mDistanceUnitsComboBox->addItem( tr( "Feet" ), QGis::Feet );
  mDistanceUnitsComboBox->addItem( tr( "Yards" ), QGis::Yards );
  mDistanceUnitsComboBox->addItem( tr( "Miles" ), QGis::Miles );
  mDistanceUnitsComboBox->addItem( tr( "Nautical miles" ), QGis::NauticalMiles );
  mDistanceUnitsComboBox->addItem( tr( "Degrees" ), QGis::Degrees );
  mDistanceUnitsComboBox->addItem( tr( "Map units" ), QGis::UnknownUnit );

  bool ok = false;
  QGis::UnitType distanceUnits = QgsUnitTypes::decodeDistanceUnit( mSettings->value( "/qgis/measure/displayunits" ).toString(), &ok );
  if ( !ok )
    distanceUnits = QGis::Meters;
  mDistanceUnitsComboBox->setCurrentIndex( mDistanceUnitsComboBox->findData( distanceUnits ) );

  mAreaUnitsComboBox->addItem( tr( "Square meters" ), QgsUnitTypes::SquareMeters );
  mAreaUnitsComboBox->addItem( tr( "Square kilometers" ), QgsUnitTypes::SquareKilometers );
  mAreaUnitsComboBox->addItem( tr( "Square feet" ), QgsUnitTypes::SquareFeet );
  mAreaUnitsComboBox->addItem( tr( "Square yards" ), QgsUnitTypes::SquareYards );
  mAreaUnitsComboBox->addItem( tr( "Square miles" ), QgsUnitTypes::SquareMiles );
  mAreaUnitsComboBox->addItem( tr( "Hectares" ), QgsUnitTypes::Hectares );
  mAreaUnitsComboBox->addItem( tr( "Acres" ), QgsUnitTypes::Acres );
  mAreaUnitsComboBox->addItem( tr( "Square nautical miles" ), QgsUnitTypes::SquareNauticalMiles );
  mAreaUnitsComboBox->addItem( tr( "Square degrees" ), QgsUnitTypes::SquareDegrees );
  mAreaUnitsComboBox->addItem( tr( "Map units" ), QgsUnitTypes::UnknownAreaUnit );

  QgsUnitTypes::AreaUnit areaUnits = QgsUnitTypes::decodeAreaUnit( mSettings->value( "/qgis/measure/areaunits" ).toString(), &ok );
  if ( !ok )
    areaUnits = QgsUnitTypes::SquareMeters;
  mAreaUnitsComboBox->setCurrentIndex( mAreaUnitsComboBox->findData( areaUnits ) );

  mAngleUnitsComboBox->addItem( tr( "Degrees" ), QgsUnitTypes::AngleDegrees );
  mAngleUnitsComboBox->addItem( tr( "Radians" ), QgsUnitTypes::Radians );
  mAngleUnitsComboBox->addItem( tr( "Gon/gradians" ), QgsUnitTypes::Gon );
  mAngleUnitsComboBox->addItem( tr( "Minutes of arc" ), QgsUnitTypes::MinutesOfArc );
  mAngleUnitsComboBox->addItem( tr( "Seconds of arc" ), QgsUnitTypes::SecondsOfArc );
  mAngleUnitsComboBox->addItem( tr( "Turns/revolutions" ), QgsUnitTypes::Turn );

  QgsUnitTypes::AngleUnit unit = QgsUnitTypes::decodeAngleUnit( mSettings->value( "/qgis/measure/angleunits", QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleDegrees ) ).toString() );
  mAngleUnitsComboBox->setCurrentIndex( mAngleUnitsComboBox->findData( unit ) );

  // set decimal places of the measure tool
  int decimalPlaces = mSettings->value( "/qgis/measure/decimalplaces", "3" ).toInt();
  mDecimalPlacesSpinBox->setRange( 0, 12 );
  mDecimalPlacesSpinBox->setValue( decimalPlaces );

  // set if base unit of measure tool should be changed
  bool baseUnit = mSettings->value( "qgis/measure/keepbaseunit", false ).toBool();
  if ( baseUnit )
  {
    mKeepBaseUnitCheckBox->setChecked( true );
  }
  else
  {
    mKeepBaseUnitCheckBox->setChecked( false );
  }

  cmbIconSize->setCurrentIndex( cmbIconSize->findText( mSettings->value( "/IconSize", QGIS_ICON_SIZE ).toString() ) );

  // set font size and family
  spinFontSize->blockSignals( true );
  mFontFamilyRadioQt->blockSignals( true );
  mFontFamilyRadioCustom->blockSignals( true );
  mFontFamilyComboBox->blockSignals( true );

  spinFontSize->setValue( mStyleSheetOldOpts.value( "fontPointSize" ).toInt() );
  QString fontFamily = mStyleSheetOldOpts.value( "fontFamily" ).toString();
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
  mCustomGroupBoxChkBx->setChecked( mStyleSheetOldOpts.value( "groupBoxCustom" ).toBool() );

  mMessageTimeoutSpnBx->setValue( mSettings->value( "/qgis/messageTimeout", 5 ).toInt() );

  QString name = mSettings->value( "/qgis/style" ).toString();
  cmbStyle->setCurrentIndex( cmbStyle->findText( name, Qt::MatchFixedString ) );

  QString theme = QgsApplication::themeName();
  cmbUITheme->setCurrentIndex( cmbUITheme->findText( theme, Qt::MatchFixedString ) );

  mNativeColorDialogsChkBx->setChecked( mSettings->value( "/qgis/native_color_dialogs", false ).toBool() );
  mLiveColorDialogsChkBx->setChecked( mSettings->value( "/qgis/live_color_dialogs", false ).toBool() );

  //set the state of the checkboxes
  //Changed to default to true as of QGIS 1.7
  chkAntiAliasing->setChecked( mSettings->value( "/qgis/enable_anti_aliasing", true ).toBool() );
  chkUseRenderCaching->setChecked( mSettings->value( "/qgis/enable_render_caching", true ).toBool() );
  chkParallelRendering->setChecked( mSettings->value( "/qgis/parallel_rendering", false ).toBool() );
  spinMapUpdateInterval->setValue( mSettings->value( "/qgis/map_update_interval", 250 ).toInt() );
  chkMaxThreads->setChecked( QgsApplication::maxThreads() != -1 );
  spinMaxThreads->setEnabled( chkMaxThreads->isChecked() );
  spinMaxThreads->setRange( 1, QThread::idealThreadCount() );
  spinMaxThreads->setValue( QgsApplication::maxThreads() );

  // Default simplify drawing configuration
  mSimplifyDrawingGroupBox->setChecked( mSettings->value( "/qgis/simplifyDrawingHints", ( int )QgsVectorSimplifyMethod::GeometrySimplification ).toInt() != QgsVectorSimplifyMethod::NoSimplification );
  mSimplifyDrawingSpinBox->setValue( mSettings->value( "/qgis/simplifyDrawingTol", QGis::DEFAULT_MAPTOPIXEL_THRESHOLD ).toFloat() );
  mSimplifyDrawingAtProvider->setChecked( !mSettings->value( "/qgis/simplifyLocal", true ).toBool() );

  //segmentation tolerance type
  mToleranceTypeComboBox->addItem( tr( "Maximum angle" ), 0 );
  mToleranceTypeComboBox->addItem( tr( "Maximum difference" ), 1 );
  int toleranceType = mSettings->value( "/qgis/segmentationToleranceType", "0" ).toInt();
  int toleranceTypeIndex = mToleranceTypeComboBox->findData( toleranceType );
  if ( toleranceTypeIndex != -1 )
  {
    mToleranceTypeComboBox->setCurrentIndex( toleranceTypeIndex );
  }

  double tolerance = mSettings->value( "/qgis/segmentationTolerance", "0.01745" ).toDouble();
  if ( toleranceType == 0 )
  {
    tolerance = tolerance * 180.0 / M_PI; //value shown to the user is degree, not rad
  }
  mSegmentationToleranceSpinBox->setValue( tolerance );

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  myScalesList.append( "1:1" );
  mSimplifyMaximumScaleComboBox->updateScales( myScalesList );
  mSimplifyMaximumScaleComboBox->setScale( 1.0 / mSettings->value( "/qgis/simplifyMaxScale", 1 ).toFloat() );

  // Magnifier
  double magnifierMin = 100 * mSettings->value( "/qgis/magnifier_factor_min", 0.1 ).toDouble();
  double magnifierMax = 100 * mSettings->value( "/qgis/magnifier_factor_max", 10 ).toDouble();
  double magnifierVal = 100 * mSettings->value( "/qgis/magnifier_factor_default", 1.0 ).toDouble();
  doubleSpinBoxMagnifierDefault->setRange( magnifierMin, magnifierMax );
  doubleSpinBoxMagnifierDefault->setSingleStep( 50 );
  doubleSpinBoxMagnifierDefault->setDecimals( 0 );
  doubleSpinBoxMagnifierDefault->setSuffix( "%" );
  doubleSpinBoxMagnifierDefault->setValue( magnifierVal );

  // Default local simplification algorithm
  mSimplifyAlgorithmComboBox->addItem( tr( "Distance" ), ( int )QgsVectorSimplifyMethod::Distance );
  mSimplifyAlgorithmComboBox->addItem( tr( "SnapToGrid" ), ( int )QgsVectorSimplifyMethod::SnapToGrid );
  mSimplifyAlgorithmComboBox->addItem( tr( "Visvalingam" ), ( int )QgsVectorSimplifyMethod::Visvalingam );
  mSimplifyAlgorithmComboBox->setCurrentIndex( mSimplifyAlgorithmComboBox->findData( mSettings->value( "/qgis/simplifyAlgorithm", 0 ).toInt() ) );

  // Slightly awkard here at the settings value is true to use QImage,
  // but the checkbox is true to use QPixmap
  chkAddedVisibility->setChecked( mSettings->value( "/qgis/new_layers_visible", true ).toBool() );
  cbxLegendClassifiers->setChecked( mSettings->value( "/qgis/showLegendClassifiers", false ).toBool() );
  mLegendLayersBoldChkBx->setChecked( mSettings->value( "/qgis/legendLayersBold", true ).toBool() );
  mLegendGroupsBoldChkBx->setChecked( mSettings->value( "/qgis/legendGroupsBold", false ).toBool() );
  cbxHideSplash->setChecked( mSettings->value( "/qgis/hideSplash", false ).toBool() );
  cbxShowTips->setChecked( mSettings->value( QString( "/qgis/showTips%1" ).arg( QGis::QGIS_VERSION_INT / 100 ), true ).toBool() );
  cbxCheckVersion->setChecked( mSettings->value( "/qgis/checkVersion", true ).toBool() );
  cbxAttributeTableDocked->setChecked( mSettings->value( "/qgis/dockAttributeTable", false ).toBool() );
  cbxSnappingOptionsDocked->setChecked( mSettings->value( "/qgis/dockSnapping", false ).toBool() );
  cbxAddPostgisDC->setChecked( mSettings->value( "/qgis/addPostgisDC", false ).toBool() );
  cbxAddOracleDC->setChecked( mSettings->value( "/qgis/addOracleDC", false ).toBool() );
  cbxCompileExpressions->setChecked( mSettings->value( "/qgis/compileExpressions", true ).toBool() );
  cbxCreateRasterLegendIcons->setChecked( mSettings->value( "/qgis/createRasterLegendIcons", false ).toBool() );

  mComboCopyFeatureFormat->addItem( tr( "Plain text, no geometry" ), QgsClipboard::AttributesOnly );
  mComboCopyFeatureFormat->addItem( tr( "Plain text, WKT geometry" ), QgsClipboard::AttributesWithWKT );
  mComboCopyFeatureFormat->addItem( tr( "GeoJSON" ), QgsClipboard::GeoJSON );
  if ( mSettings->contains( "/qgis/copyFeatureFormat" ) )
    mComboCopyFeatureFormat->setCurrentIndex( mComboCopyFeatureFormat->findData( mSettings->value( "/qgis/copyFeatureFormat", true ).toInt() ) );
  else
    mComboCopyFeatureFormat->setCurrentIndex( mComboCopyFeatureFormat->findData( mSettings->value( "/qgis/copyGeometryAsWKT", true ).toBool() ?
        QgsClipboard::AttributesWithWKT : QgsClipboard::AttributesOnly ) );
  leNullValue->setText( mSettings->value( "qgis/nullValue", "NULL" ).toString() );
  cbxIgnoreShapeEncoding->setChecked( mSettings->value( "/qgis/ignoreShapeEncoding", true ).toBool() );
  cbxCanvasRotation->setChecked( QgsMapCanvas::rotationEnabled() );

  cmbLegendDoubleClickAction->setCurrentIndex( mSettings->value( "/qgis/legendDoubleClickAction", 0 ).toInt() );

  // WMS getLegendGraphic setting
  mLegendGraphicResolutionSpinBox->setValue( mSettings->value( "/qgis/defaultLegendGraphicResolution", 0 ).toInt() );

  //
  // Raster properties
  //
  spnRed->setValue( mSettings->value( "/Raster/defaultRedBand", 1 ).toInt() );
  spnGreen->setValue( mSettings->value( "/Raster/defaultGreenBand", 2 ).toInt() );
  spnBlue->setValue( mSettings->value( "/Raster/defaultBlueBand", 3 ).toInt() );

  initContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, "singleBand", "StretchToMinimumMaximum" );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, "multiBandSingleByte", "NoEnhancement" );
  initContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, "multiBandMultiByte", "StretchToMinimumMaximum" );

  cboxContrastEnhancementLimits->addItem( tr( "Cumulative pixel count cut" ), "CumulativeCut" );
  cboxContrastEnhancementLimits->addItem( tr( "Minimum / maximum" ), "MinMax" );
  cboxContrastEnhancementLimits->addItem( tr( "Mean +/- standard deviation" ), "StdDev" );

  QString contrastEnchacementLimits = mSettings->value( "/Raster/defaultContrastEnhancementLimits", "CumulativeCut" ).toString();

  cboxContrastEnhancementLimits->setCurrentIndex( cboxContrastEnhancementLimits->findData( contrastEnchacementLimits ) );

  spnThreeBandStdDev->setValue( mSettings->value( "/Raster/defaultStandardDeviation", 2.0 ).toDouble() );

  mRasterCumulativeCutLowerDoubleSpinBox->setValue( 100.0 * mSettings->value( "/Raster/cumulativeCutLower", QString::number( QgsRasterLayer::CUMULATIVE_CUT_LOWER ) ).toDouble() );
  mRasterCumulativeCutUpperDoubleSpinBox->setValue( 100.0 * mSettings->value( "/Raster/cumulativeCutUpper", QString::number( QgsRasterLayer::CUMULATIVE_CUT_UPPER ) ).toDouble() );

  //set the color for selections
  int myRed = mSettings->value( "/qgis/default_selection_color_red", 255 ).toInt();
  int myGreen = mSettings->value( "/qgis/default_selection_color_green", 255 ).toInt();
  int myBlue = mSettings->value( "/qgis/default_selection_color_blue", 0 ).toInt();
  int myAlpha = mSettings->value( "/qgis/default_selection_color_alpha", 255 ).toInt();
  pbnSelectionColor->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  pbnSelectionColor->setColorDialogTitle( tr( "Set selection color" ) );
  pbnSelectionColor->setAllowAlpha( true );
  pbnSelectionColor->setContext( "gui" );
  pbnSelectionColor->setDefaultColor( QColor( 255, 255, 0, 255 ) );

  //set the default color for canvas background
  myRed = mSettings->value( "/qgis/default_canvas_color_red", 255 ).toInt();
  myGreen = mSettings->value( "/qgis/default_canvas_color_green", 255 ).toInt();
  myBlue = mSettings->value( "/qgis/default_canvas_color_blue", 255 ).toInt();
  pbnCanvasColor->setColor( QColor( myRed, myGreen, myBlue ) );
  pbnCanvasColor->setColorDialogTitle( tr( "Set canvas color" ) );
  pbnCanvasColor->setContext( "gui" );
  pbnCanvasColor->setDefaultColor( Qt::white );

  // set the default color for the measure tool
  myRed = mSettings->value( "/qgis/default_measure_color_red", 222 ).toInt();
  myGreen = mSettings->value( "/qgis/default_measure_color_green", 155 ).toInt();
  myBlue = mSettings->value( "/qgis/default_measure_color_blue", 67 ).toInt();
  pbnMeasureColor->setColor( QColor( myRed, myGreen, myBlue ) );
  pbnMeasureColor->setColorDialogTitle( tr( "Set measuring tool color" ) );
  pbnMeasureColor->setContext( "gui" );
  pbnMeasureColor->setDefaultColor( QColor( 222, 155, 67 ) );

  capitaliseCheckBox->setChecked( mSettings->value( "/qgis/capitaliseLayerName", QVariant( false ) ).toBool() );

  int projOpen = mSettings->value( "/qgis/projOpenAtLaunch", 0 ).toInt();
  mProjectOnLaunchCmbBx->setCurrentIndex( projOpen );
  mProjectOnLaunchLineEdit->setText( mSettings->value( "/qgis/projOpenAtLaunchPath" ).toString() );
  mProjectOnLaunchLineEdit->setEnabled( projOpen == 2 );
  mProjectOnLaunchPushBtn->setEnabled( projOpen == 2 );

  chbAskToSaveProjectChanges->setChecked( mSettings->value( "qgis/askToSaveProjectChanges", QVariant( true ) ).toBool() );
  mLayerDeleteConfirmationChkBx->setChecked( mSettings->value( "qgis/askToDeleteLayers", true ).toBool() );
  chbWarnOldProjectVersion->setChecked( mSettings->value( "/qgis/warnOldProjectVersion", QVariant( true ) ).toBool() );
  cmbEnableMacros->setCurrentIndex( mSettings->value( "/qgis/enableMacros", 1 ).toInt() );

  // templates
  cbxProjectDefaultNew->setChecked( mSettings->value( "/qgis/newProjectDefault", QVariant( false ) ).toBool() );
  QString templateDirName = mSettings->value( "/qgis/projectTemplateDir",
                            QgsApplication::qgisSettingsDirPath() + "project_templates" ).toString();
  // make dir if it doesn't exists - should just be called once
  QDir templateDir;
  if ( ! templateDir.exists( templateDirName ) )
  {
    templateDir.mkdir( templateDirName );
  }
  leTemplateFolder->setText( templateDirName );

  spinZoomFactor->setValue( mSettings->value( "/qgis/zoom_factor", 2 ).toDouble() );

  // predefined scales for scale combobox
  myPaths = mSettings->value( "Map/scales", PROJECT_SCALES ).toString();
  if ( !myPaths.isEmpty() )
  {
    QStringList myScalesList = myPaths.split( ',' );
    Q_FOREACH ( const QString& scale, myScalesList )
    {
      addScaleToScaleList( scale );
    }
  }
  connect( mListGlobalScales, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT( scaleItemChanged( QListWidgetItem* ) ) );

  //
  // Color palette
  //
  connect( mButtonCopyColors, SIGNAL( clicked() ), mTreeCustomColors, SLOT( copyColors() ) );
  connect( mButtonRemoveColor, SIGNAL( clicked() ), mTreeCustomColors, SLOT( removeSelection() ) );
  connect( mButtonPasteColors, SIGNAL( clicked() ), mTreeCustomColors, SLOT( pasteColors() ) );

  //find custom color scheme from registry
  QList<QgsCustomColorScheme *> customSchemes;
  QgsColorSchemeRegistry::instance()->schemes( customSchemes );
  if ( customSchemes.length() > 0 )
  {
    mTreeCustomColors->setScheme( customSchemes.at( 0 ) );
  }

  //
  // Composer settings
  //

  //default composer font
  mComposerFontComboBox->blockSignals( true );

  QString composerFontFamily = mSettings->value( "/Composer/defaultFont" ).toString();

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
  gridRed = mSettings->value( "/Composer/gridRed", 190 ).toInt();
  gridGreen = mSettings->value( "/Composer/gridGreen", 190 ).toInt();
  gridBlue = mSettings->value( "/Composer/gridBlue", 190 ).toInt();
  gridAlpha = mSettings->value( "/Composer/gridAlpha", 100 ).toInt();
  QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );
  mGridColorButton->setColor( gridColor );
  mGridColorButton->setColorDialogTitle( tr( "Select grid color" ) );
  mGridColorButton->setAllowAlpha( true );
  mGridColorButton->setContext( "gui" );
  mGridColorButton->setDefaultColor( QColor( 190, 190, 190, 100 ) );

  //default composer grid style
  QString gridStyleString;
  gridStyleString = mSettings->value( "/Composer/gridStyle", "Dots" ).toString();
  mGridStyleComboBox->insertItem( 0, tr( "Solid" ) );
  mGridStyleComboBox->insertItem( 1, tr( "Dots" ) );
  mGridStyleComboBox->insertItem( 2, tr( "Crosses" ) );
  if ( gridStyleString == "Solid" )
  {
    mGridStyleComboBox->setCurrentIndex( 0 );
  }
  else if ( gridStyleString == "Crosses" )
  {
    mGridStyleComboBox->setCurrentIndex( 2 );
  }
  else
  {
    //default grid is dots
    mGridStyleComboBox->setCurrentIndex( 1 );
  }

  //grid and guide defaults
  mGridResolutionSpinBox->setValue( mSettings->value( "/Composer/defaultSnapGridResolution", 10.0 ).toDouble() );
  mSnapToleranceSpinBox->setValue( mSettings->value( "/Composer/defaultSnapTolerancePixels", 5 ).toInt() );
  mOffsetXSpinBox->setValue( mSettings->value( "/Composer/defaultSnapGridOffsetX", 0 ).toDouble() );
  mOffsetYSpinBox->setValue( mSettings->value( "/Composer/defaultSnapGridOffsetY", 0 ).toDouble() );

  //
  // Locale settings
  //
  QString mySystemLocale = QLocale::system().name();
  lblSystemLocale->setText( tr( "Detected active locale on your system: %1" ).arg( mySystemLocale ) );
  QString myUserLocale = mSettings->value( "locale/userLocale", "" ).toString();
  QStringList myI18nList = i18nList();
  Q_FOREACH ( const QString& l, myI18nList )
  {
    cboLocale->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( l ) ), QLocale( l ).nativeLanguageName(), l );
  }
  cboLocale->setCurrentIndex( cboLocale->findData( myUserLocale ) );
  bool myLocaleOverrideFlag = mSettings->value( "locale/overrideFlag", false ).toBool();
  grpLocale->setChecked( myLocaleOverrideFlag );

  //set elements in digitizing tab
  mLineWidthSpinBox->setValue( mSettings->value( "/qgis/digitizing/line_width", 1 ).toInt() );
  myRed = mSettings->value( "/qgis/digitizing/line_color_red", 255 ).toInt();
  myGreen = mSettings->value( "/qgis/digitizing/line_color_green", 0 ).toInt();
  myBlue = mSettings->value( "/qgis/digitizing/line_color_blue", 0 ).toInt();
  myAlpha = mSettings->value( "/qgis/digitizing/line_color_alpha", 200 ).toInt();
  mLineColorToolButton->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  mLineColorToolButton->setAllowAlpha( true );
  mLineColorToolButton->setContext( "gui" );
  mLineColorToolButton->setDefaultColor( QColor( 255, 0, 0, 200 ) );

  myRed = mSettings->value( "/qgis/digitizing/fill_color_red", 255 ).toInt();
  myGreen = mSettings->value( "/qgis/digitizing/fill_color_green", 0 ).toInt();
  myBlue = mSettings->value( "/qgis/digitizing/fill_color_blue", 0 ).toInt();
  myAlpha = mSettings->value( "/qgis/digitizing/fill_color_alpha", 30 ).toInt();
  mFillColorToolButton->setColor( QColor( myRed, myGreen, myBlue, myAlpha ) );
  mFillColorToolButton->setAllowAlpha( true );
  mFillColorToolButton->setContext( "gui" );
  mFillColorToolButton->setDefaultColor( QColor( 255, 0, 0, 30 ) );

  mLineGhostCheckBox->setChecked( mSettings->value( "/qgis/digitizing/line_ghost", false ).toBool() );

  //default snap mode
  mDefaultSnapModeComboBox->insertItem( 0, tr( "To vertex" ), "to vertex" );
  mDefaultSnapModeComboBox->insertItem( 1, tr( "To segment" ), "to segment" );
  mDefaultSnapModeComboBox->insertItem( 2, tr( "To vertex and segment" ), "to vertex and segment" );
  mDefaultSnapModeComboBox->insertItem( 3, tr( "Off" ), "off" );
  QString defaultSnapString = mSettings->value( "/qgis/digitizing/default_snap_mode", "off" ).toString();
  mDefaultSnapModeComboBox->setCurrentIndex( mDefaultSnapModeComboBox->findData( defaultSnapString ) );
  mDefaultSnappingToleranceSpinBox->setValue( mSettings->value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble() );
  mSearchRadiusVertexEditSpinBox->setValue( mSettings->value( "/qgis/digitizing/search_radius_vertex_edit", 10 ).toDouble() );
  int defSnapUnits = mSettings->value( "/qgis/digitizing/default_snapping_tolerance_unit", QgsTolerance::ProjectUnits ).toInt();
  if ( defSnapUnits == QgsTolerance::ProjectUnits || defSnapUnits == QgsTolerance::LayerUnits )
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "pixels" ) );
  }
  mDefaultSnappingToleranceComboBox->setCurrentIndex( index );
  int defRadiusUnits = mSettings->value( "/qgis/digitizing/search_radius_vertex_edit_unit", QgsTolerance::Pixels ).toInt();
  if ( defRadiusUnits == QgsTolerance::ProjectUnits || defRadiusUnits == QgsTolerance::LayerUnits )
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "pixels" ) );
  }
  mSearchRadiusVertexEditComboBox->setCurrentIndex( index );

  //vertex marker
  mMarkersOnlyForSelectedCheckBox->setChecked( mSettings->value( "/qgis/digitizing/marker_only_for_selected", false ).toBool() );

  mMarkerStyleComboBox->addItem( tr( "Semi transparent circle" ) );
  mMarkerStyleComboBox->addItem( tr( "Cross" ) );
  mMarkerStyleComboBox->addItem( tr( "None" ) );

  mValidateGeometries->clear();
  mValidateGeometries->addItem( tr( "Off" ) );
  mValidateGeometries->addItem( tr( "QGIS" ) );
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ( (GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR>=3) || GEOS_VERSION_MAJOR>3)
  mValidateGeometries->addItem( tr( "GEOS" ) );
#endif

  QString markerStyle = mSettings->value( "/qgis/digitizing/marker_style", "Cross" ).toString();
  if ( markerStyle == "SemiTransparentCircle" )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Semi transparent circle" ) ) );
  }
  else if ( markerStyle == "Cross" )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Cross" ) ) );
  }
  else if ( markerStyle == "None" )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "None" ) ) );
  }
  mMarkerSizeSpinBox->setValue( mSettings->value( "/qgis/digitizing/marker_size", 3 ).toInt() );

  chkReuseLastValues->setChecked( mSettings->value( "/qgis/digitizing/reuseLastValues", false ).toBool() );
  chkDisableAttributeValuesDlg->setChecked( mSettings->value( "/qgis/digitizing/disable_enter_attribute_values_dialog", false ).toBool() );
  mValidateGeometries->setCurrentIndex( mSettings->value( "/qgis/digitizing/validate_geometries", 1 ).toInt() );

  mOffsetJoinStyleComboBox->addItem( tr( "Round" ), 0 );
  mOffsetJoinStyleComboBox->addItem( tr( "Mitre" ), 1 );
  mOffsetJoinStyleComboBox->addItem( tr( "Bevel" ), 2 );
  mOffsetJoinStyleComboBox->setCurrentIndex( mSettings->value( "/qgis/digitizing/offset_join_style", 0 ).toInt() );
  mOffsetQuadSegSpinBox->setValue( mSettings->value( "/qgis/digitizing/offset_quad_seg", 8 ).toInt() );
  mCurveOffsetMiterLimitComboBox->setValue( mSettings->value( "/qgis/digitizing/offset_miter_limit", 5.0 ).toDouble() );

  // load gdal driver list only when gdal tab is first opened
  mLoadedGdalDriverList = false;

  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 0 );



  mAdvancedSettingsEditor->setSettingsObject( mSettings );

  // restore window and widget geometry/state
  restoreOptionsBaseUi();
}

//! Destructor
QgsOptions::~QgsOptions()
{
  delete mSettings;
}

void QgsOptions::setCurrentPage( const QString& pageWidgetName )
{
  //find the page with a matching widget name
  for ( int idx = 0; idx < mOptionsStackedWidget->count(); ++idx )
  {
    QWidget* currentPage = mOptionsStackedWidget->widget( idx );
    if ( currentPage->objectName() == pageWidgetName )
    {
      //found the page, set it as current
      mOptionsStackedWidget->setCurrentIndex( idx );
      return;
    }
  }
}

void QgsOptions::on_mProxyTypeComboBox_currentIndexChanged( int idx )
{
  frameManualProxy->setEnabled( idx != 0 );
}

void QgsOptions::on_cbxProjectDefaultNew_toggled( bool checked )
{
  if ( checked )
  {
    QString fileName = QgsApplication::qgisSettingsDirPath() + QLatin1String( "project_default.qgs" );
    if ( ! QFile::exists( fileName ) )
    {
      QMessageBox::information( nullptr, tr( "Save default project" ), tr( "You must set a default project" ) );
      cbxProjectDefaultNew->setChecked( false );
    }
  }
}

void QgsOptions::on_pbnProjectDefaultSetCurrent_clicked()
{
  QString fileName = QgsApplication::qgisSettingsDirPath() + QLatin1String( "project_default.qgs" );
  if ( QgsProject::instance()->write( QFileInfo( fileName ) ) )
  {
    QMessageBox::information( nullptr, tr( "Save default project" ), tr( "Current project saved as default" ) );
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Save default project" ), tr( "Error saving current project as default" ) );
  }
}

void QgsOptions::on_pbnProjectDefaultReset_clicked()
{
  QString fileName = QgsApplication::qgisSettingsDirPath() + QLatin1String( "project_default.qgs" );
  if ( QFile::exists( fileName ) )
  {
    QFile::remove( fileName );
  }
  cbxProjectDefaultNew->setChecked( false );
}

void QgsOptions::on_pbnTemplateFolderBrowse_pressed()
{
  QString newDir = QFileDialog::getExistingDirectory( nullptr, tr( "Choose a directory to store project template files" ),
                   leTemplateFolder->text() );
  if ( ! newDir.isNull() )
  {
    leTemplateFolder->setText( newDir );
  }
}

void QgsOptions::on_pbnTemplateFolderReset_pressed()
{
  leTemplateFolder->setText( QgsApplication::qgisSettingsDirPath() + QLatin1String( "project_templates" ) );
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

void QgsOptions::on_mProjectOnLaunchCmbBx_currentIndexChanged( int indx )
{
  bool specific = ( indx == 2 );
  mProjectOnLaunchLineEdit->setEnabled( specific );
  mProjectOnLaunchPushBtn->setEnabled( specific );
}

void QgsOptions::on_mProjectOnLaunchPushBtn_pressed()
{
  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = mSettings->value( "/UI/lastProjectDir", QDir::homePath() ).toString();
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
  QSettings settings;

  mSettings->setValue( "UI/UITheme", cmbUITheme->currentText() );

  // custom environment variables
  mSettings->setValue( "qgis/customEnvVarsUse", QVariant( mCustomVariablesChkBx->isChecked() ) );
  QStringList customVars;
  for ( int i = 0; i < mCustomVariablesTable->rowCount(); ++i )
  {
    if ( mCustomVariablesTable->item( i, 1 )->text().isEmpty() )
      continue;
    QComboBox* varApplyCmbBx = qobject_cast<QComboBox*>( mCustomVariablesTable->cellWidget( i, 0 ) );
    QString customVar = varApplyCmbBx->itemData( varApplyCmbBx->currentIndex() ).toString();
    customVar += '|';
    customVar += mCustomVariablesTable->item( i, 1 )->text();
    customVar += '=';
    customVar += mCustomVariablesTable->item( i, 2 )->text();
    customVars << customVar;
  }
  mSettings->setValue( "qgis/customEnvVars", QVariant( customVars ) );

  //search directories for user plugins
  QString myPaths;
  for ( int i = 0; i < mListPluginPaths->count(); ++i )
  {
    if ( i != 0 )
    {
      myPaths += '|';
    }
    myPaths += mListPluginPaths->item( i )->text();
  }
  mSettings->setValue( "plugins/searchPathsForPlugins", myPaths );

  //search directories for svgs
  myPaths.clear();
  for ( int i = 0; i < mListSVGPaths->count(); ++i )
  {
    if ( i != 0 )
    {
      myPaths += '|';
    }
    myPaths += mListSVGPaths->item( i )->text();
  }
  mSettings->setValue( "svg/searchPathsForSVG", myPaths );

  myPaths.clear();
  for ( int i = 0; i < mListComposerTemplatePaths->count(); ++i )
  {
    if ( i != 0 )
    {
      myPaths += '|';
    }
    myPaths += mListComposerTemplatePaths->item( i )->text();
  }
  mSettings->setValue( "composer/searchPathsForTemplates", myPaths );

  QStringList paths;
  for ( int i = 0; i < mListHiddenBrowserPaths->count(); ++i )
  {
    paths << mListHiddenBrowserPaths->item( i )->text();
  }
  mSettings->setValue( "/browser/hiddenPaths", paths );

  //Network timeout
  mSettings->setValue( "/qgis/networkAndProxy/networkTimeout", mNetworkTimeoutSpinBox->value() );
  mSettings->setValue( "/qgis/networkAndProxy/userAgent", leUserAgent->text() );

  // WMS capabiltiies expiry time
  mSettings->setValue( "/qgis/defaultCapabilitiesExpiry", mDefaultCapabilitiesExpirySpinBox->value() );

  // WMS/WMS-C tile expiry time
  mSettings->setValue( "/qgis/defaultTileExpiry", mDefaultTileExpirySpinBox->value() );

  // WMS/WMS-C default max retry in case of tile request errors
  mSettings->setValue( "/qgis/defaultTileMaxRetry", mDefaultTileMaxRetrySpinBox->value() );

  //Web proxy settings
  mSettings->setValue( "proxy/proxyEnabled", grpProxy->isChecked() );
  mSettings->setValue( "proxy/proxyHost", leProxyHost->text() );
  mSettings->setValue( "proxy/proxyPort", leProxyPort->text() );
  mSettings->setValue( "proxy/proxyUser", leProxyUser->text() );
  mSettings->setValue( "proxy/proxyPassword", leProxyPassword->text() );
  mSettings->setValue( "proxy/proxyType", mProxyTypeComboBox->currentText() );

  if ( !mCacheDirectory->text().isEmpty() )
    mSettings->setValue( "cache/directory", mCacheDirectory->text() );
  else
    mSettings->remove( "cache/directory" );

  mSettings->setValue( "cache/size", QVariant::fromValue( mCacheSize->value()*1024L ) );

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
  mSettings->setValue( "proxy/proxyExcludedUrls", proxyExcludeString );

  QgisApp::instance()->namUpdate();

  //wms search url
  mSettings->setValue( "/qgis/WMSSearchUrl", leWmsSearch->text() );

  //general settings
  mSettings->setValue( "/Map/searchRadiusMM", spinBoxIdentifyValue->value() );
  mSettings->setValue( "/Map/highlight/color", mIdentifyHighlightColorButton->color().name() );
  mSettings->setValue( "/Map/highlight/colorAlpha", mIdentifyHighlightColorButton->color().alpha() );
  mSettings->setValue( "/Map/highlight/buffer", mIdentifyHighlightBufferSpinBox->value() );
  mSettings->setValue( "/Map/highlight/minWidth", mIdentifyHighlightMinWidthSpinBox->value() );

  bool showLegendClassifiers = mSettings->value( "/qgis/showLegendClassifiers", false ).toBool();
  mSettings->setValue( "/qgis/showLegendClassifiers", cbxLegendClassifiers->isChecked() );
  bool legendLayersBold = mSettings->value( "/qgis/legendLayersBold", true ).toBool();
  mSettings->setValue( "/qgis/legendLayersBold", mLegendLayersBoldChkBx->isChecked() );
  bool legendGroupsBold = mSettings->value( "/qgis/legendGroupsBold", false ).toBool();
  mSettings->setValue( "/qgis/legendGroupsBold", mLegendGroupsBoldChkBx->isChecked() );
  mSettings->setValue( "/qgis/hideSplash", cbxHideSplash->isChecked() );
  mSettings->setValue( QString( "/qgis/showTips%1" ).arg( QGis::QGIS_VERSION_INT / 100 ), cbxShowTips->isChecked() );
  mSettings->setValue( "/qgis/checkVersion", cbxCheckVersion->isChecked() );
  mSettings->setValue( "/qgis/dockAttributeTable", cbxAttributeTableDocked->isChecked() );
  mSettings->setValue( "/qgis/attributeTableBehaviour", cmbAttrTableBehaviour->itemData( cmbAttrTableBehaviour->currentIndex() ) );
  mSettings->setValue( "/qgis/attributeTableView", mAttrTableViewComboBox->itemData( mAttrTableViewComboBox->currentIndex() ) );
  mSettings->setValue( "/qgis/attributeTableRowCache", spinBoxAttrTableRowCache->value() );
  mSettings->setValue( "/qgis/promptForRasterSublayers", cmbPromptRasterSublayers->currentIndex() );
  mSettings->setValue( "/qgis/scanItemsInBrowser2",
                       cmbScanItemsInBrowser->itemData( cmbScanItemsInBrowser->currentIndex() ).toString() );
  mSettings->setValue( "/qgis/scanZipInBrowser2",
                       cmbScanZipInBrowser->itemData( cmbScanZipInBrowser->currentIndex() ).toString() );
  mSettings->setValue( "/qgis/ignoreShapeEncoding", cbxIgnoreShapeEncoding->isChecked() );
  mSettings->setValue( "/qgis/dockSnapping", cbxSnappingOptionsDocked->isChecked() );
  mSettings->setValue( "/qgis/addPostgisDC", cbxAddPostgisDC->isChecked() );
  mSettings->setValue( "/qgis/addOracleDC", cbxAddOracleDC->isChecked() );
  mSettings->setValue( "/qgis/compileExpressions", cbxCompileExpressions->isChecked() );
  mSettings->setValue( "/qgis/defaultLegendGraphicResolution", mLegendGraphicResolutionSpinBox->value() );
  bool createRasterLegendIcons = mSettings->value( "/qgis/createRasterLegendIcons", false ).toBool();
  mSettings->setValue( "/qgis/createRasterLegendIcons", cbxCreateRasterLegendIcons->isChecked() );
  mSettings->setValue( "/qgis/copyFeatureFormat", mComboCopyFeatureFormat->itemData( mComboCopyFeatureFormat->currentIndex() ).toInt() );

  mSettings->setValue( "/qgis/new_layers_visible", chkAddedVisibility->isChecked() );
  mSettings->setValue( "/qgis/enable_anti_aliasing", chkAntiAliasing->isChecked() );
  mSettings->setValue( "/qgis/enable_render_caching", chkUseRenderCaching->isChecked() );
  mSettings->setValue( "/qgis/parallel_rendering", chkParallelRendering->isChecked() );
  int maxThreads = chkMaxThreads->isChecked() ? spinMaxThreads->value() : -1;
  QgsApplication::setMaxThreads( maxThreads );
  mSettings->setValue( "/qgis/max_threads", maxThreads );

  mSettings->setValue( "/qgis/map_update_interval", spinMapUpdateInterval->value() );
  mSettings->setValue( "/qgis/legendDoubleClickAction", cmbLegendDoubleClickAction->currentIndex() );
  bool legendLayersCapitalise = mSettings->value( "/qgis/capitaliseLayerName", false ).toBool();
  mSettings->setValue( "/qgis/capitaliseLayerName", capitaliseCheckBox->isChecked() );
  QgsMapCanvas::enableRotation( cbxCanvasRotation->isChecked() );

  // Default simplify drawing configuration
  QgsVectorSimplifyMethod::SimplifyHints simplifyHints = QgsVectorSimplifyMethod::NoSimplification;
  if ( mSimplifyDrawingGroupBox->isChecked() )
  {
    simplifyHints |= QgsVectorSimplifyMethod::GeometrySimplification;
    if ( mSimplifyDrawingSpinBox->value() > 1 ) simplifyHints |= QgsVectorSimplifyMethod::AntialiasingSimplification;
  }
  mSettings->setValue( "/qgis/simplifyDrawingHints", ( int ) simplifyHints );
  mSettings->setValue( "/qgis/simplifyAlgorithm", mSimplifyAlgorithmComboBox->itemData( mSimplifyAlgorithmComboBox->currentIndex() ).toInt() );
  mSettings->setValue( "/qgis/simplifyDrawingTol", mSimplifyDrawingSpinBox->value() );
  mSettings->setValue( "/qgis/simplifyLocal", !mSimplifyDrawingAtProvider->isChecked() );
  mSettings->setValue( "/qgis/simplifyMaxScale", 1.0 / mSimplifyMaximumScaleComboBox->scale() );

  // magnification
  mSettings->setValue( "/qgis/magnifier_factor_default", doubleSpinBoxMagnifierDefault->value() / 100 );

  //curve segmentation
  int segmentationType = mToleranceTypeComboBox->itemData( mToleranceTypeComboBox->currentIndex() ).toInt();
  mSettings->setValue( "/qgis/segmentationToleranceType", segmentationType );
  double segmentationTolerance = mSegmentationToleranceSpinBox->value();
  if ( segmentationType == 0 )
  {
    segmentationTolerance = segmentationTolerance / 180.0 * M_PI; //user sets angle tolerance in degrees, internal classes need value in rad
  }
  mSettings->setValue( "/qgis/segmentationTolerance", segmentationTolerance );

  // project
  mSettings->setValue( "/qgis/projOpenAtLaunch", mProjectOnLaunchCmbBx->currentIndex() );
  mSettings->setValue( "/qgis/projOpenAtLaunchPath", mProjectOnLaunchLineEdit->text() );

  mSettings->setValue( "/qgis/askToSaveProjectChanges", chbAskToSaveProjectChanges->isChecked() );
  mSettings->setValue( "qgis/askToDeleteLayers", mLayerDeleteConfirmationChkBx->isChecked() );
  mSettings->setValue( "/qgis/warnOldProjectVersion", chbWarnOldProjectVersion->isChecked() );
  if (( mSettings->value( "/qgis/projectTemplateDir" ).toString() != leTemplateFolder->text() ) ||
      ( mSettings->value( "/qgis/newProjectDefault" ).toBool() != cbxProjectDefaultNew->isChecked() ) )
  {
    mSettings->setValue( "/qgis/newProjectDefault", cbxProjectDefaultNew->isChecked() );
    mSettings->setValue( "/qgis/projectTemplateDir", leTemplateFolder->text() );
    QgisApp::instance()->updateProjectFromTemplates();
  }
  mSettings->setValue( "/qgis/enableMacros", cmbEnableMacros->currentIndex() );

  mSettings->setValue( "/qgis/nullValue", leNullValue->text() );
  mSettings->setValue( "/qgis/style", cmbStyle->currentText() );
  mSettings->setValue( "/IconSize", cmbIconSize->currentText() );

  mSettings->setValue( "/qgis/messageTimeout", mMessageTimeoutSpnBx->value() );

  mSettings->setValue( "/qgis/native_color_dialogs", mNativeColorDialogsChkBx->isChecked() );
  mSettings->setValue( "/qgis/live_color_dialogs", mLiveColorDialogsChkBx->isChecked() );

  // rasters settings
  mSettings->setValue( "/Raster/defaultRedBand", spnRed->value() );
  mSettings->setValue( "/Raster/defaultGreenBand", spnGreen->value() );
  mSettings->setValue( "/Raster/defaultBlueBand", spnBlue->value() );

  saveContrastEnhancement( cboxContrastEnhancementAlgorithmSingleBand, "singleBand" );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandSingleByte, "multiBandSingleByte" );
  saveContrastEnhancement( cboxContrastEnhancementAlgorithmMultiBandMultiByte, "multiBandMultiByte" );

  QString contrastEnhancementLimits = cboxContrastEnhancementLimits->itemData( cboxContrastEnhancementLimits->currentIndex() ).toString();
  mSettings->setValue( "/Raster/defaultContrastEnhancementLimits", contrastEnhancementLimits );

  mSettings->setValue( "/Raster/defaultStandardDeviation", spnThreeBandStdDev->value() );

  mSettings->setValue( "/Raster/cumulativeCutLower", mRasterCumulativeCutLowerDoubleSpinBox->value() / 100.0 );
  mSettings->setValue( "/Raster/cumulativeCutUpper", mRasterCumulativeCutUpperDoubleSpinBox->value() / 100.0 );

  // log rendering events, for userspace debugging
  mSettings->setValue( "/Map/logCanvasRefreshEvent", mLogCanvasRefreshChkBx->isChecked() );

  //check behaviour so default projection when new layer is added with no
  //projection defined...
  if ( radPromptForProjection->isChecked() )
  {
    mSettings->setValue( "/Projections/defaultBehaviour", "prompt" );
  }
  else if ( radUseProjectProjection->isChecked() )
  {
    mSettings->setValue( "/Projections/defaultBehaviour", "useProject" );
  }
  else //assumes radUseGlobalProjection is checked
  {
    mSettings->setValue( "/Projections/defaultBehaviour", "useGlobal" );
  }

  mSettings->setValue( "/Projections/layerDefaultCrs", mLayerDefaultCrs.authid() );

  // save 'on the fly' CRS transformation settings
  mSettings->setValue( "/Projections/otfTransformAutoEnable", radOtfAuto->isChecked() );
  mSettings->setValue( "/Projections/otfTransformEnabled", radOtfTransform->isChecked() );
  mSettings->setValue( "/Projections/projectDefaultCrs", mDefaultCrs.authid() );

  mSettings->setValue( "/Projections/showDatumTransformDialog", chkShowDatumTransformDialog->isChecked() );

  //measurement settings

  QGis::UnitType distanceUnit = static_cast< QGis::UnitType >( mDistanceUnitsComboBox->itemData( mDistanceUnitsComboBox->currentIndex() ).toInt() );
  mSettings->setValue( "/qgis/measure/displayunits", QgsUnitTypes::encodeUnit( distanceUnit ) );

  QgsUnitTypes::AreaUnit areaUnit = static_cast< QgsUnitTypes::AreaUnit >( mAreaUnitsComboBox->itemData( mAreaUnitsComboBox->currentIndex() ).toInt() );
  mSettings->setValue( "/qgis/measure/areaunits", QgsUnitTypes::encodeUnit( areaUnit ) );

  QgsUnitTypes::AngleUnit angleUnit = static_cast< QgsUnitTypes::AngleUnit >( mAngleUnitsComboBox->itemData( mAngleUnitsComboBox->currentIndex() ).toInt() );
  mSettings->setValue( "/qgis/measure/angleunits", QgsUnitTypes::encodeUnit( angleUnit ) );

  int decimalPlaces = mDecimalPlacesSpinBox->value();
  mSettings->setValue( "/qgis/measure/decimalplaces", decimalPlaces );

  bool baseUnit = mKeepBaseUnitCheckBox->isChecked();
  mSettings->setValue( "/qgis/measure/keepbaseunit", baseUnit );

  //set the color for selections
  QColor myColor = pbnSelectionColor->color();
  mSettings->setValue( "/qgis/default_selection_color_red", myColor.red() );
  mSettings->setValue( "/qgis/default_selection_color_green", myColor.green() );
  mSettings->setValue( "/qgis/default_selection_color_blue", myColor.blue() );
  mSettings->setValue( "/qgis/default_selection_color_alpha", myColor.alpha() );

  //set the default color for canvas background
  myColor = pbnCanvasColor->color();
  mSettings->setValue( "/qgis/default_canvas_color_red", myColor.red() );
  mSettings->setValue( "/qgis/default_canvas_color_green", myColor.green() );
  mSettings->setValue( "/qgis/default_canvas_color_blue", myColor.blue() );

  //set the default color for the measure tool
  myColor = pbnMeasureColor->color();
  mSettings->setValue( "/qgis/default_measure_color_red", myColor.red() );
  mSettings->setValue( "/qgis/default_measure_color_green", myColor.green() );
  mSettings->setValue( "/qgis/default_measure_color_blue", myColor.blue() );

  mSettings->setValue( "/qgis/zoom_factor", spinZoomFactor->value() );

  //digitizing
  mSettings->setValue( "/qgis/digitizing/line_width", mLineWidthSpinBox->value() );
  QColor digitizingColor = mLineColorToolButton->color();
  mSettings->setValue( "/qgis/digitizing/line_color_red", digitizingColor.red() );
  mSettings->setValue( "/qgis/digitizing/line_color_green", digitizingColor.green() );
  mSettings->setValue( "/qgis/digitizing/line_color_blue", digitizingColor.blue() );
  mSettings->setValue( "/qgis/digitizing/line_color_alpha", digitizingColor.alpha() );

  digitizingColor = mFillColorToolButton->color();
  mSettings->setValue( "/qgis/digitizing/fill_color_red", digitizingColor.red() );
  mSettings->setValue( "/qgis/digitizing/fill_color_green", digitizingColor.green() );
  mSettings->setValue( "/qgis/digitizing/fill_color_blue", digitizingColor.blue() );
  mSettings->setValue( "/qgis/digitizing/fill_color_alpha", digitizingColor.alpha() );

  settings.setValue( "/qgis/digitizing/line_ghost", mLineGhostCheckBox->isChecked() );

  //default snap mode
  QString defaultSnapModeString = mDefaultSnapModeComboBox->itemData( mDefaultSnapModeComboBox->currentIndex() ).toString();
  mSettings->setValue( "/qgis/digitizing/default_snap_mode", defaultSnapModeString );
  mSettings->setValue( "/qgis/digitizing/default_snapping_tolerance", mDefaultSnappingToleranceSpinBox->value() );
  mSettings->setValue( "/qgis/digitizing/search_radius_vertex_edit", mSearchRadiusVertexEditSpinBox->value() );
  mSettings->setValue( "/qgis/digitizing/default_snapping_tolerance_unit",
                       ( mDefaultSnappingToleranceComboBox->currentIndex() == 0 ? QgsTolerance::ProjectUnits : QgsTolerance::Pixels ) );
  mSettings->setValue( "/qgis/digitizing/search_radius_vertex_edit_unit",
                       ( mSearchRadiusVertexEditComboBox->currentIndex()  == 0 ? QgsTolerance::ProjectUnits : QgsTolerance::Pixels ) );

  mSettings->setValue( "/qgis/digitizing/marker_only_for_selected", mMarkersOnlyForSelectedCheckBox->isChecked() );

  QString markerComboText = mMarkerStyleComboBox->currentText();
  if ( markerComboText == tr( "Semi transparent circle" ) )
  {
    mSettings->setValue( "/qgis/digitizing/marker_style", "SemiTransparentCircle" );
  }
  else if ( markerComboText == tr( "Cross" ) )
  {
    mSettings->setValue( "/qgis/digitizing/marker_style", "Cross" );
  }
  else if ( markerComboText == tr( "None" ) )
  {
    mSettings->setValue( "/qgis/digitizing/marker_style", "None" );
  }
  mSettings->setValue( "/qgis/digitizing/marker_size", ( mMarkerSizeSpinBox->value() ) );

  mSettings->setValue( "/qgis/digitizing/reuseLastValues", chkReuseLastValues->isChecked() );
  mSettings->setValue( "/qgis/digitizing/disable_enter_attribute_values_dialog", chkDisableAttributeValuesDlg->isChecked() );
  mSettings->setValue( "/qgis/digitizing/validate_geometries", mValidateGeometries->currentIndex() );

  mSettings->setValue( "/qgis/digitizing/offset_join_style", mOffsetJoinStyleComboBox->itemData( mOffsetJoinStyleComboBox->currentIndex() ).toInt() );
  mSettings->setValue( "/qgis/digitizing/offset_quad_seg", mOffsetQuadSegSpinBox->value() );
  mSettings->setValue( "/qgis/digitizing/offset_miter_limit", mCurveOffsetMiterLimitComboBox->value() );

  // default scale list
  myPaths.clear();
  for ( int i = 0; i < mListGlobalScales->count(); ++i )
  {
    if ( i != 0 )
    {
      myPaths += ',';
    }
    myPaths += mListGlobalScales->item( i )->text();
  }
  mSettings->setValue( "Map/scales", myPaths );

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
  mSettings->setValue( "/Composer/defaultFont", composerFont );

  //grid color
  mSettings->setValue( "/Composer/gridRed", mGridColorButton->color().red() );
  mSettings->setValue( "/Composer/gridGreen", mGridColorButton->color().green() );
  mSettings->setValue( "/Composer/gridBlue", mGridColorButton->color().blue() );
  mSettings->setValue( "/Composer/gridAlpha", mGridColorButton->color().alpha() );

  //grid style
  if ( mGridStyleComboBox->currentText() == tr( "Solid" ) )
  {
    mSettings->setValue( "/Composer/gridStyle", "Solid" );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Dots" ) )
  {
    mSettings->setValue( "/Composer/gridStyle", "Dots" );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Crosses" ) )
  {
    mSettings->setValue( "/Composer/gridStyle", "Crosses" );
  }

  //grid and guide defaults
  mSettings->setValue( "/Composer/defaultSnapGridResolution", mGridResolutionSpinBox->value() );
  mSettings->setValue( "/Composer/defaultSnapTolerancePixels", mSnapToleranceSpinBox->value() );
  mSettings->setValue( "/Composer/defaultSnapGridOffsetX", mOffsetXSpinBox->value() );
  mSettings->setValue( "/Composer/defaultSnapGridOffsetY", mOffsetYSpinBox->value() );

  //
  // Locale settings
  //
  mSettings->setValue( "locale/userLocale", cboLocale->itemData( cboLocale->currentIndex() ).toString() );
  mSettings->setValue( "locale/overrideFlag", grpLocale->isChecked() );

  // Gdal skip driver list
  if ( mLoadedGdalDriverList )
    saveGdalDriverList();

  // refresh legend if any legend item's state is to be changed
  if ( legendLayersBold != mLegendLayersBoldChkBx->isChecked()
       || legendGroupsBold != mLegendGroupsBoldChkBx->isChecked()
       || legendLayersCapitalise != capitaliseCheckBox->isChecked() )
  {
    // TODO[MD] QgisApp::instance()->legend()->updateLegendItemStyles();
  }

  // refresh symbology for any legend items, only if needed
  if ( showLegendClassifiers != cbxLegendClassifiers->isChecked()
       || createRasterLegendIcons != cbxCreateRasterLegendIcons->isChecked() )
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

  saveDefaultDatumTransformations();
}

void QgsOptions::rejectOptions()
{
  // don't reset stylesheet if we don't have to
  if ( mStyleSheetNewOpts != mStyleSheetOldOpts )
  {
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetOldOpts );
  }
}

void QgsOptions::on_spinFontSize_valueChanged( int fontSize )
{
  mStyleSheetNewOpts.insert( "fontPointSize", QVariant( fontSize ) );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
}

void QgsOptions::on_mFontFamilyRadioQt_released()
{
  if ( mStyleSheetNewOpts.value( "fontFamily" ).toString() != mStyleSheetBuilder->defaultFont().family() )
  {
    mStyleSheetNewOpts.insert( "fontFamily", QVariant( mStyleSheetBuilder->defaultFont().family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::on_mFontFamilyRadioCustom_released()
{
  if ( mFontFamilyComboBox->currentFont().family() != mStyleSheetBuilder->defaultFont().family() )
  {
    mStyleSheetNewOpts.insert( "fontFamily", QVariant( mFontFamilyComboBox->currentFont().family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::on_mFontFamilyComboBox_currentFontChanged( const QFont& font )
{
  if ( mFontFamilyRadioCustom->isChecked()
       && mStyleSheetNewOpts.value( "fontFamily" ).toString() != font.family() )
  {
    mStyleSheetNewOpts.insert( "fontFamily", QVariant( font.family() ) );
    mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
  }
}

void QgsOptions::on_mCustomGroupBoxChkBx_clicked( bool chkd )
{
  mStyleSheetNewOpts.insert( "groupBoxCustom", QVariant( chkd ) );
  mStyleSheetBuilder->buildStyleSheet( mStyleSheetNewOpts );
}

void QgsOptions::on_leProjectGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem& crs )
{
  mDefaultCrs = crs;
}

void QgsOptions::on_leLayerGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem& crs )
{
  mLayerDefaultCrs = crs;
}

void QgsOptions::on_lstGdalDrivers_itemDoubleClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );
  // edit driver if driver supports write
  if ( item && ( cmbEditCreateOptions->findText( item->text( 0 ) ) != -1 ) )
  {
    editGdalDriver( item->text( 0 ) );
  }
}

void QgsOptions::on_pbnEditCreateOptions_pressed()
{
  editGdalDriver( cmbEditCreateOptions->currentText() );
}

void QgsOptions::on_pbnEditPyramidsOptions_pressed()
{
  editGdalDriver( "_pyramids" );
}

void QgsOptions::editGdalDriver( const QString& driverName )
{
  if ( driverName.isEmpty() )
    return;

  QgsDialog dlg( this, nullptr, QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  QVBoxLayout *layout = dlg.layout();
  QString title = tr( "Create Options - %1 Driver" ).arg( driverName );
  if ( driverName == "_pyramids" )
    title = tr( "Create Options - pyramids" );
  dlg.setWindowTitle( title );
  if ( driverName == "_pyramids" )
  {
    QgsRasterPyramidsOptionsWidget* optionsWidget =
      new QgsRasterPyramidsOptionsWidget( &dlg, "gdal" );
    layout->addWidget( optionsWidget );
    dlg.resize( 400, 400 );
    if ( dlg.exec() == QDialog::Accepted )
      optionsWidget->apply();
  }
  else
  {
    QgsRasterFormatSaveOptionsWidget* optionsWidget =
      new QgsRasterFormatSaveOptionsWidget( &dlg, driverName,
                                            QgsRasterFormatSaveOptionsWidget::Full, "gdal" );
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
  myList << "en_US"; //there is no qm file for this so we add it manually
  QString myI18nPath = QgsApplication::i18nPath();
  QDir myDir( myI18nPath, "qgis*.qm" );
  QStringList myFileList = myDir.entryList();
  QStringListIterator myIterator( myFileList );
  while ( myIterator.hasNext() )
  {
    QString myFileName = myIterator.next();

    // Ignore the 'en' translation file, already added as 'en_US'.
    if ( myFileName.compare( "qgis_en.qm" ) == 0 ) continue;

    myList << myFileName.remove( "qgis_" ).remove( ".qm" );
  }
  return myList;
}

void QgsOptions::on_mRestoreDefaultWindowStateBtn_clicked()
{
  // richard
  if ( QMessageBox::warning( this, tr( "Restore UI defaults" ), tr( "Are you sure to reset the UI to default (needs restart)?" ), QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
    return;
  mSettings->setValue( "/qgis/restoreDefaultWindowState", true );
}

void QgsOptions::on_mCustomVariablesChkBx_toggled( bool chkd )
{
  mAddCustomVarBtn->setEnabled( chkd );
  mRemoveCustomVarBtn->setEnabled( chkd );
  mCustomVariablesTable->setEnabled( chkd );
}

void QgsOptions::addCustomEnvVarRow( const QString& varName, const QString& varVal, const QString& varApply )
{
  int rowCnt = mCustomVariablesTable->rowCount();
  mCustomVariablesTable->insertRow( rowCnt );

  QComboBox* varApplyCmbBx = new QComboBox( this );
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

  QTableWidgetItem* varNameItm = new QTableWidgetItem( varName );
  varNameItm->setFlags( itmFlags );
  mCustomVariablesTable->setItem( rowCnt, 1, varNameItm );

  QTableWidgetItem* varValueItm = new QTableWidgetItem( varVal );
  varNameItm->setFlags( itmFlags );
  mCustomVariablesTable->setItem( rowCnt, 2, varValueItm );

  mCustomVariablesTable->setRowHeight( rowCnt, cbfm.height() + 8 );
}

void QgsOptions::on_mAddCustomVarBtn_clicked()
{
  addCustomEnvVarRow( QString(), QString() );
  mCustomVariablesTable->setFocus();
  mCustomVariablesTable->setCurrentCell( mCustomVariablesTable->rowCount() - 1, 1 );
  mCustomVariablesTable->edit( mCustomVariablesTable->currentIndex() );
}

void QgsOptions::on_mRemoveCustomVarBtn_clicked()
{
  mCustomVariablesTable->removeRow( mCustomVariablesTable->currentRow() );
}

void QgsOptions::on_mCurrentVariablesQGISChxBx_toggled( bool qgisSpecific )
{
  for ( int i = mCurrentVariablesTable->rowCount() - 1; i >= 0; --i )
  {
    if ( qgisSpecific )
    {
      QString itmTxt = mCurrentVariablesTable->item( i, 0 )->text();
      if ( !itmTxt.startsWith( "QGIS", Qt::CaseInsensitive ) )
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

void QgsOptions::on_mBtnAddPluginPath_clicked()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem* newItem = new QListWidgetItem( mListPluginPaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListPluginPaths->addItem( newItem );
    mListPluginPaths->setCurrentItem( newItem );
  }
}

void QgsOptions::on_mBtnRemovePluginPath_clicked()
{
  int currentRow = mListPluginPaths->currentRow();
  QListWidgetItem* itemToRemove = mListPluginPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::on_mBtnAddTemplatePath_clicked()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem* newItem = new QListWidgetItem( mListComposerTemplatePaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListComposerTemplatePaths->addItem( newItem );
    mListComposerTemplatePaths->setCurrentItem( newItem );
  }
}

void QgsOptions::on_mBtnRemoveTemplatePath_clicked()
{
  int currentRow = mListComposerTemplatePaths->currentRow();
  QListWidgetItem* itemToRemove = mListComposerTemplatePaths->takeItem( currentRow );
  delete itemToRemove;
}


void QgsOptions::on_mBtnAddSVGPath_clicked()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( QDir::homePath() ),
                    QFileDialog::ShowDirsOnly
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem* newItem = new QListWidgetItem( mListSVGPaths );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListSVGPaths->addItem( newItem );
    mListSVGPaths->setCurrentItem( newItem );
  }
}

void QgsOptions::on_mBtnRemoveHiddenPath_clicked()
{
  int currentRow = mListHiddenBrowserPaths->currentRow();
  QListWidgetItem* itemToRemove = mListHiddenBrowserPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::on_mBtnRemoveSVGPath_clicked()
{
  int currentRow = mListSVGPaths->currentRow();
  QListWidgetItem* itemToRemove = mListSVGPaths->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::on_mAddUrlPushButton_clicked()
{
  QListWidgetItem* newItem = new QListWidgetItem( mExcludeUrlListWidget );
  newItem->setText( "URL" );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mExcludeUrlListWidget->addItem( newItem );
  mExcludeUrlListWidget->setCurrentItem( newItem );
}

void QgsOptions::on_mRemoveUrlPushButton_clicked()
{
  int currentRow = mExcludeUrlListWidget->currentRow();
  QListWidgetItem* itemToRemove = mExcludeUrlListWidget->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::on_mBrowseCacheDirectory_clicked()
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

void QgsOptions::on_mClearCache_clicked()
{
  QgsNetworkAccessManager::instance()->cache()->clear();
}

void QgsOptions::on_mOptionsStackedWidget_currentChanged( int theIndx )
{
  Q_UNUSED( theIndx );
  // load gdal driver list when gdal tab is first opened
  if ( mOptionsStackedWidget->currentWidget()->objectName() == "mOptionsPageGDAL"
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
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_RASTER, nullptr ) ) != "YES" )
      continue;
#endif
#endif

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
    myDriversFlags[myGdalDriverDescription] = QString( "%1%2" ).arg( pszRWFlag, pszVirtualIO );

    // get driver extensions and long name
    // the gdal provider can override/add extensions but there is no interface to query this
    // aside from parsing QgsRasterLayer::buildSupportedRasterFileFilter()
    myDriversExt[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_EXTENSION", "" ) ).toLower();
    myDriversLongName[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_LONGNAME", "" ) );

  }
  // restore GDAL_SKIP just in case
  CPLSetConfigOption( "GDAL_SKIP", mySkippedDrivers.join( " " ).toUtf8() );

  myDrivers.removeDuplicates();
  // myDrivers.sort();
  // sort list case insensitive - no existing function for this!
  QMap<QString, QString> strMap;
  Q_FOREACH ( const QString& str, myDrivers )
    strMap.insert( str.toLower(), str );
  myDrivers = strMap.values();

  Q_FOREACH ( const QString& myName, myDrivers )
  {
    QTreeWidgetItem * mypItem = new QTreeWidgetItem( QStringList( myName ) );
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
  Q_FOREACH ( const QString& str, myGdalWriteDrivers )
    strMap.insert( str.toLower(), str );
  myGdalWriteDrivers = strMap.values();
  myGdalWriteDrivers.removeAll( "Gtiff" );
  myGdalWriteDrivers.prepend( "GTiff" );
  cmbEditCreateOptions->clear();
  Q_FOREACH ( const QString& myName, myGdalWriteDrivers )
  {
    cmbEditCreateOptions->addItem( myName );
  }

}

void QgsOptions::saveGdalDriverList()
{
  for ( int i = 0; i < lstGdalDrivers->topLevelItemCount(); i++ )
  {
    QTreeWidgetItem * mypItem = lstGdalDrivers->topLevelItem( i );
    if ( mypItem->checkState( 0 ) == Qt::Unchecked )
    {
      QgsApplication::skipGdalDriver( mypItem->text( 0 ) );
    }
    else
    {
      QgsApplication::restoreGdalDriver( mypItem->text( 0 ) );
    }
  }
  mSettings->setValue( "gdal/skipList", QgsApplication::skippedGdalDrivers().join( " " ) );
}

void QgsOptions::on_pbnAddScale_clicked()
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
    QListWidgetItem* newItem = addScaleToScaleList( QString( "1:%1" ).arg( myScale ) );
    mListGlobalScales->setCurrentItem( newItem );
  }
}

void QgsOptions::on_pbnRemoveScale_clicked()
{
  int currentRow = mListGlobalScales->currentRow();
  QListWidgetItem* itemToRemove = mListGlobalScales->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::on_pbnDefaultScaleValues_clicked()
{
  mListGlobalScales->clear();

  QStringList myScalesList = PROJECT_SCALES.split( ',' );
  Q_FOREACH ( const QString& scale, myScalesList )
  {
    addScaleToScaleList( scale );
  }
}

void QgsOptions::on_pbnImportScales_clicked()
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

  Q_FOREACH ( const QString& scale, myScales )
  {
    addScaleToScaleList( scale );
  }
}

void QgsOptions::on_pbnExportScales_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save scales" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never ommited the extension from the file name
  if ( !fileName.endsWith( ".xml", Qt::CaseInsensitive ) )
  {
    fileName += ".xml";
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

void QgsOptions::initContrastEnhancement( QComboBox *cbox, const QString& name, const QString& defaultVal )
{
  QSettings settings;

  //add items to the color enhanceContrast combo box
  cbox->addItem( tr( "No Stretch" ), "NoEnhancement" );
  cbox->addItem( tr( "Stretch To MinMax" ), "StretchToMinimumMaximum" );
  cbox->addItem( tr( "Stretch And Clip To MinMax" ), "StretchAndClipToMinimumMaximum" );
  cbox->addItem( tr( "Clip To MinMax" ), "ClipToMinimumMaximum" );

  QString contrastEnchacement = mSettings->value( "/Raster/defaultContrastEnhancementAlgorithm/" + name, defaultVal ).toString();
  cbox->setCurrentIndex( cbox->findData( contrastEnchacement ) );
}

void QgsOptions::saveContrastEnhancement( QComboBox *cbox, const QString& name )
{
  QSettings settings;
  QString value = cbox->itemData( cbox->currentIndex() ).toString();
  mSettings->setValue( "/Raster/defaultContrastEnhancementAlgorithm/" + name, value );
}

void QgsOptions::on_mRemoveDefaultTransformButton_clicked()
{
  QList<QTreeWidgetItem*> items = mDefaultDatumTransformTreeWidget->selectedItems();
  for ( int i = 0; i < items.size(); ++i )
  {
    int idx = mDefaultDatumTransformTreeWidget->indexOfTopLevelItem( items.at( i ) );
    if ( idx >= 0 )
    {
      delete mDefaultDatumTransformTreeWidget->takeTopLevelItem( idx );
    }
  }
}

void QgsOptions::on_mAddDefaultTransformButton_clicked()
{
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setText( 0, "" );
  item->setText( 1, "" );
  item->setText( 2, "" );
  item->setText( 3, "" );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mDefaultDatumTransformTreeWidget->addTopLevelItem( item );
}

void QgsOptions::saveDefaultDatumTransformations()
{
  QSettings s;
  s.beginGroup( "/Projections" );
  QStringList groupKeys = s.allKeys();
  QStringList::const_iterator groupKeyIt = groupKeys.constBegin();
  for ( ; groupKeyIt != groupKeys.constEnd(); ++groupKeyIt )
  {
    if ( groupKeyIt->contains( "srcTransform" ) || groupKeyIt->contains( "destTransform" ) )
    {
      s.remove( *groupKeyIt );
    }
  }

  int nDefaultTransforms = mDefaultDatumTransformTreeWidget->topLevelItemCount();
  for ( int i = 0; i < nDefaultTransforms; ++i )
  {
    QTreeWidgetItem* item = mDefaultDatumTransformTreeWidget->topLevelItem( i );
    QString srcAuthId = item->text( 0 );
    QString destAuthId = item->text( 1 );
    if ( srcAuthId.isEmpty() || destAuthId.isEmpty() )
    {
      continue;
    }

    bool conversionOk;
    int srcDatumTransform = item->text( 2 ).toInt( &conversionOk );
    if ( conversionOk )
    {
      s.setValue( srcAuthId + "//" + destAuthId + "_srcTransform", srcDatumTransform );
    }
    int destDatumTransform = item->text( 3 ).toInt( &conversionOk );
    if ( conversionOk )
    {
      s.setValue( srcAuthId + "//" + destAuthId + "_destTransform", destDatumTransform );
    }
  }

  s.endGroup();
}


void QgsOptions::on_mButtonAddColor_clicked()
{
  QColor newColor = QgsColorDialogV2::getColor( QColor(), this->parentWidget(), tr( "Select color" ), true );
  if ( !newColor.isValid() )
  {
    return;
  }
  activateWindow();

  mTreeCustomColors->addColor( newColor, QgsSymbolLayerV2Utils::colorToName( newColor ) );
}

void QgsOptions::on_mButtonImportColors_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/UI/lastGplPaletteDir", QDir::homePath() ).toString();
  QString filePath = QFileDialog::getOpenFileName( this, tr( "Select palette file" ), lastDir, "GPL (*.gpl);;All files (*.*)" );
  activateWindow();
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( nullptr, tr( "Invalid file" ), tr( "Error, file does not exist or is not readable" ) );
    return;
  }

  s.setValue( "/UI/lastGplPaletteDir", fileInfo.absolutePath() );
  QFile file( filePath );
  bool importOk = mTreeCustomColors->importColorsFromGpl( file );
  if ( !importOk )
  {
    QMessageBox::critical( nullptr, tr( "Invalid file" ), tr( "Error, no colors found in palette file" ) );
    return;
  }
}

void QgsOptions::on_mButtonExportColors_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/UI/lastGplPaletteDir", QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Palette file" ), lastDir, "GPL (*.gpl)" );
  activateWindow();
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure filename contains extension
  if ( !fileName.endsWith( ".gpl", Qt::CaseInsensitive ) )
  {
    fileName += ".gpl";
  }

  QFileInfo fileInfo( fileName );
  s.setValue( "/UI/lastGplPaletteDir", fileInfo.absolutePath() );

  QFile file( fileName );
  bool exportOk = mTreeCustomColors->exportColorsToGpl( file );
  if ( !exportOk )
  {
    QMessageBox::critical( nullptr, tr( "Error exporting" ), tr( "Error writing palette file" ) );
    return;
  }
}

QListWidgetItem* QgsOptions::addScaleToScaleList( const QString &newScale )
{
  QListWidgetItem* newItem = new QListWidgetItem( newScale );
  addScaleToScaleList( newItem );
  return newItem;
}

void QgsOptions::addScaleToScaleList( QListWidgetItem* newItem )
{
  // If the new scale already exists, delete it.
  QListWidgetItem* duplicateItem = mListGlobalScales->findItems( newItem->text(), Qt::MatchExactly ).value( 0 );
  delete duplicateItem;

  int newDenominator = newItem->text().split( ":" ).value( 1 ).toInt();
  int i;
  for ( i = 0; i < mListGlobalScales->count(); i++ )
  {
    int denominator = mListGlobalScales->item( i )->text().split( ":" ).value( 1 ).toInt();
    if ( newDenominator > denominator )
      break;
  }

  newItem->setData( Qt::UserRole, newItem->text() );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mListGlobalScales->insertItem( i, newItem );
}

void QgsOptions::scaleItemChanged( QListWidgetItem* changedScaleItem )
{
  // Check if the new value is valid, restore the old value if not.
  QRegExp regExp( "1:0*[1-9]\\d*" );
  if ( regExp.exactMatch( changedScaleItem->text() ) )
  {
    //Remove leading zeroes from the denominator
    regExp.setPattern( "1:0*" );
    changedScaleItem->setText( changedScaleItem->text().replace( regExp, "1:" ) );
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
