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
#include "qgsoptions.h"
#include "qgis.h"
#include "qgisapp.h"
#include "qgisappstylesheet.h"
#include "qgsgdalutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgstolerance.h"
#include "qgsscaleutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"
#include "qgsdualview.h"
#include "qgsexpressioncontextutils.h"
#include "qgslocaldefaultsettings.h"
#include "qgsnumericformatwidget.h"
#include "qgslayertreemodellegendnode.h"

#include "qgsattributetablefiltermodel.h"
#include "qgslocalizeddatapathregistry.h"
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
#include "qgslayout.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgssettingsregistrygui.h"
#include "qgsoptionswidgetfactory.h"
#include "qgslocatorwidget.h"
#include "qgslocatoroptionswidget.h"
#include "qgsgui.h"
#include "qgswelcomepage.h"
#include "qgsnewsfeedparser.h"
#include "qgsbearingnumericformat.h"
#include "qgscoordinatenumericformat.h"
#include "options/qgsadvancedoptions.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsentryenumflag.h"
#include "qgsmeasuredialog.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

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
#include <QStandardPaths>
#include <QRegularExpression>

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

  mTreeModel = new QStandardItemModel( this );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "General" ), QCoreApplication::translate( "QgsOptionsBase", "General" ), QStringLiteral( "propertyicons/general.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "System" ), QCoreApplication::translate( "QgsOptionsBase", "System" ), QStringLiteral( "propertyicons/system.svg" ) ) );

  QStandardItem *crsGroup = new QStandardItem( QCoreApplication::translate( "QgsOptionsBase", "CRS and Transforms" ) );
  crsGroup->setData( QStringLiteral( "crs_and_transforms" ) );
  crsGroup->setToolTip( tr( "CRS and Transforms" ) );
  crsGroup->setSelectable( false );
  crsGroup->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "CRS Handling" ), QCoreApplication::translate( "QgsOptionsBase", "General CRS handling" ), QStringLiteral( "propertyicons/CRS.svg" ) ) );
  crsGroup->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Coordinate Transforms" ), QCoreApplication::translate( "QgsOptionsBase", "Coordinate transformations and operations" ), QStringLiteral( "transformation.svg" ) ) );
  mTreeModel->appendRow( crsGroup );

  QStandardItem *dataSources = createItem( QCoreApplication::translate( "QgsOptionsBase", "Data Sources" ), QCoreApplication::translate( "QgsOptionsBase", "Data sources" ), QStringLiteral( "propertyicons/attributes.svg" ) );
  mTreeModel->appendRow( dataSources );
  dataSources->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "GDAL" ), QCoreApplication::translate( "QgsOptionsBase", "GDAL" ), QStringLiteral( "propertyicons/gdal.svg" ) ) );

  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Canvas & Legend" ), QCoreApplication::translate( "QgsOptionsBase", "Canvas and legend" ), QStringLiteral( "propertyicons/overlay.svg" ) ) );

  QStandardItem *mapTools = createItem( QCoreApplication::translate( "QgsOptionsBase", "Map Tools" ), QCoreApplication::translate( "QgsOptionsBase", "Map tools" ), QStringLiteral( "propertyicons/map_tools.svg" ) );
  mTreeModel->appendRow( mapTools );
  mapTools->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Digitizing" ), QCoreApplication::translate( "QgsOptionsBase", "Digitizing" ), QStringLiteral( "propertyicons/digitizing.svg" ) ) );

  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Colors" ), QCoreApplication::translate( "QgsOptionsBase", "Colors" ), QStringLiteral( "propertyicons/colors.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Layouts" ), QCoreApplication::translate( "QgsOptionsBase", "Print layouts" ), QStringLiteral( "mIconLayout.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Variables" ), QCoreApplication::translate( "QgsOptionsBase", "Variables" ), QStringLiteral( "mIconExpression.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Authentication" ), QCoreApplication::translate( "QgsOptionsBase", "Authentication" ), QStringLiteral( "locked.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Network" ), QCoreApplication::translate( "QgsOptionsBase", "Network" ), QStringLiteral( "propertyicons/network_and_proxy.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Locator" ), tr( "Locator" ), QStringLiteral( "search.svg" ) ) );
  mTreeModel->appendRow( createItem( QCoreApplication::translate( "QgsOptionsBase", "Acceleration" ), tr( "GPU acceleration" ), QStringLiteral( "mIconGPU.svg" ) ) );

  mOptionsTreeView->setModel( mTreeModel );

  // stylesheet setup
  mStyleSheetBuilder = QgisApp::instance()->styleSheetBuilder();

  spinFontSize->setClearValue( mStyleSheetBuilder->defaultFont().pointSizeF() );

  connect( cbxProjectDefaultNew, &QCheckBox::toggled, this, &QgsOptions::cbxProjectDefaultNew_toggled );
  connect( leLayerGlobalCrs, &QgsProjectionSelectionWidget::crsChanged, this, &QgsOptions::leLayerGlobalCrs_crsChanged );
  connect( lstRasterDrivers, &QTreeWidget::itemDoubleClicked, this, &QgsOptions::lstRasterDrivers_itemDoubleClicked );

  // some drivers apply for both raster and vector -- we treat these as siblings
  // and must ensure that checking/unchecking one also checks/unchecks the other
  // (we can't selectively just disable the raster/vector part of a GDAL driver)
  auto syncItem = [ = ]( QTreeWidgetItem * changedItem, QTreeWidget * otherTree )
  {
    const QString driver = changedItem->data( 0, Qt::UserRole ).toString();
    for ( int i = 0; i < otherTree->topLevelItemCount(); ++ i )
    {
      if ( QTreeWidgetItem *otherItem = otherTree->topLevelItem( i ) )
      {
        const QString otherDriver = otherItem->data( 0, Qt::UserRole ).toString();
        if ( otherDriver == driver )
        {
          otherItem->setCheckState( 0, changedItem->checkState( 0 ) );
          return;
        }
      }
    }
  };
  connect( lstRasterDrivers, &QTreeWidget::itemChanged, this, [ = ]( QTreeWidgetItem * item, int )
  {
    syncItem( item, lstVectorDrivers );
  } );
  connect( lstVectorDrivers, &QTreeWidget::itemChanged, this, [ = ]( QTreeWidgetItem * item, int )
  {
    syncItem( item, lstRasterDrivers );
  } );

  connect( mProjectOnLaunchCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOptions::mProjectOnLaunchCmbBx_currentIndexChanged );
  connect( mProxyTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsOptions::mProxyTypeComboBox_currentIndexChanged );
  connect( mCustomVariablesChkBx, &QCheckBox::toggled, this, &QgsOptions::mCustomVariablesChkBx_toggled );
  connect( mCurrentVariablesQGISChxBx, &QCheckBox::toggled, this, &QgsOptions::mCurrentVariablesQGISChxBx_toggled );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOptions::showHelp );
  connect( cboGlobalLocale, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { updateSampleLocaleText( ); } );
  connect( cbShowGroupSeparator, &QCheckBox::toggled, this, [ = ]( bool ) { updateSampleLocaleText(); } );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );
  // disconnect default connection setup by initOptionsBase for accepting dialog, and insert logic
  // to validate widgets before allowing dialog to be closed
  disconnect( mOptButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mOptButtonBox, &QDialogButtonBox::accepted, this, [ = ]
  {
    for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalOptionWidgets ) )
    {
      if ( !widget->isValid() )
      {
        setCurrentPage( widget->objectName() );
        return;
      }
    }
    accept();
  } );

  connect( mFontFamilyRadioCustom, &QAbstractButton::toggled, mFontFamilyComboBox, &QWidget::setEnabled );

  connect( cmbIconSize, qOverload< int >( &QComboBox::activated ), this, &QgsOptions::iconSizeChanged );
  connect( cmbIconSize, qOverload< int >( &QComboBox::highlighted ), this, &QgsOptions::iconSizeChanged );
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
    // oops - none left!.. have to let user use a broken style
    filteredStyles = styles;
  }

  cmbStyle->addItems( filteredStyles );

  QStringList themes = QgsApplication::uiThemes().keys();
  cmbUITheme->addItems( themes );

  // non-default themes are best rendered using the Fusion style, therefore changing themes must require a restart to
  lblUITheme->setText( QStringLiteral( "%1 <i>(%2)</i>" ).arg( lblUITheme->text(), tr( "QGIS restart required" ) ) );

  mEnableMacrosComboBox->addItem( tr( "Never" ), QVariant::fromValue( Qgis::PythonMacroMode::Never ) );
  mEnableMacrosComboBox->addItem( tr( "Ask" ), QVariant::fromValue( Qgis::PythonMacroMode::Ask ) );
  mEnableMacrosComboBox->addItem( tr( "For This Session Only" ), QVariant::fromValue( Qgis::PythonMacroMode::SessionOnly ) );
  mEnableMacrosComboBox->addItem( tr( "Not During This Session" ), QVariant::fromValue( Qgis::PythonMacroMode::NotForThisSession ) );
  mEnableMacrosComboBox->addItem( tr( "Always (Not Recommended)" ), QVariant::fromValue( Qgis::PythonMacroMode::Always ) );

  mIdentifyHighlightColorButton->setColorDialogTitle( tr( "Identify Highlight Color" ) );
  mIdentifyHighlightColorButton->setAllowOpacity( true );
  mIdentifyHighlightColorButton->setContext( QStringLiteral( "gui" ) );
  mIdentifyHighlightColorButton->setDefaultColor( Qgis::DEFAULT_HIGHLIGHT_COLOR );

  mSettings = new QgsSettings();

  double identifyValue = mSettings->value( QStringLiteral( "/Map/searchRadiusMM" ), Qgis::DEFAULT_SEARCH_RADIUS_MM ).toDouble();
  QgsDebugMsgLevel( QStringLiteral( "Standard Identify radius setting read from settings file: %1" ).arg( identifyValue ), 3 );
  if ( identifyValue <= 0.0 )
    identifyValue = Qgis::DEFAULT_SEARCH_RADIUS_MM;
  spinBoxIdentifyValue->setMinimum( 0.0 );
  spinBoxIdentifyValue->setClearValue( Qgis::DEFAULT_SEARCH_RADIUS_MM );
  spinBoxIdentifyValue->setValue( identifyValue );
  QColor highlightColor = QColor( mSettings->value( QStringLiteral( "/Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int highlightAlpha = mSettings->value( QStringLiteral( "/Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  highlightColor.setAlpha( highlightAlpha );
  mIdentifyHighlightColorButton->setColor( highlightColor );
  double highlightBuffer = mSettings->value( QStringLiteral( "/Map/highlight/buffer" ), Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  mIdentifyHighlightBufferSpinBox->setClearValue( Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM );
  mIdentifyHighlightBufferSpinBox->setValue( highlightBuffer );
  double highlightMinWidth = mSettings->value( QStringLiteral( "/Map/highlight/minWidth" ), Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();
  mIdentifyHighlightMinWidthSpinBox->setClearValue( Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM );
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
      varNameItm->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
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
  const QStringList composerTemplatePathList = QgsApplication::layoutTemplatePaths();
  for ( const QString &path : composerTemplatePathList )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mListComposerTemplatePaths );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListComposerTemplatePaths->addItem( newItem );
  }
  connect( mBtnAddTemplatePath, &QAbstractButton::clicked, this, &QgsOptions::addTemplatePath );
  connect( mBtnRemoveTemplatePath, &QAbstractButton::clicked, this, &QgsOptions::removeTemplatePath );

  // localized data paths
  connect( mLocalizedDataPathAddButton, &QAbstractButton::clicked, this, &QgsOptions::addLocalizedDataPath );
  connect( mLocalizedDataPathRemoveButton, &QAbstractButton::clicked, this, &QgsOptions::removeLocalizedDataPath );
  connect( mLocalizedDataPathUpButton, &QAbstractButton::clicked, this, &QgsOptions::moveLocalizedDataPathUp );
  connect( mLocalizedDataPathDownButton, &QAbstractButton::clicked, this, &QgsOptions::moveLocalizedDataPathDown );

  const QStringList localizedPaths = QgsApplication::localizedDataPathRegistry()->paths();
  for ( const QString &path : localizedPaths )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mLocalizedDataPathListWidget );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mLocalizedDataPathListWidget->addItem( newItem );
  }

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
  mNetworkTimeoutSpinBox->setValue( QgsNetworkAccessManager::timeout() );
  mNetworkTimeoutSpinBox->setClearValue( QgsNetworkAccessManager::settingsNetworkTimeout->defaultValue() );
  leUserAgent->setText( mSettings->value( QStringLiteral( "/qgis/networkAndProxy/userAgent" ), "Mozilla/5.0" ).toString() );

  // WMS capabilities expiry time
  mDefaultCapabilitiesExpirySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultCapabilitiesExpiry" ), 24 ).toInt() );
  mDefaultCapabilitiesExpirySpinBox->setClearValue( 24 );

  // WMS/WMS-C tile expiry time
  mDefaultTileExpirySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultTileExpiry" ), 24 ).toInt() );
  mDefaultTileExpirySpinBox->setClearValue( 24 );

  // WMS/WMS-C default max retry in case of tile request errors
  mDefaultTileMaxRetrySpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultTileMaxRetry" ), 3 ).toInt() );
  mDefaultTileMaxRetrySpinBox->setClearValue( 3 );

  // Proxy stored authentication configurations
  mAuthSettings->setDataprovider( QStringLiteral( "proxy" ) );
  QString authcfg = mSettings->value( QStringLiteral( "proxy/authcfg" ) ).toString();
  mAuthSettings->setConfigId( authcfg );
  mAuthSettings->setWarningText( QgsAuthSettingsWidget::formattedWarning( QgsAuthSettingsWidget::UserSettings ) );

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

  //url with no proxy at all
  const QStringList noProxyUrlPathList = mSettings->value( QStringLiteral( "proxy/noProxyUrls" ) ).toStringList();
  for ( const QString &path : noProxyUrlPathList )
  {
    if ( path.trimmed().isEmpty() )
      continue;

    QListWidgetItem *newItem = new QListWidgetItem( mNoProxyUrlListWidget );
    newItem->setText( path );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mNoProxyUrlListWidget->addItem( newItem );
  }
  connect( mAddUrlPushButton, &QAbstractButton::clicked, this, &QgsOptions::addNoProxyUrl );
  connect( mRemoveUrlPushButton, &QAbstractButton::clicked, this, &QgsOptions::removeNoProxyUrl );

  // cache settings
  mCacheDirectory->setText( mSettings->value( QStringLiteral( "cache/directory" ) ).toString() );
  mCacheDirectory->setPlaceholderText( QStandardPaths::writableLocation( QStandardPaths::CacheLocation ) );
  mCacheSize->setMinimum( 0 );
  mCacheSize->setMaximum( std::numeric_limits<int>::max() );
  mCacheSize->setSingleStep( 1024 );
  qint64 cacheSize = mSettings->value( QStringLiteral( "cache/size" ), 256 * 1024 * 1024 ).toLongLong();
  mCacheSize->setValue( static_cast<int>( cacheSize / 1024 ) );
  mCacheSize->setClearValue( 50 * 1024 );
  connect( mBrowseCacheDirectory, &QAbstractButton::clicked, this, &QgsOptions::browseCacheDirectory );
  connect( mClearCache, &QAbstractButton::clicked, this, &QgsOptions::clearCache );

  // Access (auth) cache settings
  mAutoClearAccessCache->setChecked( mSettings->value( QStringLiteral( "clear_auth_cache_on_errors" ), true, QgsSettings::Section::Auth ).toBool( ) );
  connect( mClearAccessCache, &QAbstractButton::clicked, this, &QgsOptions::clearAccessCache );

  connect( mAutoClearAccessCache, &QCheckBox::clicked, this, [ = ]( bool checked )
  {
    mSettings->setValue( QStringLiteral( "clear_auth_cache_on_errors" ), checked, QgsSettings::Section::Auth );
  } );

  // set the attribute table default filter
  cmbAttrTableBehavior->clear();
  cmbAttrTableBehavior->addItem( tr( "Show All Features" ), QgsAttributeTableFilterModel::ShowAll );
  cmbAttrTableBehavior->addItem( tr( "Show Selected Features" ), QgsAttributeTableFilterModel::ShowSelected );
  cmbAttrTableBehavior->addItem( tr( "Show Features Visible on Map" ), QgsAttributeTableFilterModel::ShowVisible );
  cmbAttrTableBehavior->setCurrentIndex( cmbAttrTableBehavior->findData( mSettings->enumValue( QStringLiteral( "/qgis/attributeTableBehavior" ), QgsAttributeTableFilterModel::ShowAll ) ) );

  mAttrTableViewComboBox->clear();
  mAttrTableViewComboBox->addItem( tr( "Remember Last View" ), -1 );
  mAttrTableViewComboBox->addItem( tr( "Table View" ), QgsDualView::AttributeTable );
  mAttrTableViewComboBox->addItem( tr( "Form View" ), QgsDualView::AttributeEditor );
  mAttrTableViewComboBox->setCurrentIndex( mAttrTableViewComboBox->findData( mSettings->value( QStringLiteral( "/qgis/attributeTableView" ), -1 ).toInt() ) );

  spinBoxAttrTableRowCache->setValue( mSettings->value( QStringLiteral( "/qgis/attributeTableRowCache" ), 10000 ).toInt() );
  spinBoxAttrTableRowCache->setClearValue( 10000 );
  spinBoxAttrTableRowCache->setSpecialValueText( tr( "All" ) );

  cmbPromptSublayers->clear();
  cmbPromptSublayers->addItem( tr( "Always" ), static_cast< int >( Qgis::SublayerPromptMode::AlwaysAsk ) );
  cmbPromptSublayers->addItem( tr( "If Needed" ), static_cast< int >( Qgis::SublayerPromptMode::AskExcludingRasterBands ) ); //this means, prompt if there are sublayers but no band in the main dataset
  cmbPromptSublayers->addItem( tr( "Never" ), static_cast< int >( Qgis::SublayerPromptMode::NeverAskSkip ) );
  cmbPromptSublayers->addItem( tr( "Load All" ), static_cast< int >( Qgis::SublayerPromptMode::NeverAskLoadAll ) );
  cmbPromptSublayers->setCurrentIndex( cmbPromptSublayers->findData( static_cast< int >( mSettings->enumValue( QStringLiteral( "/qgis/promptForSublayers" ), Qgis::SublayerPromptMode::AlwaysAsk ) ) ) );

  // Scan for valid items in the browser dock
  cmbScanItemsInBrowser->clear();
  cmbScanItemsInBrowser->addItem( tr( "Check File Contents" ), "contents" ); // 0
  cmbScanItemsInBrowser->addItem( tr( "Check Extension" ), "extension" );    // 1
  int index = cmbScanItemsInBrowser->findData( mSettings->value( QStringLiteral( "/qgis/scanItemsInBrowser2" ), QString() ) );
  if ( index == -1 ) index = 1;
  cmbScanItemsInBrowser->setCurrentIndex( index );

  // Scan for contents of compressed files (.zip) in browser dock
  cmbScanZipInBrowser->clear();
  cmbScanZipInBrowser->addItem( tr( "No" ), QVariant( "no" ) );
  // cmbScanZipInBrowser->addItem( tr( "Passthru" ) );     // 1 - removed
  cmbScanZipInBrowser->addItem( tr( "Basic Scan" ), QVariant( "basic" ) );
  cmbScanZipInBrowser->addItem( tr( "Full Scan" ), QVariant( "full" ) );
  index = cmbScanZipInBrowser->findData( mSettings->value( QStringLiteral( "/qgis/scanZipInBrowser2" ), QString() ) );
  if ( index == -1 ) index = 1;
  cmbScanZipInBrowser->setCurrentIndex( index );

  mCheckMonitorDirectories->setChecked( mSettings->value( QStringLiteral( "/qgis/monitorDirectoriesInBrowser" ), true ).toBool() );

  //set the default projection behavior radio buttons
  const QgsOptions::UnknownLayerCrsBehavior mode = QgsSettings().enumValue( QStringLiteral( "/projections/unknownCrsBehavior" ), QgsOptions::UnknownLayerCrsBehavior::NoAction, QgsSettings::App );
  switch ( mode )
  {
    case NoAction:
      radCrsNoAction->setChecked( true );
      break;
    case PromptUserForCrs:
      radPromptForProjection->setChecked( true );
      break;
    case UseProjectCrs:
      radUseProjectProjection->setChecked( true );
      break;
    case UseDefaultCrs:
      radUseGlobalProjection->setChecked( true );
      break;
  }

  QString myLayerDefaultCrs = mSettings->value( QStringLiteral( "/Projections/layerDefaultCrs" ), geoEpsgCrsAuthId() ).toString();
  mLayerDefaultCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( myLayerDefaultCrs );
  leLayerGlobalCrs->setCrs( mLayerDefaultCrs );

  const QString defaultProjectCrs = mSettings->value( QStringLiteral( "/projections/defaultProjectCrs" ), geoEpsgCrsAuthId(), QgsSettings::App ).toString();
  leProjectGlobalCrs->setOptionVisible( QgsProjectionSelectionWidget::DefaultCrs, false );
  leProjectGlobalCrs->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  leProjectGlobalCrs->setNotSetText( tr( "No projection (or unknown/non-Earth projection)" ) );
  leProjectGlobalCrs->setCrs( QgsCoordinateReferenceSystem( defaultProjectCrs ) );
  leProjectGlobalCrs->setMessage(
    tr( "<h1>Default projection for new projects</h1>"
        "Select a projection that should be used for new projects that are created in QGIS."
      ) );

  const QgsGui::ProjectCrsBehavior projectCrsBehavior = mSettings->enumValue( QStringLiteral( "/projections/newProjectCrsBehavior" ),  QgsGui::UseCrsOfFirstLayerAdded, QgsSettings::App );
  switch ( projectCrsBehavior )
  {
    case QgsGui::UseCrsOfFirstLayerAdded:
      radProjectUseCrsOfFirstLayer->setChecked( true );
      break;

    case QgsGui::UsePresetCrs:
      radProjectUseDefaultCrs->setChecked( true );
      break;
  }

  const double crsAccuracyWarningThreshold = mSettings->value( QStringLiteral( "/projections/crsAccuracyWarningThreshold" ), 0.0, QgsSettings::App ).toDouble();
  mCrsAccuracySpin->setMinimumWidth( QFontMetrics( font() ).horizontalAdvance( '0' ) * 20 );
  mCrsAccuracySpin->setClearValue( 0.0, tr( "Always show" ) );
  mCrsAccuracySpin->setValue( crsAccuracyWarningThreshold );

  const bool crsAccuracyIndicator = mSettings->value( QStringLiteral( "/projections/crsAccuracyIndicator" ), false, QgsSettings::App ).toBool();
  mCrsAccuracyIndicatorCheck->setChecked( crsAccuracyIndicator );

  mShowDatumTransformDialogCheckBox->setChecked( mSettings->value( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App ).toBool() );

  // Datum transforms
  QgsCoordinateTransformContext context;
  context.readSettings();
  mDefaultDatumTransformTableWidget->setTransformContext( context );


  // Set the units for measuring
  mDistanceUnitsComboBox->addItem( tr( "Meters" ), static_cast< int >( Qgis::DistanceUnit::Meters ) );
  mDistanceUnitsComboBox->addItem( tr( "Kilometers" ), static_cast< int >( Qgis::DistanceUnit::Kilometers ) );
  mDistanceUnitsComboBox->addItem( tr( "Feet" ), static_cast< int >( Qgis::DistanceUnit::Feet ) );
  mDistanceUnitsComboBox->addItem( tr( "Yards" ), static_cast< int >( Qgis::DistanceUnit::Yards ) );
  mDistanceUnitsComboBox->addItem( tr( "Miles" ), static_cast< int >( Qgis::DistanceUnit::Miles ) );
  mDistanceUnitsComboBox->addItem( tr( "Nautical Miles" ), static_cast< int >( Qgis::DistanceUnit::NauticalMiles ) );
  mDistanceUnitsComboBox->addItem( tr( "Centimeters" ), static_cast< int >( Qgis::DistanceUnit::Centimeters ) );
  mDistanceUnitsComboBox->addItem( tr( "Millimeters" ), static_cast< int >( Qgis::DistanceUnit::Millimeters ) );
  mDistanceUnitsComboBox->addItem( tr( "Inches" ), static_cast< int >( Qgis::DistanceUnit::Inches ) );
  mDistanceUnitsComboBox->addItem( tr( "Degrees" ), static_cast< int >( Qgis::DistanceUnit::Degrees ) );
  mDistanceUnitsComboBox->addItem( tr( "Map Units" ), static_cast< int >( Qgis::DistanceUnit::Unknown ) );

  bool ok = false;
  Qgis::DistanceUnit distanceUnits = QgsUnitTypes::decodeDistanceUnit( mSettings->value( QStringLiteral( "/qgis/measure/displayunits" ) ).toString(), &ok );
  if ( !ok )
    distanceUnits = Qgis::DistanceUnit::Meters;
  mDistanceUnitsComboBox->setCurrentIndex( mDistanceUnitsComboBox->findData( static_cast< int >( distanceUnits ) ) );

  mAreaUnitsComboBox->addItem( tr( "Square Meters" ), static_cast< int >( Qgis::AreaUnit::SquareMeters ) );
  mAreaUnitsComboBox->addItem( tr( "Square Kilometers" ), static_cast< int >( Qgis::AreaUnit::SquareKilometers ) );
  mAreaUnitsComboBox->addItem( tr( "Square Feet" ), static_cast< int >( Qgis::AreaUnit::SquareFeet ) );
  mAreaUnitsComboBox->addItem( tr( "Square Yards" ), static_cast< int >( Qgis::AreaUnit::SquareYards ) );
  mAreaUnitsComboBox->addItem( tr( "Square Miles" ), static_cast< int >( Qgis::AreaUnit::SquareMiles ) );
  mAreaUnitsComboBox->addItem( tr( "Hectares" ), static_cast< int >( Qgis::AreaUnit::Hectares ) );
  mAreaUnitsComboBox->addItem( tr( "Acres" ), static_cast< int >( Qgis::AreaUnit::Acres ) );
  mAreaUnitsComboBox->addItem( tr( "Square Nautical Miles" ), static_cast< int >( Qgis::AreaUnit::SquareNauticalMiles ) );
  mAreaUnitsComboBox->addItem( tr( "Square Centimeters" ), static_cast< int >( Qgis::AreaUnit::SquareCentimeters ) );
  mAreaUnitsComboBox->addItem( tr( "Square Millimeters" ), static_cast< int >( Qgis::AreaUnit::SquareMillimeters ) );
  mAreaUnitsComboBox->addItem( tr( "Square Inches" ), static_cast< int >( Qgis::AreaUnit::SquareInches ) );
  mAreaUnitsComboBox->addItem( tr( "Square Degrees" ), static_cast< int >( Qgis::AreaUnit::SquareDegrees ) );
  mAreaUnitsComboBox->addItem( tr( "Map Units" ), static_cast< int >( Qgis::AreaUnit::Unknown ) );

  Qgis::AreaUnit areaUnits = QgsUnitTypes::decodeAreaUnit( mSettings->value( QStringLiteral( "/qgis/measure/areaunits" ) ).toString(), &ok );
  if ( !ok )
    areaUnits = Qgis::AreaUnit::SquareMeters;
  mAreaUnitsComboBox->setCurrentIndex( mAreaUnitsComboBox->findData( static_cast< int >( areaUnits ) ) );

  mAngleUnitsComboBox->addItem( tr( "Degrees" ), static_cast< int >( Qgis::AngleUnit::Degrees ) );
  mAngleUnitsComboBox->addItem( tr( "Radians" ), static_cast< int >( Qgis::AngleUnit::Radians ) );
  mAngleUnitsComboBox->addItem( tr( "Gon/gradians" ), static_cast< int >( Qgis::AngleUnit::Gon ) );
  mAngleUnitsComboBox->addItem( tr( "Minutes of Arc" ), static_cast< int >( Qgis::AngleUnit::MinutesOfArc ) );
  mAngleUnitsComboBox->addItem( tr( "Seconds of Arc" ), static_cast< int >( Qgis::AngleUnit::SecondsOfArc ) );
  mAngleUnitsComboBox->addItem( tr( "Turns/revolutions" ), static_cast< int >( Qgis::AngleUnit::Turn ) );
  mAngleUnitsComboBox->addItem( tr( "Milliradians (SI Definition)" ), static_cast< int >( Qgis::AngleUnit::MilliradiansSI ) );
  mAngleUnitsComboBox->addItem( tr( "Mil (NATO/military Definition)" ), static_cast< int >( Qgis::AngleUnit::MilNATO ) );

  Qgis::AngleUnit unit = QgsUnitTypes::decodeAngleUnit( mSettings->value( QStringLiteral( "/qgis/measure/angleunits" ), QgsUnitTypes::encodeUnit( Qgis::AngleUnit::Degrees ) ).toString() );
  mAngleUnitsComboBox->setCurrentIndex( mAngleUnitsComboBox->findData( static_cast<int>( unit ) ) );

  // set decimal places of the measure tool
  int decimalPlaces = mSettings->value( QStringLiteral( "/qgis/measure/decimalplaces" ), 3 ).toInt();
  mDecimalPlacesSpinBox->setClearValue( 3 );
  mDecimalPlacesSpinBox->setRange( 0, 12 );
  mDecimalPlacesSpinBox->setValue( decimalPlaces );

  // set if base unit of measure tool should be changed
  bool baseUnit = mSettings->value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
  mKeepBaseUnitCheckBox->setChecked( baseUnit );

  mPlanimetricMeasurementsComboBox->setChecked( mSettings->value( QStringLiteral( "measure/planimetric" ), false, QgsSettings::Core ).toBool() );

  // set the measure tool copy settings
  connect( mSeparatorOther, &QRadioButton::toggled, mSeparatorCustom, &QLineEdit::setEnabled );
  mIncludeHeader->setChecked( QgsMeasureDialog::settingClipboardHeader->value() );

  const QString sep = QgsMeasureDialog::settingClipboardSeparator->value();

  if ( sep.isEmpty() || sep == QLatin1String( "\t" ) )
    mSeparatorTab->setChecked( true );
  else if ( sep == QLatin1String( "," ) )
    mSeparatorComma->setChecked( true );
  else if ( sep == QLatin1String( ";" ) )
    mSeparatorSemicolon->setChecked( true );
  else if ( sep == QLatin1String( " " ) )
    mSeparatorSpace->setChecked( true );
  else if ( sep == QLatin1String( ":" ) )
    mSeparatorColon->setChecked( true );
  else
  {
    mSeparatorOther->setChecked( true );
    mSeparatorCustom->setText( sep );
  }
  mAlwaysUseDecimalPoint->setChecked( QgsMeasureDialog::settingClipboardAlwaysUseDecimalPoint->value() );
  connect( mAlwaysUseDecimalPoint, &QCheckBox::toggled, this, &QgsOptions::alwaysUseDecimalPointChanged );
  alwaysUseDecimalPointChanged( mAlwaysUseDecimalPoint->isChecked() );

  // set the default icon size
  cmbIconSize->setCurrentIndex( cmbIconSize->findText( mSettings->value( QStringLiteral( "/qgis/toolbarIconSize" ), QGIS_ICON_SIZE ).toString() ) );

  // set font size and family
  spinFontSize->blockSignals( true );
  mFontFamilyRadioQt->blockSignals( true );
  mFontFamilyRadioCustom->blockSignals( true );
  mFontFamilyComboBox->blockSignals( true );

  spinFontSize->setValue( mStyleSheetBuilder->fontSize() );
  const QString userFontFamily = mStyleSheetBuilder->userFontFamily();
  bool isQtDefault = userFontFamily.isEmpty();
  if ( !isQtDefault )
  {
    const QFont tempFont( userFontFamily );
    // is exact family match returned from system?
    if ( tempFont.family() != userFontFamily )
    {
      // fallback to default
      isQtDefault = true;
    }
  }
  mFontFamilyRadioQt->setChecked( isQtDefault );
  mFontFamilyRadioCustom->setChecked( !isQtDefault );
  mFontFamilyComboBox->setEnabled( !isQtDefault );
  if ( !isQtDefault )
  {
    mFontFamilyComboBox->setCurrentFont( userFontFamily );
  }
  else
  {
    mFontFamilyComboBox->setCurrentFont( mStyleSheetBuilder->defaultFont() );
  }

  spinFontSize->blockSignals( false );
  mFontFamilyRadioQt->blockSignals( false );
  mFontFamilyRadioCustom->blockSignals( false );
  mFontFamilyComboBox->blockSignals( false );

  mMessageTimeoutSpnBx->setValue( mSettings->value( QStringLiteral( "/qgis/messageTimeout" ), 5 ).toInt() );
  mMessageTimeoutSpnBx->setClearValue( 5 );

  QString name = mSettings->value( QStringLiteral( "/qgis/style" ) ).toString();
  whileBlocking( cmbStyle )->setCurrentIndex( cmbStyle->findText( name, Qt::MatchFixedString ) );

  QString theme = mSettings->value( QStringLiteral( "UI/UITheme" ), QStringLiteral( "default" ) ).toString();
  if ( !QgsApplication::uiThemes().contains( theme ) )
  {
    theme = QStringLiteral( "default" );
  }
  whileBlocking( cmbUITheme )->setCurrentIndex( cmbUITheme->findText( theme, Qt::MatchFixedString ) );

  mNativeColorDialogsChkBx->setChecked( mSettings->value( QStringLiteral( "/qgis/native_color_dialogs" ), false ).toBool() );

  cbxLegendClassifiers->setChecked( mSettings->value( QStringLiteral( "/qgis/showLegendClassifiers" ), false ).toBool() );
  mShowFeatureCountByDefaultCheckBox->setChecked( QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers->value() );
  cbxHideSplash->setChecked( mSettings->value( QStringLiteral( "/qgis/hideSplash" ), false ).toBool() );
  cbxShowNews->setChecked( !mSettings->value( QStringLiteral( "%1/disabled" ).arg( QgsNewsFeedParser::keyForFeed( QgsWelcomePage::newsFeedUrl() ) ), false, QgsSettings::Core ).toBool() );
  cbxCheckVersion->setChecked( mSettings->value( QStringLiteral( "/qgis/checkVersion" ), true ).toBool() );
  cbxCheckVersion->setVisible( mSettings->value( QStringLiteral( "/qgis/allowVersionCheck" ), true ).toBool() );
  cbxAttributeTableDocked->setChecked( mSettings->value( QStringLiteral( "/qgis/dockAttributeTable" ), false ).toBool() );

  mComboCopyFeatureFormat->addItem( tr( "Plain Text, No Geometry" ), QgsClipboard::AttributesOnly );
  mComboCopyFeatureFormat->addItem( tr( "Plain Text, WKT Geometry" ), QgsClipboard::AttributesWithWKT );
  mComboCopyFeatureFormat->addItem( tr( "Plain Text, WKB Geometry" ), QgsClipboard::AttributesWithWKB );
  mComboCopyFeatureFormat->addItem( tr( "GeoJSON" ), QgsClipboard::GeoJSON );
  mComboCopyFeatureFormat->setCurrentIndex( mComboCopyFeatureFormat->findData( mSettings->enumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesWithWKT ) ) );
  leNullValue->setText( QgsApplication::nullRepresentation() );

  cmbLegendDoubleClickAction->setCurrentIndex( mSettings->value( QStringLiteral( "/qgis/legendDoubleClickAction" ), 0 ).toInt() );

  mLayerTreeInsertionMethod->addItem( tr( "Above currently selected layer" ), QVariant::fromValue( Qgis::LayerTreeInsertionMethod::AboveInsertionPoint ) );
  mLayerTreeInsertionMethod->addItem( tr( "Always on top of the layer tree" ), QVariant::fromValue( Qgis::LayerTreeInsertionMethod::TopOfTree ) );
  mLayerTreeInsertionMethod->addItem( tr( "Optimal index within current layer tree group" ), QVariant::fromValue( Qgis::LayerTreeInsertionMethod::OptimalInInsertionGroup ) );
  mLayerTreeInsertionMethod->setCurrentIndex( mLayerTreeInsertionMethod->findData( QVariant::fromValue( mSettings->enumValue( QStringLiteral( "/qgis/layerTreeInsertionMethod" ), Qgis::LayerTreeInsertionMethod::AboveInsertionPoint ) ) ) );

  // Legend symbol minimum / maximum values
  mLegendSymbolMinimumSizeSpinBox->setClearValue( 0.0, tr( "none" ) );
  mLegendSymbolMaximumSizeSpinBox->setClearValue( 0.0, tr( "none" ) );
  mLegendSymbolMinimumSizeSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/legendsymbolMinimumSize" ), 0.1 ).toDouble() );
  mLegendSymbolMaximumSizeSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/legendsymbolMaximumSize" ), 20.0 ).toDouble() );

  // WMS getLegendGraphic setting
  mLegendGraphicResolutionSpinBox->setValue( mSettings->value( QStringLiteral( "/qgis/defaultLegendGraphicResolution" ), 0 ).toInt() );

  // Map Tips delay
  mMapTipsDelaySpinBox->setValue( mSettings->value( QStringLiteral( "qgis/mapTipsDelay" ), 850 ).toInt() );
  mMapTipsDelaySpinBox->setClearValue( 850 );

  mRespectScreenDpiCheckBox->setChecked( QgsSettingsRegistryGui::settingsRespectScreenDPI->value() );

  //set the color for selections
  int red = mSettings->value( QStringLiteral( "/qgis/default_selection_color_red" ), 255 ).toInt();
  int green = mSettings->value( QStringLiteral( "/qgis/default_selection_color_green" ), 255 ).toInt();
  int blue = mSettings->value( QStringLiteral( "/qgis/default_selection_color_blue" ), 0 ).toInt();
  int alpha = mSettings->value( QStringLiteral( "/qgis/default_selection_color_alpha" ), 255 ).toInt();
  pbnSelectionColor->setColor( QColor( red, green, blue, alpha ) );
  pbnSelectionColor->setColorDialogTitle( tr( "Set Selection Color" ) );
  pbnSelectionColor->setAllowOpacity( true );
  pbnSelectionColor->setContext( QStringLiteral( "gui" ) );
  pbnSelectionColor->setDefaultColor( QColor( 255, 255, 0, 255 ) );

  //set the default color for canvas background
  red = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_red" ), 255 ).toInt();
  green = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_green" ), 255 ).toInt();
  blue = mSettings->value( QStringLiteral( "/qgis/default_canvas_color_blue" ), 255 ).toInt();
  pbnCanvasColor->setColor( QColor( red, green, blue ) );
  pbnCanvasColor->setColorDialogTitle( tr( "Set Canvas Color" ) );
  pbnCanvasColor->setContext( QStringLiteral( "gui" ) );
  pbnCanvasColor->setDefaultColor( Qt::white );

  // set the default color for the measure tool
  red = mSettings->value( QStringLiteral( "/qgis/default_measure_color_red" ), 222 ).toInt();
  green = mSettings->value( QStringLiteral( "/qgis/default_measure_color_green" ), 155 ).toInt();
  blue = mSettings->value( QStringLiteral( "/qgis/default_measure_color_blue" ), 67 ).toInt();
  pbnMeasureColor->setColor( QColor( red, green, blue ) );
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
  Qgis::PythonMacroMode pyMacroMode = mSettings->enumValue( QStringLiteral( "/qgis/enableMacros" ), Qgis::PythonMacroMode::Ask );
  mEnableMacrosComboBox->setCurrentIndex( mEnableMacrosComboBox->findData( QVariant::fromValue( pyMacroMode ) ) );

  mDefaultPathsComboBox->addItem( tr( "Absolute" ), static_cast< int >( Qgis::FilePathType::Absolute ) );
  mDefaultPathsComboBox->addItem( tr( "Relative" ), static_cast< int >( Qgis::FilePathType::Relative ) );
  mDefaultPathsComboBox->setCurrentIndex(
    mDefaultPathsComboBox->findData(
      static_cast< int >(
        mSettings->value( QStringLiteral( "/qgis/defaultProjectPathsRelative" ), QVariant( true ) ).toBool() ? Qgis::FilePathType::Relative : Qgis::FilePathType::Absolute )
    )
  );

  Qgis::ProjectFileFormat defaultProjectFileFormat = mSettings->enumValue( QStringLiteral( "/qgis/defaultProjectFileFormat" ), Qgis::ProjectFileFormat::Qgz );
  mFileFormatQgzButton->setChecked( defaultProjectFileFormat == Qgis::ProjectFileFormat::Qgz );
  mFileFormatQgsButton->setChecked( defaultProjectFileFormat == Qgis::ProjectFileFormat::Qgs );

  // templates
  cbxProjectDefaultNew->setChecked( mSettings->value( QStringLiteral( "/qgis/newProjectDefault" ), QVariant( false ) ).toBool() );
  QString templateDirName = mSettings->value( QStringLiteral( "/qgis/projectTemplateDir" ),
                            QString( QgsApplication::qgisSettingsDirPath() + "project_templates" ) ).toString();
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
  spinZoomFactor->setClearValue( 200 );
  reverseWheelZoom->setChecked( mSettings->value( QStringLiteral( "/qgis/reverse_wheel_zoom" ), false ).toBool() );

  // predefined scales for scale combobox
  const QStringList scalePaths = QgsSettingsRegistryCore::settingsMapScales->value();
  for ( const QString &scale : scalePaths )
  {
    if ( ! scale.isEmpty() )
    {
      addScaleToScaleList( scale );
    }
  }

  connect( mListGlobalScales, &QListWidget::itemChanged, this, &QgsOptions::scaleItemChanged );
  connect( pbnAddScale, &QAbstractButton::clicked, this, &QgsOptions::addScale );
  connect( pbnRemoveScale, &QAbstractButton::clicked, this, &QgsOptions::removeScale );
  connect( pbnExportScales, &QAbstractButton::clicked, this, &QgsOptions::exportScales );
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

  connect( mActionImportPalette, &QAction::triggered, this, [ = ]
  {
    if ( QgsCompoundColorWidget::importUserPaletteFromFile( this ) )
    {
      //refresh combobox
      refreshSchemeComboBox();
      mColorSchemesComboBox->setCurrentIndex( mColorSchemesComboBox->count() - 1 );
    }
  } );
  connect( mActionRemovePalette, &QAction::triggered, this, [ = ]
  {
    //get current scheme
    QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes();
    int prevIndex = mColorSchemesComboBox->currentIndex();
    if ( prevIndex >= schemeList.length() )
    {
      return;
    }

    //make user scheme is a user removable scheme
    QgsUserColorScheme *userScheme = dynamic_cast<QgsUserColorScheme *>( schemeList.at( prevIndex ) );
    if ( !userScheme )
    {
      return;
    }

    if ( QgsCompoundColorWidget::removeUserPalette( userScheme, this ) )
    {
      refreshSchemeComboBox();
      prevIndex = std::max( std::min( prevIndex, mColorSchemesComboBox->count() - 1 ), 0 );
      mColorSchemesComboBox->setCurrentIndex( prevIndex );
    }
  } );
  connect( mActionNewPalette, &QAction::triggered, this, [ = ]
  {
    if ( QgsCompoundColorWidget::createNewUserPalette( this ) )
    {
      //refresh combobox
      refreshSchemeComboBox();
      mColorSchemesComboBox->setCurrentIndex( mColorSchemesComboBox->count() - 1 );
    }
  } );

  connect( mActionShowInButtons, &QAction::toggled, this, [ = ]( bool state )
  {
    QgsUserColorScheme *scheme = dynamic_cast< QgsUserColorScheme * >( mTreeCustomColors->scheme() );
    if ( scheme )
    {
      scheme->setShowSchemeInMenu( state );
    }
  } );

  QMenu *schemeMenu = new QMenu( mSchemeToolButton );
  schemeMenu->addAction( mActionNewPalette );
  schemeMenu->addAction( mActionImportPalette );
  schemeMenu->addAction( mActionRemovePalette );
  schemeMenu->addSeparator();
  schemeMenu->addAction( mActionShowInButtons );
  mSchemeToolButton->setMenu( schemeMenu );

  //find custom color scheme from registry
  refreshSchemeComboBox();
  QList<QgsCustomColorScheme *> customSchemes;
  QgsApplication::colorSchemeRegistry()->schemes( customSchemes );
  if ( customSchemes.length() > 0 )
  {
    mTreeCustomColors->setScheme( customSchemes.at( 0 ) );
    mColorSchemesComboBox->setCurrentIndex( mColorSchemesComboBox->findText( customSchemes.at( 0 )->schemeName() ) );
    updateActionsForCurrentColorScheme( customSchemes.at( 0 ) );
  }
  connect( mColorSchemesComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int index )
  {
    //save changes to scheme
    if ( mTreeCustomColors->isDirty() )
    {
      mTreeCustomColors->saveColorsToScheme();
    }

    QgsColorScheme *scheme = QgsApplication::colorSchemeRegistry()->schemes().value( index );
    if ( scheme )
    {
      mTreeCustomColors->setScheme( scheme );
      updateActionsForCurrentColorScheme( scheme );
    }
  } );

  //
  // Layout settings
  //

  //default layout font
  mComposerFontComboBox->blockSignals( true );

  QString layoutFontFamily = mSettings->value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();

  QFont tempLayoutFont( layoutFontFamily );
  // is exact family match returned from system?
  if ( tempLayoutFont.family() == layoutFontFamily )
  {
    mComposerFontComboBox->setCurrentFont( tempLayoutFont );
  }

  mComposerFontComboBox->blockSignals( false );

  //default layout grid color
  int gridRed, gridGreen, gridBlue, gridAlpha;
  gridRed = mSettings->value( QStringLiteral( "LayoutDesigner/gridRed" ), 190, QgsSettings::Gui ).toInt();
  gridGreen = mSettings->value( QStringLiteral( "LayoutDesigner/gridGreen" ), 190, QgsSettings::Gui ).toInt();
  gridBlue = mSettings->value( QStringLiteral( "LayoutDesigner/gridBlue" ), 190, QgsSettings::Gui ).toInt();
  gridAlpha = mSettings->value( QStringLiteral( "LayoutDesigner/gridAlpha" ), 100, QgsSettings::Gui ).toInt();
  QColor gridColor = QColor( gridRed, gridGreen, gridBlue, gridAlpha );
  mGridColorButton->setColor( gridColor );
  mGridColorButton->setColorDialogTitle( tr( "Select Grid Color" ) );
  mGridColorButton->setAllowOpacity( true );
  mGridColorButton->setContext( QStringLiteral( "gui" ) );
  mGridColorButton->setDefaultColor( QColor( 190, 190, 190, 100 ) );

  //default layout grid style
  QString gridStyleString;
  gridStyleString = mSettings->value( QStringLiteral( "LayoutDesigner/gridStyle" ), "Dots", QgsSettings::Gui ).toString();
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
  mGridResolutionSpinBox->setValue( mSettings->value( QStringLiteral( "LayoutDesigner/defaultSnapGridResolution" ), 10.0, QgsSettings::Gui ).toDouble() );
  mGridResolutionSpinBox->setClearValue( 10.0 );
  mSnapToleranceSpinBox->setValue( mSettings->value( QStringLiteral( "LayoutDesigner/defaultSnapTolerancePixels" ), 5, QgsSettings::Gui ).toInt() );
  mSnapToleranceSpinBox->setClearValue( 5 );
  mOffsetXSpinBox->setValue( mSettings->value( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetX" ), 0, QgsSettings::Gui ).toDouble() );
  mOffsetYSpinBox->setValue( mSettings->value( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetY" ), 0, QgsSettings::Gui ).toDouble() );

  //
  // Translation and locale settings
  //
  QString currentLocale = QLocale().name();
  lblSystemLocale->setText( tr( "Detected active locale on your system: %1" ).arg( currentLocale ) );
  QString userLocale = QgsApplication::settingsLocaleUserLocale->value();
  bool showGroupSeparator = QgsApplication::settingsLocaleShowGroupSeparator->value();
  QString globalLocale = QgsApplication::settingsLocaleGlobalLocale->value();
  const QStringList language18nList( i18nList() );
  for ( const auto &l : language18nList )
  {
    QString displayName = l.startsWith( QLatin1String( "sc" ) ) ? QStringLiteral( "sardu" ) : QLocale( l ).nativeLanguageName();
    cboTranslation->addItem( QIcon( QString( ":/images/flags/%1.svg" ).arg( l ) ), displayName, l );
  }

  const QList<QLocale> allLocales = QLocale::matchingLocales(
                                      QLocale::AnyLanguage,
                                      QLocale::AnyScript,
                                      QLocale::AnyCountry );

  QSet<QString> addedLocales;
  for ( const auto &l : allLocales )
  {
    // Do not add duplicates (like en_US)
    if ( ! addedLocales.contains( l.name() ) )
    {
      cboGlobalLocale->addItem( QStringLiteral( "%1 %2 (%3)" ).arg( QLocale::languageToString( l.language() ), QLocale::countryToString( l.country() ), l.name() ), l.name() );
      addedLocales.insert( l.name() );
    }
  }

  cboTranslation->setCurrentIndex( cboTranslation->findData( userLocale ) );
  cboGlobalLocale->setCurrentIndex( cboGlobalLocale->findData( globalLocale ) );
  grpLocale->setChecked( QgsApplication::settingsLocaleOverrideFlag->value() );
  cbShowGroupSeparator->setChecked( showGroupSeparator );


  //set elements in digitizing tab
  mLineWidthSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingLineWidth->value() );
  mLineColorToolButton->setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
  mLineColorToolButton->setAllowOpacity( true );
  mLineColorToolButton->setContext( QStringLiteral( "gui" ) );
  mLineColorToolButton->setDefaultColor( QColor( 255, 0, 0, 200 ) );

  mFillColorToolButton->setColor( QgsSettingsRegistryCore::settingsDigitizingFillColor->value() );
  mFillColorToolButton->setAllowOpacity( true );
  mFillColorToolButton->setContext( QStringLiteral( "gui" ) );
  mFillColorToolButton->setDefaultColor( QColor( 255, 0, 0, 30 ) );

  mLineGhostCheckBox->setChecked( QgsSettingsRegistryCore::settingsDigitizingLineGhost->value() );

  mDefaultZValueSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->value() );
  mDefaultZValueSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->defaultValue() );

  mDefaultMValueSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->value() );
  mDefaultMValueSpinBox->setClearValue( Qgis::DEFAULT_M_COORDINATE );

  //default snap mode
  mSnappingEnabledDefault->setChecked( QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled->value() );

  for ( Qgis::SnappingType type : qgsEnumList<Qgis::SnappingType>() )
  {
    mDefaultSnapTypeComboBox->addItem( QgsSnappingConfig::snappingTypeToIcon( type ),
                                       QgsSnappingConfig::snappingTypeToString( type ),
                                       QVariant::fromValue( type ) );
  }

  Qgis::SnappingTypes defaultSnapType = QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType->value();
  mDefaultSnapTypeComboBox->setCurrentIndex( mDefaultSnapTypeComboBox->findData( static_cast<int>( defaultSnapType ) ) );
  mDefaultSnappingToleranceSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance->value() );
  mDefaultSnappingToleranceSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance->defaultValue() );
  mSearchRadiusVertexEditSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit->value() );
  mSearchRadiusVertexEditSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit->defaultValue() );
  Qgis::MapToolUnit defSnapUnits = QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit->value();
  if ( defSnapUnits == Qgis::MapToolUnit::Project || defSnapUnits == Qgis::MapToolUnit::Layer )
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mDefaultSnappingToleranceComboBox->findText( tr( "pixels" ) );
  }
  mDefaultSnappingToleranceComboBox->setCurrentIndex( index );
  Qgis::MapToolUnit defRadiusUnits = QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit->value();
  if ( defRadiusUnits == Qgis::MapToolUnit::Project || defRadiusUnits == Qgis::MapToolUnit::Layer )
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "map units" ) );
  }
  else
  {
    index = mSearchRadiusVertexEditComboBox->findText( tr( "pixels" ) );
  }
  mSearchRadiusVertexEditComboBox->setCurrentIndex( index );

  mSnappingMarkerColorButton->setColor( QgsSettingsRegistryCore::settingsDigitizingSnapColor->value() );
  mSnappingTooltipsCheckbox->setChecked( QgsSettingsRegistryCore::settingsDigitizingSnapTooltip->value() );
  mEnableSnappingOnInvisibleFeatureCheckbox->setChecked( QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature->value() );

  //vertex marker
  mMarkersOnlyForSelectedCheckBox->setChecked( QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected->value() );

  mMarkerStyleComboBox->addItem( tr( "Semi Transparent Circle" ) );
  mMarkerStyleComboBox->addItem( tr( "Cross" ) );
  mMarkerStyleComboBox->addItem( tr( "None" ) );

  mValidateGeometries->clear();
  mValidateGeometries->addItem( tr( "Off" ) );
  mValidateGeometries->addItem( tr( "QGIS" ) );
  mValidateGeometries->addItem( tr( "GEOS" ) );

  QString markerStyle = QgsSettingsRegistryCore::settingsDigitizingMarkerStyle->value();
  if ( markerStyle == QLatin1String( "SemiTransparentCircle" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Semi Transparent Circle" ) ) );
  }
  else if ( markerStyle == QLatin1String( "Cross" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "Cross" ) ) );
  }
  else if ( markerStyle == QLatin1String( "None" ) )
  {
    mMarkerStyleComboBox->setCurrentIndex( mMarkerStyleComboBox->findText( tr( "None" ) ) );
  }
  mMarkerSizeSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm->value() );
  mMarkerSizeSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm->defaultValue() );

  chkReuseLastValues->setChecked( QgsSettingsRegistryCore::settingsDigitizingReuseLastValues->value() );
  chkDisableAttributeValuesDlg->setChecked( QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog->value() );
  mValidateGeometries->setCurrentIndex( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->value() );

  mSnappingMainDialogComboBox->clear();
  mSnappingMainDialogComboBox->addItem( tr( "Dialog" ), "dialog" );
  mSnappingMainDialogComboBox->addItem( tr( "Dock" ), "dock" );
  mSnappingMainDialogComboBox->setCurrentIndex( mSnappingMainDialogComboBox->findData( mSettings->value( QStringLiteral( "/qgis/mainSnappingWidgetMode" ), "dialog" ).toString() ) );

  mOffsetJoinStyleComboBox->addItem( tr( "Round" ), static_cast< int >( Qgis::JoinStyle::Round ) );
  mOffsetJoinStyleComboBox->addItem( tr( "Miter" ), static_cast< int >( Qgis::JoinStyle::Miter ) );
  mOffsetJoinStyleComboBox->addItem( tr( "Bevel" ), static_cast< int >( Qgis::JoinStyle::Bevel ) );
  Qgis::JoinStyle joinStyleSetting = QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->value();
  mOffsetJoinStyleComboBox->setCurrentIndex( mOffsetJoinStyleComboBox->findData( static_cast< int >( joinStyleSetting ) ) );
  mOffsetQuadSegSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->value() );
  mOffsetQuadSegSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->defaultValue() );
  mCurveOffsetMiterLimitComboBox->setValue( QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit->value() );
  mCurveOffsetMiterLimitComboBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit->defaultValue() );
  mOffsetCapStyleComboBox->addItem( tr( "Round" ), QVariant::fromValue( Qgis::EndCapStyle::Round ) );
  mOffsetCapStyleComboBox->addItem( tr( "Flat" ), QVariant::fromValue( Qgis::EndCapStyle::Flat ) );
  mOffsetCapStyleComboBox->addItem( tr( "Square" ), QVariant::fromValue( Qgis::EndCapStyle::Square ) );
  Qgis::EndCapStyle capStyleSetting = QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle->value();
  mOffsetCapStyleComboBox->setCurrentIndex( mOffsetCapStyleComboBox->findData( QVariant::fromValue( capStyleSetting ) ) );

  mTracingConvertToCurveCheckBox->setChecked( QgsSettingsRegistryCore::settingsDigitizingConvertToCurve->value() );
  mTracingCustomAngleToleranceSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance->value() );
  mTracingCustomAngleToleranceSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance->defaultValue() );
  mTracingCustomDistanceToleranceSpinBox->setValue( QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance->value() );
  mTracingCustomDistanceToleranceSpinBox->setClearValue( QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance->defaultValue() );

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

  QList< QgsOptionsWidgetFactory *> factories = optionsFactories;
  // ensure advanced factory is always last
  QgsAdvancedSettingsOptionsFactory advancedFactory;
  factories << &advancedFactory;
  for ( QgsOptionsWidgetFactory *factory : std::as_const( factories ) )
  {
    QgsOptionsPageWidget *page = factory->createWidget( this );
    if ( !page )
      continue;

    mAdditionalOptionWidgets << page;
    const QString beforePage = factory->pagePositionHint();
    if ( beforePage.isEmpty() )
      addPage( factory->title(), factory->title(), factory->icon(), page, factory->path(), factory->key() );
    else
      insertPage( factory->title(), factory->title(), factory->icon(), page, beforePage, factory->path(), factory->key() );
  }

  if ( mOptTreeView )
    mOptTreeView->expandAll();

