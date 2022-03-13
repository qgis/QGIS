/***************************************************************************
                            qgsprojectproperties.cpp
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
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

#include "qgsprojectproperties.h"

//qgis includes
#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgisapp.h"
#include "qgis.h"
#include "qgscoordinatetransform.h"
#include "qgsdatumtransformtablewidget.h"
#include "qgslayoutmanager.h"
#include "qgslogger.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsnative.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsscaleutils.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgsstylemanagerdialog.h"
#include "qgscolorramp.h"
#include "qgssymbolselectordialog.h"
#include "qgsrelationmanagerdialog.h"
#include "qgsrelationmanager.h"
#include "qgsdoublevalidator.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscolordialog.h"
#include "qgsexpressioncontext.h"
#include "qgsmapoverviewcanvas.h"
#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertreemodel.h"
#include "qgsunittypes.h"
#include "qgstablewidgetitem.h"
#include "qgstreewidgetitem.h"
#include "qgslayertree.h"
#include "qgsprintlayout.h"
#include "qgsmetadatawidget.h"
#include "qgsmessagelog.h"
#include "qgslayercapabilitiesmodel.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprojectservervalidator.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"
#include "qgsprojectviewsettings.h"
#include "qgsnumericformatwidget.h"
#include "qgsbearingnumericformat.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalutils.h"

//qt includes
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAbstractListModel>
#include <QList>
#include <QRegularExpressionValidator>

const char *QgsProjectProperties::GEO_NONE_DESC = QT_TRANSLATE_NOOP( "QgsOptions", "None / Planimetric" );

//stdc++ includes

QgsProjectProperties::QgsProjectProperties( QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags fl, const QList<QgsOptionsWidgetFactory *> &optionsFactories )
  : QgsOptionsDialogBase( QStringLiteral( "ProjectProperties" ), parent, fl )
  , mMapCanvas( mapCanvas )
  , mEllipsoidIndex( 0 )
{
  // set wait cursor since construction of the project properties
  // dialog can be slow
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  setupUi( this );

  mExtentGroupBox->setTitleBase( tr( "Set Project Full Extent" ) );
  mExtentGroupBox->setMapCanvas( mapCanvas, false );

  mMetadataWidget = new QgsMetadataWidget();
  mMetadataPage->layout()->addWidget( mMetadataWidget );

  connect( pbnAddScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnAddScale_clicked );
  connect( pbnRemoveScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnRemoveScale_clicked );
  connect( pbnImportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnImportScales_clicked );
  connect( pbnExportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnExportScales_clicked );
  connect( pbnWMSExtCanvas, &QPushButton::clicked, this, &QgsProjectProperties::pbnWMSExtCanvas_clicked );
  connect( pbnWMSAddSRS, &QToolButton::clicked, this, &QgsProjectProperties::pbnWMSAddSRS_clicked );
  connect( pbnWMSRemoveSRS, &QToolButton::clicked, this, &QgsProjectProperties::pbnWMSRemoveSRS_clicked );
  connect( pbnWMSSetUsedSRS, &QPushButton::clicked, this, &QgsProjectProperties::pbnWMSSetUsedSRS_clicked );
  connect( mAddWMSPrintLayoutButton, &QToolButton::clicked, this, &QgsProjectProperties::mAddWMSPrintLayoutButton_clicked );
  connect( mRemoveWMSPrintLayoutButton, &QToolButton::clicked, this, &QgsProjectProperties::mRemoveWMSPrintLayoutButton_clicked );
  connect( mAddLayerRestrictionButton, &QToolButton::clicked, this, &QgsProjectProperties::mAddLayerRestrictionButton_clicked );
  connect( mRemoveLayerRestrictionButton, &QToolButton::clicked, this, &QgsProjectProperties::mRemoveLayerRestrictionButton_clicked );
  connect( mWMSInspireScenario1, &QGroupBox::toggled, this, &QgsProjectProperties::mWMSInspireScenario1_toggled );
  connect( mWMSInspireScenario2, &QGroupBox::toggled, this, &QgsProjectProperties::mWMSInspireScenario2_toggled );
  connect( pbnWFSLayersSelectAll, &QPushButton::clicked, this, &QgsProjectProperties::pbnWFSLayersSelectAll_clicked );
  connect( pbnWFSLayersDeselectAll, &QPushButton::clicked, this, &QgsProjectProperties::pbnWFSLayersDeselectAll_clicked );
  connect( pbnWCSLayersSelectAll, &QPushButton::clicked, this, &QgsProjectProperties::pbnWCSLayersSelectAll_clicked );
  connect( pbnWCSLayersDeselectAll, &QPushButton::clicked, this, &QgsProjectProperties::pbnWCSLayersDeselectAll_clicked );
  connect( pbnLaunchOWSChecker, &QPushButton::clicked, this, &QgsProjectProperties::pbnLaunchOWSChecker_clicked );
  connect( pbtnStyleManager, &QPushButton::clicked, this, &QgsProjectProperties::pbtnStyleManager_clicked );
  connect( pbtnStyleMarker, &QToolButton::clicked, this, &QgsProjectProperties::pbtnStyleMarker_clicked );
  connect( pbtnStyleLine, &QToolButton::clicked, this, &QgsProjectProperties::pbtnStyleLine_clicked );
  connect( pbtnStyleFill, &QToolButton::clicked, this, &QgsProjectProperties::pbtnStyleFill_clicked );
  connect( pbtnStyleColorRamp, &QToolButton::clicked, this, &QgsProjectProperties::pbtnStyleColorRamp_clicked );
  connect( mButtonAddColor, &QToolButton::clicked, this, &QgsProjectProperties::mButtonAddColor_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectProperties::showHelp );
  connect( mCustomizeBearingFormatButton, &QPushButton::clicked, this, &QgsProjectProperties::customizeBearingFormat );
  connect( mCalculateFromLayerButton, &QPushButton::clicked, this, &QgsProjectProperties::calculateFromLayersButton_clicked );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mCoordinateDisplayComboBox->addItem( tr( "Map Units" ), MapUnits );
  mCoordinateDisplayComboBox->addItem( tr( "Decimal Degrees" ), DecimalDegrees );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, Minutes" ), DegreesMinutes );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, Minutes, Seconds" ), DegreesMinutesSeconds );

  mDistanceUnitsCombo->addItem( tr( "Meters" ), QgsUnitTypes::DistanceMeters );
  mDistanceUnitsCombo->addItem( tr( "Kilometers" ), QgsUnitTypes::DistanceKilometers );
  mDistanceUnitsCombo->addItem( tr( "Feet" ), QgsUnitTypes::DistanceFeet );
  mDistanceUnitsCombo->addItem( tr( "Yards" ), QgsUnitTypes::DistanceYards );
  mDistanceUnitsCombo->addItem( tr( "Miles" ), QgsUnitTypes::DistanceMiles );
  mDistanceUnitsCombo->addItem( tr( "Nautical Miles" ), QgsUnitTypes::DistanceNauticalMiles );
  mDistanceUnitsCombo->addItem( tr( "Centimeters" ), QgsUnitTypes::DistanceCentimeters );
  mDistanceUnitsCombo->addItem( tr( "Millimeters" ), QgsUnitTypes::DistanceMillimeters );
  mDistanceUnitsCombo->addItem( tr( "Degrees" ), QgsUnitTypes::DistanceDegrees );
  mDistanceUnitsCombo->addItem( tr( "Map Units" ), QgsUnitTypes::DistanceUnknownUnit );

  mAreaUnitsCombo->addItem( tr( "Square Meters" ), QgsUnitTypes::AreaSquareMeters );
  mAreaUnitsCombo->addItem( tr( "Square Kilometers" ), QgsUnitTypes::AreaSquareKilometers );
  mAreaUnitsCombo->addItem( tr( "Square Feet" ), QgsUnitTypes::AreaSquareFeet );
  mAreaUnitsCombo->addItem( tr( "Square Yards" ), QgsUnitTypes::AreaSquareYards );
  mAreaUnitsCombo->addItem( tr( "Square Miles" ), QgsUnitTypes::AreaSquareMiles );
  mAreaUnitsCombo->addItem( tr( "Hectares" ), QgsUnitTypes::AreaHectares );
  mAreaUnitsCombo->addItem( tr( "Acres" ), QgsUnitTypes::AreaAcres );
  mAreaUnitsCombo->addItem( tr( "Square Nautical Miles" ), QgsUnitTypes::AreaSquareNauticalMiles );
  mAreaUnitsCombo->addItem( tr( "Square Centimeters" ), QgsUnitTypes::AreaSquareCentimeters );
  mAreaUnitsCombo->addItem( tr( "Square Millimeters" ), QgsUnitTypes::AreaSquareMillimeters );
  mAreaUnitsCombo->addItem( tr( "Square Degrees" ), QgsUnitTypes::AreaSquareDegrees );
  mAreaUnitsCombo->addItem( tr( "Map Units" ), QgsUnitTypes::AreaUnknownUnit );

  mTransactionModeComboBox->addItem( tr( "Local Edit Buffer" ), static_cast< int >( Qgis::TransactionMode::Disabled ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Edits are buffered locally and sent to the provider when toggling layer editing mode." ), Qt::ToolTipRole );
  mTransactionModeComboBox->addItem( tr( "Automatic Transaction Groups" ), static_cast< int >( Qgis::TransactionMode::AutomaticGroups ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Automatic transactional editing means that on supported datasources (postgres databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction." ), Qt::ToolTipRole );
  mTransactionModeComboBox->addItem( tr( "Buffered Transaction Groups" ), static_cast< int >( Qgis::TransactionMode::BufferedGroups ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider)." ), Qt::ToolTipRole );

  projectionSelector->setShowNoProjection( true );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsProjectProperties::apply );
  connect( this, &QDialog::accepted, this, &QgsProjectProperties::apply );
  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, [ = ]
  {
    if ( mBlockCrsUpdates || !projectionSelector->hasValidSelection() )
      return;

    crsChanged( projectionSelector->crs() );
  } );

  connect( cmbEllipsoid, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsProjectProperties::updateEllipsoidUI );

  connect( radAutomatic, &QAbstractButton::toggled, spinBoxDP, &QWidget::setDisabled );
  connect( radAutomatic, &QAbstractButton::toggled, labelDP, &QWidget::setDisabled );
  connect( radManual, &QAbstractButton::toggled, spinBoxDP, &QWidget::setEnabled );
  connect( radManual, &QAbstractButton::toggled, labelDP, &QWidget::setEnabled );

  leSemiMajor->setValidator( new QgsDoubleValidator( leSemiMajor ) );
  leSemiMinor->setValidator( new QgsDoubleValidator( leSemiMinor ) );

  QgsSettings settings;

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  mCrs = QgsProject::instance()->crs();
  updateGuiForMapUnits();
  setSelectedCrs( QgsProject::instance()->crs() );

  // Datum transforms
  QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
  mDatumTransformTableWidget->setTransformContext( context );

  bool show = settings.value( QStringLiteral( "/projections/promptWhenMultipleTransformsExist" ), false, QgsSettings::App ).toBool();
  mShowDatumTransformDialogCheckBox->setChecked( show );

  QPolygonF mainCanvasPoly = mapCanvas->mapSettings().visiblePolygon();
  QgsGeometry g = QgsGeometry::fromQPolygonF( mainCanvasPoly );
  // close polygon
  mainCanvasPoly << mainCanvasPoly.at( 0 );
  if ( QgsProject::instance()->crs() !=
       QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
  {
    // reproject extent
    QgsCoordinateTransform ct( QgsProject::instance()->crs(),
                               QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance() );
    ct.setBallparkTransformsAreAppropriate( true );

    g = g.densifyByCount( 5 );
    try
    {
      g.transform( ct );
    }
    catch ( QgsCsException & )
    {
    }
  }
  projectionSelector->setPreviewRect( g.boundingBox() );

  mMapTileRenderingCheckBox->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "RenderMapTile" ), QStringLiteral( "/" ), false ) );

  // see end of constructor for updating of projection selector

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  const auto layers { QgsProject::instance()->layers<QgsVectorLayer *>() };
  for ( QgsVectorLayer *layer : layers )
  {
    if ( layer->isEditable() )
    {
      mTransactionModeComboBox->setEnabled( false );
      mTransactionModeComboBox->setToolTip( tr( "Layers are in edit mode. Stop edit mode on all layers to toggle transactional editing." ) );
    }
  }

  // Set time settings input
  QgsDateTimeRange range = QgsProject::instance()->timeSettings()->temporalRange();

  mStartDateTimeEdit->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTimeEdit->maximumDateTime() );
  mEndDateTimeEdit->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mEndDateTimeEdit->maximumDateTime() );
  mStartDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mStartDateTimeEdit->setDateTime( range.begin() );
  mEndDateTimeEdit->setDateTime( range.end() );

  title( QgsProject::instance()->title() );
  mProjectFileLineEdit->setText( QDir::toNativeSeparators( !QgsProject::instance()->fileName().isEmpty() ? QgsProject::instance()->fileName() : QgsProject::instance()->originalPath() ) );
  mProjectHomeLineEdit->setShowClearButton( true );
  mProjectHomeLineEdit->setText( QDir::toNativeSeparators( QgsProject::instance()->presetHomePath() ) );
  connect( mButtonSetProjectHome, &QToolButton::clicked, this, [ = ]
  {
    auto getAbsoluteHome = [this]()->QString
    {
      QString currentHome = QDir::fromNativeSeparators( mProjectHomeLineEdit->text() );
      if ( !currentHome.isEmpty() )
      {
        QFileInfo homeInfo( currentHome );
        if ( !homeInfo.isRelative() )
          return currentHome;
      }

      QFileInfo pfi( QgsProject::instance()->fileName() );
      if ( !pfi.exists() )
        return QDir::homePath();

      if ( !currentHome.isEmpty() )
      {
        // path is relative to project file
        return QDir::cleanPath( pfi.path() + '/' + currentHome );
      }
      else
      {
        return pfi.canonicalPath();
      }
    };

    QString newDir = QFileDialog::getExistingDirectory( this, tr( "Select Project Home Path" ), getAbsoluteHome() );
    if ( ! newDir.isNull() )
    {
      mProjectHomeLineEdit->setText( QDir::toNativeSeparators( newDir ) );
    }
  } );

  connect( mButtonOpenProjectFolder, &QToolButton::clicked, this, [ = ]
  {
    QString path;
    QString project = !QgsProject::instance()->fileName().isEmpty() ? QgsProject::instance()->fileName() : QgsProject::instance()->originalPath();
    QgsProjectStorage *storage = QgsProject::instance()->projectStorage();
    if ( storage )
      path = storage->filePath( project );
    else
      path = project;

    if ( !path.isEmpty() )
    {
      QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( path );
    }

  } );

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), true );
  if ( automaticPrecision )
  {
    radAutomatic->setChecked( true );
    spinBoxDP->setEnabled( false );
    labelDP->setEnabled( false );
  }
  else
  {
    radManual->setChecked( true );
    spinBoxDP->setEnabled( true );
    labelDP->setEnabled( true );
  }
  int dp = QgsProject::instance()->readNumEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ) );
  spinBoxDP->setValue( dp );

  cbxAbsolutePath->addItem( tr( "Absolute" ), static_cast< int >( Qgis::FilePathType::Absolute ) );
  cbxAbsolutePath->addItem( tr( "Relative" ), static_cast< int >( Qgis::FilePathType::Relative ) );
  cbxAbsolutePath->setCurrentIndex( cbxAbsolutePath->findData( static_cast< int >( QgsProject::instance()->filePathStorage() ) ) );

  // populate combo box with ellipsoids
  // selection of the ellipsoid from settings is deferred to a later point, because it would
  // be overridden in the meanwhile by the projection selector
  populateEllipsoidList();
  if ( !QgsProject::instance()->crs().isValid() )
  {
    cmbEllipsoid->setCurrentIndex( 0 );
    cmbEllipsoid->setEnabled( false );
  }
  else
  {
    // attempt to reset the projection ellipsoid according to the srs
    int index = 0;
    for ( int i = 0; i < mEllipsoidList.length(); i++ )
    {
      // TODO - use parameters instead of acronym
      if ( mEllipsoidList[ i ].acronym == QgsProject::instance()->crs().ellipsoidAcronym() )
      {
        index = i;
        break;
      }
    }
    updateEllipsoidUI( index );
  }

  QString format = QgsProject::instance()->readEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), QStringLiteral( "MU" ) );
  if ( format == QLatin1String( "MU" ) )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( MapUnits ) );
  else if ( format == QLatin1String( "DM" ) )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DegreesMinutes ) );
  else if ( format == QLatin1String( "DMS" ) )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DegreesMinutesSeconds ) );
  else
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DecimalDegrees ) );

  mDistanceUnitsCombo->setCurrentIndex( mDistanceUnitsCombo->findData( QgsProject::instance()->distanceUnits() ) );
  mAreaUnitsCombo->setCurrentIndex( mAreaUnitsCombo->findData( QgsProject::instance()->areaUnits() ) );

  //get the color selections and set the button color accordingly
  int myRedInt = settings.value( QStringLiteral( "qgis/default_selection_color_red" ), 255 ).toInt();
  int myGreenInt = settings.value( QStringLiteral( "qgis/default_selection_color_green" ), 255 ).toInt();
  int myBlueInt = settings.value( QStringLiteral( "qgis/default_selection_color_blue" ), 0 ).toInt();
  int myAlphaInt = settings.value( QStringLiteral( "qgis/default_selection_color_alpha" ), 255 ).toInt();
  QColor defaultSelectionColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );

  pbnSelectionColor->setContext( QStringLiteral( "gui" ) );
  pbnSelectionColor->setColor( QgsProject::instance()->selectionColor() );
  pbnSelectionColor->setDefaultColor( defaultSelectionColor );
  pbnSelectionColor->setColorDialogTitle( tr( "Selection Color" ) );
  pbnSelectionColor->setAllowOpacity( true );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  myGreenInt = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  myBlueInt = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  QColor defaultCanvasColor = QColor( myRedInt, myGreenInt, myBlueInt );

  pbnCanvasColor->setContext( QStringLiteral( "gui" ) );
  pbnCanvasColor->setColor( QgsProject::instance()->backgroundColor() );
  pbnCanvasColor->setDefaultColor( defaultCanvasColor );

  //get project scales
  const QVector< double > projectScales = QgsProject::instance()->viewSettings()->mapScales();
  if ( !projectScales.isEmpty() )
  {
    for ( double scale : projectScales )
    {
      addScaleToScaleList( QStringLiteral( "1:%1" ).arg( QLocale().toString( scale, 'f', 0 ) ) );
    }
  }
  connect( lstScales, &QListWidget::itemChanged, this, &QgsProjectProperties::scaleItemChanged );

  grpProjectScales->setChecked( QgsProject::instance()->viewSettings()->useProjectScales() );

  const QgsReferencedRectangle presetExtent = QgsProject::instance()->viewSettings()->presetFullExtent();
  mExtentGroupBox->setOutputCrs( QgsProject::instance()->crs() );
  if ( presetExtent.isNull() )
    mExtentGroupBox->setOutputExtentFromUser( QgsProject::instance()->viewSettings()->fullExtent(), QgsProject::instance()->crs() );
  else
    mExtentGroupBox->setOutputExtentFromUser( presetExtent, presetExtent.crs() );
  mExtentGroupBox->setChecked( !presetExtent.isNull() );

  mLayerCapabilitiesModel = new QgsLayerCapabilitiesModel( QgsProject::instance(), this );
  mLayerCapabilitiesModel->setLayerTreeModel( new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), mLayerCapabilitiesModel ) );
  mLayerCapabilitiesTree->setModel( mLayerCapabilitiesModel );
  mLayerCapabilitiesTree->resizeColumnToContents( 0 );
  mLayerCapabilitiesTree->header()->show();
  mLayerCapabilitiesTree->setSelectionBehavior( QAbstractItemView::SelectItems );
  mLayerCapabilitiesTree->setSelectionMode( QAbstractItemView::MultiSelection );
  mLayerCapabilitiesTree->expandAll();
  connect( mLayerCapabilitiesTree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
           [ = ]( const QItemSelection & selected, const QItemSelection & deselected )
  {
    Q_UNUSED( selected )
    Q_UNUSED( deselected )
    bool hasSelection = !mLayerCapabilitiesTree->selectionModel()->selectedIndexes().isEmpty();
    mLayerCapabilitiesToggleSelectionButton->setEnabled( hasSelection );
  } );

  mLayerCapabilitiesTreeFilterLineEdit->setShowClearButton( true );
  mLayerCapabilitiesTreeFilterLineEdit->setShowSearchIcon( true );
  mLayerCapabilitiesTreeFilterLineEdit->setPlaceholderText( tr( "Filter layers…" ) );
  connect( mLayerCapabilitiesTreeFilterLineEdit, &QgsFilterLineEdit::textChanged, this, [ = ]( const QString & filterText )
  {
    mLayerCapabilitiesModel->setFilterText( filterText );
    mLayerCapabilitiesTree->expandAll();
  } );

  connect( mLayerCapabilitiesToggleSelectionButton, &QToolButton::clicked, this, [ = ]( bool clicked )
  {
    Q_UNUSED( clicked )
    const QModelIndexList indexes = mLayerCapabilitiesTree->selectionModel()->selectedIndexes();
    mLayerCapabilitiesModel->toggleSelectedItems( indexes );
    mLayerCapabilitiesTree->repaint();
  } );

  connect( mShowSpatialLayersCheckBox, &QCheckBox::stateChanged, this, [ = ]( int state )
  {
    mLayerCapabilitiesModel->setShowSpatialLayersOnly( static_cast<bool>( state ) );
  } );

  grpOWSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ) ) );
  mWMSName->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSRootName" ), QStringLiteral( "/" ) ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ), QString() ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ), QString() ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ), QString() ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ), QString() ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ), QString() ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ), QString() ) );
  mWMSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), QString() ) );
  mWMSKeywordList->setText( QgsProject::instance()->readListEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ) ).join( QLatin1String( ", " ) ) );

  mWMSOnlineResourceExpressionButton->registerExpressionContextGenerator( this );
  mWMSOnlineResourceExpressionButton->setToProperty( QgsProject::instance()->dataDefinedServerProperties().property( QgsProject::DataDefinedServerProperty::WMSOnlineResource ) );

  // WMS Name validator
  QValidator *shortNameValidator = new QRegularExpressionValidator( QgsApplication::shortNameRegularExpression(), this );
  mWMSName->setValidator( shortNameValidator );

  // WMS Contact Position
  QString contactPositionText = QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ), QString() );
  mWMSContactPositionCb->addItem( QString() );
  mWMSContactPositionCb->addItem( tr( "Custodian" ), "custodian" );
  mWMSContactPositionCb->addItem( tr( "Owner" ), "owner" );
  mWMSContactPositionCb->addItem( tr( "User" ), "user" );
  mWMSContactPositionCb->addItem( tr( "Distributor" ), "distributor" );
  mWMSContactPositionCb->addItem( tr( "Originator" ), "originator" );
  mWMSContactPositionCb->addItem( tr( "Point of Contact" ), "pointOfContact" );
  mWMSContactPositionCb->addItem( tr( "Principal Investigator" ), "principalInvestigator" );
  mWMSContactPositionCb->addItem( tr( "Processor" ), "processor" );
  mWMSContactPositionCb->addItem( tr( "Publisher" ), "publisher" );
  mWMSContactPositionCb->addItem( tr( "Author" ), "author" );
  int contactPositionIndex = mWMSContactPositionCb->findData( contactPositionText );
  if ( contactPositionIndex > 0 )
  {
    mWMSContactPositionCb->setCurrentIndex( contactPositionIndex );
  }
  else if ( !contactPositionText.isEmpty() )
  {
    mWMSContactPositionCb->setEditText( contactPositionText );
  }

  // WMS Fees
  QString feesText = QgsProject::instance()->readEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ), QString() );
  mWMSFeesCb->addItem( tr( "Conditions Unknown" ), "conditions unknown" );
  mWMSFeesCb->addItem( tr( "No Conditions Apply" ), "no conditions apply" );
  int feesIndex = mWMSFeesCb->findData( feesText );
  if ( feesIndex > -1 )
  {
    mWMSFeesCb->setCurrentIndex( feesIndex );
  }
  else if ( !feesText.isEmpty() )
  {
    mWMSFeesCb->setEditText( feesText );
  }

  // WMS Access Constraints
  QString accessConstraintsText = QgsProject::instance()->readEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ), QString() );
  mWMSAccessConstraintsCb->addItem( tr( "None" ), "None" );
  mWMSAccessConstraintsCb->addItem( tr( "Copyright" ), "copyright" );
  mWMSAccessConstraintsCb->addItem( tr( "Patent" ), "patent" );
  mWMSAccessConstraintsCb->addItem( tr( "Patent Pending" ), "patentPending" );
  mWMSAccessConstraintsCb->addItem( tr( "Trademark" ), "trademark" );
  mWMSAccessConstraintsCb->addItem( tr( "License" ), "license" );
  mWMSAccessConstraintsCb->addItem( tr( "Intellectual Property Rights" ), "intellectualPropertyRights" );
  mWMSAccessConstraintsCb->addItem( tr( "Restricted" ), "restricted" );
  mWMSAccessConstraintsCb->addItem( tr( "Other Restrictions" ), "otherRestrictions" );
  int accessConstraintsIndex = mWMSAccessConstraintsCb->findData( accessConstraintsText );
  if ( accessConstraintsIndex > -1 )
  {
    mWMSAccessConstraintsCb->setCurrentIndex( accessConstraintsIndex );
  }
  else if ( !accessConstraintsText.isEmpty() )
  {
    mWMSAccessConstraintsCb->setEditText( accessConstraintsText );
  }

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "bg" ) ), QLocale( QStringLiteral( "bg" ) ).nativeLanguageName(), "bul" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cs" ) ), QLocale( QStringLiteral( "cs" ) ).nativeLanguageName(), "cze" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "da" ) ), QLocale( QStringLiteral( "da" ) ).nativeLanguageName(), "dan" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "nl" ) ), QLocale( QStringLiteral( "nl" ) ).nativeLanguageName(), "dut" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "en_GB" ) ), QLocale( QStringLiteral( "en_GB" ) ).nativeLanguageName(), "eng" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "et" ) ), QLocale( QStringLiteral( "et" ) ).nativeLanguageName(), "est" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fi" ) ), QLocale( QStringLiteral( "fi" ) ).nativeLanguageName(), "fin" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fr" ) ), QLocale( QStringLiteral( "fr" ) ).nativeLanguageName(), "fre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "de" ) ), QLocale( QStringLiteral( "de" ) ).nativeLanguageName(), "ger" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ga" ) ), QLocale( QStringLiteral( "ga" ) ).nativeLanguageName(), "gle" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "el" ) ), QLocale( QStringLiteral( "el" ) ).nativeLanguageName(), "gre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "hu" ) ), QLocale( QStringLiteral( "hu" ) ).nativeLanguageName(), "hun" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "it" ) ), QLocale( QStringLiteral( "it" ) ).nativeLanguageName(), "ita" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lv" ) ), QLocale( QStringLiteral( "lv" ) ).nativeLanguageName(), "lav" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lt" ) ), QLocale( QStringLiteral( "lt" ) ).nativeLanguageName(), "lit" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "mt" ) ), QLocale( QStringLiteral( "mt" ) ).nativeLanguageName(), "mlt" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pl" ) ), QLocale( QStringLiteral( "pl" ) ).nativeLanguageName(), "pol" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pt_PT" ) ), QLocale( QStringLiteral( "pt_PT" ) ).nativeLanguageName(), "por" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ro" ) ), QLocale( QStringLiteral( "ro" ) ).nativeLanguageName(), "rum" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sk" ) ), QLocale( QStringLiteral( "sk" ) ).nativeLanguageName(), "slo" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sl" ) ), QLocale( QStringLiteral( "sl" ) ).nativeLanguageName(), "slv" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "es" ) ), QLocale( QStringLiteral( "es" ) ).nativeLanguageName(), "spa" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sv" ) ), QLocale( QStringLiteral( "sv" ) ).nativeLanguageName(), "swe" );

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "eu" ) ), QLocale( QStringLiteral( "eu" ) ).nativeLanguageName(), "eus" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ca" ) ), QLocale( QStringLiteral( "ca" ) ).nativeLanguageName(), "cat" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gl" ) ), QLocale( QStringLiteral( "gl" ) ).nativeLanguageName(), "gal" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gd" ) ), QLocale( QStringLiteral( "gd" ) ).nativeLanguageName(), "gla" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cy" ) ), QLocale( QStringLiteral( "cy" ) ).nativeLanguageName(), "cym" );
  mWMSInspireLanguage->setCurrentIndex(
    mWMSInspireLanguage->findText(
      QLocale().nativeLanguageName()
    )
  );

  bool addWMSInspire = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/activated" ) );
  if ( addWMSInspire )
  {
    mWMSInspire->setChecked( addWMSInspire );
    QString inspireLanguage = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/language" ), QString() );
    int inspireLanguageIndex = mWMSInspireLanguage->findData( inspireLanguage );
    mWMSInspireLanguage->setCurrentIndex( inspireLanguageIndex );

    QString inspireMetadataUrl = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrl" ), QString() );
    if ( !inspireMetadataUrl.isEmpty() )
    {
      mWMSInspireScenario1->setChecked( true );
      mWMSInspireMetadataUrl->setText( inspireMetadataUrl );
      mWMSInspireMetadataUrlType->setCurrentIndex(
        mWMSInspireMetadataUrlType->findText(
          QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrlType" ), QString() )
        )
      );
    }
    else
    {
      QString inspireTemporalReference = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/temporalReference" ), QString() );
      if ( !inspireTemporalReference.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireTemporalReference->setDate( QDate::fromString( inspireTemporalReference, QStringLiteral( "yyyy-MM-dd" ) ) );
      }
      QString inspireMetadataDate = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataDate" ), QString() );
      if ( !inspireMetadataDate.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireMetadataDate->setDate( QDate::fromString( inspireMetadataDate, QStringLiteral( "yyyy-MM-dd" ) ) );
      }
    }
  }

  // WMS GetFeatureInfo precision
  int WMSprecision = QgsProject::instance()->readNumEntry( QStringLiteral( "WMSPrecision" ), QStringLiteral( "/" ), -1 );
  if ( WMSprecision != -1 )
  {
    mWMSPrecisionSpinBox->setValue( WMSprecision );
  }
  mWMSPrecisionSpinBox->setClearValue( 8 );

  bool ok = false;
  QStringList values;

  mWMSExtMinX->setValidator( new QDoubleValidator( mWMSExtMinX ) );
  mWMSExtMinY->setValidator( new QDoubleValidator( mWMSExtMinY ) );
  mWMSExtMaxX->setValidator( new QDoubleValidator( mWMSExtMaxX ) );
  mWMSExtMaxY->setValidator( new QDoubleValidator( mWMSExtMaxY ) );

  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSExtent" ), QStringLiteral( "/" ), QStringList(), &ok );
  grpWMSExt->setChecked( ok && values.size() == 4 );
  if ( grpWMSExt->isChecked() )
  {
    mWMSExtMinX->setText( values[0] );
    mWMSExtMinY->setText( values[1] );
    mWMSExtMaxX->setText( values[2] );
    mWMSExtMaxY->setText( values[3] );
  }

  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSCrsList" ), QStringLiteral( "/" ), QStringList(), &ok );
  grpWMSList->setChecked( ok && !values.isEmpty() );
  if ( grpWMSList->isChecked() )
  {
    mWMSList->addItems( values );
  }
  else
  {
    values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSEpsgList" ), QStringLiteral( "/" ), QStringList(), &ok );
    grpWMSList->setChecked( ok && !values.isEmpty() );
    if ( grpWMSList->isChecked() )
    {
      QStringList list;
      const auto constValues = values;
      for ( const QString &value : constValues )
      {
        list << QStringLiteral( "EPSG:%1" ).arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  //Layout restriction for WMS
  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ), QStringList(), &ok );
  mWMSPrintLayoutGroupBox->setChecked( ok );
  if ( ok )
  {
    mPrintLayoutListWidget->addItems( values );
  }

  //layer restriction for WMS
  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSRestrictedLayers" ), QStringLiteral( "/" ), QStringList(), &ok );
  mLayerRestrictionsGroupBox->setChecked( ok );
  if ( ok )
  {
    mLayerRestrictionsListWidget->addItems( values );
  }

  bool useAttributeFormSettings = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSFeatureInfoUseAttributeFormSettings" ), QStringLiteral( "/" ) );
  mUseAttributeFormSettingsCheckBox->setChecked( useAttributeFormSettings );

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSAddWktGeometry" ), QStringLiteral( "/" ) );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  bool segmentizeFeatureInfoGeometry = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSSegmentizeFeatureInfoGeometry" ), QStringLiteral( "/" ) );
  mSegmentizeFeatureInfoGeometryCheckBox->setChecked( segmentizeFeatureInfoGeometry );

  bool useLayerIDs = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSUseLayerIDs" ), QStringLiteral( "/" ) );
  mWmsUseLayerIDs->setChecked( useLayerIDs );

  //WMS maxWidth / maxHeight
  mMaxWidthLineEdit->setValidator( new QIntValidator( mMaxWidthLineEdit ) );
  int maxWidth = QgsProject::instance()->readNumEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ), -1 );
  if ( maxWidth != -1 )
  {
    mMaxWidthLineEdit->setText( QString::number( maxWidth ) );
  }
  mMaxHeightLineEdit->setValidator( new QIntValidator( mMaxHeightLineEdit ) );
  int maxHeight = QgsProject::instance()->readNumEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ), -1 );
  if ( maxHeight != -1 )
  {
    mMaxHeightLineEdit->setText( QString::number( maxHeight ) );
  }

  // WMS imageQuality
  int imageQuality = QgsProject::instance()->readNumEntry( QStringLiteral( "WMSImageQuality" ), QStringLiteral( "/" ), -1 );
  if ( imageQuality != -1 )
  {
    mWMSImageQualitySpinBox->setValue( imageQuality );
  }
  mWMSImageQualitySpinBox->setClearValue( 90 );

  // WMS tileBuffer
  mWMSTileBufferSpinBox->setValue( QgsProject::instance()->readNumEntry( QStringLiteral( "WMSTileBuffer" ), QStringLiteral( "/" ), 0 ) );

  mWMSMaxAtlasFeaturesSpinBox->setValue( QgsProject::instance()->readNumEntry( QStringLiteral( "WMSMaxAtlasFeatures" ), QStringLiteral( "/" ), 1 ) );
  mWMSMaxAtlasFeaturesSpinBox->setClearValue( 1 );

  QString defaultValueToolTip = tr( "In case of no other information to evaluate the map unit sized symbols, it uses default scale (on projected CRS) or default map units per mm (on geographic CRS)." );
  if ( QgsProject::instance()->crs().isGeographic() )
  {
    mWMSDefaultMapUnitsPerMm = new QDoubleSpinBox();
    mWMSDefaultMapUnitsPerMm->setDecimals( 4 );
    mWMSDefaultMapUnitsPerMm->setSingleStep( 0.001 );
    mWMSDefaultMapUnitsPerMm->setValue( QgsProject::instance()->readDoubleEntry( QStringLiteral( "WMSDefaultMapUnitsPerMm" ), QStringLiteral( "/" ), 1 ) );
    mWMSDefaultMapUnitsPerMm->setToolTip( defaultValueToolTip );
    mWMSDefaultMapUnitsPerMmLayout->addWidget( mWMSDefaultMapUnitsPerMm );
  }
  else
  {
    mWMSDefaultMapUnitScale = new QgsScaleWidget();
    mWMSDefaultMapUnitScale->setScale( QgsProject::instance()->readDoubleEntry( QStringLiteral( "WMSDefaultMapUnitsPerMm" ), QStringLiteral( "/" ), 1 ) * QgsUnitTypes::fromUnitToUnitFactor( QgsProject::instance()->crs().mapUnits(), QgsUnitTypes::DistanceMillimeters ) );
    mWMSDefaultMapUnitScale->setToolTip( defaultValueToolTip );
    mWMSDefaultMapUnitsPerMmLayout->addWidget( mWMSDefaultMapUnitScale );
    mWMSDefaultMapUnitsPerMmLabel->setText( tr( "Default scale for legend" ) );
  }

  mWMTSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMTSUrl" ), QStringLiteral( "/" ), QString() ) );
  mWMTSMinScaleSpinBox->setValue( QgsProject::instance()->readNumEntry( QStringLiteral( "WMTSMinScale" ), QStringLiteral( "/" ), 5000 ) );
  mWMTSMinScaleSpinBox->setClearValue( 5000 );

  bool wmtsProject = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Project" ) );
  bool wmtsPngProject = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Project" ) );
  bool wmtsJpegProject = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Project" ) );
  QStringList wmtsGroupNameList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Group" ) );
  QStringList wmtsPngGroupNameList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Group" ) );
  QStringList wmtsJpegGroupNameList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Group" ) );
  QStringList wmtsLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Layer" ) );
  QStringList wmtsPngLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Layer" ) );
  QStringList wmtsJpegLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Layer" ) );

  QgsTreeWidgetItem *projItem = new QgsTreeWidgetItem( QStringList() << QStringLiteral( "Project" ) );
  projItem->setFlags( projItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  projItem->setCheckState( 1, wmtsProject ? Qt::Checked : Qt::Unchecked );
  projItem->setCheckState( 2, wmtsPngProject ? Qt::Checked : Qt::Unchecked );
  projItem->setCheckState( 3, wmtsJpegProject ? Qt::Checked : Qt::Unchecked );
  projItem->setData( 0, Qt::UserRole, QStringLiteral( "project" ) );
  twWmtsLayers->addTopLevelItem( projItem );
  populateWmtsTree( QgsProject::instance()->layerTreeRoot(), projItem );
  projItem->setExpanded( true );
  twWmtsLayers->header()->resizeSections( QHeaderView::ResizeToContents );
  for ( QTreeWidgetItem *item : twWmtsLayers->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 ) )
  {
    QString itemType = item->data( 0, Qt::UserRole ).toString();
    if ( itemType == QLatin1String( "group" ) )
    {
      QString gName = item->data( 0, Qt::UserRole + 1 ).toString();
      item->setCheckState( 1, wmtsGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 2, wmtsPngGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 3, wmtsJpegGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
    }
    else if ( itemType == QLatin1String( "layer" ) )
    {
      QString lId = item->data( 0, Qt::UserRole + 1 ).toString();
      item->setCheckState( 1, wmtsLayerIdList.contains( lId ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 2, wmtsPngLayerIdList.contains( lId ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 3, wmtsJpegLayerIdList.contains( lId ) ? Qt::Checked : Qt::Unchecked );
    }
  }
  connect( twWmtsLayers, &QTreeWidget::itemChanged, this, &QgsProjectProperties::twWmtsItemChanged );

  for ( int i = 0; i < mWMSList->count(); i++ )
  {
    QString crsStr = mWMSList->item( i )->text();
    addWmtsGrid( crsStr );
  }
  connect( mWMSList->model(), &QAbstractListModel::rowsInserted, this, &QgsProjectProperties::lwWmsRowsInserted );
  connect( mWMSList->model(), &QAbstractListModel::rowsRemoved, this, &QgsProjectProperties::lwWmsRowsRemoved );
  connect( twWmtsGrids, &QTreeWidget::itemDoubleClicked, this, &QgsProjectProperties::twWmtsGridItemDoubleClicked );
  connect( twWmtsGrids, &QTreeWidget::itemChanged, this, &QgsProjectProperties::twWmtsGridItemChanged );
  const QStringList wmtsGridList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "CRS" ) );
  if ( !wmtsGridList.isEmpty() )
  {
    const QStringList wmtsGridConfigList = QgsProject::instance()->readListEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "Config" ) );
    QMap<QString, QStringList> wmtsGridConfigs;
    for ( const QString &c : wmtsGridConfigList )
    {
      QStringList config = c.split( ',' );
      wmtsGridConfigs[config[0]] = config;
    }
    for ( QTreeWidgetItem *item : twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 ) )
    {
      QString crsStr = item->data( 0, Qt::UserRole ).toString();
      if ( !wmtsGridList.contains( crsStr ) )
        continue;

      item->setCheckState( 1, Qt::Checked );
      QStringList config = wmtsGridConfigs[crsStr];
      item->setData( 2, Qt::DisplayRole, QVariant( config[1] ).toDouble() );
      item->setData( 3, Qt::DisplayRole, QVariant( config[2] ).toDouble() );
      item->setData( 4, Qt::DisplayRole, QVariant( config[3] ).toDouble() );
      item->setData( 5, Qt::DisplayRole, QVariant( config[4] ).toInt() );
    }
  }

  mWFSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), QString() ) );
  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSLayers" ), QStringLiteral( "/" ) );
  QStringList wfstUpdateLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Update" ) );
  QStringList wfstInsertLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Insert" ) );
  QStringList wfstDeleteLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Delete" ) );

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();

  twWFSLayers->setColumnCount( 6 );
  twWFSLayers->horizontalHeader()->setVisible( true );
  twWFSLayers->setRowCount( mapLayers.size() );

  QgsMapLayer *currentLayer = nullptr;
  int i = 0;
  int j = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == QgsMapLayerType::VectorLayer )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      QgsVectorDataProvider *provider = vlayer->dataProvider();
      if ( !provider )
        continue;
      QTableWidgetItem *twi = new QTableWidgetItem( QString::number( j ) );
      twWFSLayers->setVerticalHeaderItem( j, twi );

      twi = new QTableWidgetItem( currentLayer->name() );
      twi->setData( Qt::UserRole, it.key() );
      twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
      twWFSLayers->setItem( j, 0, twi );

      QCheckBox *cbp = new QCheckBox();
      cbp->setChecked( wfsLayerIdList.contains( currentLayer->id() ) );
      twWFSLayers->setCellWidget( j, 1, cbp );
      connect( cbp, &QCheckBox::stateChanged, this, [ = ] { cbxWCSPubliedStateChanged( j ); } );

      QSpinBox *psb = new QSpinBox();
      psb->setValue( QgsProject::instance()->readNumEntry( QStringLiteral( "WFSLayersPrecision" ), "/" + currentLayer->id(), 8 ) );
      twWFSLayers->setCellWidget( j, 2, psb );

      if ( ( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) && ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) )
      {
        QCheckBox *cbu = new QCheckBox();
        cbu->setChecked( wfstUpdateLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 3, cbu );
      }
      if ( ( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
      {
        QCheckBox *cbi = new QCheckBox();
        cbi->setChecked( wfstInsertLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 4, cbi );
      }
      if ( ( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
      {
        QCheckBox *cbd = new QCheckBox();
        cbd->setChecked( wfstDeleteLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 5, cbd );
      }

      j++;
    }
  }
  twWFSLayers->setRowCount( j );
  twWFSLayers->resizeColumnToContents( 0 );
  twWFSLayers->resizeColumnToContents( 2 );
  twWFSLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  mWCSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), QString() ) );
  QStringList wcsLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ) );

  twWCSLayers->setColumnCount( 2 );
  twWCSLayers->horizontalHeader()->setVisible( true );
  twWCSLayers->setRowCount( mapLayers.size() );

  i = 0;
  j = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == QgsMapLayerType::RasterLayer )
    {

      QTableWidgetItem *twi = new QTableWidgetItem( QString::number( j ) );
      twWCSLayers->setVerticalHeaderItem( j, twi );

      twi = new QTableWidgetItem( currentLayer->name() );
      twi->setData( Qt::UserRole, it.key() );
      twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
      twWCSLayers->setItem( j, 0, twi );

      QCheckBox *cbp = new QCheckBox();
      cbp->setChecked( wcsLayerIdList.contains( currentLayer->id() ) );
      twWCSLayers->setCellWidget( j, 1, cbp );

      connect( cbp, &QCheckBox::stateChanged, this, [ = ] { cbxWCSPubliedStateChanged( j ); } );

      j++;
    }
  }
  twWCSLayers->setRowCount( j );
  twWCSLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // Default Styles
  mStyle = QgsStyle::defaultStyle();
  populateStyles();

  // Color palette
  connect( mButtonCopyColors, &QAbstractButton::clicked, mTreeProjectColors, &QgsColorSchemeList::copyColors );
  connect( mButtonRemoveColor, &QAbstractButton::clicked, mTreeProjectColors, &QgsColorSchemeList::removeSelection );
  connect( mButtonPasteColors, &QAbstractButton::clicked, mTreeProjectColors, &QgsColorSchemeList::pasteColors );
  connect( mButtonImportColors, &QAbstractButton::clicked, mTreeProjectColors, &QgsColorSchemeList::showImportColorsDialog );
  connect( mButtonExportColors, &QAbstractButton::clicked, mTreeProjectColors, &QgsColorSchemeList::showExportColorsDialog );

  QList<QgsProjectColorScheme *> projectSchemes;
  QgsApplication::colorSchemeRegistry()->schemes( projectSchemes );
  if ( projectSchemes.length() > 0 )
  {
    mTreeProjectColors->setScheme( projectSchemes.at( 0 ) );
  }


  // Project macros
  QString pythonMacros = QgsProject::instance()->readEntry( QStringLiteral( "Macros" ), QStringLiteral( "/pythonCode" ), QString() );
  grpPythonMacros->setChecked( !pythonMacros.isEmpty() );
  if ( !pythonMacros.isEmpty() )
  {
    ptePythonMacros->setText( pythonMacros );
  }
  else
  {
    resetPythonMacros();
  }

  // Initialize relation manager
  mRelationManagerDlg = new QgsRelationManagerDialog( QgsProject::instance()->relationManager(), mTabRelations );
  mTabRelations->layout()->addWidget( mRelationManagerDlg );

  QList<QgsVectorLayer *> vectorLayers;
  const auto constMapLayers = mapLayers;
  for ( QgsMapLayer *mapLayer : constMapLayers )
  {
    if ( QgsMapLayerType::VectorLayer == mapLayer->type() )
    {
      vectorLayers.append( qobject_cast<QgsVectorLayer *>( mapLayer ) );
    }
  }
  mRelationManagerDlg->setLayers( vectorLayers );

  mTransactionModeComboBox->setCurrentIndex( mTransactionModeComboBox->findData( static_cast<int>( QgsProject::instance()->transactionMode() ) ) );
  mEvaluateDefaultValues->setChecked( QgsProject::instance()->evaluateDefaultValues() );
  mTrustProjectCheckBox->setChecked( QgsProject::instance()->trustLayerMetadata() );

  // Variables editor
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 1 );

  // metadata
  mMetadataWidget->setMode( QgsMetadataWidget::ProjectMetadata );
  mMetadataWidget->setMetadata( &QgsProject::instance()->metadata() );

  // sync metadata title and project title fields
  connect( mMetadataWidget, &QgsMetadataWidget::titleChanged, titleEdit, &QLineEdit::setText, Qt::QueuedConnection );
  connect( titleEdit, &QLineEdit::textChanged, this, [ = ] { whileBlocking( mMetadataWidget )->setTitle( title() ) ;} );

  //fill ts language checkbox
  QString i18nPath = QgsApplication::i18nPath();
  QDir i18Dir( i18nPath, QStringLiteral( "qgis*.qm" ) );
  const QStringList qmFileList = i18Dir.entryList();
  for ( const QString &qmFile : qmFileList )
  {
    // Ignore the 'en' translation file, already added as 'en_US'.
    if ( qmFile.compare( QLatin1String( "qgis_en.qm" ) ) == 0 ) continue;

    QString qmFileName = qmFile;
    QString l = qmFileName.remove( QStringLiteral( "qgis_" ) ).remove( QStringLiteral( ".qm" ) );

    // QTBUG-57802: eo locale is improperly handled
    QString displayName = l.startsWith( QLatin1String( "eo" ) ) ? QLocale::languageToString( QLocale::Esperanto ) : QLocale( l ).nativeLanguageName();
    cbtsLocale->addItem( QIcon( QStringLiteral( ":/images/flags/%1.svg" ).arg( l ) ), displayName, l );
  }

  cbtsLocale->addItem( QIcon( QStringLiteral( ":/images/flags/%1.svg" ).arg( QLatin1String( "en_US" ) ) ), QLocale( QStringLiteral( "en_US" ) ).nativeLanguageName(), QStringLiteral( "en_US" ) );
  cbtsLocale->setCurrentIndex( cbtsLocale->findData( QgsApplication::settingsLocaleUserLocale.value() ) );

  connect( generateTsFileButton, &QPushButton::clicked, this, &QgsProjectProperties::onGenerateTsFileButton );

  // Reading ellipsoid from settings
  setCurrentEllipsoid( QgsProject::instance()->ellipsoid() );

  mBearingFormat.reset( QgsProject::instance()->displaySettings()->bearingFormat()->clone() );

  const auto constOptionsFactories = optionsFactories;
  for ( QgsOptionsWidgetFactory *factory : constOptionsFactories )
  {
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon( factory->icon() );
    item->setText( factory->title() );
    item->setToolTip( factory->title() );

    mOptionsListWidget->addItem( item );

    QgsOptionsPageWidget *page = factory->createWidget( this );
    if ( !page )
      continue;

    mAdditionalProjectPropertiesWidgets << page;
    mOptionsStackedWidget->addWidget( page );
  }

  restoreOptionsBaseUi();

#ifdef QGISDEBUG
  checkPageWidgetNameMap();
#endif
}

QgsProjectProperties::~QgsProjectProperties() = default;

QString QgsProjectProperties::title() const
{
  return titleEdit->text();
}

void QgsProjectProperties::title( QString const &title )
{
  titleEdit->setText( title );
  QgsProject::instance()->setTitle( title );
}

void QgsProjectProperties::setSelectedCrs( const QgsCoordinateReferenceSystem &crs )
{
  mBlockCrsUpdates = true;
  projectionSelector->setCrs( crs );
  mBlockCrsUpdates = false;
  crsChanged( projectionSelector->crs() );
}

QgsExpressionContext QgsProjectProperties::createExpressionContext() const
{
  QgsExpressionContext context;
  context
      << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  return context;
}

void QgsProjectProperties::apply()
{
  mMapCanvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "RenderMapTile" ), QStringLiteral( "/" ), mMapTileRenderingCheckBox->isChecked() );

  // important - set the transform context first, as changing the project CRS may otherwise change this and
  // cause loss of user changes
  QgsCoordinateTransformContext transformContext = mDatumTransformTableWidget->transformContext();
  QgsProject::instance()->setTransformContext( transformContext );
  if ( projectionSelector->hasValidSelection() )
  {
    QgsCoordinateReferenceSystem srs = projectionSelector->crs();
    QgsProject::instance()->setCrs( srs );
    if ( srs.isValid() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Selected CRS " ) + srs.userFriendlyIdentifier(), 4 );
      // write the currently selected projections _proj string_ to project settings
      QgsDebugMsgLevel( QStringLiteral( "SpatialRefSys/ProjectCRSProj4String: %1" ).arg( srs.toProj() ), 4 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "CRS set to no projection!" ), 4 );
    }
  }

  mMetadataWidget->acceptMetadata();

  // Set the project title
  QgsProject::instance()->setTitle( title() );
  QgsProject::instance()->setPresetHomePath( QDir::fromNativeSeparators( mProjectHomeLineEdit->text() ) );

  // DB-related options
  QgsProject::instance()->setTransactionMode( mTransactionModeComboBox->currentData().value<Qgis::TransactionMode>() );
  QgsProject::instance()->setEvaluateDefaultValues( mEvaluateDefaultValues->isChecked() );
  QgsProject::instance()->setTrustLayerMetadata( mTrustProjectCheckBox->isChecked() );

  // Time settings
  QDateTime start = mStartDateTimeEdit->dateTime();
  QDateTime end = mEndDateTimeEdit->dateTime();

  QgsProject::instance()->timeSettings()->setTemporalRange( QgsDateTimeRange( start, end ) );

  // set the mouse display precision method and the
  // number of decimal places for the manual option
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), radAutomatic->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), spinBoxDP->value() );
  QString degreeFormat;
  switch ( static_cast< CoordinateFormat >( mCoordinateDisplayComboBox->currentData().toInt() ) )
  {
    case DegreesMinutes:
      degreeFormat = QStringLiteral( "DM" );
      break;
    case DegreesMinutesSeconds:
      degreeFormat = QStringLiteral( "DMS" );
      break;
    case MapUnits:
      degreeFormat = QStringLiteral( "MU" );
      break;
    case DecimalDegrees:
      degreeFormat = QStringLiteral( "D" );
      break;
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DegreeFormat" ), degreeFormat );

  // Announce that we may have a new display precision setting
  emit displayPrecisionChanged();

  QgsUnitTypes::DistanceUnit distanceUnits = static_cast< QgsUnitTypes::DistanceUnit >( mDistanceUnitsCombo->currentData().toInt() );
  QgsProject::instance()->setDistanceUnits( distanceUnits );

  QgsUnitTypes::AreaUnit areaUnits = static_cast< QgsUnitTypes::AreaUnit >( mAreaUnitsCombo->currentData().toInt() );
  QgsProject::instance()->setAreaUnits( areaUnits );

  QgsProject::instance()->setFilePathStorage( static_cast< Qgis::FilePathType >( cbxAbsolutePath->currentData().toInt() ) );

  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( QLatin1String( "PARAMETER" ) ) )
  {
    double major = mEllipsoidList.at( mEllipsoidIndex ).semiMajor;
    double minor = mEllipsoidList.at( mEllipsoidIndex ).semiMinor;
    // If the user fields have changed, use them instead.
    if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using parameteric major/minor" ), 4 );
      bool ok;
      double val {QgsDoubleValidator::toDouble( leSemiMajor->text(), &ok )};
      if ( ok )
      {
        major = val;
      }
      val = QgsDoubleValidator::toDouble( leSemiMinor->text(), &ok );
      if ( ok )
      {
        minor = val;
      }
    }
    QgsProject::instance()->setEllipsoid( QStringLiteral( "PARAMETER:%1:%2" )
                                          .arg( major, 0, 'g', 17 )
                                          .arg( minor, 0, 'g', 17 ) );
  }
  else
  {
    QgsProject::instance()->setEllipsoid( mEllipsoidList[ mEllipsoidIndex ].acronym );
  }

  //set the color for selections and canvas background color
  QgsProject::instance()->setSelectionColor( pbnSelectionColor->color() );
  QgsProject::instance()->setBackgroundColor( pbnCanvasColor->color() );

  const auto constMapCanvases = QgisApp::instance()->mapCanvases();
  for ( QgsMapCanvas *canvas : constMapCanvases )
  {
    canvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );
  }

  //save project scales
  QVector< double > projectScales;
  projectScales.reserve( lstScales->count() );
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    const QString scaleText = lstScales->item( i )->text();
    const QStringList parts = scaleText.split( ':' );
    if ( parts.size() == 2 )
    {
      bool ok = false;
      const double scale = parts.at( 1 ).toDouble( &ok );
      if ( ok )
        projectScales.append( scale );
    }
  }

  if ( !projectScales.isEmpty() )
  {
    QgsProject::instance()->viewSettings()->setMapScales( projectScales );
    QgsProject::instance()->viewSettings()->setUseProjectScales( grpProjectScales->isChecked() );
  }
  else
  {
    QgsProject::instance()->viewSettings()->setMapScales( QVector< double >() );
    QgsProject::instance()->viewSettings()->setUseProjectScales( false );
  }

  if ( mExtentGroupBox->isChecked() )
  {
    QgsProject::instance()->viewSettings()->setPresetFullExtent( QgsReferencedRectangle( mExtentGroupBox->outputExtent(), mExtentGroupBox->outputCrs() ) );
  }
  else
  {
    QgsProject::instance()->viewSettings()->setPresetFullExtent( QgsReferencedRectangle() );
  }

  bool isDirty = false;
  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    QgsMapLayer *layer = it.value();
    QgsMapLayer::LayerFlags oldFlags = layer->flags();
    QgsMapLayer::LayerFlags flags = layer->flags();

    if ( mLayerCapabilitiesModel->identifiable( layer ) )
      flags |= QgsMapLayer::Identifiable;
    else
      flags &= ~QgsMapLayer::Identifiable;

    if ( mLayerCapabilitiesModel->removable( layer ) )
      flags |= QgsMapLayer::Removable;
    else
      flags &= ~QgsMapLayer::Removable;

    if ( mLayerCapabilitiesModel->searchable( layer ) )
      flags |= QgsMapLayer::Searchable;
    else
      flags &= ~QgsMapLayer::Searchable;

    if ( mLayerCapabilitiesModel->privateLayer( layer ) )
      flags |= QgsMapLayer::Private;
    else
      flags &= ~QgsMapLayer::Private;

    layer->setFlags( flags );

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( vl )
    {
      // read only is for vector layers only for now
      vl->setReadOnly( mLayerCapabilitiesModel->readOnly( vl ) );
    }

    if ( oldFlags != flags )
    {
      isDirty = true;
    }
  }

  if ( isDirty )
  {
    QgsProject::instance()->setDirty( true );
  }

  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), grpOWSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ), mWMSTitle->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSRootName" ), QStringLiteral( "/" ), mWMSName->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ), mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ), mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ), mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ), mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ), mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ), mWMSOnlineResourceLineEdit->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), mWMSUrlLineEdit->text() );

  QgsPropertyCollection propertyCollection = QgsProject::instance()->dataDefinedServerProperties();
  propertyCollection.setProperty( QgsProject::DataDefinedServerProperty::WMSOnlineResource, mWMSOnlineResourceExpressionButton->toProperty() );
  QgsProject::instance()->setDataDefinedServerProperties( propertyCollection );

  // WMS Contact Position
  int contactPositionIndex = mWMSContactPositionCb->currentIndex();
  QString contactPositionText = mWMSContactPositionCb->currentText();
  if ( !contactPositionText.isEmpty() && contactPositionText == mWMSContactPositionCb->itemText( contactPositionIndex ) )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ), mWMSContactPositionCb->itemData( contactPositionIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ), contactPositionText );
  }

  // WMS Fees
  int feesIndex = mWMSFeesCb->currentIndex();
  QString feesText = mWMSFeesCb->currentText();
  if ( !feesText.isEmpty() && feesText == mWMSFeesCb->itemText( feesIndex ) )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ), mWMSFeesCb->itemData( feesIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ), feesText );
  }

  // WMS Access Constraints
  int accessConstraintsIndex = mWMSAccessConstraintsCb->currentIndex();
  QString accessConstraintsText = mWMSAccessConstraintsCb->currentText();
  if ( !accessConstraintsText.isEmpty() && accessConstraintsText == mWMSAccessConstraintsCb->itemText( accessConstraintsIndex ) )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ), mWMSAccessConstraintsCb->itemData( accessConstraintsIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ), accessConstraintsText );
  }

  //WMS keyword list
  QStringList keywordStringList = mWMSKeywordList->text().split( ',' );
  if ( !keywordStringList.isEmpty() )
  {
    keywordStringList.replaceInStrings( QRegExp( "^\\s+" ), QString() ).replaceInStrings( QRegExp( "\\s+$" ), QString() );
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ), keywordStringList );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ) );
  }

  // WMS INSPIRE configuration
  QgsProject::instance()->removeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/" ) );
  if ( mWMSInspire->isChecked() )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/activated" ), mWMSInspire->isChecked() );

    int inspireLanguageIndex = mWMSInspireLanguage->currentIndex();
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/language" ), mWMSInspireLanguage->itemData( inspireLanguageIndex ).toString() );

    if ( mWMSInspireScenario1->isChecked() )
    {
      QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrl" ), mWMSInspireMetadataUrl->text() );
      QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrlType" ), mWMSInspireMetadataUrlType->currentText() );
    }
    else if ( mWMSInspireScenario2->isChecked() )
    {
      QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/temporalReference" ), mWMSInspireTemporalReference->date().toString( QStringLiteral( "yyyy-MM-dd" ) ) );
      QgsProject::instance()->writeEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataDate" ), mWMSInspireMetadataDate->date().toString( QStringLiteral( "yyyy-MM-dd" ) ) );
    }
  }

  // WMS GetFeatureInfo geometry precision (decimal places)
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSPrecision" ), QStringLiteral( "/" ), mWMSPrecisionSpinBox->text() );

  if ( grpWMSExt->isChecked() )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSExtent" ), QStringLiteral( "/" ),
                                        QStringList()
                                        << mWMSExtMinX->text()
                                        << mWMSExtMinY->text()
                                        << mWMSExtMaxX->text()
                                        << mWMSExtMaxY->text() );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSExtent" ), QStringLiteral( "/" ) );
  }

  if ( grpWMSList->isChecked() && mWMSList->count() == 0 )
  {
    QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "No coordinate systems selected. Disabling restriction." ) );
    grpWMSList->setChecked( false );
  }

  QgsProject::instance()->removeEntry( QStringLiteral( "WMSEpsgList" ), QStringLiteral( "/" ) );

  if ( grpWMSList->isChecked() )
  {
    QStringList crslist;
    for ( int i = 0; i < mWMSList->count(); i++ )
    {
      crslist << mWMSList->item( i )->text();
    }

    QgsProject::instance()->writeEntry( QStringLiteral( "WMSCrsList" ), QStringLiteral( "/" ), crslist );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSCrsList" ), QStringLiteral( "/" ) );
  }

  //WMS composer restrictions
  if ( mWMSPrintLayoutGroupBox->isChecked() )
  {
    QStringList composerTitles;
    for ( int i = 0; i < mPrintLayoutListWidget->count(); ++i )
    {
      composerTitles << mPrintLayoutListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ), composerTitles );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ) );
  }

  //WMS layer restrictions
  if ( mLayerRestrictionsGroupBox->isChecked() )
  {
    QStringList layerNames;
    for ( int i = 0; i < mLayerRestrictionsListWidget->count(); ++i )
    {
      layerNames << mLayerRestrictionsListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSRestrictedLayers" ), QStringLiteral( "/" ), layerNames );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSRestrictedLayers" ), QStringLiteral( "/" ) );
  }

  QgsProject::instance()->writeEntry( QStringLiteral( "WMSFeatureInfoUseAttributeFormSettings" ), QStringLiteral( "/" ), mUseAttributeFormSettingsCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSAddWktGeometry" ), QStringLiteral( "/" ), mAddWktGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSSegmentizeFeatureInfoGeometry" ), QStringLiteral( "/" ), mSegmentizeFeatureInfoGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSUseLayerIDs" ), QStringLiteral( "/" ), mWmsUseLayerIDs->isChecked() );

  QString maxWidthText = mMaxWidthLineEdit->text();
  if ( maxWidthText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ) );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSMaxWidth" ), QStringLiteral( "/" ), maxWidthText.toInt() );
  }
  QString maxHeightText = mMaxHeightLineEdit->text();
  if ( maxHeightText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ) );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSMaxHeight" ), QStringLiteral( "/" ), maxHeightText.toInt() );
  }

  // WMS Image quality
  int imageQualityValue = mWMSImageQualitySpinBox->value();
  if ( imageQualityValue == 0 )
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "WMSImageQuality" ), QStringLiteral( "/" ) );
  }
  else
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSImageQuality" ), QStringLiteral( "/" ), imageQualityValue );
  }

  // WMS TileBuffer
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSTileBuffer" ), QStringLiteral( "/" ), mWMSTileBufferSpinBox->value() );

  int maxAtlasFeatures = mWMSMaxAtlasFeaturesSpinBox->value();
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSMaxAtlasFeatures" ), QStringLiteral( "/" ), maxAtlasFeatures );

  if ( QgsProject::instance()->crs().isGeographic() && mWMSDefaultMapUnitsPerMm )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSDefaultMapUnitsPerMm" ), QStringLiteral( "/" ), mWMSDefaultMapUnitsPerMm->value() );
  }
  else if ( mWMSDefaultMapUnitScale )
  {
    double defaultMapUnitsPerMm = mWMSDefaultMapUnitScale->scale() / QgsUnitTypes::fromUnitToUnitFactor( QgsProject::instance()->crs().mapUnits(), QgsUnitTypes::DistanceMillimeters );
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSDefaultMapUnitsPerMm" ), QStringLiteral( "/" ), defaultMapUnitsPerMm );
  }

  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSUrl" ), QStringLiteral( "/" ), mWMTSUrlLineEdit->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSMinScale" ), QStringLiteral( "/" ), mWMTSMinScaleSpinBox->value() );
  bool wmtsProject = false;
  bool wmtsPngProject = false;
  bool wmtsJpegProject = false;
  QStringList wmtsGroupList;
  QStringList wmtsPngGroupList;
  QStringList wmtsJpegGroupList;
  QStringList wmtsLayerList;
  QStringList wmtsPngLayerList;
  QStringList wmtsJpegLayerList;
  for ( const QTreeWidgetItem *item : twWmtsLayers->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 ) )
  {
    if ( !item->checkState( 1 ) )
      continue;

    QString itemType = item->data( 0, Qt::UserRole ).toString();
    if ( itemType == QLatin1String( "project" ) )
    {
      wmtsProject = true;
      wmtsPngProject = item->checkState( 2 );
      wmtsJpegProject = item->checkState( 3 );
    }
    else if ( itemType == QLatin1String( "group" ) )
    {
      QString gName = item->data( 0, Qt::UserRole + 1 ).toString();
      wmtsGroupList << gName;
      if ( item->checkState( 2 ) )
        wmtsPngGroupList << gName;
      if ( item->checkState( 3 ) )
        wmtsJpegGroupList << gName;
    }
    else if ( itemType == QLatin1String( "layer" ) )
    {
      QString lId = item->data( 0, Qt::UserRole + 1 ).toString();
      wmtsLayerList << lId;
      if ( item->checkState( 2 ) )
        wmtsPngLayerList << lId;
      if ( item->checkState( 3 ) )
        wmtsJpegLayerList << lId;
    }
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Project" ), wmtsProject );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Project" ), wmtsPngProject );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Project" ), wmtsJpegProject );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Group" ), wmtsGroupList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Group" ), wmtsPngGroupList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Group" ), wmtsJpegGroupList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Layer" ), wmtsLayerList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Layer" ), wmtsPngLayerList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Layer" ), wmtsJpegLayerList );

  QStringList wmtsGridList;
  QStringList wmtsGridConfigList;
  for ( const QTreeWidgetItem *item : twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 ) )
  {
    if ( !item->checkState( 1 ) )
      continue;
    wmtsGridList << item->data( 0, Qt::UserRole ).toString();
    wmtsGridConfigList << QStringLiteral( "%1,%2,%3,%4,%5" ).arg( item->data( 0, Qt::UserRole ).toString(), item->data( 2, Qt::DisplayRole ).toString(), item->data( 3, Qt::DisplayRole ).toString(), item->data( 4, Qt::DisplayRole ).toString(), item->data( 5, Qt::DisplayRole ).toString() );
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "CRS" ), wmtsGridList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMTSGrids" ), QStringLiteral( "Config" ), wmtsGridConfigList );

  QgsProject::instance()->writeEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), mWFSUrlLineEdit->text() );
  QStringList wfsLayerList;
  QStringList wfstUpdateLayerList;
  QStringList wfstInsertLayerList;
  QStringList wfstDeleteLayerList;
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QString id = twWFSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
    QCheckBox *cb = nullptr;
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      wfsLayerList << id;

      QSpinBox *sb = qobject_cast<QSpinBox *>( twWFSLayers->cellWidget( i, 2 ) );
      QgsProject::instance()->writeEntry( QStringLiteral( "WFSLayersPrecision" ), "/" + id, sb->value() );

      cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 3 ) );
      if ( cb && cb->isChecked() )
      {
        wfstUpdateLayerList << id;
      }
      cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 4 ) );
      if ( cb && cb->isChecked() )
      {
        wfstInsertLayerList << id;
      }
      cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 5 ) );
      if ( cb && cb->isChecked() )
      {
        wfstDeleteLayerList << id;
      }
    }
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "WFSLayers" ), QStringLiteral( "/" ), wfsLayerList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Update" ), wfstUpdateLayerList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Insert" ), wfstInsertLayerList );
  QgsProject::instance()->writeEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Delete" ), wfstDeleteLayerList );

  QgsProject::instance()->writeEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), mWCSUrlLineEdit->text() );
  QStringList wcsLayerList;
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QString id = twWCSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
    QCheckBox *cb = nullptr;
    cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      wcsLayerList << id;
    }
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ), wcsLayerList );

  // Default Styles
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ), cboStyleMarker->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ), cboStyleLine->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ), cboStyleFill->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), cboStyleColorRamp->currentText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Opacity" ), mDefaultOpacityWidget->opacity() );
  QgsProject::instance()->writeEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/RandomColors" ), cbxStyleRandomColors->isChecked() );
  if ( mTreeProjectColors->isDirty() )
  {
    mTreeProjectColors->saveColorsToScheme();
  }

  // store project macros
  QString pythonMacros = ptePythonMacros->text();
  if ( !grpPythonMacros->isChecked() || pythonMacros.isEmpty() )
  {
    pythonMacros = QString();
    resetPythonMacros();
  }
  QgsProject::instance()->writeEntry( QStringLiteral( "Macros" ), QStringLiteral( "/pythonCode" ), pythonMacros );

  QgsProject::instance()->relationManager()->setRelations( mRelationManagerDlg->relations() );
  QgsProject::instance()->relationManager()->setPolymorphicRelations( mRelationManagerDlg->polymorphicRelations() );

  //save variables
  QgsProject::instance()->setCustomVariables( mVariableEditor->variablesInActiveScope() );

  QgsProject::instance()->displaySettings()->setBearingFormat( mBearingFormat->clone() );

  for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalProjectPropertiesWidgets ) )
  {
    widget->apply();
  }

  //refresh canvases to reflect new properties, eg background color and scale bar after changing display units.
  for ( QgsMapCanvas *canvas : constMapCanvases )
  {
    canvas->redrawAllLayers();
  }
}

void QgsProjectProperties::lwWmsRowsInserted( const QModelIndex &parent, int first, int last )
{
  Q_UNUSED( parent )
  for ( int i = first; i <= last; i++ )
  {
    QString crsStr = mWMSList->item( i )->text();
    addWmtsGrid( crsStr );
  }
  twWmtsGrids->header()->resizeSections( QHeaderView::ResizeToContents );
}

void QgsProjectProperties::lwWmsRowsRemoved( const QModelIndex &parent, int first, int last )
{
  Q_UNUSED( parent )
  Q_UNUSED( first )
  Q_UNUSED( last )
  QStringList crslist;
  for ( int i = 0; i < mWMSList->count(); i++ )
  {
    crslist << mWMSList->item( i )->text();
  }
  for ( const QTreeWidgetItem *item : twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 ) )
  {
    QString crsStr = item->data( 0, Qt::UserRole ).toString();
    if ( crslist.contains( crsStr ) )
      continue;
    delete item;
  }
}

void QgsProjectProperties::twWmtsGridItemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Qt::ItemFlags flags = item->flags();
  if ( column == 0 || column == 6 )
  {
    item->setFlags( flags & ( ~Qt::ItemIsEditable ) );
  }
  else
  {
    QString crsStr = item->text( 0 );
    if ( crsStr == QLatin1String( "EPSG:3857" ) && column != 5 )
    {
      item->setFlags( flags & ( ~Qt::ItemIsEditable ) );
    }
    else if ( crsStr == QLatin1String( "EPSG:4326" ) && column != 5 )
    {
      item->setFlags( flags & ( ~Qt::ItemIsEditable ) );
    }
    else
    {
      item->setFlags( flags | Qt::ItemIsEditable );
    }
  }
}

void QgsProjectProperties::twWmtsGridItemChanged( QTreeWidgetItem *item, int column )
{
  if ( column == 4 || column == 5 )
  {
    double maxScale = item->data( 4, Qt::DisplayRole ).toFloat();
    int lastLevel = item->data( 5, Qt::DisplayRole ).toInt();
    item->setData( 6, Qt::DisplayRole, ( maxScale / std::pow( 2, lastLevel ) ) );
  }
}

void QgsProjectProperties::twWmtsItemChanged( QTreeWidgetItem *item, int column )
{
  if ( column == 1 && !item->checkState( 1 ) )
  {
    item->setCheckState( 2, Qt::Unchecked );
    item->setCheckState( 3, Qt::Unchecked );
  }
  else if ( column == 1 && item->checkState( 1 ) &&
            !item->checkState( 2 ) && !item->checkState( 3 ) )
  {
    item->setCheckState( 2, Qt::Checked );
    item->setCheckState( 3, Qt::Checked );
  }
  else if ( ( column == 2 && item->checkState( 2 ) ) ||
            ( column == 3 && item->checkState( 3 ) ) )
  {
    item->setCheckState( 1, Qt::Checked );
  }
  else if ( ( column == 2 && !item->checkState( 2 ) && !item->checkState( 3 ) ) ||
            ( column == 3 && !item->checkState( 2 ) && !item->checkState( 3 ) ) )
  {
    item->setCheckState( 1, Qt::Unchecked );
  }
}

void QgsProjectProperties::cbxWFSPubliedStateChanged( int aIdx )
{
  QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 1 ) );
  if ( cb && !cb->isChecked() )
  {
    QCheckBox *cbUpdate = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 3 ) );
    if ( cbUpdate )
    {
      cbUpdate->setChecked( false );
    }
    QCheckBox *cbInsert = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 4 ) );
    if ( cbInsert )
    {
      cbInsert->setChecked( false );
    }
    QCheckBox *cbDelete = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 5 ) );
    if ( cbDelete )
    {
      cbDelete->setChecked( false );
    }
  }
}

void QgsProjectProperties::cbxWCSPubliedStateChanged( int aIdx )
{
  QCheckBox *cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( aIdx, 1 ) );
  if ( cb && !cb->isChecked() )
  {
    QCheckBox *cbn = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( aIdx, 2 ) );
    if ( cbn )
      cbn->setChecked( false );
  }
}

void QgsProjectProperties::updateGuiForMapUnits()
{
  if ( !mCrs.isValid() )
  {
    // no projection set - disable everything!
    int idx = mDistanceUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit );
    if ( idx >= 0 )
    {
      mDistanceUnitsCombo->setItemText( idx, tr( "Unknown units" ) );
      mDistanceUnitsCombo->setCurrentIndex( idx );
    }
    idx = mAreaUnitsCombo->findData( QgsUnitTypes::AreaUnknownUnit );
    if ( idx >= 0 )
    {
      mAreaUnitsCombo->setItemText( idx, tr( "Unknown units" ) );
      mAreaUnitsCombo->setCurrentIndex( idx );
    }
    idx = mCoordinateDisplayComboBox->findData( MapUnits );
    if ( idx >= 0 )
    {
      mCoordinateDisplayComboBox->setItemText( idx, tr( "Unknown units" ) );
      mCoordinateDisplayComboBox->setCurrentIndex( idx );
    }
    mDistanceUnitsCombo->setEnabled( false );
    mAreaUnitsCombo->setEnabled( false );
    mCoordinateDisplayComboBox->setEnabled( false );
  }
  else
  {
    QgsUnitTypes::DistanceUnit units = mCrs.mapUnits();

    mDistanceUnitsCombo->setEnabled( true );
    mAreaUnitsCombo->setEnabled( true );
    mCoordinateDisplayComboBox->setEnabled( true );

    //make sure map units option is shown in coordinate display combo
    int idx = mCoordinateDisplayComboBox->findData( MapUnits );
    QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( units ) );
    mCoordinateDisplayComboBox->setItemText( idx, mapUnitString );

    //also update unit combo boxes
    idx = mDistanceUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit );
    if ( idx >= 0 )
    {
      QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( units ) );
      mDistanceUnitsCombo->setItemText( idx, mapUnitString );
    }
    idx = mAreaUnitsCombo->findData( QgsUnitTypes::AreaUnknownUnit );
    if ( idx >= 0 )
    {
      QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( QgsUnitTypes::distanceToAreaUnit( units ) ) );
      mAreaUnitsCombo->setItemText( idx, mapUnitString );
    }
  }
}

void QgsProjectProperties::crsChanged( const QgsCoordinateReferenceSystem &crs )
{
  const bool prevWasValid = mCrs.isValid();
  mCrs = crs;

  updateGuiForMapUnits();

  if ( mCrs.isValid() )
  {
    cmbEllipsoid->setEnabled( true );
    // don't automatically change a "NONE" ellipsoid when the crs changes, UNLESS
    // the project has gone from no CRS to a valid CRS
    if ( !prevWasValid || cmbEllipsoid->currentIndex() != 0 )
    {
      setCurrentEllipsoid( mCrs.ellipsoidAcronym() );
    }
  }
  else
  {
    cmbEllipsoid->setCurrentIndex( 0 );
    cmbEllipsoid->setEnabled( false );
  }

  mExtentGroupBox->setOutputCrs( crs );
}

void QgsProjectProperties::pbnWMSExtCanvas_clicked()
{
  QgsRectangle ext = mMapCanvas->extent();
  mWMSExtMinX->setText( qgsDoubleToString( ext.xMinimum() ) );
  mWMSExtMinY->setText( qgsDoubleToString( ext.yMinimum() ) );
  mWMSExtMaxX->setText( qgsDoubleToString( ext.xMaximum() ) );
  mWMSExtMaxY->setText( qgsDoubleToString( ext.yMaximum() ) );
}

void QgsProjectProperties::pbnWMSAddSRS_clicked()
{
  QgsProjectionSelectionDialog *mySelector = new QgsProjectionSelectionDialog( this );
  if ( mWMSList->count() > 0 )
  {
    mySelector->setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( mWMSList->item( mWMSList->count() - 1 )->text() ) );
  }
  if ( mySelector->exec() && mySelector->crs().isValid() )
  {
    QString authid = mySelector->crs().authid();

    QList<QListWidgetItem *> items = mWMSList->findItems( authid.mid( 5 ), Qt::MatchFixedString );
    if ( items.isEmpty() )
    {
      mWMSList->addItem( authid );
    }
    else
    {
      QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "CRS %1 was already selected" ).arg( authid ) );
    }
  }

  delete mySelector;
}

void QgsProjectProperties::pbnWMSRemoveSRS_clicked()
{
  const auto constSelectedItems = mWMSList->selectedItems();
  for ( QListWidgetItem *item : constSelectedItems )
  {
    delete item;
  }
}

void QgsProjectProperties::pbnWMSSetUsedSRS_clicked()
{
  if ( mWMSList->count() > 1 )
  {
    if ( QMessageBox::question( this,
                                tr( "Coordinate System Restrictions" ),
                                tr( "The current selection of coordinate systems will be lost.\nProceed?" ) ) == QMessageBox::No )
      return;
  }

  QSet<QString> crsList;
  if ( mCrs.isValid() )
    crsList << mCrs.authid();

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    crsList << it.value()->crs().authid();
  }

  mWMSList->clear();
  mWMSList->addItems( crsList.values() );
}

void QgsProjectProperties::mAddWMSPrintLayoutButton_clicked()
{
  const QList<QgsPrintLayout *> projectLayouts( QgsProject::instance()->layoutManager()->printLayouts() );
  QStringList layoutTitles;
  for ( const auto &layout : projectLayouts )
  {
    layoutTitles << layout->name();
  }

  bool ok;
  QString name = QInputDialog::getItem( this, tr( "Select layout" ), tr( "Layout Title" ), layoutTitles, 0, false, &ok );
  if ( ok )
  {
    if ( mPrintLayoutListWidget->findItems( name, Qt::MatchExactly ).empty() )
    {
      mPrintLayoutListWidget->addItem( name );
    }
  }
}

void QgsProjectProperties::mRemoveWMSPrintLayoutButton_clicked()
{
  QListWidgetItem *currentItem = mPrintLayoutListWidget->currentItem();
  if ( currentItem )
  {
    delete mPrintLayoutListWidget->takeItem( mPrintLayoutListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::mAddLayerRestrictionButton_clicked()
{
  QgsProjectLayerGroupDialog d( QgsProject::instance(), this );
  d.setWindowTitle( tr( "Select Restricted Layers and Groups" ) );
  if ( d.exec() == QDialog::Accepted )
  {
    QStringList layerNames = d.selectedLayerNames();
    QStringList::const_iterator layerIt = layerNames.constBegin();
    for ( ; layerIt != layerNames.constEnd(); ++layerIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *layerIt, Qt::MatchExactly ).empty() )
      {
        mLayerRestrictionsListWidget->addItem( *layerIt );
      }
    }

    QStringList groups = d.selectedGroups();
    QStringList::const_iterator groupIt = groups.constBegin();
    for ( ; groupIt != groups.constEnd(); ++groupIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *groupIt, Qt::MatchExactly ).empty() )
      {
        mLayerRestrictionsListWidget->addItem( *groupIt );
      }
    }
  }
}

void QgsProjectProperties::mRemoveLayerRestrictionButton_clicked()
{
  QListWidgetItem *currentItem = mLayerRestrictionsListWidget->currentItem();
  if ( currentItem )
  {
    delete mLayerRestrictionsListWidget->takeItem( mLayerRestrictionsListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::pbnWFSLayersSelectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( true );
  }
}

void QgsProjectProperties::mWMSInspireScenario1_toggled( bool on )
{
  mWMSInspireScenario2->blockSignals( true );
  mWMSInspireScenario2->setChecked( !on );
  mWMSInspireScenario2->blockSignals( false );
}

void QgsProjectProperties::mWMSInspireScenario2_toggled( bool on )
{
  mWMSInspireScenario1->blockSignals( true );
  mWMSInspireScenario1->setChecked( !on );
  mWMSInspireScenario1->blockSignals( false );
}

void QgsProjectProperties::pbnWFSLayersDeselectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( false );
  }
}

void QgsProjectProperties::pbnWCSLayersSelectAll_clicked()
{
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    cb->setChecked( true );
  }
}

void QgsProjectProperties::pbnWCSLayersDeselectAll_clicked()
{
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    cb->setChecked( false );
  }
}

void QgsProjectProperties::pbnLaunchOWSChecker_clicked()
{
  QList<QgsProjectServerValidator::ValidationResult> validationResults;
  bool results = QgsProjectServerValidator::validate( QgsProject::instance(), validationResults );

  QString errors;
  if ( !results )
  {
    for ( const QgsProjectServerValidator::ValidationResult &result : std::as_const( validationResults ) )
    {
      errors += QLatin1String( "<b>" ) % QgsProjectServerValidator::displayValidationError( result.error ) % QLatin1String( " :</b> " );
      errors += result.identifier.toString();
    }
  }
  else
  {
    errors += tr( "Project is valid." );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( QStringLiteral( "body { margin: 10px; }\n " ) );
  teOWSChecker->clear();
  teOWSChecker->document()->setDefaultStyleSheet( myStyle );
  teOWSChecker->setHtml( errors );
}

void QgsProjectProperties::pbnAddScale_clicked()
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
    lstScales->setCurrentItem( newItem );
  }
}

void QgsProjectProperties::pbnRemoveScale_clicked()
{
  int currentRow = lstScales->currentRow();
  QListWidgetItem *itemToRemove = lstScales->takeItem( currentRow );
  delete itemToRemove;
}

void QgsProjectProperties::pbnImportScales_clicked()
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

  const auto constMyScales = myScales;
  for ( const QString &scale : constMyScales )
  {
    addScaleToScaleList( scale );
  }
}

void QgsProjectProperties::pbnExportScales_clicked()
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
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( lstScales->item( i )->text() );
  }

  QString msg;
  if ( !QgsScaleUtils::saveScaleList( fileName, myScales, msg ) )
  {
    QgsDebugMsg( msg );
  }
}

void QgsProjectProperties::populateStyles()
{
  // Styles - taken from qgsstylemanagerdialog

  // use QComboBox and QString lists for shorter code
  QStringList prefList;
  QList<QComboBox *> cboList;
  cboList << cboStyleMarker;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ), QString() );
  cboList << cboStyleLine;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ), QString() );
  cboList << cboStyleFill;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ), QString() );
  cboList << cboStyleColorRamp;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QString() );
  for ( int i = 0; i < cboList.count(); i++ )
  {
    cboList[i]->clear();
    cboList[i]->addItem( QString() );
  }

  // populate symbols
  QStringList symbolNames = mStyle->symbolNames();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( name ) );
    if ( !symbol )
      continue;

    QComboBox *cbo = nullptr;
    switch ( symbol->type() )
    {
      case Qgis::SymbolType::Marker :
        cbo = cboStyleMarker;
        break;
      case Qgis::SymbolType::Line :
        cbo = cboStyleLine;
        break;
      case Qgis::SymbolType::Fill :
        cbo = cboStyleFill;
        break;
      case Qgis::SymbolType::Hybrid:
        // Shouldn't get here
        break;
    }
    if ( cbo )
    {
      QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol.get(), cbo->iconSize() );
      cbo->addItem( icon, name );
    }
  }

  // populate color ramps
  QStringList colorRamps = mStyle->colorRampNames();
  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    std::unique_ptr< QgsColorRamp > ramp( mStyle->colorRamp( name ) );
    QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( ramp.get(), cboStyleColorRamp->iconSize() );
    cboStyleColorRamp->addItem( icon, name );
  }

  // set current index if found
  for ( int i = 0; i < cboList.count(); i++ )
  {
    int index = cboList[i]->findText( prefList[i], Qt::MatchCaseSensitive );
    if ( index >= 0 )
      cboList[i]->setCurrentIndex( index );
  }

  // random colors
  cbxStyleRandomColors->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/RandomColors" ), true ) );

  // alpha transparency
  double opacity = 1.0;
  bool ok = false;
  double alpha = QgsProject::instance()->readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/AlphaInt" ), 255, &ok );
  if ( ok )
    opacity = 1.0 - alpha / 255.0;
  double newOpacity = QgsProject::instance()->readDoubleEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Opacity" ), 1.0, &ok );
  if ( ok )
    opacity = newOpacity;

  mDefaultOpacityWidget->setOpacity( opacity );
}

void QgsProjectProperties::pbtnStyleManager_clicked()
{
  QgsStyleManagerDialog dlg( mStyle, this );
  dlg.exec();
  populateStyles();
}

void QgsProjectProperties::pbtnStyleMarker_clicked()
{
  editSymbol( cboStyleMarker );
}

void QgsProjectProperties::pbtnStyleLine_clicked()
{
  editSymbol( cboStyleLine );
}

void QgsProjectProperties::pbtnStyleFill_clicked()
{
  editSymbol( cboStyleFill );
}

void QgsProjectProperties::pbtnStyleColorRamp_clicked()
{
  // TODO for now just open style manager
  // code in QgsStyleManagerDialog::editColorRamp()
  pbtnStyleManager_clicked();
}

void QgsProjectProperties::editSymbol( QComboBox *cbo )
{
  QString symbolName = cbo->currentText();
  if ( symbolName.isEmpty() )
  {
    QMessageBox::information( this, QString(), tr( "Select a valid symbol" ) );
    return;
  }
  std::unique_ptr< QgsSymbol > symbol( mStyle->symbol( symbolName ) );
  if ( ! symbol )
  {
    QMessageBox::warning( this, QString(), tr( "Invalid symbol : " ) + symbolName );
    return;
  }

  // let the user edit the symbol and update list when done
  QgsSymbolSelectorDialog dlg( symbol.get(), mStyle, nullptr, this );
  if ( dlg.exec() == 0 )
  {
    return;
  }

  // update icon
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol.get(), cbo->iconSize() );
  cbo->setItemIcon( cbo->currentIndex(), icon );

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol.release() );

}

void QgsProjectProperties::resetPythonMacros()
{
  grpPythonMacros->setChecked( false );
  ptePythonMacros->setText( "def openProject():\n    pass\n\n" \
                            "def saveProject():\n    pass\n\n" \
                            "def closeProject():\n    pass\n" );
}

void QgsProjectProperties::populateWmtsTree( const QgsLayerTreeGroup *treeGroup, QgsTreeWidgetItem *treeItem )
{
  for ( QgsLayerTreeNode *treeNode : treeGroup->children() )
  {
    QgsTreeWidgetItem *childItem = nullptr;
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      QString gName = treeGroupChild->name();

      childItem = new QgsTreeWidgetItem( QStringList() << gName );
      childItem->setFlags( childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );

      childItem->setData( 0, Qt::UserRole, QStringLiteral( "group" ) );
      childItem->setData( 0, Qt::UserRole + 1, gName );

      treeItem->addChild( childItem );

      populateWmtsTree( treeGroupChild, childItem );

      treeItem->setExpanded( true );
    }
    else
    {
      QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer *l = treeLayer->layer();

      if ( !l )
      {
        continue;
      }

      childItem = new QgsTreeWidgetItem( QStringList() << l->name() );
      childItem->setFlags( childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );

      childItem->setData( 0, Qt::UserRole, QStringLiteral( "layer" ) );
      childItem->setData( 0, Qt::UserRole + 1, l->id() );

      treeItem->addChild( childItem );
    }
  }
}

void QgsProjectProperties::addWmtsGrid( const QString &crsStr )
{
  QgsTreeWidgetItem *gridItem = new QgsTreeWidgetItem( QStringList() << crsStr );
  gridItem->setFlags( gridItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  gridItem->setCheckState( 1, Qt::Unchecked );
  gridItem->setData( 0, Qt::UserRole, crsStr );

  // Define or calculate top, left, max. scale
  int lastLevel = 18;
  double scaleDenominator = 0.0;
  if ( crsStr == QLatin1String( "EPSG:3857" ) )
  {
    gridItem->setData( 2, Qt::DisplayRole, 20037508.3427892480 );
    gridItem->setData( 3, Qt::DisplayRole, -20037508.3427892480 );
    scaleDenominator = 559082264.0287179;
  }
  else if ( crsStr == QLatin1String( "EPSG:4326" ) )
  {
    gridItem->setData( 2, Qt::DisplayRole, 90.0 );
    gridItem->setData( 3, Qt::DisplayRole, -180.0 );
    scaleDenominator = 279541132.0143588675418869;
  }
  else
  {
    // calculate top, left and scale based on CRS bounds
    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsStr );
    QgsCoordinateTransform crsTransform( QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() ), crs, QgsProject::instance() );
    crsTransform.setBallparkTransformsAreAppropriate( true );
    try
    {
      // firstly transform CRS bounds expressed in WGS84 to CRS
      QgsRectangle extent = crsTransform.transformBoundingBox( crs.bounds() );
      // Constant
      int tileSize = 256;
      double POINTS_TO_M = 2.83464567 / 10000;
      // Calculate scale denominator
      int colRes = ( extent.xMaximum() - extent.xMinimum() ) / tileSize;
      int rowRes = ( extent.yMaximum() - extent.yMinimum() ) / tileSize;
      if ( colRes > rowRes )
      {
        scaleDenominator = std::ceil( colRes * QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters ) / POINTS_TO_M );
      }
      else
      {
        scaleDenominator = std::ceil( rowRes * QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters ) / POINTS_TO_M );
      }
      // Calculate resolution based on scale denominator
      double res = POINTS_TO_M * scaleDenominator / QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), QgsUnitTypes::DistanceMeters );
      // Get numbers of column and row for the resolution
      int col = std::ceil( ( extent.xMaximum() - extent.xMinimum() ) / ( tileSize * res ) );
      int row = std::ceil( ( extent.yMaximum() - extent.yMinimum() ) / ( tileSize * res ) );
      // Update resolution and scale denominator to get 1 tile at this scale
      if ( col > 1 || row > 1 )
      {
        // Update scale
        if ( col > row )
        {
          res = col * res;
          scaleDenominator = col * scaleDenominator;
        }
        else
        {
          res = row * res;
          scaleDenominator = row * scaleDenominator;
        }
        // set col and row to 1 for the square
        col = 1;
        row = 1;
      }
      double top = ( extent.yMinimum() + ( extent.yMaximum() - extent.yMinimum() ) / 2.0 ) + ( row / 2.0 ) * ( tileSize * res );
      double left = ( extent.xMinimum() + ( extent.xMaximum() - extent.xMinimum() ) / 2.0 ) - ( col / 2.0 ) * ( tileSize * res );

      gridItem->setData( 2, Qt::DisplayRole, top );
      gridItem->setData( 3, Qt::DisplayRole, left );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse )
    }
  }
  gridItem->setData( 4, Qt::DisplayRole, scaleDenominator );
  gridItem->setData( 5, Qt::DisplayRole, lastLevel );
  gridItem->setData( 6, Qt::DisplayRole, ( scaleDenominator / std::pow( 2, lastLevel ) ) );
  twWmtsGrids->blockSignals( true );
  twWmtsGrids->addTopLevelItem( gridItem );
  twWmtsGrids->blockSignals( false );
}

void QgsProjectProperties::populateEllipsoidList()
{
  //
  // Populate the ellipsoid list
  //
  EllipsoidDefs myItem;

  myItem.acronym = geoNone();
  myItem.description = tr( GEO_NONE_DESC );
  myItem.semiMajor = 0.0;
  myItem.semiMinor = 0.0;
  mEllipsoidList.append( myItem );

  myItem.acronym = QStringLiteral( "PARAMETER:6370997:6370997" );
  myItem.description = tr( "Custom" );
  myItem.semiMajor = 6370997.0;
  myItem.semiMinor = 6370997.0;
  mEllipsoidList.append( myItem );

  const auto definitions {QgsEllipsoidUtils::definitions()};
  for ( const QgsEllipsoidUtils::EllipsoidDefinition &def : definitions )
  {
    myItem.acronym = def.acronym;
    myItem.description = def.description;
    // Fall-back values
    myItem.semiMajor = 0.0;
    myItem.semiMinor = 0.0;
    myItem.semiMajor = def.parameters.semiMajor;
    myItem.semiMinor = def.parameters.semiMinor;
    mEllipsoidList.append( myItem );
  }
  // Add all items to selector

  for ( const EllipsoidDefs &i : std::as_const( mEllipsoidList ) )
  {
    cmbEllipsoid->addItem( i.description );
  }
}

void QgsProjectProperties::updateEllipsoidUI( int newIndex )
{
  // Just return if the list isn't populated yet
  if ( mEllipsoidList.isEmpty() )
  {
    return;
  }
  // Called whenever settings change, adjusts the UI accordingly
  // Pre-select current ellipsoid

  // Check if CRS transformation is on, or else turn everything off
  double myMajor = mEllipsoidList.at( newIndex ).semiMajor;
  double myMinor = mEllipsoidList.at( newIndex ).semiMinor;

  // If user has modified the radii (only possible if parametric!), before
  // changing ellipsoid, save the modified coordinates
  if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Saving major/minor" ), 4 );
    mEllipsoidList[ mEllipsoidIndex ].semiMajor = QgsDoubleValidator::toDouble( leSemiMajor->text() );
    mEllipsoidList[ mEllipsoidIndex ].semiMinor = QgsDoubleValidator::toDouble( leSemiMinor->text() );
  }

  mEllipsoidIndex = newIndex;
  leSemiMajor->setEnabled( false );
  leSemiMinor->setEnabled( false );
  leSemiMajor->clear();
  leSemiMinor->clear();

  cmbEllipsoid->setEnabled( mCrs.isValid() );
  cmbEllipsoid->setToolTip( QString() );
  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( QLatin1String( "PARAMETER:" ) ) )
  {
    leSemiMajor->setEnabled( true );
    leSemiMinor->setEnabled( true );
  }
  else
  {
    leSemiMajor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Custom" ) ) );
    leSemiMinor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Custom" ) ) );
  }
  if ( mEllipsoidList[ mEllipsoidIndex ].acronym != geoNone() )
  {
    leSemiMajor->setText( QLocale().toString( myMajor, 'f', 3 ) );
    leSemiMinor->setText( QLocale().toString( myMinor, 'f', 3 ) );
  }

  if ( mCrs.isValid() )
    cmbEllipsoid->setCurrentIndex( mEllipsoidIndex ); // Not always necessary
}

void QgsProjectProperties::setCurrentEllipsoid( const QString &ellipsoidAcronym )
{
  int index = 0;
  if ( ellipsoidAcronym.startsWith( QLatin1String( "PARAMETER" ) ) )
  {
    // Update parameters if present.
    const QStringList mySplitEllipsoid = ellipsoidAcronym.split( ':' );
    for ( int i = 0; i < mEllipsoidList.length(); i++ )
    {
      if ( mEllipsoidList.at( i ).acronym.startsWith( QLatin1String( "PARAMETER" ), Qt::CaseInsensitive ) )
      {
        index = i;
        mEllipsoidList[ index ].semiMajor = mySplitEllipsoid[ 1 ].toDouble();
        mEllipsoidList[ index ].semiMinor = mySplitEllipsoid[ 2 ].toDouble();
      }
    }
  }
  else
  {
    for ( int i = 0; i < mEllipsoidList.length(); i++ )
    {
      if ( mEllipsoidList.at( i ).acronym.compare( ellipsoidAcronym, Qt::CaseInsensitive ) == 0 )
      {
        index = i;
        break;
      }
    }
  }

  updateEllipsoidUI( index );
}

void QgsProjectProperties::mButtonAddColor_clicked()
{
  QColor newColor = QgsColorDialog::getColor( QColor(), this->parentWidget(), tr( "Select Color" ), true );
  if ( !newColor.isValid() )
  {
    return;
  }
  activateWindow();

  mTreeProjectColors->addColor( newColor, QgsSymbolLayerUtils::colorToName( newColor ) );
}

void QgsProjectProperties::calculateFromLayersButton_clicked()
{
  const QgsDateTimeRange range = QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject::instance() );
  mStartDateTimeEdit->setDateTime( range.begin() );
  mEndDateTimeEdit->setDateTime( range.end() );
}

QListWidgetItem *QgsProjectProperties::addScaleToScaleList( const QString &newScale )
{
  // TODO QGIS3: Rework the scale list widget to be a reusable piece of code, see PR #2558
  QListWidgetItem *newItem = new QListWidgetItem( newScale );
  addScaleToScaleList( newItem );
  return newItem;
}

void QgsProjectProperties::addScaleToScaleList( QListWidgetItem *newItem )
{
  // If the new scale already exists, delete it.
  QListWidgetItem *duplicateItem = lstScales->findItems( newItem->text(), Qt::MatchExactly ).value( 0 );
  delete duplicateItem;

  int newDenominator = newItem->text().split( ':' ).value( 1 ).toInt();
  int i;
  for ( i = 0; i < lstScales->count(); i++ )
  {
    int denominator = lstScales->item( i )->text().split( ':' ).value( 1 ).toInt();
    if ( newDenominator > denominator )
      break;
  }

  newItem->setData( Qt::UserRole, newItem->text() );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  lstScales->insertItem( i, newItem );
}

void QgsProjectProperties::scaleItemChanged( QListWidgetItem *changedScaleItem )
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
    QMessageBox::warning( this, tr( "Set Scale" ), tr( "The text you entered is not a valid scale." ) );
    changedScaleItem->setText( changedScaleItem->data( Qt::UserRole ).toString() );
  }

  // Take the changed item out of the list and re-add it. This keeps things ordered and creates correct meta-data for the changed item.
  int row = lstScales->row( changedScaleItem );
  lstScales->takeItem( row );
  addScaleToScaleList( changedScaleItem );
  lstScales->setCurrentItem( changedScaleItem );
}

void QgsProjectProperties::showHelp()
{
  QWidget *activeTab = mOptionsStackedWidget->currentWidget();
  QString link = QStringLiteral( "introduction/qgis_configuration.html#project-properties" );

  if ( activeTab == mTabRelations )
  {
    link = QStringLiteral( "working_with_vector/attribute_table.html#creating-one-or-many-to-many-relations" );
  }
  else if ( activeTab == mTab_Variables )
  {
    link = QStringLiteral( "introduction/general_tools.html#variables" );
  }
  else if ( activeTab == mProjOptsCRS )
  {
    link = QStringLiteral( "working_with_projections/working_with_projections.html" );
  }
  else if ( activeTab == mProjOptsOWS )
  {
    link = QStringLiteral( "working_with_ogc/server/getting_started.html#prepare-a-project-to-serve" );
  }

  // give first priority to created pages which have specified a help key
  for ( const QgsOptionsPageWidget *widget : std::as_const( mAdditionalProjectPropertiesWidgets ) )
  {
    if ( widget == activeTab )
    {
      link = widget->helpKey();
      break;
    }
  }

  QgsHelp::openHelp( link );
}

void QgsProjectProperties::checkPageWidgetNameMap()
{
  const QMap< QString, QString > pageNames = QgisApp::instance()->projectPropertiesPagesMap();
  Q_ASSERT_X( pageNames.count() == mOptionsListWidget->count(), "QgsProjectProperties::checkPageWidgetNameMap()", "QgisApp::projectPropertiesPagesMap() is outdated, contains too many entries" );
  for ( int idx = 0; idx < mOptionsListWidget->count(); ++idx )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( idx );
    QListWidgetItem *item = mOptionsListWidget->item( idx );
    const QString title = item->text();
    const QString name = currentPage->objectName();
    Q_ASSERT_X( pageNames.contains( title ), "QgsProjectProperties::checkPageWidgetNameMap()", QStringLiteral( "QgisApp::projectPropertiesPagesMap() is outdated, please update. Missing %1" ).arg( title ).toLocal8Bit().constData() );
    Q_ASSERT_X( pageNames.value( title ) == name, "QgsProjectProperties::checkPageWidgetNameMap()", QStringLiteral( "QgisApp::projectPropertiesPagesMap() is outdated, please update. %1 should be %2 not %3" ).arg( title, name, pageNames.value( title ) ).toLocal8Bit().constData() );
  }
}

void QgsProjectProperties::setCurrentPage( const QString &pageWidgetName )
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

void QgsProjectProperties::onGenerateTsFileButton() const
{
  QString l = cbtsLocale->currentData().toString();
  QgsProject::instance()->generateTsFile( l );
  QMessageBox::information( nullptr, tr( "General TS file generated" ), tr( "TS file generated with source language %1.\n"
                            "- open it with Qt Linguist\n"
                            "- translate strings\n"
                            "- save the TS file with the suffix of the target language (e.g. _de.ts)\n"
                            "- release to get the QM file including the suffix (e.g. aproject_de.qm)\n"
                            "- open the original QGIS file (e.g. aproject.qgs)\n"
                            "- if your QGIS is set to use a specific language and the QM file for that language is found, the translated QGIS project will be generated on the fly.\n"
                            "- you will be redirected to this new project (e.g. aproject_de.qgs)." ).arg( l ) ) ;
}

void QgsProjectProperties::customizeBearingFormat()
{
  QgsBearingNumericFormatDialog dlg( mBearingFormat.get(), this );
  dlg.setWindowTitle( tr( "Bearing Format" ) );
  if ( dlg.exec() )
  {
    mBearingFormat.reset( dlg.format() );
  }
}