#ifdef HAVE_OPENCL

  // Setup OpenCL Acceleration widget

  connect( mGPUEnableCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      // Since this may crash and lock users out of the settings, let's disable opencl setting before entering
      // and restore after available was successfully called
      const bool openClStatus { QgsOpenClUtils::enabled() };
      QgsOpenClUtils::setEnabled( false );
      if ( QgsOpenClUtils::available( ) )
      {
        QgsOpenClUtils::setEnabled( openClStatus );
        mOpenClContainerWidget->setEnabled( true );
        mOpenClDevicesCombo->clear();

        for ( const auto &dev : QgsOpenClUtils::devices( ) )
        {
          mOpenClDevicesCombo->addItem( QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Name, dev ), QgsOpenClUtils::deviceId( dev ) );
        }
        // Info updater
        std::function<void( int )> infoUpdater = [ = ]( int )
        {
          mGPUInfoTextBrowser->setText( QgsOpenClUtils::deviceDescription( mOpenClDevicesCombo->currentData().toString() ) );
        };
        connect( mOpenClDevicesCombo, qOverload< int >( &QComboBox::currentIndexChanged ), infoUpdater );
        mOpenClDevicesCombo->setCurrentIndex( mOpenClDevicesCombo->findData( QgsOpenClUtils::deviceId( QgsOpenClUtils::activeDevice() ) ) );
        infoUpdater( -1 );
        mOpenClContainerWidget->show();
      }
      else
      {
        mGPUInfoTextBrowser->setText( tr( "No OpenCL compatible devices were found on your system.<br>"
                                          "You may need to install additional libraries in order to enable OpenCL.<br>"
                                          "Please check your logs for further details." ) );
        mOpenClContainerWidget->setEnabled( false );
        mGPUEnableCheckBox->setChecked( false );
      }
    }
    else
    {
      mOpenClContainerWidget->setEnabled( false );
    }
  } );

  mOpenClContainerWidget->setEnabled( false );
  mGPUEnableCheckBox->setChecked( QgsOpenClUtils::enabled( ) );


#else

  mGPUEnableCheckBox->setChecked( false );
  for ( int idx = 0; idx < mOptionsPageAccelerationLayout->count(); ++idx )
  {
    QWidget *item = mOptionsPageAccelerationLayout->itemAt( idx )->widget();
    if ( item )
    {
      item->setEnabled( false );
    }
  }
  QLabel *noOpenCL = new QLabel( tr( "QGIS is compiled without OpenCL support. "
                                     "GPU acceleration is not available." ), this );
  mOptionsPageAccelerationLayout->insertWidget( 0, noOpenCL );

#endif

  connect( pbnEditCreateOptions, &QAbstractButton::pressed, this, &QgsOptions::editCreateOptions );
  connect( pbnEditPyramidsOptions, &QAbstractButton::pressed, this, &QgsOptions::editPyramidsOptions );

  // restore window and widget geometry/state
  connect( mRestoreDefaultWindowStateBtn, &QAbstractButton::clicked, this, &QgsOptions::restoreDefaultWindowState );

  mBearingFormat.reset( QgsLocalDefaultSettings::bearingFormat() );
  mCoordinateFormat.reset( QgsLocalDefaultSettings::geographicCoordinateFormat() );
  connect( mCustomizeBearingFormatButton, &QPushButton::clicked, this, &QgsOptions::customizeBearingFormat );
  connect( mCustomizeCoordinateFormatButton, &QPushButton::clicked, this, &QgsOptions::customizeCoordinateFormat );

  restoreOptionsBaseUi();

#ifdef QGISDEBUG
  checkPageWidgetNameMap();
#endif
}


QgsOptions::~QgsOptions()
{
  delete mSettings;
}

void QgsOptions::checkPageWidgetNameMap()
{
  const QMap< QString, QString > pageNames = QgisApp::instance()->optionsPagesMap();

  std::function<void( const QModelIndex & )> traverseModel;

  // traverse through the model, collecting all entries which correspond to pages
  QStringList pageTitles;
  traverseModel = [&]( const QModelIndex & parent )
  {
    for ( int row = 0; row < mTreeModel->rowCount( parent ); ++row )
    {
      const QModelIndex currentIndex = mTreeModel->index( row, 0, parent );

      if ( mTreeModel->itemFromIndex( currentIndex )->isSelectable() )
        pageTitles << currentIndex.data().toString();

      traverseModel( currentIndex );
    }
  };
  traverseModel( QModelIndex() );

  Q_ASSERT_X( pageNames.count() == pageTitles.count(),  "QgsOptions::checkPageWidgetNameMap()",
              QStringLiteral( "QgisApp::optionsPagesMap() is outdated, contains too many entries, "
                              " this is often a problem with missing translations for the entries (extra entries: %1)" ).arg(
                qgsSetJoin( QSet( pageNames.keyBegin(), pageNames.keyEnd() ) - QSet( pageTitles.constBegin(), pageTitles.constEnd() ),
                            QStringLiteral( "," ) ) ).toLocal8Bit().constData() );

  int page = 0;
  for ( const QString &pageTitle : std::as_const( pageTitles ) )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( page );
    Q_ASSERT_X( pageNames.contains( pageTitle ), "QgsOptions::checkPageWidgetNameMap()", QStringLiteral( "QgisApp::optionsPagesMap() is outdated, please update. Missing %1" ).arg( pageTitle ).toLocal8Bit().constData() );
    Q_ASSERT_X( pageNames.value( pageTitle ) == currentPage->objectName() || pageNames.value( pageTitle ) == pageTitle, "QgsOptions::checkPageWidgetNameMap()", QStringLiteral( "QgisApp::optionsPagesMap() is outdated, please update. %1 should be %2 or %1 not %3" ).arg( pageTitle ).arg( currentPage->objectName() ).arg( pageNames.value( pageTitle ) ).toLocal8Bit().constData() );

    page++;
  }
}

void QgsOptions::setCurrentPage( const QString &pageWidgetName )
{
  //find the page with a matching widget name
  for ( int page = 0; page < mOptionsStackedWidget->count(); ++page )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( page );
    if ( currentPage->objectName() == pageWidgetName )
    {
      //found the page, set it as current
      mOptionsStackedWidget->setCurrentIndex( page );
      return;
    }
    else if ( mTreeProxyModel )
    {
      const QModelIndex sourceIndex = mTreeProxyModel->pageNumberToSourceIndex( page );
      if ( sourceIndex.data().toString() == pageWidgetName || sourceIndex.data( Qt::UserRole + 1 ).toString() == pageWidgetName )
      {
        mOptionsStackedWidget->setCurrentIndex( page );
        return;
      }
    }
  }
}

void QgsOptions::setCurrentPage( const int pageNumber )
{
  if ( pageNumber >= 0 && pageNumber < mOptionsStackedWidget->count() )
    mOptionsStackedWidget->setCurrentIndex( pageNumber );
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
      QMessageBox::information( nullptr, tr( "Save Default Project" ), tr( "You must set a default project" ) );
      cbxProjectDefaultNew->setChecked( false );
    }
  }
}

void QgsOptions::setCurrentProjectDefault()
{
  QString fileName = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "project_default.qgs" );
  if ( QgsProject::instance()->write( fileName ) )
  {
    QMessageBox::information( nullptr, tr( "Save Default Project" ), tr( "Current project saved as default" ) );
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Save Default Project" ), tr( "Error saving current project as default" ) );
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

void QgsOptions::iconSizeChanged()
{
  QgisApp::instance()->setIconSizes( cmbIconSize->currentText().toInt() );
}


void QgsOptions::uiThemeChanged( const QString &theme )
{
  if ( theme == QgsApplication::themeName() )
    return;

  QgisApp::instance()->setTheme( theme );
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
                     tr( "QGIS files" ) + " (*.qgs *.qgz *.QGS *.QGZ)" );
  if ( !projPath.isNull() )
  {
    mProjectOnLaunchLineEdit->setText( projPath );
  }
}

void QgsOptions::saveOptions()
{
  for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalOptionWidgets ) )
  {
    if ( !widget->isValid() )
    {
      setCurrentPage( widget->objectName() );
      return;
    }
  }

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
  QgsApplication::setSvgPaths( pathsList );

  pathsList.clear();
  for ( int i = 0; i < mListComposerTemplatePaths->count(); ++i )
  {
    pathsList << mListComposerTemplatePaths->item( i )->text();
  }
  QgsLayout::settingsSearchPathForTemplates->setValue( pathsList );

  pathsList.clear();
  for ( int r = 0; r < mLocalizedDataPathListWidget->count(); r++ )
    pathsList << mLocalizedDataPathListWidget->item( r )->text();
  QgsApplication::localizedDataPathRegistry()->setPaths( pathsList );

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
  QgsNetworkAccessManager::setTimeout( mNetworkTimeoutSpinBox->value() );
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

  mSettings->setValue( QStringLiteral( "cache/size" ), QVariant::fromValue( mCacheSize->value() * 1024LL ) );

  //url with no proxy at all
  QStringList noProxyUrls;
  noProxyUrls.reserve( mNoProxyUrlListWidget->count() );
  for ( int i = 0; i < mNoProxyUrlListWidget->count(); ++i )
  {
    const QString host = mNoProxyUrlListWidget->item( i )->text();
    if ( !host.trimmed().isEmpty() )
      noProxyUrls << host;
  }
  mSettings->setValue( QStringLiteral( "proxy/noProxyUrls" ), noProxyUrls );

  QgisApp::instance()->namUpdate();

  //general settings
  mSettings->setValue( QStringLiteral( "/Map/searchRadiusMM" ), spinBoxIdentifyValue->value() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/color" ), mIdentifyHighlightColorButton->color().name() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/colorAlpha" ), mIdentifyHighlightColorButton->color().alpha() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/buffer" ), mIdentifyHighlightBufferSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/Map/highlight/minWidth" ), mIdentifyHighlightMinWidthSpinBox->value() );

  bool showLegendClassifiers = mSettings->value( QStringLiteral( "/qgis/showLegendClassifiers" ), false ).toBool();
  mSettings->setValue( QStringLiteral( "/qgis/showLegendClassifiers" ), cbxLegendClassifiers->isChecked() );
  QgsSettingsRegistryCore::settingsLayerTreeShowFeatureCountForNewLayers->setValue( mShowFeatureCountByDefaultCheckBox->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/hideSplash" ), cbxHideSplash->isChecked() );
  mSettings->setValue( QStringLiteral( "%1/disabled" ).arg( QgsNewsFeedParser::keyForFeed( QgsWelcomePage::newsFeedUrl() ) ), !cbxShowNews->isChecked(), QgsSettings::Core );

  mSettings->setValue( QStringLiteral( "/qgis/checkVersion" ), cbxCheckVersion->isChecked() );
  mSettings->setValue( QStringLiteral( "/qgis/dockAttributeTable" ), cbxAttributeTableDocked->isChecked() );
  mSettings->setEnumValue( QStringLiteral( "/qgis/attributeTableBehavior" ), ( QgsAttributeTableFilterModel::FilterMode )cmbAttrTableBehavior->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/attributeTableView" ), mAttrTableViewComboBox->currentData() );
  mSettings->setValue( QStringLiteral( "/qgis/attributeTableRowCache" ), spinBoxAttrTableRowCache->value() );
  mSettings->setEnumValue( QStringLiteral( "/qgis/promptForSublayers" ), static_cast< Qgis::SublayerPromptMode >( cmbPromptSublayers->currentData().toInt() ) );

  mSettings->setValue( QStringLiteral( "/qgis/scanItemsInBrowser2" ),
                       cmbScanItemsInBrowser->currentData().toString() );
  mSettings->setValue( QStringLiteral( "/qgis/scanZipInBrowser2" ),
                       cmbScanZipInBrowser->currentData().toString() );
  mSettings->setValue( QStringLiteral( "/qgis/monitorDirectoriesInBrowser" ), mCheckMonitorDirectories->isChecked() );

  mSettings->setValue( QStringLiteral( "/qgis/mainSnappingWidgetMode" ), mSnappingMainDialogComboBox->currentData() );

  mSettings->setValue( QStringLiteral( "/qgis/legendsymbolMinimumSize" ), mLegendSymbolMinimumSizeSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/legendsymbolMaximumSize" ), mLegendSymbolMaximumSizeSpinBox->value() );
  QgsSymbolLegendNode::MINIMUM_SIZE = mLegendSymbolMinimumSizeSpinBox->value();
  QgsSymbolLegendNode::MAXIMUM_SIZE = mLegendSymbolMaximumSizeSpinBox->value();

  mSettings->setValue( QStringLiteral( "/qgis/defaultLegendGraphicResolution" ), mLegendGraphicResolutionSpinBox->value() );
  mSettings->setValue( QStringLiteral( "/qgis/mapTipsDelay" ), mMapTipsDelaySpinBox->value() );
  QgsSettingsRegistryGui::settingsRespectScreenDPI->setValue( mRespectScreenDpiCheckBox->isChecked() );

  mSettings->setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), ( QgsClipboard::CopyFormat )mComboCopyFeatureFormat->currentData().toInt() );
  QgisApp::instance()->setMapTipsDelay( mMapTipsDelaySpinBox->value() );

  mSettings->setValue( QStringLiteral( "/qgis/legendDoubleClickAction" ), cmbLegendDoubleClickAction->currentIndex() );
  mSettings->setEnumValue( QStringLiteral( "/qgis/layerTreeInsertionMethod" ), mLayerTreeInsertionMethod->currentData().value<Qgis::LayerTreeInsertionMethod>() );

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
  mSettings->setEnumValue( QStringLiteral( "/qgis/enableMacros" ), mEnableMacrosComboBox->currentData().value<Qgis::PythonMacroMode>() );

  mSettings->setValue( QStringLiteral( "/qgis/defaultProjectPathsRelative" ),
                       static_cast< Qgis::FilePathType >( mDefaultPathsComboBox->currentData().toInt() ) == Qgis::FilePathType::Relative );

  mSettings->setEnumValue( QStringLiteral( "/qgis/defaultProjectFileFormat" ), mFileFormatQgsButton->isChecked() ? Qgis::ProjectFileFormat::Qgs : Qgis::ProjectFileFormat::Qgz );

  QgsApplication::setNullRepresentation( leNullValue->text() );
  mSettings->setValue( QStringLiteral( "/qgis/style" ), cmbStyle->currentText() );
  mSettings->setValue( QStringLiteral( "/qgis/toolbarIconSize" ), cmbIconSize->currentText() );

  mSettings->setValue( QStringLiteral( "/qgis/messageTimeout" ), mMessageTimeoutSpnBx->value() );

  mSettings->setValue( QStringLiteral( "/qgis/native_color_dialogs" ), mNativeColorDialogsChkBx->isChecked() );

  //check behavior so default projection when new layer is added with no
  //projection defined...
  if ( radPromptForProjection->isChecked() )
  {
    mSettings->setEnumValue( QStringLiteral( "/projections/unknownCrsBehavior" ), QgsOptions::UnknownLayerCrsBehavior::PromptUserForCrs, QgsSettings::App );
  }
  else if ( radUseProjectProjection->isChecked() )
  {
    mSettings->setEnumValue( QStringLiteral( "/projections/unknownCrsBehavior" ), QgsOptions::UnknownLayerCrsBehavior::UseProjectCrs, QgsSettings::App );
  }
  else if ( radCrsNoAction->isChecked() )
  {
    mSettings->setEnumValue( QStringLiteral( "/projections/unknownCrsBehavior" ), QgsOptions::UnknownLayerCrsBehavior::NoAction, QgsSettings::App );
  }
  else
  {
    mSettings->setEnumValue( QStringLiteral( "/projections/unknownCrsBehavior" ), QgsOptions::UnknownLayerCrsBehavior::UseDefaultCrs, QgsSettings::App );
  }

  mSettings->setValue( QStringLiteral( "/Projections/layerDefaultCrs" ), mLayerDefaultCrs.authid() );
  mSettings->setValue( QStringLiteral( "/projections/defaultProjectCrs" ), leProjectGlobalCrs->crs().authid(), QgsSettings::App );
  mSettings->setEnumValue( QStringLiteral( "/projections/newProjectCrsBehavior" ), radProjectUseCrsOfFirstLayer->isChecked() ? QgsGui::UseCrsOfFirstLayerAdded : QgsGui::UsePresetCrs, QgsSettings::App );
  mSettings->setValue( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), mShowDatumTransformDialogCheckBox->isChecked(), QgsSettings::App );
  mSettings->setValue( QStringLiteral( "/projections/crsAccuracyWarningThreshold" ), mCrsAccuracySpin->value(), QgsSettings::App );
  mSettings->setValue( QStringLiteral( "/projections/crsAccuracyIndicator" ), mCrsAccuracyIndicatorCheck->isChecked(), QgsSettings::App );

  //measurement settings
  mSettings->setValue( QStringLiteral( "measure/planimetric" ), mPlanimetricMeasurementsComboBox->isChecked(), QgsSettings::Core );

  Qgis::DistanceUnit distanceUnit = static_cast< Qgis::DistanceUnit >( mDistanceUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/displayunits" ), QgsUnitTypes::encodeUnit( distanceUnit ) );

  Qgis::AreaUnit areaUnit = static_cast< Qgis::AreaUnit >( mAreaUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/areaunits" ), QgsUnitTypes::encodeUnit( areaUnit ) );

  Qgis::AngleUnit angleUnit = static_cast< Qgis::AngleUnit >( mAngleUnitsComboBox->currentData().toInt() );
  mSettings->setValue( QStringLiteral( "/qgis/measure/angleunits" ), QgsUnitTypes::encodeUnit( angleUnit ) );

  int decimalPlaces = mDecimalPlacesSpinBox->value();
  mSettings->setValue( QStringLiteral( "/qgis/measure/decimalplaces" ), decimalPlaces );

  bool baseUnit = mKeepBaseUnitCheckBox->isChecked();
  mSettings->setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), baseUnit );

  QgsMeasureDialog::settingClipboardHeader->setValue( mIncludeHeader->isChecked() );
  QgsMeasureDialog::settingClipboardAlwaysUseDecimalPoint->setValue( mAlwaysUseDecimalPoint->isChecked() );
  QString separator;
  if ( mSeparatorTab->isChecked() )
    separator = QStringLiteral( "\t" );
  else if ( mSeparatorComma->isChecked() )
    separator = QStringLiteral( "," );
  else if ( mSeparatorSemicolon->isChecked() )
    separator = QStringLiteral( ";" );
  else if ( mSeparatorSpace->isChecked() )
    separator = QStringLiteral( " " );
  else if ( mSeparatorColon->isChecked() )
    separator = QStringLiteral( ":" );
  else
    separator = mSeparatorCustom->text();

  QgsMeasureDialog::settingClipboardSeparator->setValue( separator );

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
  mSettings->setValue( QStringLiteral( "/qgis/reverse_wheel_zoom" ), reverseWheelZoom->isChecked() );

  //digitizing
  QgsSettingsRegistryCore::settingsDigitizingLineWidth->setValue( mLineWidthSpinBox->value() );
  QColor digitizingColor = mLineColorToolButton->color();
  QgsSettingsRegistryCore::settingsDigitizingLineColor->setValue( digitizingColor );

  digitizingColor = mFillColorToolButton->color();
  QgsSettingsRegistryCore::settingsDigitizingFillColor->setValue( digitizingColor );

  QgsSettingsRegistryCore::settingsDigitizingLineGhost->setValue( mLineGhostCheckBox->isChecked() );

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->setValue( mDefaultZValueSpinBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->setValue( mDefaultMValueSpinBox->value() );

  //default snap mode
  QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled->setValue( mSnappingEnabledDefault->isChecked() );
  QgsSettingsRegistryCore::settingsDigitizingDefaultSnapType->setValue( static_cast<Qgis::SnappingType>( mDefaultSnapTypeComboBox->currentData().toInt() ) );
  QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingTolerance->setValue( mDefaultSnappingToleranceSpinBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEdit->setValue( mSearchRadiusVertexEditSpinBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingDefaultSnappingToleranceUnit->setValue(
    ( mDefaultSnappingToleranceComboBox->currentIndex() == 0 ? Qgis::MapToolUnit::Project : Qgis::MapToolUnit::Pixels ) );
  QgsSettingsRegistryCore::settingsDigitizingSearchRadiusVertexEditUnit->setValue(
    ( mSearchRadiusVertexEditComboBox->currentIndex()  == 0 ? Qgis::MapToolUnit::Project : Qgis::MapToolUnit::Pixels ) );

  QgsSettingsRegistryCore::settingsDigitizingSnapColor->setValue( mSnappingMarkerColorButton->color() );
  QgsSettingsRegistryCore::settingsDigitizingSnapTooltip->setValue( mSnappingTooltipsCheckbox->isChecked() );
  QgsSettingsRegistryCore::settingsDigitizingSnapInvisibleFeature->setValue( mEnableSnappingOnInvisibleFeatureCheckbox->isChecked() );

  QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected->setValue( mMarkersOnlyForSelectedCheckBox->isChecked() );

  QString markerComboText = mMarkerStyleComboBox->currentText();
  if ( markerComboText == tr( "Semi Transparent Circle" ) )
  {
    QgsSettingsRegistryCore::settingsDigitizingMarkerStyle->setValue( QStringLiteral( "SemiTransparentCircle" ) );
  }
  else if ( markerComboText == tr( "Cross" ) )
  {
    QgsSettingsRegistryCore::settingsDigitizingMarkerStyle->setValue( QStringLiteral( "Cross" ) );
  }
  else if ( markerComboText == tr( "None" ) )
  {
    QgsSettingsRegistryCore::settingsDigitizingMarkerStyle->setValue( QStringLiteral( "None" ) );
  }
  QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm->setValue( mMarkerSizeSpinBox->value() );

  QgsSettingsRegistryCore::settingsDigitizingReuseLastValues->setValue( chkReuseLastValues->isChecked() );
  QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog->setValue( chkDisableAttributeValuesDlg->isChecked() );
  QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->setValue( mValidateGeometries->currentIndex() );

  QgsSettingsRegistryCore::settingsDigitizingOffsetJoinStyle->setValue( mOffsetJoinStyleComboBox->currentData().value<Qgis::JoinStyle>() );
  QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->setValue( mOffsetQuadSegSpinBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingOffsetMiterLimit->setValue( mCurveOffsetMiterLimitComboBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingOffsetCapStyle->setValue( mOffsetCapStyleComboBox->currentData().value<Qgis::EndCapStyle>() );

  QgsSettingsRegistryCore::settingsDigitizingConvertToCurve->setValue( mTracingConvertToCurveCheckBox->isChecked() );
  QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance->setValue( mTracingCustomAngleToleranceSpinBox->value() );
  QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance->setValue( mTracingCustomDistanceToleranceSpinBox->value() );

  // default scale list
  QStringList myPaths;
  for ( int i = 0; i < mListGlobalScales->count(); ++i )
  {
    myPaths << mListGlobalScales->item( i )->text();
  }
  QgsSettingsRegistryCore::settingsMapScales->setValue( myPaths );

  //
  // Color palette
  //
  if ( mTreeCustomColors->isDirty() )
  {
    mTreeCustomColors->saveColorsToScheme();
  }

  //
  // Layout settings
  //

  //default font
  QString layoutFont = mComposerFontComboBox->currentFont().family();
  mSettings->setValue( QStringLiteral( "LayoutDesigner/defaultFont" ), layoutFont, QgsSettings::Gui );

  //grid color
  mSettings->setValue( QStringLiteral( "LayoutDesigner/gridRed" ), mGridColorButton->color().red(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/gridGreen" ), mGridColorButton->color().green(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/gridBlue" ), mGridColorButton->color().blue(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/gridAlpha" ), mGridColorButton->color().alpha(), QgsSettings::Gui );

  //grid style
  if ( mGridStyleComboBox->currentText() == tr( "Solid" ) )
  {
    mSettings->setValue( QStringLiteral( "LayoutDesigner/gridStyle" ), "Solid", QgsSettings::Gui );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Dots" ) )
  {
    mSettings->setValue( QStringLiteral( "LayoutDesigner/gridStyle" ), "Dots", QgsSettings::Gui );
  }
  else if ( mGridStyleComboBox->currentText() == tr( "Crosses" ) )
  {
    mSettings->setValue( QStringLiteral( "LayoutDesigner/gridStyle" ), "Crosses", QgsSettings::Gui );
  }

  //grid and guide defaults
  mSettings->setValue( QStringLiteral( "LayoutDesigner/defaultSnapGridResolution" ), mGridResolutionSpinBox->value(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/defaultSnapTolerancePixels" ), mSnapToleranceSpinBox->value(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetX" ), mOffsetXSpinBox->value(), QgsSettings::Gui );
  mSettings->setValue( QStringLiteral( "LayoutDesigner/defaultSnapGridOffsetY" ), mOffsetYSpinBox->value(), QgsSettings::Gui );

  //
  // Locale settings
  //
  QgsApplication::settingsLocaleUserLocale->setValue( cboTranslation->currentData().toString() );
  QgsApplication::settingsLocaleOverrideFlag->setValue( grpLocale->isChecked() );
  QgsApplication::settingsLocaleGlobalLocale->setValue( cboGlobalLocale->currentData( ).toString() );

  // Number settings
  QgsApplication::settingsLocaleShowGroupSeparator->setValue( cbShowGroupSeparator->isChecked( ) );

  QgsLocalDefaultSettings::setBearingFormat( mBearingFormat.get() );
  QgsLocalDefaultSettings::setGeographicCoordinateFormat( mCoordinateFormat.get() );

#ifdef HAVE_OPENCL
  // OpenCL settings
  QgsOpenClUtils::setEnabled( mGPUEnableCheckBox->isChecked() );
  QString preferredDevice( mOpenClDevicesCombo->currentData().toString() );
  QgsOpenClUtils::storePreferredDevice( preferredDevice );
#endif

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

  // only update stylesheet if changed
  const double newFontSize = spinFontSize->value();
  const QString newUserFontFamily = mFontFamilyRadioQt->isChecked() ? QString() : mFontFamilyComboBox->currentFont().family();
  if ( ( newUserFontFamily.isEmpty() && !mStyleSheetBuilder->userFontFamily().isEmpty() )
       || ( !newUserFontFamily.isEmpty() && newUserFontFamily != mStyleSheetBuilder->fontFamily() )
       || ( newFontSize != mStyleSheetBuilder->fontSize() ) )
  {
    mStyleSheetBuilder->setUserFontSize( newFontSize );
    mStyleSheetBuilder->setUserFontFamily( newUserFontFamily );
    // trigger a style sheet build to propagate saved settings
    mStyleSheetBuilder->updateStyleSheet();
  }

  mDefaultDatumTransformTableWidget->transformContext().writeSettings();

  mLocatorOptionsWidget->commitChanges();

  for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalOptionWidgets ) )
  {
    widget->apply();
  }

  QgsGui::instance()->emitOptionsChanged();
}

void QgsOptions::rejectOptions()
{
}

void QgsOptions::leLayerGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  mLayerDefaultCrs = crs;
}

void QgsOptions::lstRasterDrivers_itemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column )
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

  QgsDialog dlg( this, Qt::WindowFlags(), QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
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

QStringList QgsOptions::i18nList()
{
  QStringList myList;
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
  varApplyCmbBx->addItem( tr( "Skip" ), QVariant( "skip" ) );
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
                    QFileDialog::Options()
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
                    QFileDialog::Options()
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
                    QFileDialog::Options()
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

void QgsOptions::addNoProxyUrl()
{
  QListWidgetItem *newItem = new QListWidgetItem( mNoProxyUrlListWidget );
  newItem->setText( QStringLiteral( "URL" ) );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  mNoProxyUrlListWidget->addItem( newItem );
  mNoProxyUrlListWidget->setCurrentItem( newItem );
}

void QgsOptions::removeNoProxyUrl()
{
  int currentRow = mNoProxyUrlListWidget->currentRow();
  QListWidgetItem *itemToRemove = mNoProxyUrlListWidget->takeItem( currentRow );
  delete itemToRemove;
}

void QgsOptions::browseCacheDirectory()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a directory" ),
                    QDir::toNativeSeparators( mCacheDirectory->text() ),
                    QFileDialog::Options()
                  );

  if ( !myDir.isEmpty() )
  {
    mCacheDirectory->setText( QDir::toNativeSeparators( myDir ) );
  }

}

void QgsOptions::clearCache()
{
  QgsNetworkAccessManager::instance()->cache()->clear();

  // Clear WFS XSD cache used by OGR GMLAS driver
  QString cacheDirectory = mSettings->value( QStringLiteral( "cache/directory" ) ).toString();
  if ( cacheDirectory.isEmpty() )
    cacheDirectory = QStandardPaths::writableLocation( QStandardPaths::CacheLocation );
  if ( !cacheDirectory.endsWith( QDir::separator() ) )
  {
    cacheDirectory.push_back( QDir::separator() );
  }
  // Must be kept in sync with QgsWFSProvider::readAttributesFromSchemaWithGMLAS()
  cacheDirectory += QLatin1String( "gmlas_xsd_cache" );
  QDir dir( cacheDirectory );
  dir.removeRecursively();

  QMessageBox::information( this, tr( "Clear Cache" ), tr( "Content cache has been cleared." ) );
}

void QgsOptions::clearAccessCache()
{
  QgsNetworkAccessManager::instance()->clearAccessCache();
  QMessageBox::information( this, tr( "Clear Cache" ), tr( "Connection authentication cache has been cleared." ) );
}

void QgsOptions::optionsStackedWidget_CurrentChanged( int index )
{
  QgsOptionsDialogBase::optionsStackedWidget_CurrentChanged( index );

  Q_UNUSED( index )
  // load gdal driver list when gdal tab is first opened
  if ( mOptionsStackedWidget->currentWidget()->objectName() == QLatin1String( "mOptionsPageGDAL" )
       && ! mLoadedGdalDriverList )
  {
    loadGdalDriverList();
  }
}

void QgsOptions::loadGdalDriverList()
{
  QgsApplication::registerGdalDriversFromSettings();

  const QStringList mySkippedDrivers = QgsApplication::skippedGdalDrivers();
  GDALDriverH myGdalDriver; // current driver
  QString myGdalDriverDescription;
  QStringList myDrivers;
  QStringList myGdalWriteDrivers;
  QMap<QString, QString> rasterDriversFlags;
  QMap<QString, QString> vectorDriversFlags;
  QMap<QString, QString> myDriversExt, myDriversLongName;
  QMap<QString, QSet< int >> driversType;

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

    // in GDAL 2.0 both vector and raster drivers are returned by GDALGetDriver
    myGdalDriverDescription = GDALGetDescription( myGdalDriver );
    myDrivers << myGdalDriverDescription;
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_RASTER, nullptr ) ) == QLatin1String( "YES" ) )
    {
      driversType[myGdalDriverDescription].insert( static_cast< int >( Qgis::LayerType::Raster ) );
    }
    if ( QString( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_VECTOR, nullptr ) ) == QLatin1String( "YES" ) )
    {
      driversType[myGdalDriverDescription].insert( static_cast< int >( Qgis::LayerType::Vector ) );
    }

    QgsDebugMsgLevel( QStringLiteral( "driver #%1 - %2" ).arg( i ).arg( myGdalDriverDescription ), 2 );

    // get driver R/W flags, adopted from GDALGeneralCmdLineProcessor()
    if ( driversType.value( myGdalDriverDescription ).contains( static_cast< int >( Qgis::LayerType::Raster ) ) )
    {
      QString driverFlags = "";
      if ( QgsGdalUtils::supportsRasterCreate( myGdalDriver ) )
      {
        myGdalWriteDrivers << myGdalDriverDescription;
        driverFlags = "rw+";
      }
      else if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_CREATECOPY,
                                     nullptr ) )
        driverFlags = "rw";
      else
        driverFlags = "ro";

      if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_VIRTUALIO, nullptr ) )
        driverFlags += "v";

      rasterDriversFlags[myGdalDriverDescription] = driverFlags;
    }
    if ( driversType.value( myGdalDriverDescription ).contains( static_cast< int >( Qgis::LayerType::Vector ) ) )
    {
      QString driverFlags = "";
      if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_OPEN, nullptr ) )
        driverFlags = "r";

      if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_CREATE, nullptr ) )
        driverFlags += "w+";
      else if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_CREATECOPY, nullptr ) )
        driverFlags += "w";
      else
        driverFlags += "o";

      if ( GDALGetMetadataItem( myGdalDriver, GDAL_DCAP_VIRTUALIO, nullptr ) )
        driverFlags += "v";

      vectorDriversFlags[myGdalDriverDescription] = driverFlags;
    }

    // get driver extensions and long name
    // the gdal provider can override/add extensions but there is no interface to query this
    // aside from parsing QgsRasterLayer::buildSupportedRasterFileFilter()
    myDriversExt[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_EXTENSION", "" ) ).toLower();
    myDriversLongName[myGdalDriverDescription] = QString( GDALGetMetadataItem( myGdalDriver, "DMD_LONGNAME", "" ) );
  }

  // restore active drivers
  QgsApplication::applyGdalSkippedDrivers();

  myDrivers.removeDuplicates();
  // myDrivers.sort();
  // sort list case insensitive - no existing function for this!
  QMap<QString, QString> strMap;

  for ( const QString &str : std::as_const( myDrivers ) )
    strMap.insert( str.toLower(), str );
  myDrivers = strMap.values();

  for ( const QString &driverName : std::as_const( myDrivers ) )
  {
    const QSet< int > layerTypes = driversType[driverName];
    for ( int layerType : layerTypes )
    {
      QTreeWidgetItem *driverItem = new QTreeWidgetItem( QStringList( driverName ) );
      if ( mySkippedDrivers.contains( driverName ) )
      {
        driverItem->setCheckState( 0, Qt::Unchecked );
      }
      else
      {
        driverItem->setCheckState( 0, Qt::Checked );
      }

      // add driver metadata
      driverItem->setData( 0, Qt::UserRole, driverName );
      driverItem->setText( 1, myDriversExt[driverName] );
      const QString driverFlags = static_cast< Qgis::LayerType >( layerType ) == Qgis::LayerType::Raster ? rasterDriversFlags[driverName] : vectorDriversFlags[driverName];
      driverItem->setText( 2, driverFlags );
      driverItem->setText( 3, myDriversLongName[driverName] );
      if ( static_cast< Qgis::LayerType >( layerType ) == Qgis::LayerType::Raster )
      {
        lstRasterDrivers->addTopLevelItem( driverItem );
      }
      else
      {
        lstVectorDrivers->addTopLevelItem( driverItem );
      }
    }
  }

  // adjust column width
  for ( int i = 0; i < 4; i++ )
  {
    lstRasterDrivers->resizeColumnToContents( i );
    lstRasterDrivers->setColumnWidth( i, lstRasterDrivers->columnWidth( i ) + 5 );

    lstVectorDrivers->resizeColumnToContents( i );
    lstVectorDrivers->setColumnWidth( i, lstVectorDrivers->columnWidth( i ) + 5 );
  }

  // populate cmbEditCreateOptions with gdal write drivers - sorted, GTiff first
  strMap.clear();
  for ( const QString &str : std::as_const( myGdalWriteDrivers ) )
    strMap.insert( str.toLower(), str );
  myGdalWriteDrivers = strMap.values();
  myGdalWriteDrivers.removeAll( QStringLiteral( "Gtiff" ) );
  myGdalWriteDrivers.prepend( QStringLiteral( "GTiff" ) );
  cmbEditCreateOptions->clear();
  for ( const QString &myName : std::as_const( myGdalWriteDrivers ) )
  {
    cmbEditCreateOptions->addItem( myName );
  }

}

void QgsOptions::saveGdalDriverList()
{
  bool driverUnregisterNeeded = false;
  const auto oldSkippedGdalDrivers = QgsApplication::skippedGdalDrivers();
  auto deferredSkippedGdalDrivers = QgsApplication::deferredSkippedGdalDrivers();
  QStringList skippedGdalDrivers;

  auto checkDriver = [ & ]( QTreeWidgetItem * item )
  {
    const auto &driverName( item->text( 0 ) );
    if ( item->checkState( 0 ) == Qt::Unchecked )
    {
      skippedGdalDrivers << driverName;
      if ( !deferredSkippedGdalDrivers.contains( driverName ) &&
           !oldSkippedGdalDrivers.contains( driverName ) )
      {
        deferredSkippedGdalDrivers << driverName;
        driverUnregisterNeeded = true;
      }
    }
    else
    {
      if ( deferredSkippedGdalDrivers.contains( driverName ) )
      {
        deferredSkippedGdalDrivers.removeAll( driverName );
      }
    }
  };

  // raster drivers
  for ( int i = 0; i < lstRasterDrivers->topLevelItemCount(); i++ )
  {
    checkDriver( lstRasterDrivers->topLevelItem( i ) );
  }

  // vector drivers
  for ( int i = 0; i < lstVectorDrivers->topLevelItemCount(); i++ )
  {
    checkDriver( lstVectorDrivers->topLevelItem( i ) );
  }

  if ( driverUnregisterNeeded )
  {
    QMessageBox::information( this, tr( "Drivers Disabled" ),
                              tr( "One or more drivers have been disabled. This will only take effect after QGIS is restarted." ) );
  }
  QgsApplication::setSkippedGdalDrivers( skippedGdalDrivers, deferredSkippedGdalDrivers );
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

  QStringList myScalesList = Qgis::defaultProjectScales().split( ',' );
  const auto constMyScalesList = myScalesList;
  for ( const QString &scale : constMyScalesList )
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
    QgsDebugError( msg );
  }

  const auto constMyScales = myScales;
  for ( const QString &scale : constMyScales )
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
    QgsDebugError( msg );
  }
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

void QgsOptions::removeLocalizedDataPath()
{
  qDeleteAll( mLocalizedDataPathListWidget->selectedItems() );
}

void QgsOptions::addLocalizedDataPath()
{
  QString myDir = QFileDialog::getExistingDirectory(
                    this,
                    tr( "Choose a Directory" ),
                    QDir::homePath(),
                    QFileDialog::Options()
                  );

  if ( ! myDir.isEmpty() )
  {
    QListWidgetItem *newItem = new QListWidgetItem( mLocalizedDataPathListWidget );
    newItem->setText( myDir );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mLocalizedDataPathListWidget->addItem( newItem );
    mLocalizedDataPathListWidget->setCurrentItem( newItem );
  }
}

void QgsOptions::moveLocalizedDataPathUp()
{
  QList<QListWidgetItem *> selectedItems = mLocalizedDataPathListWidget->selectedItems();
  QList<QListWidgetItem *>::iterator itemIt = selectedItems.begin();
  for ( ; itemIt != selectedItems.end(); ++itemIt )
  {
    int row = mLocalizedDataPathListWidget->row( *itemIt );
    mLocalizedDataPathListWidget->takeItem( row );
    mLocalizedDataPathListWidget->insertItem( row - 1, *itemIt );
  }
}

void QgsOptions::moveLocalizedDataPathDown()
{
  QList<QListWidgetItem *> selectedItems = mLocalizedDataPathListWidget->selectedItems();
  QList<QListWidgetItem *>::iterator itemIt = selectedItems.begin();
  for ( ; itemIt != selectedItems.end(); ++itemIt )
  {
    int row = mLocalizedDataPathListWidget->row( *itemIt );
    mLocalizedDataPathListWidget->takeItem( row );
    mLocalizedDataPathListWidget->insertItem( row + 1, *itemIt );
  }
}

void QgsOptions::alwaysUseDecimalPointChanged( bool checked )
{
  // In the Measure Tool copy feature,
  // comma is only allowed as field separator if the locale do not use it as decimal separator
  // or if the user has overridden the decimal separator to always be a point
  mSeparatorComma->setEnabled( checked || QLocale().decimalPoint() != QLatin1String( "," ) );

  // If comma was checked and is now disabled, switch to semicolon
  if ( mSeparatorComma->isChecked() && !mSeparatorComma->isEnabled() )
  {
    mSeparatorSemicolon->setChecked( true );
  }
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

void QgsOptions::refreshSchemeComboBox()
{
  mColorSchemesComboBox->blockSignals( true );
  mColorSchemesComboBox->clear();
  QList<QgsColorScheme *> schemeList = QgsApplication::colorSchemeRegistry()->schemes();
  QList<QgsColorScheme *>::const_iterator schemeIt = schemeList.constBegin();
  for ( ; schemeIt != schemeList.constEnd(); ++schemeIt )
  {
    mColorSchemesComboBox->addItem( ( *schemeIt )->schemeName() );
  }
  mColorSchemesComboBox->blockSignals( false );
}

void QgsOptions::updateSampleLocaleText()
{
  QLocale locale( cboGlobalLocale->currentData( ).toString() );
  if ( cbShowGroupSeparator->isChecked( ) )
  {
    locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  }
  else
  {
    locale.setNumberOptions( locale.numberOptions() |= QLocale::NumberOption::OmitGroupSeparator );
  }
  lblLocaleSample->setText( tr( "Sample date: %1 money: %2 int: %3 float: %4" ).arg(
                              QDate::currentDate().toString( locale.dateFormat( QLocale::FormatType::ShortFormat ) ),
                              locale.toCurrencyString( 1000.00 ),
                              locale.toString( 1000 ),
                              locale.toString( 1000.00, 'f', 2 ) ) );
}

void QgsOptions::updateActionsForCurrentColorScheme( QgsColorScheme *scheme )
{
  if ( !scheme )
    return;

  mButtonImportColors->setEnabled( scheme->isEditable() );
  mButtonPasteColors->setEnabled( scheme->isEditable() );
  mButtonAddColor->setEnabled( scheme->isEditable() );
  mButtonRemoveColor->setEnabled( scheme->isEditable() );

  QgsUserColorScheme *userScheme = dynamic_cast<QgsUserColorScheme *>( scheme );
  mActionRemovePalette->setEnabled( static_cast< bool >( userScheme ) && userScheme->isEditable() );
  if ( userScheme )
  {
    mActionShowInButtons->setEnabled( true );
    whileBlocking( mActionShowInButtons )->setChecked( userScheme->flags() & QgsColorScheme::ShowInColorButtonMenu );
  }
  else
  {
    whileBlocking( mActionShowInButtons )->setChecked( false );
    mActionShowInButtons->setEnabled( false );
  }
}

void QgsOptions::scaleItemChanged( QListWidgetItem *changedScaleItem )
{
  // Check if the new value is valid, restore the old value if not.
  const thread_local QRegularExpression sRegExp( QStringLiteral( "^1:0*[1-9]\\d*$" ) );
  if ( sRegExp.match( changedScaleItem->text() ).hasMatch() )
  {
    //Remove leading zeroes from the denominator
    const thread_local QRegularExpression sRemoveLeadingZeroesRegEx( QStringLiteral( "^1:0*" ) );
    changedScaleItem->setText( changedScaleItem->text().replace( sRemoveLeadingZeroesRegEx, QStringLiteral( "1:" ) ) );
  }
  else
  {
    QMessageBox::warning( this, tr( "Set Scale" ), tr( "The text you entered is not a valid scale." ) );
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
  if ( mSettings->value( QStringLiteral( "/qgis/zoom_factor" ), 2 ).toDouble() <= 1.01 )
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
  for ( const QgsOptionsPageWidget *widget : std::as_const( mAdditionalOptionWidgets ) )
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

    if ( activeTab == mOptionsPageGeneral )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#general-options" );
    }
    else if ( activeTab == mOptionsPageSystem )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#env-options" );
    }
    else if ( activeTab == mOptionsPageTransformations )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#transformations-options" );
    }
    else if ( activeTab == mOptionsPageDataSources )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#datasources-options" );
    }
    else if ( activeTab == mOptionsPageGDAL )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#gdal-options" );
    }
    else if ( activeTab == mOptionsPageMapCanvas )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#canvas-legend-options" );
    }
    else if ( activeTab == mOptionsPageMapTools )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#maptools-options" );
    }
    else if ( activeTab == mOptionsPageDigitizing )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#digitizing-options" );
    }
    else if ( activeTab == mOptionsPageColors )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#colors-options" );
    }
    else if ( activeTab == mOptionsPageComposer )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#layout-options" );
    }
    else if ( activeTab == mOptionsPageNetwork )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#network-options" );
    }
    else if ( activeTab == mOptionsLocatorSettings )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#locator-options" );
    }
    else if ( activeTab == mOptionsPageAcceleration )
    {
      link = QStringLiteral( "introduction/qgis_configuration.html#acceleration-options" );
    }
    else if ( activeTab == mOptionsPageAuth )
    {
      link = QStringLiteral( "auth_system/index.html" );
    }
    else if ( activeTab == mOptionsPageVariables )
    {
      link = QStringLiteral( "introduction/general_tools.html#general-tools-variables" );
    }
    else if ( activeTab == mOptionsPageCRS )
    {
      link = QStringLiteral( "working_with_projections/working_with_projections.html" );
    }
  }
  QgsHelp::openHelp( link );
}

void QgsOptions::customizeBearingFormat()
{
  QgsBearingNumericFormatDialog dlg( mBearingFormat.get(), this );
  dlg.setWindowTitle( tr( "Bearing Format" ) );
  if ( dlg.exec() )
  {
    mBearingFormat.reset( dlg.format() );
  }
}

void QgsOptions::customizeCoordinateFormat()
{
  QgsGeographicCoordinateNumericFormatDialog dlg( mCoordinateFormat.get(), false, this );
  dlg.setWindowTitle( tr( "Coordinate Format" ) );
  if ( dlg.exec() )
  {
    mCoordinateFormat.reset( dlg.format() );
  }
}
