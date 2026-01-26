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

#include "moc_qgsprojectproperties.cpp"

//qgis includes
#include "qgsapplication.h"
#include "qgisapp.h"
#include "qgis.h"
#include "qgscolorutils.h"
#include "qgscoordinatetransform.h"
#include "qgsdatumtransformtablewidget.h"
#include "qgslayoutmanager.h"
#include "qgslogger.h"
#include "qgsgui.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsprojectstylesettings.h"
#include "qgsnative.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsscaleutils.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgscolorramp.h"
#include "qgsrelationmanagerdialog.h"
#include "qgsrelationmanager.h"
#include "qgsdoublevalidator.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscolordialog.h"
#include "qgsexpressioncontext.h"
#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertreemodel.h"
#include "qgsunittypes.h"
#include "qgstreewidgetitem.h"
#include "qgslayertree.h"
#include "qgsprintlayout.h"
#include "qgsmetadatawidget.h"
#include "qgslayercapabilitiesmodel.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprojectservervalidator.h"
#include "qgsprojectstorage.h"
#include "qgsprojectviewsettings.h"
#include "qgsnumericformatwidget.h"
#include "qgsbearingnumericformat.h"
#include "qgscoordinatenumericformat.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalutils.h"
#include "qgsstylemanagerdialog.h"
#include "qgsfileutils.h"

//qt includes
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QAbstractListModel>
#include <QList>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

const char *QgsProjectProperties::GEO_NONE_DESC = QT_TRANSLATE_NOOP( "QgsOptions", "None / Planimetric" );

//stdc++ includes

QgsProjectProperties::QgsProjectProperties( QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags fl, const QList<QgsOptionsWidgetFactory *> &optionsFactories )
  : QgsOptionsDialogBase( u"ProjectProperties"_s, parent, fl )
  , mMapCanvas( mapCanvas )
{
  // set wait cursor since construction of the project properties
  // dialog can be slow
  QgsTemporaryCursorOverride cursorOverride( Qt::WaitCursor );

  setupUi( this );

  mExtentGroupBox->setCheckable( true );

  mExtentWidget = new QgsExtentWidget( parent, QgsExtentWidget::ExpandedStyle );
  QVBoxLayout *mExtentGroupBoxLayout = qobject_cast<QVBoxLayout *>( mExtentGroupBox->layout() );
  mExtentGroupBoxLayout->insertWidget( 0, mExtentWidget );

  mExtentWidget->setMapCanvas( mapCanvas, false );

  mAdvertisedExtentServer->setOutputCrs( QgsProject::instance()->crs() );
  mAdvertisedExtentServer->setMapCanvas( mapCanvas, false );

  mMetadataWidget = new QgsMetadataWidget();
  mMetadataPage->layout()->addWidget( mMetadataWidget );

  connect( pbnAddScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnAddScale_clicked );
  connect( pbnRemoveScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnRemoveScale_clicked );
  connect( pbnImportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnImportScales_clicked );
  connect( pbnExportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnExportScales_clicked );
  connect( grpWMSExt, &QGroupBox::toggled, this, &QgsProjectProperties::wmsExtent_toggled );
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
  connect( mButtonAddColor, &QToolButton::clicked, this, &QgsProjectProperties::mButtonAddColor_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsProjectProperties::showHelp );
  connect( mCustomizeBearingFormatButton, &QPushButton::clicked, this, &QgsProjectProperties::customizeBearingFormat );
  connect( mCustomizeCoordinateFormatButton, &QPushButton::clicked, this, &QgsProjectProperties::customizeGeographicCoordinateFormat );
  connect( mCalculateFromLayerButton, &QPushButton::clicked, this, &QgsProjectProperties::calculateFromLayersButton_clicked );
  connect( mButtonAddStyleDatabase, &QAbstractButton::clicked, this, &QgsProjectProperties::addStyleDatabase );
  connect( mButtonRemoveStyleDatabase, &QAbstractButton::clicked, this, &QgsProjectProperties::removeStyleDatabase );
  connect( mButtonNewStyleDatabase, &QAbstractButton::clicked, this, &QgsProjectProperties::newStyleDatabase );
  connect( mCoordinateDisplayComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [this]( int ) { updateGuiForCoordinateType(); } );
  connect( mCoordinateCrs, &QgsProjectionSelectionWidget::crsChanged, this, [this]( const QgsCoordinateReferenceSystem & ) { updateGuiForCoordinateCrs(); } );
#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
  connect( mAddIccProfile, &QToolButton::clicked, this, static_cast<void ( QgsProjectProperties::* )()>( &QgsProjectProperties::addIccProfile ) );
  connect( mRemoveIccProfile, &QToolButton::clicked, this, &QgsProjectProperties::removeIccProfile );
  connect( mSaveIccProfile, &QToolButton::clicked, this, &QgsProjectProperties::saveIccProfile );
#endif

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  pixStyleMarker->setFixedSize( 24, 24 );
  pixStyleMarker->load( QgsApplication::iconPath( u"mIconPointLayer.svg"_s ) );

  pixStyleFill->setFixedSize( 24, 24 );
  pixStyleFill->load( QgsApplication::iconPath( u"mIconPolygonLayer.svg"_s ) );

  pixStyleLine->setFixedSize( 24, 24 );
  pixStyleLine->load( QgsApplication::iconPath( u"mIconLineLayer.svg"_s ) );

  pixStyleColorRamp->setFixedSize( 24, 24 );
  pixStyleColorRamp->load( QgsApplication::iconPath( u"styleicons/color.svg"_s ) );

  pixStyleTextFormat->setFixedSize( 24, 24 );
  pixStyleTextFormat->load( QgsApplication::iconPath( u"text.svg"_s ) );

  mCoordinateDisplayComboBox->addItem( tr( "Map Units" ), static_cast<int>( Qgis::CoordinateDisplayType::MapCrs ) );
  mCoordinateDisplayComboBox->addItem( tr( "Map Geographic (degrees)" ), static_cast<int>( Qgis::CoordinateDisplayType::MapGeographic ) );
  mCoordinateDisplayComboBox->addItem( tr( "Custom Projection Units" ), static_cast<int>( Qgis::CoordinateDisplayType::CustomCrs ) );

  mCoordinateOrderComboBox->addItem( tr( "Default" ), static_cast<int>( Qgis::CoordinateOrder::Default ) );
  mCoordinateOrderComboBox->addItem( tr( "Easting, Northing (Longitude, Latitude)" ), static_cast<int>( Qgis::CoordinateOrder::XY ) );
  mCoordinateOrderComboBox->addItem( tr( "Northing, Easting (Latitude, Longitude)" ), static_cast<int>( Qgis::CoordinateOrder::YX ) );

  mDistanceUnitsCombo->addItem( tr( "Meters" ), static_cast<int>( Qgis::DistanceUnit::Meters ) );
  mDistanceUnitsCombo->addItem( tr( "Kilometers" ), static_cast<int>( Qgis::DistanceUnit::Kilometers ) );
  mDistanceUnitsCombo->addItem( tr( "Feet" ), static_cast<int>( Qgis::DistanceUnit::Feet ) );
  mDistanceUnitsCombo->addItem( tr( "Yards" ), static_cast<int>( Qgis::DistanceUnit::Yards ) );
  mDistanceUnitsCombo->addItem( tr( "Miles" ), static_cast<int>( Qgis::DistanceUnit::Miles ) );
  mDistanceUnitsCombo->addItem( tr( "Nautical Miles" ), static_cast<int>( Qgis::DistanceUnit::NauticalMiles ) );
  mDistanceUnitsCombo->addItem( tr( "Centimeters" ), static_cast<int>( Qgis::DistanceUnit::Centimeters ) );
  mDistanceUnitsCombo->addItem( tr( "Millimeters" ), static_cast<int>( Qgis::DistanceUnit::Millimeters ) );
  mDistanceUnitsCombo->addItem( tr( "Inches" ), static_cast<int>( Qgis::DistanceUnit::Inches ) );
  mDistanceUnitsCombo->addItem( tr( "Degrees" ), static_cast<int>( Qgis::DistanceUnit::Degrees ) );
  mDistanceUnitsCombo->addItem( tr( "Map Units" ), static_cast<int>( Qgis::DistanceUnit::Unknown ) );

  mAreaUnitsCombo->addItem( tr( "Square Meters" ), static_cast<int>( Qgis::AreaUnit::SquareMeters ) );
  mAreaUnitsCombo->addItem( tr( "Square Kilometers" ), static_cast<int>( Qgis::AreaUnit::SquareKilometers ) );
  mAreaUnitsCombo->addItem( tr( "Square Feet" ), static_cast<int>( Qgis::AreaUnit::SquareFeet ) );
  mAreaUnitsCombo->addItem( tr( "Square Yards" ), static_cast<int>( Qgis::AreaUnit::SquareYards ) );
  mAreaUnitsCombo->addItem( tr( "Square Miles" ), static_cast<int>( Qgis::AreaUnit::SquareMiles ) );
  mAreaUnitsCombo->addItem( tr( "Hectares" ), static_cast<int>( Qgis::AreaUnit::Hectares ) );
  mAreaUnitsCombo->addItem( tr( "Acres" ), static_cast<int>( Qgis::AreaUnit::Acres ) );
  mAreaUnitsCombo->addItem( tr( "Square Nautical Miles" ), static_cast<int>( Qgis::AreaUnit::SquareNauticalMiles ) );
  mAreaUnitsCombo->addItem( tr( "Square Centimeters" ), static_cast<int>( Qgis::AreaUnit::SquareCentimeters ) );
  mAreaUnitsCombo->addItem( tr( "Square Millimeters" ), static_cast<int>( Qgis::AreaUnit::SquareMillimeters ) );
  mAreaUnitsCombo->addItem( tr( "Square Inches" ), static_cast<int>( Qgis::AreaUnit::SquareInches ) );
  mAreaUnitsCombo->addItem( tr( "Square Degrees" ), static_cast<int>( Qgis::AreaUnit::SquareDegrees ) );
  mAreaUnitsCombo->addItem( tr( "Map Units" ), static_cast<int>( Qgis::AreaUnit::Unknown ) );

  mTransactionModeComboBox->addItem( tr( "Local Edit Buffer" ), static_cast<int>( Qgis::TransactionMode::Disabled ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Edits are buffered locally and sent to the provider when toggling layer editing mode." ), Qt::ToolTipRole );
  mTransactionModeComboBox->addItem( tr( "Automatic Transaction Groups" ), static_cast<int>( Qgis::TransactionMode::AutomaticGroups ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Automatic transactional editing means that on supported datasources (postgres databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction." ), Qt::ToolTipRole );
  mTransactionModeComboBox->addItem( tr( "Buffered Transaction Groups" ), static_cast<int>( Qgis::TransactionMode::BufferedGroups ) );
  mTransactionModeComboBox->setItemData( mTransactionModeComboBox->count() - 1, tr( "Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider)." ), Qt::ToolTipRole );

  projectionSelector->setShowNoProjection( true );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsProjectProperties::apply );
  connect( this, &QDialog::finished, this, [this]( int result ) { if ( result == QDialog::Rejected ) cancel(); } );

  // disconnect default connection setup by initOptionsBase for accepting dialog, and insert logic
  // to validate widgets before allowing dialog to be closed
  disconnect( mOptButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mOptButtonBox, &QDialogButtonBox::accepted, this, [this] {
    for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalProjectPropertiesWidgets ) )
    {
      if ( !widget->isValid() )
      {
        setCurrentPage( widget->objectName() );
        return;
      }
    }
    apply();
    accept();
  } );

  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, [this] {
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

  bool show = settings.value( u"/projections/promptWhenMultipleTransformsExist"_s, false, QgsSettings::App ).toBool();
  mShowDatumTransformDialogCheckBox->setChecked( show );

  QPolygonF mainCanvasPoly = mapCanvas->mapSettings().visiblePolygon();
  QgsGeometry g = QgsGeometry::fromQPolygonF( mainCanvasPoly );
  // close polygon
  mainCanvasPoly << mainCanvasPoly.at( 0 );
  if ( QgsProject::instance()->crs() != QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
  {
    // reproject extent
    QgsCoordinateTransform ct( QgsProject::instance()->crs(), QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), QgsProject::instance() );
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

  mMapTileRenderingCheckBox->setChecked( QgsProject::instance()->readBoolEntry( u"RenderMapTile"_s, u"/"_s, false ) );

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
  mStartDateTimeEdit->setDisplayFormat( u"yyyy-MM-dd HH:mm:ss"_s );
  mEndDateTimeEdit->setDisplayFormat( u"yyyy-MM-dd HH:mm:ss"_s );

  mStartDateTimeEdit->setDateTime( range.begin() );
  mEndDateTimeEdit->setDateTime( range.end() );

  title( QgsProject::instance()->title() );

  QString name = !QgsProject::instance()->fileName().isEmpty() ? QgsProject::instance()->fileName() : QgsProject::instance()->originalPath();
  if ( QgsProject::instance()->projectStorage() )
  {
    name = QgsDataSourceUri::removePassword( name, true );
  }

  mProjectFileLineEdit->setText( QDir::toNativeSeparators( name ) );

  mProjectHomeLineEdit->setShowClearButton( true );
  mProjectHomeLineEdit->setText( QDir::toNativeSeparators( QgsProject::instance()->presetHomePath() ) );
  connect( mButtonSetProjectHome, &QToolButton::clicked, this, [this] {
    auto getAbsoluteHome = [this]() -> QString {
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
    if ( !newDir.isNull() )
    {
      mProjectHomeLineEdit->setText( QDir::toNativeSeparators( newDir ) );
    }
  } );

  connect( mButtonOpenProjectFolder, &QToolButton::clicked, this, [] {
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
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( u"PositionPrecision"_s, u"/Automatic"_s, true );
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
  int dp = QgsProject::instance()->readNumEntry( u"PositionPrecision"_s, u"/DecimalPlaces"_s );
  spinBoxDP->setValue( dp );

  cbxAbsolutePath->addItem( tr( "Absolute" ), static_cast<int>( Qgis::FilePathType::Absolute ) );
  cbxAbsolutePath->addItem( tr( "Relative" ), static_cast<int>( Qgis::FilePathType::Relative ) );
  cbxAbsolutePath->setCurrentIndex( cbxAbsolutePath->findData( static_cast<int>( QgsProject::instance()->filePathStorage() ) ) );

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
      if ( mEllipsoidList[i].acronym == QgsProject::instance()->crs().ellipsoidAcronym() )
      {
        index = i;
        break;
      }
    }
    updateEllipsoidUI( index );
  }

  const Qgis::CoordinateDisplayType coordinateType = QgsProject::instance()->displaySettings()->coordinateType();
  mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( static_cast<int>( coordinateType ) ) );
  switch ( coordinateType )
  {
    case Qgis::CoordinateDisplayType::MapCrs:
      mCoordinateCrs->setEnabled( false );
      mCoordinateCrs->setCrs( mCrs );
      break;
    case Qgis::CoordinateDisplayType::MapGeographic:
      mCoordinateCrs->setEnabled( false );
      mCoordinateCrs->setCrs( !mCrs.isGeographic() ? mCrs.toGeographicCrs() : mCrs );
      break;
    case Qgis::CoordinateDisplayType::CustomCrs:
      mCoordinateCrs->setEnabled( true );
      mCoordinateCrs->setCrs( QgsProject::instance()->displaySettings()->coordinateCustomCrs() );
      break;
  }

  const Qgis::CoordinateOrder axisOrder = QgsProject::instance()->displaySettings()->coordinateAxisOrder();
  mCoordinateOrderComboBox->setCurrentIndex( mCoordinateOrderComboBox->findData( static_cast<int>( axisOrder ) ) );

  mDistanceUnitsCombo->setCurrentIndex( mDistanceUnitsCombo->findData( static_cast<int>( QgsProject::instance()->distanceUnits() ) ) );
  mAreaUnitsCombo->setCurrentIndex( mAreaUnitsCombo->findData( static_cast<int>( QgsProject::instance()->areaUnits() ) ) );

  mScaleMethodWidget->setScaleMethod( QgsProject::instance()->scaleMethod() );

  //get the color selections and set the button color accordingly
  int myRedInt = settings.value( u"qgis/default_selection_color_red"_s, 255 ).toInt();
  int myGreenInt = settings.value( u"qgis/default_selection_color_green"_s, 255 ).toInt();
  int myBlueInt = settings.value( u"qgis/default_selection_color_blue"_s, 0 ).toInt();
  int myAlphaInt = settings.value( u"qgis/default_selection_color_alpha"_s, 255 ).toInt();
  QColor defaultSelectionColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );

  pbnSelectionColor->setContext( u"gui"_s );
  pbnSelectionColor->setColor( QgsProject::instance()->selectionColor() );
  pbnSelectionColor->setDefaultColor( defaultSelectionColor );
  pbnSelectionColor->setColorDialogTitle( tr( "Selection Color" ) );
  pbnSelectionColor->setAllowOpacity( true );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = settings.value( u"qgis/default_canvas_color_red"_s, 255 ).toInt();
  myGreenInt = settings.value( u"qgis/default_canvas_color_green"_s, 255 ).toInt();
  myBlueInt = settings.value( u"qgis/default_canvas_color_blue"_s, 255 ).toInt();
  QColor defaultCanvasColor = QColor( myRedInt, myGreenInt, myBlueInt );

  pbnCanvasColor->setContext( u"gui"_s );
  pbnCanvasColor->setColor( QgsProject::instance()->backgroundColor() );
  pbnCanvasColor->setDefaultColor( defaultCanvasColor );

  //get project scales
  const QVector<double> projectScales = QgsProject::instance()->viewSettings()->mapScales();
  if ( !projectScales.isEmpty() )
  {
    for ( double scale : projectScales )
    {
      addScaleToScaleList( scale );
    }
  }
  connect( lstScales, &QListWidget::itemChanged, this, &QgsProjectProperties::scaleItemChanged );

  grpProjectScales->setChecked( QgsProject::instance()->viewSettings()->useProjectScales() );

  const QgsReferencedRectangle presetExtent = QgsProject::instance()->viewSettings()->presetFullExtent();
  mExtentWidget->setOutputCrs( QgsProject::instance()->crs() );
  if ( presetExtent.isNull() )
    mExtentWidget->setOutputExtentFromUser( QgsProject::instance()->viewSettings()->fullExtent(), QgsProject::instance()->crs() );
  else
    mExtentWidget->setOutputExtentFromUser( presetExtent, presetExtent.crs() );
  mExtentGroupBox->setChecked( !presetExtent.isNull() );
  mCheckBoxLoadProjectExtent->setChecked( QgsProject::instance()->viewSettings()->restoreProjectExtentOnProjectLoad() );

  mLayerCapabilitiesModel = new QgsLayerCapabilitiesModel( QgsProject::instance(), this );
  mLayerCapabilitiesModel->setLayerTreeModel( new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), mLayerCapabilitiesModel ) );
  mLayerCapabilitiesTree->setModel( mLayerCapabilitiesModel );
  mLayerCapabilitiesTree->resizeColumnToContents( 0 );
  mLayerCapabilitiesTree->header()->show();
  mLayerCapabilitiesTree->setSelectionBehavior( QAbstractItemView::SelectItems );
  mLayerCapabilitiesTree->setSelectionMode( QAbstractItemView::MultiSelection );
  mLayerCapabilitiesTree->expandAll();
  connect( mLayerCapabilitiesTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this]( const QItemSelection &selected, const QItemSelection &deselected ) {
    Q_UNUSED( selected )
    Q_UNUSED( deselected )
    bool hasSelection = !mLayerCapabilitiesTree->selectionModel()->selectedIndexes().isEmpty();
    mLayerCapabilitiesToggleSelectionButton->setEnabled( hasSelection );
  } );

  mLayerCapabilitiesTreeFilterLineEdit->setShowClearButton( true );
  mLayerCapabilitiesTreeFilterLineEdit->setShowSearchIcon( true );
  mLayerCapabilitiesTreeFilterLineEdit->setPlaceholderText( tr( "Filter layers…" ) );
  connect( mLayerCapabilitiesTreeFilterLineEdit, &QgsFilterLineEdit::textChanged, this, [this]( const QString &filterText ) {
    mLayerCapabilitiesModel->setFilterText( filterText );
    mLayerCapabilitiesTree->expandAll();
  } );

  connect( mLayerCapabilitiesToggleSelectionButton, &QToolButton::clicked, this, [this]( bool clicked ) {
    Q_UNUSED( clicked )
    const QModelIndexList indexes = mLayerCapabilitiesTree->selectionModel()->selectedIndexes();
    mLayerCapabilitiesModel->toggleSelectedItems( indexes );
    mLayerCapabilitiesTree->repaint();
  } );

  connect( mShowSpatialLayersCheckBox, &QCheckBox::stateChanged, this, [this]( int state ) {
    mLayerCapabilitiesModel->setShowSpatialLayersOnly( static_cast<bool>( state ) );
  } );

  grpOWSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( u"WMSServiceCapabilities"_s, u"/"_s, false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( u"WMSServiceTitle"_s, u"/"_s ) );
  mWMSName->setText( QgsProject::instance()->readEntry( u"WMSRootName"_s, u"/"_s ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( u"WMSContactOrganization"_s, u"/"_s, QString() ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( u"WMSContactPerson"_s, u"/"_s, QString() ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( u"WMSContactMail"_s, u"/"_s, QString() ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( u"WMSContactPhone"_s, u"/"_s, QString() ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( u"WMSServiceAbstract"_s, u"/"_s, QString() ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( u"WMSOnlineResource"_s, u"/"_s, QString() ) );
  mWMSUrlLineEdit->setText( QgsProject::instance()->readEntry( u"WMSUrl"_s, u"/"_s, QString() ) );
  mWMSKeywordList->setText( QgsProject::instance()->readListEntry( u"WMSKeywordList"_s, u"/"_s ).join( ", "_L1 ) );

  mWMSOnlineResourceExpressionButton->registerExpressionContextGenerator( this );
  mWMSOnlineResourceExpressionButton->setToProperty( QgsProject::instance()->dataDefinedServerProperties().property( QgsProject::DataDefinedServerProperty::WMSOnlineResource ) );

  // WMS Name validator
  QValidator *shortNameValidator = new QRegularExpressionValidator( QgsApplication::shortNameRegularExpression(), this );
  mWMSName->setValidator( shortNameValidator );

  // WMS Contact Position
  QString contactPositionText = QgsProject::instance()->readEntry( u"WMSContactPosition"_s, u"/"_s, QString() );
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
  QString feesText = QgsProject::instance()->readEntry( u"WMSFees"_s, u"/"_s, QString() );
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
  QString accessConstraintsText = QgsProject::instance()->readEntry( u"WMSAccessConstraints"_s, u"/"_s, QString() );
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

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "bg" ) ), QLocale( u"bg"_s ).nativeLanguageName(), "bul" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cs" ) ), QLocale( u"cs"_s ).nativeLanguageName(), "cze" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "da" ) ), QLocale( u"da"_s ).nativeLanguageName(), "dan" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "nl" ) ), QLocale( u"nl"_s ).nativeLanguageName(), "dut" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "en_GB" ) ), QLocale( u"en_GB"_s ).nativeLanguageName(), "eng" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "et" ) ), QLocale( u"et"_s ).nativeLanguageName(), "est" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fi" ) ), QLocale( u"fi"_s ).nativeLanguageName(), "fin" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fr" ) ), QLocale( u"fr"_s ).nativeLanguageName(), "fre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "de" ) ), QLocale( u"de"_s ).nativeLanguageName(), "ger" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ga" ) ), QLocale( u"ga"_s ).nativeLanguageName(), "gle" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "el" ) ), QLocale( u"el"_s ).nativeLanguageName(), "gre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "hu" ) ), QLocale( u"hu"_s ).nativeLanguageName(), "hun" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "it" ) ), QLocale( u"it"_s ).nativeLanguageName(), "ita" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lv" ) ), QLocale( u"lv"_s ).nativeLanguageName(), "lav" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lt" ) ), QLocale( u"lt"_s ).nativeLanguageName(), "lit" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "mt" ) ), QLocale( u"mt"_s ).nativeLanguageName(), "mlt" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pl" ) ), QLocale( u"pl"_s ).nativeLanguageName(), "pol" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pt_PT" ) ), QLocale( u"pt_PT"_s ).nativeLanguageName(), "por" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ro" ) ), QLocale( u"ro"_s ).nativeLanguageName(), "rum" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sk" ) ), QLocale( u"sk"_s ).nativeLanguageName(), "slo" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sl" ) ), QLocale( u"sl"_s ).nativeLanguageName(), "slv" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "es" ) ), QLocale( u"es"_s ).nativeLanguageName(), "spa" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sv" ) ), QLocale( u"sv"_s ).nativeLanguageName(), "swe" );

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "eu" ) ), QLocale( u"eu"_s ).nativeLanguageName(), "eus" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ca" ) ), QLocale( u"ca"_s ).nativeLanguageName(), "cat" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gl" ) ), QLocale( u"gl"_s ).nativeLanguageName(), "gal" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gd" ) ), QLocale( u"gd"_s ).nativeLanguageName(), "gla" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cy" ) ), QLocale( u"cy"_s ).nativeLanguageName(), "cym" );
  mWMSInspireLanguage->setCurrentIndex(
    mWMSInspireLanguage->findText(
      QLocale().nativeLanguageName()
    )
  );

  bool addWMSInspire = QgsProject::instance()->readBoolEntry( u"WMSInspire"_s, u"/activated"_s );
  if ( addWMSInspire )
  {
    mWMSInspire->setChecked( addWMSInspire );
    QString inspireLanguage = QgsProject::instance()->readEntry( u"WMSInspire"_s, u"/language"_s, QString() );
    int inspireLanguageIndex = mWMSInspireLanguage->findData( inspireLanguage );
    mWMSInspireLanguage->setCurrentIndex( inspireLanguageIndex );

    QString inspireMetadataUrl = QgsProject::instance()->readEntry( u"WMSInspire"_s, u"/metadataUrl"_s, QString() );
    if ( !inspireMetadataUrl.isEmpty() )
    {
      mWMSInspireScenario1->setChecked( true );
      mWMSInspireMetadataUrl->setText( inspireMetadataUrl );
      mWMSInspireMetadataUrlType->setCurrentIndex(
        mWMSInspireMetadataUrlType->findText(
          QgsProject::instance()->readEntry( u"WMSInspire"_s, u"/metadataUrlType"_s, QString() )
        )
      );
    }
    else
    {
      QString inspireTemporalReference = QgsProject::instance()->readEntry( u"WMSInspire"_s, u"/temporalReference"_s, QString() );
      if ( !inspireTemporalReference.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireTemporalReference->setDate( QDate::fromString( inspireTemporalReference, u"yyyy-MM-dd"_s ) );
      }
      QString inspireMetadataDate = QgsProject::instance()->readEntry( u"WMSInspire"_s, u"/metadataDate"_s, QString() );
      if ( !inspireMetadataDate.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireMetadataDate->setDate( QDate::fromString( inspireMetadataDate, u"yyyy-MM-dd"_s ) );
      }
    }
  }

  // WMS GetFeatureInfo precision
  int WMSprecision = QgsProject::instance()->readNumEntry( u"WMSPrecision"_s, u"/"_s, -1 );
  if ( WMSprecision != -1 )
  {
    mWMSPrecisionSpinBox->setValue( WMSprecision );
  }
  mWMSPrecisionSpinBox->setClearValue( 8 );

  bool ok = false;
  QStringList values;

  values = QgsProject::instance()->readListEntry( u"WMSExtent"_s, u"/"_s, QStringList(), &ok );
  grpWMSExt->setChecked( ok && values.size() == 4 );
  if ( grpWMSExt->isChecked() )
  {
    mAdvertisedExtentServer->setOriginalExtent(
      QgsRectangle(
        values[0].toDouble(),
        values[1].toDouble(),
        values[2].toDouble(),
        values[3].toDouble()
      ),
      QgsProject::instance()->crs()
    );
    mAdvertisedExtentServer->setOutputExtentFromOriginal();
  }

  values = QgsProject::instance()->readListEntry( u"WMSCrsList"_s, u"/"_s, QStringList(), &ok );
  grpWMSList->setChecked( ok && !values.isEmpty() );
  if ( grpWMSList->isChecked() )
  {
    mWMSList->addItems( values );
  }
  else
  {
    const QStringList wmsEpsgListValues = QgsProject::instance()->readListEntry( u"WMSEpsgList"_s, u"/"_s, QStringList(), &ok );
    grpWMSList->setChecked( ok && !values.isEmpty() );
    if ( grpWMSList->isChecked() )
    {
      QStringList list;
      list.reserve( wmsEpsgListValues.size() );
      for ( const QString &value : wmsEpsgListValues )
      {
        list << u"EPSG:%1"_s.arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  //Layout restriction for WMS
  values = QgsProject::instance()->readListEntry( u"WMSRestrictedComposers"_s, u"/"_s, QStringList(), &ok );
  mWMSPrintLayoutGroupBox->setChecked( ok );
  if ( ok )
  {
    mPrintLayoutListWidget->addItems( values );
  }

  //layer restriction for WMS
  values = QgsProject::instance()->readListEntry( u"WMSRestrictedLayers"_s, u"/"_s, QStringList(), &ok );
  mLayerRestrictionsGroupBox->setChecked( ok );
  if ( ok )
  {
    mLayerRestrictionsListWidget->addItems( values );
  }

  bool useAttributeFormSettings = QgsProject::instance()->readBoolEntry( u"WMSFeatureInfoUseAttributeFormSettings"_s, u"/"_s );
  mUseAttributeFormSettingsCheckBox->setChecked( useAttributeFormSettings );

  bool useOnlyMaptip = QgsProject::instance()->readBoolEntry( u"WMSHTMLFeatureInfoUseOnlyMaptip"_s, u"/"_s );
  mHTMLFiOnlyMaptip->setChecked( useOnlyMaptip );

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( u"WMSAddWktGeometry"_s, u"/"_s );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  bool segmentizeFeatureInfoGeometry = QgsProject::instance()->readBoolEntry( u"WMSSegmentizeFeatureInfoGeometry"_s, u"/"_s );
  mSegmentizeFeatureInfoGeometryCheckBox->setChecked( segmentizeFeatureInfoGeometry );

  bool addLayerGroupsLegendGraphic = QgsProject::instance()->readBoolEntry( u"WMSAddLayerGroupsLegendGraphic"_s, u"/"_s );
  mAddLayerGroupsLegendGraphicCheckBox->setChecked( addLayerGroupsLegendGraphic );
  bool skipNameForGroup = QgsProject::instance()->readBoolEntry( u"WMSSkipNameForGroup"_s, u"/"_s );
  mSkipNameForGroupCheckBox->setChecked( skipNameForGroup );

  bool useLayerIDs = QgsProject::instance()->readBoolEntry( u"WMSUseLayerIDs"_s, u"/"_s );
  mWmsUseLayerIDs->setChecked( useLayerIDs );

  //WMS maxWidth / maxHeight
  mMaxWidthLineEdit->setValidator( new QIntValidator( mMaxWidthLineEdit ) );
  int maxWidth = QgsProject::instance()->readNumEntry( u"WMSMaxWidth"_s, u"/"_s, -1 );
  if ( maxWidth != -1 )
  {
    mMaxWidthLineEdit->setText( QString::number( maxWidth ) );
  }
  mMaxHeightLineEdit->setValidator( new QIntValidator( mMaxHeightLineEdit ) );
  int maxHeight = QgsProject::instance()->readNumEntry( u"WMSMaxHeight"_s, u"/"_s, -1 );
  if ( maxHeight != -1 )
  {
    mMaxHeightLineEdit->setText( QString::number( maxHeight ) );
  }

  // WMS imageQuality
  int imageQuality = QgsProject::instance()->readNumEntry( u"WMSImageQuality"_s, u"/"_s, -1 );
  if ( imageQuality != -1 )
  {
    mWMSImageQualitySpinBox->setValue( imageQuality );
  }
  mWMSImageQualitySpinBox->setClearValue( 90 );

  // WMS tileBuffer
  mWMSTileBufferSpinBox->setValue( QgsProject::instance()->readNumEntry( u"WMSTileBuffer"_s, u"/"_s, 0 ) );

  mWMSMaxAtlasFeaturesSpinBox->setValue( QgsProject::instance()->readNumEntry( u"WMSMaxAtlasFeatures"_s, u"/"_s, 1 ) );
  mWMSMaxAtlasFeaturesSpinBox->setClearValue( 1 );

  QString defaultValueToolTip = tr( "In case of no other information to evaluate the map unit sized symbols, it uses default scale (on projected CRS) or default map units per mm (on geographic CRS)." );
  if ( QgsProject::instance()->crs().isGeographic() )
  {
    mWMSDefaultMapUnitsPerMm = new QDoubleSpinBox();
    mWMSDefaultMapUnitsPerMm->setDecimals( 4 );
    mWMSDefaultMapUnitsPerMm->setSingleStep( 0.001 );
    mWMSDefaultMapUnitsPerMm->setValue( QgsProject::instance()->readDoubleEntry( u"WMSDefaultMapUnitsPerMm"_s, u"/"_s, 1 ) );
    mWMSDefaultMapUnitsPerMm->setToolTip( defaultValueToolTip );
    mWMSDefaultMapUnitsPerMmLayout->addWidget( mWMSDefaultMapUnitsPerMm );
  }
  else
  {
    mWMSDefaultMapUnitScale = new QgsScaleWidget();
    mWMSDefaultMapUnitScale->setScale( QgsProject::instance()->readDoubleEntry( u"WMSDefaultMapUnitsPerMm"_s, u"/"_s, 1 ) * QgsUnitTypes::fromUnitToUnitFactor( QgsProject::instance()->crs().mapUnits(), Qgis::DistanceUnit::Millimeters ) );
    mWMSDefaultMapUnitScale->setToolTip( defaultValueToolTip );
    mWMSDefaultMapUnitsPerMmLayout->addWidget( mWMSDefaultMapUnitScale );
    mWMSDefaultMapUnitsPerMmLabel->setText( tr( "Default scale for legend" ) );
  }

  mWMTSUrlLineEdit->setText( QgsProject::instance()->readEntry( u"WMTSUrl"_s, u"/"_s, QString() ) );
  mWMTSMinScaleSpinBox->setValue( QgsProject::instance()->readNumEntry( u"WMTSMinScale"_s, u"/"_s, 5000 ) );
  mWMTSMinScaleSpinBox->setClearValue( 5000 );

  bool wmtsProject = QgsProject::instance()->readBoolEntry( u"WMTSLayers"_s, u"Project"_s );
  bool wmtsPngProject = QgsProject::instance()->readBoolEntry( u"WMTSPngLayers"_s, u"Project"_s );
  bool wmtsJpegProject = QgsProject::instance()->readBoolEntry( u"WMTSJpegLayers"_s, u"Project"_s );
  QStringList wmtsGroupNameList = QgsProject::instance()->readListEntry( u"WMTSLayers"_s, u"Group"_s );
  QStringList wmtsPngGroupNameList = QgsProject::instance()->readListEntry( u"WMTSPngLayers"_s, u"Group"_s );
  QStringList wmtsJpegGroupNameList = QgsProject::instance()->readListEntry( u"WMTSJpegLayers"_s, u"Group"_s );
  QStringList wmtsLayerIdList = QgsProject::instance()->readListEntry( u"WMTSLayers"_s, u"Layer"_s );
  QStringList wmtsPngLayerIdList = QgsProject::instance()->readListEntry( u"WMTSPngLayers"_s, u"Layer"_s );
  QStringList wmtsJpegLayerIdList = QgsProject::instance()->readListEntry( u"WMTSJpegLayers"_s, u"Layer"_s );

  QgsTreeWidgetItem *projItem = new QgsTreeWidgetItem( QStringList() << u"Project"_s );
  projItem->setFlags( projItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );
  projItem->setCheckState( 1, wmtsProject ? Qt::Checked : Qt::Unchecked );
  projItem->setCheckState( 2, wmtsPngProject ? Qt::Checked : Qt::Unchecked );
  projItem->setCheckState( 3, wmtsJpegProject ? Qt::Checked : Qt::Unchecked );
  projItem->setData( 0, Qt::UserRole, u"project"_s );
  twWmtsLayers->addTopLevelItem( projItem );
  populateWmtsTree( QgsProject::instance()->layerTreeRoot(), projItem );
  projItem->setExpanded( true );
  twWmtsLayers->header()->resizeSections( QHeaderView::ResizeToContents );
  const QList<QTreeWidgetItem *> wmtsItems = twWmtsLayers->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 );
  for ( QTreeWidgetItem *item : wmtsItems )
  {
    QString itemType = item->data( 0, Qt::UserRole ).toString();
    if ( itemType == "group"_L1 )
    {
      QString gName = item->data( 0, Qt::UserRole + 1 ).toString();
      item->setCheckState( 1, wmtsGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 2, wmtsPngGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
      item->setCheckState( 3, wmtsJpegGroupNameList.contains( gName ) ? Qt::Checked : Qt::Unchecked );
    }
    else if ( itemType == "layer"_L1 )
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
  const QStringList wmtsGridList = QgsProject::instance()->readListEntry( u"WMTSGrids"_s, u"CRS"_s );
  if ( !wmtsGridList.isEmpty() )
  {
    const QStringList wmtsGridConfigList = QgsProject::instance()->readListEntry( u"WMTSGrids"_s, u"Config"_s );
    QMap<QString, QStringList> wmtsGridConfigs;
    for ( const QString &c : wmtsGridConfigList )
    {
      QStringList config = c.split( ',' );
      wmtsGridConfigs[config[0]] = config;
    }
    const QList<QTreeWidgetItem *> wmtsGridItems = twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 );
    for ( QTreeWidgetItem *item : wmtsGridItems )
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

  mWFSUrlLineEdit->setText( QgsProject::instance()->readEntry( u"WFSUrl"_s, u"/"_s, QString() ) );
  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( u"WFSLayers"_s, u"/"_s );
  QStringList wfstUpdateLayerIdList = QgsProject::instance()->readListEntry( u"WFSTLayers"_s, u"Update"_s );
  QStringList wfstInsertLayerIdList = QgsProject::instance()->readListEntry( u"WFSTLayers"_s, u"Insert"_s );
  QStringList wfstDeleteLayerIdList = QgsProject::instance()->readListEntry( u"WFSTLayers"_s, u"Delete"_s );

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
    if ( currentLayer->type() == Qgis::LayerType::Vector )
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
      connect( cbp, &QCheckBox::stateChanged, this, [this, j] { cbxWCSPubliedStateChanged( j ); } );

      QSpinBox *psb = new QSpinBox();
      psb->setValue( QgsProject::instance()->readNumEntry( u"WFSLayersPrecision"_s, "/" + currentLayer->id(), 8 ) );
      twWFSLayers->setCellWidget( j, 2, psb );

      QCheckBox *cbu = new QCheckBox();
      cbu->setEnabled( false );
      if ( ( provider->capabilities() & Qgis::VectorProviderCapability::ChangeAttributeValues ) )
      {
        if ( !currentLayer->isSpatial() || ( provider->capabilities() & Qgis::VectorProviderCapability::ChangeGeometries ) )
        {
          cbu->setEnabled( true );
          cbu->setChecked( wfstUpdateLayerIdList.contains( currentLayer->id() ) );
        }
      }
      twWFSLayers->setCellWidget( j, 3, cbu );

      QCheckBox *cbi = new QCheckBox();
      cbi->setEnabled( false );
      if ( ( provider->capabilities() & Qgis::VectorProviderCapability::AddFeatures ) )
      {
        cbi->setEnabled( true );
        cbi->setChecked( wfstInsertLayerIdList.contains( currentLayer->id() ) );
      }
      twWFSLayers->setCellWidget( j, 4, cbi );

      QCheckBox *cbd = new QCheckBox();
      cbd->setEnabled( false );
      if ( ( provider->capabilities() & Qgis::VectorProviderCapability::DeleteFeatures ) )
      {
        cbd->setEnabled( true );
        cbd->setChecked( wfstDeleteLayerIdList.contains( currentLayer->id() ) );
      }
      twWFSLayers->setCellWidget( j, 5, cbd );

      j++;
    }
  }
  twWFSLayers->setRowCount( j );
  twWFSLayers->resizeColumnToContents( 0 );
  twWFSLayers->resizeColumnToContents( 2 );
  twWFSLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  mWCSUrlLineEdit->setText( QgsProject::instance()->readEntry( u"WCSUrl"_s, u"/"_s, QString() ) );
  QStringList wcsLayerIdList = QgsProject::instance()->readListEntry( u"WCSLayers"_s, u"/"_s );

  twWCSLayers->setColumnCount( 2 );
  twWCSLayers->horizontalHeader()->setVisible( true );
  twWCSLayers->setRowCount( mapLayers.size() );

  i = 0;
  j = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == Qgis::LayerType::Raster )
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

      connect( cbp, &QCheckBox::stateChanged, this, [this, j] { cbxWCSPubliedStateChanged( j ); } );

      j++;
    }
  }
  twWCSLayers->setRowCount( j );
  twWCSLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  // Default styles
  mStyleMarkerSymbol->setShowNull( true );
  mStyleMarkerSymbol->setSymbolType( Qgis::SymbolType::Marker );
  mStyleMarkerSymbol->setSymbol( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Marker ) );

  mStyleLineSymbol->setShowNull( true );
  mStyleLineSymbol->setSymbolType( Qgis::SymbolType::Line );
  mStyleLineSymbol->setSymbol( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Line ) );

  mStyleFillSymbol->setShowNull( true );
  mStyleFillSymbol->setSymbolType( Qgis::SymbolType::Fill );
  mStyleFillSymbol->setSymbol( QgsProject::instance()->styleSettings()->defaultSymbol( Qgis::SymbolType::Fill ) );

  mStyleColorRampSymbol->setShowNull( true );
  mStyleColorRampSymbol->setColorRamp( QgsProject::instance()->styleSettings()->defaultColorRamp() );

  mStyleTextFormat->setShowNullFormat( true );
  mStyleTextFormat->setNoFormatString( tr( "Clear Current Text Format" ) );
  QgsTextFormat textFormat = QgsProject::instance()->styleSettings()->defaultTextFormat();
  if ( textFormat.isValid() )
  {
    mStyleTextFormat->setTextFormat( textFormat );
  }
  else
  {
    mStyleTextFormat->setToNullFormat();
  }

  // Random colors
  cbxStyleRandomColors->setChecked( QgsProject::instance()->styleSettings()->randomizeDefaultSymbolColor() );

  mColorModel->addItem( tr( "RGB" ), QVariant::fromValue( Qgis::ColorModel::Rgb ) );
  mColorModel->addItem( tr( "CMYK" ), QVariant::fromValue( Qgis::ColorModel::Cmyk ) );
  mColorModel->setCurrentIndex( mColorModel->findData( QVariant::fromValue( QgsProject::instance()->styleSettings()->colorModel() ) ) );

#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
  mColorSpace = QgsProject::instance()->styleSettings()->colorSpace();
  updateColorSpaceWidgets();
#else
  mIccProfileLabel->setVisible( false );
  mColorSpaceName->setVisible( false );
  mAddIccProfile->setVisible( false );
  mRemoveIccProfile->setVisible( false );
  mSaveIccProfile->setVisible( false );
#endif
  // Default alpha transparency
  mDefaultOpacityWidget->setOpacity( QgsProject::instance()->styleSettings()->defaultSymbolOpacity() );

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

  {
    const QStringList styleDatabasePaths = QgsProject::instance()->styleSettings()->styleDatabasePaths();
    for ( const QString &path : styleDatabasePaths )
    {
      QListWidgetItem *newItem = new QListWidgetItem( mListStyleDatabases );
      newItem->setText( QDir::toNativeSeparators( path ) );
      newItem->setData( Qt::UserRole, path );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mListStyleDatabases->addItem( newItem );
    }
  }

  // Project macros
  QString pythonMacros = QgsProject::instance()->readEntry( u"Macros"_s, u"/pythonCode"_s, QString() );
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
    if ( Qgis::LayerType::Vector == mapLayer->type() )
    {
      vectorLayers.append( qobject_cast<QgsVectorLayer *>( mapLayer ) );
    }
  }
  mRelationManagerDlg->setLayers( vectorLayers );

  mTransactionModeComboBox->setCurrentIndex( mTransactionModeComboBox->findData( static_cast<int>( QgsProject::instance()->transactionMode() ) ) );
  mEvaluateDefaultValues->setChecked( QgsProject::instance()->flags() & Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide );
  mTrustProjectCheckBox->setChecked( QgsProject::instance()->flags() & Qgis::ProjectFlag::TrustStoredLayerStatistics );
  mCheckRememberEditStatus->setChecked( QgsProject::instance()->flags() & Qgis::ProjectFlag::RememberLayerEditStatusBetweenSessions );
  mCheckBoxRememberAttributeTables->setChecked( QgsProject::instance()->flags() & Qgis::ProjectFlag::RememberAttributeTableWindowsBetweenSessions );

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
  connect( titleEdit, &QLineEdit::textChanged, this, [this] { whileBlocking( mMetadataWidget )->setTitle( title() ); } );

  //fill ts language checkbox
  QString i18nPath = QgsApplication::i18nPath();
  QDir i18Dir( i18nPath, u"qgis*.qm"_s );
  const QStringList qmFileList = i18Dir.entryList();
  for ( const QString &qmFile : qmFileList )
  {
    // Ignore the 'en' translation file, already added as 'en_US'.
    if ( qmFile.compare( "qgis_en.qm"_L1 ) == 0 )
      continue;

    QString qmFileName = qmFile;
    QString l = qmFileName.remove( u"qgis_"_s ).remove( u".qm"_s );

    // QTBUG-57802: eo locale is improperly handled
    QString displayName = l.startsWith( "eo"_L1 ) ? QLocale::languageToString( QLocale::Esperanto ) : QLocale( l ).nativeLanguageName();
    cbtsLocale->addItem( QIcon( u":/images/flags/%1.svg"_s.arg( l ) ), displayName, l );
  }

  cbtsLocale->addItem( QIcon( u":/images/flags/%1.svg"_s.arg( "en_US"_L1 ) ), QLocale( u"en_US"_s ).nativeLanguageName(), u"en_US"_s );
  cbtsLocale->setCurrentIndex( cbtsLocale->findData( QgsApplication::settingsLocaleUserLocale->value() ) );

  connect( generateTsFileButton, &QPushButton::clicked, this, &QgsProjectProperties::onGenerateTsFileButton );

  // Reading ellipsoid from settings
  setCurrentEllipsoid( QgsProject::instance()->ellipsoid() );

  mBearingFormat.reset( QgsProject::instance()->displaySettings()->bearingFormat()->clone() );
  mGeographicCoordinateFormat.reset( QgsProject::instance()->displaySettings()->geographicCoordinateFormat()->clone() );

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

    page->setObjectName( factory->title() );

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
  for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalProjectPropertiesWidgets ) )
  {
    if ( !widget->isValid() )
    {
      setCurrentPage( widget->objectName() );
      return;
    }
  }


  mMapCanvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"RenderMapTile"_s, u"/"_s, mMapTileRenderingCheckBox->isChecked() );

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
      QgsDebugMsgLevel( u"Selected CRS "_s + srs.userFriendlyIdentifier(), 4 );
      // write the currently selected projections _proj string_ to project settings
      QgsDebugMsgLevel( u"SpatialRefSys/ProjectCRSProj4String: %1"_s.arg( srs.toProj() ), 4 );
    }
    else
    {
      QgsDebugMsgLevel( u"CRS set to no projection!"_s, 4 );
    }
  }

  mMetadataWidget->acceptMetadata();

  // Set the project title
  QgsProject::instance()->setTitle( title() );
  QgsProject::instance()->setPresetHomePath( QDir::fromNativeSeparators( mProjectHomeLineEdit->text() ) );

  // DB-related options
  QgsProject::instance()->setTransactionMode( mTransactionModeComboBox->currentData().value<Qgis::TransactionMode>() );
  QgsProject::instance()->setFlag( Qgis::ProjectFlag::EvaluateDefaultValuesOnProviderSide, mEvaluateDefaultValues->isChecked() );
  QgsProject::instance()->setFlag( Qgis::ProjectFlag::TrustStoredLayerStatistics, mTrustProjectCheckBox->isChecked() );

  QgsProject::instance()->setFlag( Qgis::ProjectFlag::RememberLayerEditStatusBetweenSessions, mCheckRememberEditStatus->isChecked() );
  QgsProject::instance()->setFlag( Qgis::ProjectFlag::RememberAttributeTableWindowsBetweenSessions, mCheckBoxRememberAttributeTables->isChecked() );

  // Time settings
  QDateTime start = mStartDateTimeEdit->dateTime();
  QDateTime end = mEndDateTimeEdit->dateTime();

  QgsProject::instance()->timeSettings()->setTemporalRange( QgsDateTimeRange( start, end ) );

  // set the mouse display precision method and the
  // number of decimal places for the manual option
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  QgsProject::instance()->writeEntry( u"PositionPrecision"_s, u"/Automatic"_s, radAutomatic->isChecked() );
  QgsProject::instance()->writeEntry( u"PositionPrecision"_s, u"/DecimalPlaces"_s, spinBoxDP->value() );

  const Qgis::CoordinateDisplayType coordinateType = static_cast<Qgis::CoordinateDisplayType>( mCoordinateDisplayComboBox->currentData().toInt() );
  QgsProject::instance()->displaySettings()->setCoordinateType( coordinateType );
  if ( coordinateType == Qgis::CoordinateDisplayType::CustomCrs )
  {
    QgsProject::instance()->displaySettings()->setCoordinateCustomCrs( mCoordinateCrs->crs() );
  }
  else
  {
    QgsProject::instance()->displaySettings()->setCoordinateCustomCrs( QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  }

  QgsProject::instance()->displaySettings()->setCoordinateAxisOrder( static_cast<Qgis::CoordinateOrder>( mCoordinateOrderComboBox->currentData().toInt() ) );

  // Announce that we may have a new display precision setting
  emit displayPrecisionChanged();

  const Qgis::DistanceUnit distanceUnits = static_cast<Qgis::DistanceUnit>( mDistanceUnitsCombo->currentData().toInt() );
  QgsProject::instance()->setDistanceUnits( distanceUnits );

  const Qgis::AreaUnit areaUnits = static_cast<Qgis::AreaUnit>( mAreaUnitsCombo->currentData().toInt() );
  QgsProject::instance()->setAreaUnits( areaUnits );

  QgsProject::instance()->setScaleMethod( mScaleMethodWidget->scaleMethod() );

  QgsProject::instance()->setFilePathStorage( static_cast<Qgis::FilePathType>( cbxAbsolutePath->currentData().toInt() ) );

  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( "PARAMETER"_L1 ) )
  {
    double major = mEllipsoidList.at( mEllipsoidIndex ).semiMajor;
    double minor = mEllipsoidList.at( mEllipsoidIndex ).semiMinor;
    // If the user fields have changed, use them instead.
    if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
    {
      QgsDebugMsgLevel( u"Using parameteric major/minor"_s, 4 );
      bool ok;
      double val { QgsDoubleValidator::toDouble( leSemiMajor->text(), &ok ) };
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
    QgsProject::instance()->setEllipsoid( u"PARAMETER:%1:%2"_s
                                            .arg( major, 0, 'g', 17 )
                                            .arg( minor, 0, 'g', 17 ) );
  }
  else
  {
    QgsProject::instance()->setEllipsoid( mEllipsoidList[mEllipsoidIndex].acronym );
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
  QVector<double> projectScales;
  projectScales.reserve( lstScales->count() );
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    bool ok;
    const double scaleDenominator { lstScales->item( i )->data( Qt::UserRole ).toDouble( &ok ) };
    if ( ok )
    {
      projectScales.append( scaleDenominator );
    }
  }

  if ( !projectScales.isEmpty() )
  {
    QgsProject::instance()->viewSettings()->setMapScales( projectScales );
    QgsProject::instance()->viewSettings()->setUseProjectScales( grpProjectScales->isChecked() );
  }
  else
  {
    QgsProject::instance()->viewSettings()->setMapScales( QVector<double>() );
    QgsProject::instance()->viewSettings()->setUseProjectScales( false );
  }

  if ( mExtentGroupBox->isChecked() )
  {
    QgsProject::instance()->viewSettings()->setPresetFullExtent( QgsReferencedRectangle( mExtentWidget->outputExtent(), mExtentWidget->outputCrs() ) );
  }
  else
  {
    QgsProject::instance()->viewSettings()->setPresetFullExtent( QgsReferencedRectangle() );
  }

  QgsProject::instance()->viewSettings()->setRestoreProjectExtentOnProjectLoad( mCheckBoxLoadProjectExtent->isChecked() );

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

  QgsProject::instance()->writeEntry( u"WMSServiceCapabilities"_s, u"/"_s, grpOWSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSServiceTitle"_s, u"/"_s, mWMSTitle->text() );
  QgsProject::instance()->writeEntry( u"WMSRootName"_s, u"/"_s, mWMSName->text() );
  QgsProject::instance()->writeEntry( u"WMSContactOrganization"_s, u"/"_s, mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( u"WMSContactPerson"_s, u"/"_s, mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( u"WMSContactMail"_s, u"/"_s, mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( u"WMSContactPhone"_s, u"/"_s, mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( u"WMSServiceAbstract"_s, u"/"_s, mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( u"WMSOnlineResource"_s, u"/"_s, mWMSOnlineResourceLineEdit->text() );
  QgsProject::instance()->writeEntry( u"WMSUrl"_s, u"/"_s, mWMSUrlLineEdit->text() );

  QgsPropertyCollection propertyCollection = QgsProject::instance()->dataDefinedServerProperties();
  propertyCollection.setProperty( QgsProject::DataDefinedServerProperty::WMSOnlineResource, mWMSOnlineResourceExpressionButton->toProperty() );
  QgsProject::instance()->setDataDefinedServerProperties( propertyCollection );

  // WMS Contact Position
  int contactPositionIndex = mWMSContactPositionCb->currentIndex();
  QString contactPositionText = mWMSContactPositionCb->currentText();
  if ( !contactPositionText.isEmpty() && contactPositionText == mWMSContactPositionCb->itemText( contactPositionIndex ) )
  {
    QgsProject::instance()->writeEntry( u"WMSContactPosition"_s, u"/"_s, mWMSContactPositionCb->itemData( contactPositionIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSContactPosition"_s, u"/"_s, contactPositionText );
  }

  // WMS Fees
  int feesIndex = mWMSFeesCb->currentIndex();
  QString feesText = mWMSFeesCb->currentText();
  if ( !feesText.isEmpty() && feesText == mWMSFeesCb->itemText( feesIndex ) )
  {
    QgsProject::instance()->writeEntry( u"WMSFees"_s, u"/"_s, mWMSFeesCb->itemData( feesIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSFees"_s, u"/"_s, feesText );
  }

  // WMS Access Constraints
  int accessConstraintsIndex = mWMSAccessConstraintsCb->currentIndex();
  QString accessConstraintsText = mWMSAccessConstraintsCb->currentText();
  if ( !accessConstraintsText.isEmpty() && accessConstraintsText == mWMSAccessConstraintsCb->itemText( accessConstraintsIndex ) )
  {
    QgsProject::instance()->writeEntry( u"WMSAccessConstraints"_s, u"/"_s, mWMSAccessConstraintsCb->itemData( accessConstraintsIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSAccessConstraints"_s, u"/"_s, accessConstraintsText );
  }

  //WMS keyword list
  QStringList keywordStringList = mWMSKeywordList->text().split( ',' );
  if ( !keywordStringList.isEmpty() )
  {
    keywordStringList.replaceInStrings( QRegularExpression( u"^\\s+"_s ), QString() ).replaceInStrings( QRegularExpression( "\\s+$" ), QString() );
    QgsProject::instance()->writeEntry( u"WMSKeywordList"_s, u"/"_s, keywordStringList );
  }
  else
  {
    QgsProject::instance()->removeEntry( u"WMSKeywordList"_s, u"/"_s );
  }

  // WMS INSPIRE configuration
  QgsProject::instance()->removeEntry( u"WMSInspire"_s, u"/"_s );
  if ( mWMSInspire->isChecked() )
  {
    QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/activated"_s, mWMSInspire->isChecked() );

    int inspireLanguageIndex = mWMSInspireLanguage->currentIndex();
    QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/language"_s, mWMSInspireLanguage->itemData( inspireLanguageIndex ).toString() );

    if ( mWMSInspireScenario1->isChecked() )
    {
      QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/metadataUrl"_s, mWMSInspireMetadataUrl->text() );
      QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/metadataUrlType"_s, mWMSInspireMetadataUrlType->currentText() );
    }
    else if ( mWMSInspireScenario2->isChecked() )
    {
      QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/temporalReference"_s, mWMSInspireTemporalReference->date().toString( u"yyyy-MM-dd"_s ) );
      QgsProject::instance()->writeEntry( u"WMSInspire"_s, u"/metadataDate"_s, mWMSInspireMetadataDate->date().toString( u"yyyy-MM-dd"_s ) );
    }
  }

  // WMS GetFeatureInfo geometry precision (decimal places)
  QgsProject::instance()->writeEntry( u"WMSPrecision"_s, u"/"_s, mWMSPrecisionSpinBox->text() );

  if ( grpWMSExt->isChecked() )
  {
    QgsRectangle wmsExtent = mAdvertisedExtentServer->outputExtent();
    QgsProject::instance()->writeEntry(
      u"WMSExtent"_s, u"/"_s,
      QStringList()
        << qgsDoubleToString( wmsExtent.xMinimum() )
        << qgsDoubleToString( wmsExtent.yMinimum() )
        << qgsDoubleToString( wmsExtent.xMaximum() )
        << qgsDoubleToString( wmsExtent.yMaximum() )
    );
  }
  else
  {
    QgsProject::instance()->removeEntry( u"WMSExtent"_s, u"/"_s );
  }

  if ( grpWMSList->isChecked() && mWMSList->count() == 0 )
  {
    QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "No coordinate systems selected. Disabling restriction." ) );
    grpWMSList->setChecked( false );
  }

  QgsProject::instance()->removeEntry( u"WMSEpsgList"_s, u"/"_s );

  if ( grpWMSList->isChecked() )
  {
    QStringList crslist;
    crslist.reserve( mWMSList->count() );
    for ( int i = 0; i < mWMSList->count(); i++ )
    {
      crslist << mWMSList->item( i )->text();
    }

    QgsProject::instance()->writeEntry( u"WMSCrsList"_s, u"/"_s, crslist );
  }
  else
  {
    QgsProject::instance()->removeEntry( u"WMSCrsList"_s, u"/"_s );
  }

  //WMS composer restrictions
  if ( mWMSPrintLayoutGroupBox->isChecked() )
  {
    QStringList composerTitles;
    composerTitles.reserve( mPrintLayoutListWidget->count() );
    for ( int i = 0; i < mPrintLayoutListWidget->count(); ++i )
    {
      composerTitles << mPrintLayoutListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( u"WMSRestrictedComposers"_s, u"/"_s, composerTitles );
  }
  else
  {
    QgsProject::instance()->removeEntry( u"WMSRestrictedComposers"_s, u"/"_s );
  }

  //WMS layer restrictions
  if ( mLayerRestrictionsGroupBox->isChecked() )
  {
    QStringList layerNames;
    layerNames.reserve( mLayerRestrictionsListWidget->count() );
    for ( int i = 0; i < mLayerRestrictionsListWidget->count(); ++i )
    {
      layerNames << mLayerRestrictionsListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( u"WMSRestrictedLayers"_s, u"/"_s, layerNames );
  }
  else
  {
    QgsProject::instance()->removeEntry( u"WMSRestrictedLayers"_s, u"/"_s );
  }

  QgsProject::instance()->writeEntry( u"WMSFeatureInfoUseAttributeFormSettings"_s, u"/"_s, mUseAttributeFormSettingsCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSHTMLFeatureInfoUseOnlyMaptip"_s, u"/"_s, mHTMLFiOnlyMaptip->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSAddWktGeometry"_s, u"/"_s, mAddWktGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSSegmentizeFeatureInfoGeometry"_s, u"/"_s, mSegmentizeFeatureInfoGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSAddLayerGroupsLegendGraphic"_s, u"/"_s, mAddLayerGroupsLegendGraphicCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSSkipNameForGroup"_s, u"/"_s, mSkipNameForGroupCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( u"WMSUseLayerIDs"_s, u"/"_s, mWmsUseLayerIDs->isChecked() );

  QString maxWidthText = mMaxWidthLineEdit->text();
  if ( maxWidthText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( u"WMSMaxWidth"_s, u"/"_s );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSMaxWidth"_s, u"/"_s, maxWidthText.toInt() );
  }
  QString maxHeightText = mMaxHeightLineEdit->text();
  if ( maxHeightText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( u"WMSMaxHeight"_s, u"/"_s );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSMaxHeight"_s, u"/"_s, maxHeightText.toInt() );
  }

  // WMS Image quality
  int imageQualityValue = mWMSImageQualitySpinBox->value();
  if ( imageQualityValue == 0 )
  {
    QgsProject::instance()->removeEntry( u"WMSImageQuality"_s, u"/"_s );
  }
  else
  {
    QgsProject::instance()->writeEntry( u"WMSImageQuality"_s, u"/"_s, imageQualityValue );
  }

  // WMS TileBuffer
  QgsProject::instance()->writeEntry( u"WMSTileBuffer"_s, u"/"_s, mWMSTileBufferSpinBox->value() );

  int maxAtlasFeatures = mWMSMaxAtlasFeaturesSpinBox->value();
  QgsProject::instance()->writeEntry( u"WMSMaxAtlasFeatures"_s, u"/"_s, maxAtlasFeatures );

  if ( QgsProject::instance()->crs().isGeographic() && mWMSDefaultMapUnitsPerMm )
  {
    QgsProject::instance()->writeEntry( u"WMSDefaultMapUnitsPerMm"_s, u"/"_s, mWMSDefaultMapUnitsPerMm->value() );
  }
  else if ( mWMSDefaultMapUnitScale )
  {
    double defaultMapUnitsPerMm = mWMSDefaultMapUnitScale->scale() / QgsUnitTypes::fromUnitToUnitFactor( QgsProject::instance()->crs().mapUnits(), Qgis::DistanceUnit::Millimeters );
    QgsProject::instance()->writeEntry( u"WMSDefaultMapUnitsPerMm"_s, u"/"_s, defaultMapUnitsPerMm );
  }

  QgsProject::instance()->writeEntry( u"WMTSUrl"_s, u"/"_s, mWMTSUrlLineEdit->text() );
  QgsProject::instance()->writeEntry( u"WMTSMinScale"_s, u"/"_s, mWMTSMinScaleSpinBox->value() );
  bool wmtsProject = false;
  bool wmtsPngProject = false;
  bool wmtsJpegProject = false;
  QStringList wmtsGroupList;
  QStringList wmtsPngGroupList;
  QStringList wmtsJpegGroupList;
  QStringList wmtsLayerList;
  QStringList wmtsPngLayerList;
  QStringList wmtsJpegLayerList;
  const QList<QTreeWidgetItem *> wmtsLayerItems = twWmtsLayers->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 );
  for ( const QTreeWidgetItem *item : wmtsLayerItems )
  {
    if ( !item->checkState( 1 ) )
      continue;

    QString itemType = item->data( 0, Qt::UserRole ).toString();
    if ( itemType == "project"_L1 )
    {
      wmtsProject = true;
      wmtsPngProject = item->checkState( 2 );
      wmtsJpegProject = item->checkState( 3 );
    }
    else if ( itemType == "group"_L1 )
    {
      QString gName = item->data( 0, Qt::UserRole + 1 ).toString();
      wmtsGroupList << gName;
      if ( item->checkState( 2 ) )
        wmtsPngGroupList << gName;
      if ( item->checkState( 3 ) )
        wmtsJpegGroupList << gName;
    }
    else if ( itemType == "layer"_L1 )
    {
      QString lId = item->data( 0, Qt::UserRole + 1 ).toString();
      wmtsLayerList << lId;
      if ( item->checkState( 2 ) )
        wmtsPngLayerList << lId;
      if ( item->checkState( 3 ) )
        wmtsJpegLayerList << lId;
    }
  }
  QgsProject::instance()->writeEntry( u"WMTSLayers"_s, u"Project"_s, wmtsProject );
  QgsProject::instance()->writeEntry( u"WMTSPngLayers"_s, u"Project"_s, wmtsPngProject );
  QgsProject::instance()->writeEntry( u"WMTSJpegLayers"_s, u"Project"_s, wmtsJpegProject );
  QgsProject::instance()->writeEntry( u"WMTSLayers"_s, u"Group"_s, wmtsGroupList );
  QgsProject::instance()->writeEntry( u"WMTSPngLayers"_s, u"Group"_s, wmtsPngGroupList );
  QgsProject::instance()->writeEntry( u"WMTSJpegLayers"_s, u"Group"_s, wmtsJpegGroupList );
  QgsProject::instance()->writeEntry( u"WMTSLayers"_s, u"Layer"_s, wmtsLayerList );
  QgsProject::instance()->writeEntry( u"WMTSPngLayers"_s, u"Layer"_s, wmtsPngLayerList );
  QgsProject::instance()->writeEntry( u"WMTSJpegLayers"_s, u"Layer"_s, wmtsJpegLayerList );

  QStringList wmtsGridList;
  QStringList wmtsGridConfigList;
  const QList<QTreeWidgetItem *> wmtsGridItems = twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 );
  for ( const QTreeWidgetItem *item : wmtsGridItems )
  {
    if ( !item->checkState( 1 ) )
      continue;
    wmtsGridList << item->data( 0, Qt::UserRole ).toString();
    wmtsGridConfigList << u"%1,%2,%3,%4,%5"_s.arg( item->data( 0, Qt::UserRole ).toString(), item->data( 2, Qt::DisplayRole ).toString(), item->data( 3, Qt::DisplayRole ).toString(), item->data( 4, Qt::DisplayRole ).toString(), item->data( 5, Qt::DisplayRole ).toString() );
  }
  QgsProject::instance()->writeEntry( u"WMTSGrids"_s, u"CRS"_s, wmtsGridList );
  QgsProject::instance()->writeEntry( u"WMTSGrids"_s, u"Config"_s, wmtsGridConfigList );

  QgsProject::instance()->writeEntry( u"WFSUrl"_s, u"/"_s, mWFSUrlLineEdit->text() );
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
      QgsProject::instance()->writeEntry( u"WFSLayersPrecision"_s, "/" + id, sb->value() );

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
  QgsProject::instance()->writeEntry( u"WFSLayers"_s, u"/"_s, wfsLayerList );
  QgsProject::instance()->writeEntry( u"WFSTLayers"_s, u"Update"_s, wfstUpdateLayerList );
  QgsProject::instance()->writeEntry( u"WFSTLayers"_s, u"Insert"_s, wfstInsertLayerList );
  QgsProject::instance()->writeEntry( u"WFSTLayers"_s, u"Delete"_s, wfstDeleteLayerList );

  QgsProject::instance()->writeEntry( u"WCSUrl"_s, u"/"_s, mWCSUrlLineEdit->text() );
  QStringList wcsLayerList;
  wcsLayerList.reserve( twWCSLayers->rowCount() );
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
  QgsProject::instance()->writeEntry( u"WCSLayers"_s, u"/"_s, wcsLayerList );

  // Default Styles
  QgsProject::instance()->styleSettings()->setDefaultSymbol( Qgis::SymbolType::Marker, mStyleMarkerSymbol->symbol() );
  QgsProject::instance()->styleSettings()->setDefaultSymbol( Qgis::SymbolType::Line, mStyleLineSymbol->symbol() );
  QgsProject::instance()->styleSettings()->setDefaultSymbol( Qgis::SymbolType::Fill, mStyleFillSymbol->symbol() );
  QgsProject::instance()->styleSettings()->setDefaultColorRamp( mStyleColorRampSymbol->colorRamp() );
  QgsProject::instance()->styleSettings()->setDefaultTextFormat( mStyleTextFormat->textFormat() );
  QgsProject::instance()->styleSettings()->setRandomizeDefaultSymbolColor( cbxStyleRandomColors->isChecked() );
  QgsProject::instance()->styleSettings()->setDefaultSymbolOpacity( mDefaultOpacityWidget->opacity() );
  QgsProject::instance()->styleSettings()->setColorModel( mColorModel->currentData().value<Qgis::ColorModel>() );
  QgsProject::instance()->styleSettings()->setColorSpace( mColorSpace );

  {
    QStringList styleDatabasePaths;
    styleDatabasePaths.reserve( mListStyleDatabases->count() );
    for ( int i = 0; i < mListStyleDatabases->count(); ++i )
    {
      styleDatabasePaths << mListStyleDatabases->item( i )->data( Qt::UserRole ).toString();
    }
    QgsProject::instance()->styleSettings()->setStyleDatabasePaths( styleDatabasePaths );
  }

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
  QgsProject::instance()->writeEntry( u"Macros"_s, u"/pythonCode"_s, pythonMacros );

  QgsProject::instance()->relationManager()->setRelations( mRelationManagerDlg->relations() );
  QgsProject::instance()->relationManager()->setPolymorphicRelations( mRelationManagerDlg->polymorphicRelations() );

  //save variables
  QgsProject::instance()->setCustomVariables( mVariableEditor->variablesInActiveScope() );

  QgsProject::instance()->displaySettings()->setBearingFormat( mBearingFormat->clone() );
  QgsProject::instance()->displaySettings()->setGeographicCoordinateFormat( mGeographicCoordinateFormat->clone() );

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

void QgsProjectProperties::cancel()
{
  for ( QgsOptionsPageWidget *widget : std::as_const( mAdditionalProjectPropertiesWidgets ) )
  {
    widget->cancel();
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
  crslist.reserve( mWMSList->count() );
  for ( int i = 0; i < mWMSList->count(); i++ )
  {
    crslist << mWMSList->item( i )->text();
  }
  const QList<QTreeWidgetItem *> wmtsGridItems = twWmtsGrids->findItems( QString(), Qt::MatchContains | Qt::MatchRecursive, 1 );
  for ( const QTreeWidgetItem *item : wmtsGridItems )
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
    if ( crsStr == "EPSG:3857"_L1 && column != 5 )
    {
      item->setFlags( flags & ( ~Qt::ItemIsEditable ) );
    }
    else if ( crsStr == "EPSG:4326"_L1 && column != 5 )
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
    double maxScale = item->data( 4, Qt::DisplayRole ).toDouble();
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
  else if ( column == 1 && item->checkState( 1 ) && !item->checkState( 2 ) && !item->checkState( 3 ) )
  {
    item->setCheckState( 2, Qt::Checked );
    item->setCheckState( 3, Qt::Checked );
  }
  else if ( ( column == 2 && item->checkState( 2 ) ) || ( column == 3 && item->checkState( 3 ) ) )
  {
    item->setCheckState( 1, Qt::Checked );
  }
  else if ( ( column == 2 && !item->checkState( 2 ) && !item->checkState( 3 ) ) || ( column == 3 && !item->checkState( 2 ) && !item->checkState( 3 ) ) )
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

void QgsProjectProperties::updateGuiForCoordinateCrs()
{
  const Qgis::CoordinateDisplayType coordinateType = static_cast<Qgis::CoordinateDisplayType>( mCoordinateDisplayComboBox->currentData().toInt() );
  const int customIndex = mCoordinateDisplayComboBox->findData( static_cast<int>( Qgis::CoordinateDisplayType::CustomCrs ) );
  if ( coordinateType == Qgis::CoordinateDisplayType::CustomCrs )
  {
    const Qgis::DistanceUnit units = mCoordinateCrs->crs().mapUnits();
    mCoordinateDisplayComboBox->setItemText( customIndex, tr( "Custom Projection Units (%1)" ).arg( QgsUnitTypes::toString( units ) ) );
  }
  else
  {
    mCoordinateDisplayComboBox->setItemText( customIndex, tr( "Custom Projection Units" ) );
  }
}

void QgsProjectProperties::updateGuiForCoordinateType()
{
  const Qgis::CoordinateDisplayType coordinateType = static_cast<Qgis::CoordinateDisplayType>( mCoordinateDisplayComboBox->currentData().toInt() );
  switch ( coordinateType )
  {
    case Qgis::CoordinateDisplayType::MapCrs:
      mCoordinateCrs->setEnabled( false );
      mCoordinateCrs->setCrs( mCrs );
      break;

    case Qgis::CoordinateDisplayType::MapGeographic:
      mCoordinateCrs->setEnabled( false );
      mCoordinateCrs->setCrs( !mCrs.isGeographic() ? mCrs.toGeographicCrs() : mCrs );
      break;

    case Qgis::CoordinateDisplayType::CustomCrs:
      mCoordinateCrs->setEnabled( true );
      mCoordinateCrs->setCrs( QgsProject::instance()->displaySettings()->coordinateCustomCrs() );
      break;
  }

  updateGuiForCoordinateCrs();
}

void QgsProjectProperties::updateGuiForMapUnits()
{
  if ( !mCrs.isValid() )
  {
    // no projection set - disable everything!
    int idx = mDistanceUnitsCombo->findData( static_cast<int>( Qgis::DistanceUnit::Unknown ) );
    if ( idx >= 0 )
    {
      mDistanceUnitsCombo->setItemText( idx, tr( "Unknown Units" ) );
      mDistanceUnitsCombo->setCurrentIndex( idx );
    }
    idx = mAreaUnitsCombo->findData( static_cast<int>( Qgis::AreaUnit::Unknown ) );
    if ( idx >= 0 )
    {
      mAreaUnitsCombo->setItemText( idx, tr( "Unknown Units" ) );
      mAreaUnitsCombo->setCurrentIndex( idx );
    }
    idx = mCoordinateDisplayComboBox->findData( static_cast<int>( Qgis::CoordinateDisplayType::MapCrs ) );
    if ( idx >= 0 )
    {
      mCoordinateDisplayComboBox->setItemText( idx, tr( "Unknown Units" ) );
      mCoordinateDisplayComboBox->setCurrentIndex( idx );
    }
    mDistanceUnitsCombo->setEnabled( false );
    mAreaUnitsCombo->setEnabled( false );
    mCoordinateDisplayComboBox->setEnabled( false );
  }
  else
  {
    Qgis::DistanceUnit units = mCrs.mapUnits();

    mDistanceUnitsCombo->setEnabled( true );
    mAreaUnitsCombo->setEnabled( true );
    mCoordinateDisplayComboBox->setEnabled( true );

    //make sure map units option is shown in coordinate display combo
    int idx = mCoordinateDisplayComboBox->findData( static_cast<int>( Qgis::CoordinateDisplayType::MapCrs ) );
    QString mapUnitString = tr( "Map Units (%1)" ).arg( QgsUnitTypes::toString( units ) );
    mCoordinateDisplayComboBox->setItemText( idx, mapUnitString );

    //also update unit combo boxes
    idx = mDistanceUnitsCombo->findData( static_cast<int>( Qgis::DistanceUnit::Unknown ) );
    if ( idx >= 0 )
    {
      QString mapUnitString = tr( "Map Units (%1)" ).arg( QgsUnitTypes::toString( units ) );
      mDistanceUnitsCombo->setItemText( idx, mapUnitString );
    }
    idx = mAreaUnitsCombo->findData( static_cast<int>( Qgis::AreaUnit::Unknown ) );
    if ( idx >= 0 )
    {
      QString mapUnitString = tr( "Map Units (%1)" ).arg( QgsUnitTypes::toString( QgsUnitTypes::distanceToAreaUnit( units ) ) );
      mAreaUnitsCombo->setItemText( idx, mapUnitString );
    }
  }
  updateGuiForCoordinateType();
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

  mExtentWidget->setOutputCrs( crs );
  mAdvertisedExtentServer->setOutputCrs( crs );
}

void QgsProjectProperties::wmsExtent_toggled()
{
  if ( grpWMSExt->isChecked() )
  {
    if ( mAdvertisedExtentServer->outputExtent().isEmpty() )
    {
      mAdvertisedExtentServer->setOutputExtentFromCurrent();
    }
  }
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
    if ( QMessageBox::question( this, tr( "Coordinate System Restrictions" ), tr( "The current selection of coordinate systems will be lost.\nProceed?" ) ) == QMessageBox::No )
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
  layoutTitles.reserve( projectLayouts.size() );
  for ( const QgsPrintLayout *layout : projectLayouts )
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
      errors += "<b>"_L1 % QgsProjectServerValidator::displayValidationError( result.error ) % " :</b> "_L1;
      errors += result.identifier.toString();
    }
  }
  else
  {
    errors += tr( "Project is valid." );
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  myStyle.append( u"body { margin: 10px; }\n "_s );
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
    QListWidgetItem *newItem = addScaleToScaleList( myScale );
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
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load scales" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
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
  bool ok;
  for ( const QString &scale : constMyScales )
  {
    // Parse scale string
    const QStringList parts { scale.split( ':' ) };
    if ( parts.count() == 2 )
    {
      const double scaleDenominator { parts.at( 1 ).toDouble( &ok ) };
      if ( ok )
      {
        addScaleToScaleList( scaleDenominator );
      }
    }
  }
}

void QgsProjectProperties::pbnExportScales_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save scales" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  // return dialog focus on Mac
  activateWindow();
  raise();
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  if ( !fileName.endsWith( ".xml"_L1, Qt::CaseInsensitive ) )
  {
    fileName += ".xml"_L1;
  }

  QStringList myScales;
  myScales.reserve( lstScales->count() );
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( u"1:%1"_s.arg( lstScales->item( i )->data( Qt::UserRole ).toDouble() ) );
  }

  QString msg;
  if ( !QgsScaleUtils::saveScaleList( fileName, myScales, msg ) )
  {
    QgsDebugError( msg );
  }
}

void QgsProjectProperties::resetPythonMacros()
{
  grpPythonMacros->setChecked( false );
  ptePythonMacros->setText( "def openProject():\n    pass\n\n"
                            "def saveProject():\n    pass\n\n"
                            "def closeProject():\n    pass\n" );
}

void QgsProjectProperties::populateWmtsTree( const QgsLayerTreeGroup *treeGroup, QgsTreeWidgetItem *treeItem )
{
  const QList<QgsLayerTreeNode *> children = treeGroup->children();
  for ( QgsLayerTreeNode *treeNode : children )
  {
    QgsTreeWidgetItem *childItem = nullptr;
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      QString gName = treeGroupChild->name();

      childItem = new QgsTreeWidgetItem( QStringList() << gName );
      childItem->setFlags( childItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable );

      childItem->setData( 0, Qt::UserRole, u"group"_s );
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

      childItem->setData( 0, Qt::UserRole, u"layer"_s );
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
  if ( crsStr == "EPSG:3857"_L1 )
  {
    gridItem->setData( 2, Qt::DisplayRole, 20037508.3427892480 );
    gridItem->setData( 3, Qt::DisplayRole, -20037508.3427892480 );
    scaleDenominator = 559082264.0287179;
  }
  else if ( crsStr == "EPSG:4326"_L1 )
  {
    gridItem->setData( 2, Qt::DisplayRole, 90.0 );
    gridItem->setData( 3, Qt::DisplayRole, -180.0 );
    scaleDenominator = 279541132.0143588675418869;
  }
  else
  {
    // calculate top, left and scale based on CRS bounds
    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsStr );
    QgsCoordinateTransform crsTransform( QgsCoordinateReferenceSystem::fromOgcWmsCrs( Qgis::geographicCrsAuthId() ), crs, QgsProject::instance() );
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
        scaleDenominator = std::ceil( colRes * QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters ) / POINTS_TO_M );
      }
      else
      {
        scaleDenominator = std::ceil( rowRes * QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters ) / POINTS_TO_M );
      }
      // Calculate resolution based on scale denominator
      double res = POINTS_TO_M * scaleDenominator / QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters );
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

  myItem.acronym = Qgis::geoNone();
  myItem.description = tr( GEO_NONE_DESC );
  myItem.semiMajor = 0.0;
  myItem.semiMinor = 0.0;
  mEllipsoidList.append( myItem );

  myItem.acronym = u"PARAMETER:6370997:6370997"_s;
  myItem.description = tr( "Custom" );
  myItem.semiMajor = 6370997.0;
  myItem.semiMinor = 6370997.0;
  mEllipsoidList.append( myItem );

  const auto definitions { QgsEllipsoidUtils::definitions() };
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
    QgsDebugMsgLevel( u"Saving major/minor"_s, 4 );
    mEllipsoidList[mEllipsoidIndex].semiMajor = QgsDoubleValidator::toDouble( leSemiMajor->text() );
    mEllipsoidList[mEllipsoidIndex].semiMinor = QgsDoubleValidator::toDouble( leSemiMinor->text() );
  }

  mEllipsoidIndex = newIndex;
  leSemiMajor->setEnabled( false );
  leSemiMinor->setEnabled( false );
  leSemiMajor->clear();
  leSemiMinor->clear();

  cmbEllipsoid->setEnabled( mCrs.isValid() );
  cmbEllipsoid->setToolTip( QString() );
  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( "PARAMETER:"_L1 ) )
  {
    leSemiMajor->setEnabled( true );
    leSemiMinor->setEnabled( true );
  }
  else
  {
    leSemiMajor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Custom" ) ) );
    leSemiMinor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Custom" ) ) );
  }
  if ( mEllipsoidList[mEllipsoidIndex].acronym != Qgis::geoNone() )
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
  if ( ellipsoidAcronym.startsWith( "PARAMETER"_L1 ) )
  {
    // Update parameters if present.
    const QStringList mySplitEllipsoid = ellipsoidAcronym.split( ':' );
    for ( int i = 0; i < mEllipsoidList.length(); i++ )
    {
      if ( mEllipsoidList.at( i ).acronym.startsWith( "PARAMETER"_L1, Qt::CaseInsensitive ) )
      {
        index = i;
        mEllipsoidList[index].semiMajor = mySplitEllipsoid[1].toDouble();
        mEllipsoidList[index].semiMinor = mySplitEllipsoid[2].toDouble();
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
  const Qgis::ColorModel colorModel = mColorModel->currentData().value<Qgis::ColorModel>();
  const QColor defaultColor = colorModel == Qgis::ColorModel::Cmyk ? QColor::fromCmykF( 0., 1., 1., 0. ) : QColor::fromRgbF( 1., 0., 0. );

  QColor newColor = QgsColorDialog::getColor( defaultColor, this->parentWidget(), tr( "Select Color" ), true );
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

void QgsProjectProperties::addStyleDatabase()
{
  addStyleDatabasePrivate( false );
}

void QgsProjectProperties::newStyleDatabase()
{
  addStyleDatabasePrivate( true );
}

void QgsProjectProperties::addStyleDatabasePrivate( bool createNew )
{
  QString initialFolder = QgsStyleManagerDialog::settingLastStyleDatabaseFolder->value();
  if ( initialFolder.isEmpty() )
    initialFolder = QDir::homePath();

  QString databasePath = createNew
                           ? QFileDialog::getSaveFileName(
                               this,
                               tr( "Create Style Database" ),
                               initialFolder,
                               tr( "Style databases" ) + " (*.db)"
                             )
                           : QFileDialog::getOpenFileName(
                               this,
                               tr( "Add Style Database" ),
                               initialFolder,
                               tr( "Style databases" ) + " (*.db *.xml)"
                             );
  // return dialog focus on Mac
  activateWindow();
  raise();
  if ( !databasePath.isEmpty() )
  {
    QgsStyleManagerDialog::settingLastStyleDatabaseFolder->setValue( QFileInfo( databasePath ).path() );

    if ( createNew )
    {
      databasePath = QgsFileUtils::ensureFileNameHasExtension( databasePath, { u"db"_s } );
      if ( QFile::exists( databasePath ) )
      {
        QFile::remove( databasePath );
      }
      QgsStyle s;
      if ( !s.createDatabase( databasePath ) )
      {
        QMessageBox::warning( this, tr( "Create Style Database" ), tr( "The style database could not be created" ) );
        return;
      }
    }

    QListWidgetItem *newItem = new QListWidgetItem( mListStyleDatabases );
    newItem->setText( QDir::toNativeSeparators( databasePath ) );
    newItem->setData( Qt::UserRole, databasePath );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mListStyleDatabases->addItem( newItem );
    mListStyleDatabases->setCurrentItem( newItem );
  }
}

void QgsProjectProperties::removeStyleDatabase()
{
  int currentRow = mListStyleDatabases->currentRow();
  delete mListStyleDatabases->takeItem( currentRow );
}

#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )

void QgsProjectProperties::addIccProfile()
{
  const QString iccProfileFilePath = QFileDialog::getOpenFileName(
    this,
    tr( "Load ICC Profile" ),
    QDir::homePath(),
    tr( "ICC Profile" ) + u" (*.icc)"_s
  );

  addIccProfile( iccProfileFilePath );
}

void QgsProjectProperties::addIccProfile( const QString &iccProfileFilePath )
{
  if ( iccProfileFilePath.isEmpty() )
    return;

  QString errorMsg;
  QColorSpace colorSpace = QgsColorUtils::iccProfile( iccProfileFilePath, errorMsg );
  if ( !colorSpace.isValid() )
  {
    QMessageBox::warning( this, tr( "Load ICC Profile" ), errorMsg );
    return;
  }

  mColorSpace = colorSpace;
  updateColorSpaceWidgets();
}

void QgsProjectProperties::removeIccProfile()
{
  mColorSpace = QColorSpace();
  updateColorSpaceWidgets();
}

void QgsProjectProperties::saveIccProfile()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save ICC Profile" ), QDir::homePath(), tr( "ICC profile files (*.icc *.ICC)" ) );

  if ( fileName.isEmpty() )
    return;

  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { u"icc"_s } );
  const QString error = QgsColorUtils::saveIccProfile( mColorSpace, fileName );
  if ( !error.isEmpty() )
  {
    QMessageBox::warning( this, tr( "Save ICC profile" ), error );
  }
}


void QgsProjectProperties::updateColorSpaceWidgets()
{
  mColorSpaceName->setText( mColorSpace.isValid() ? mColorSpace.description() : tr( "<i>None</i>" ) );
  mRemoveIccProfile->setEnabled( mColorSpace.isValid() );
  mSaveIccProfile->setEnabled( mColorSpace.isValid() );

  // force color model index according to color space one
  if ( mColorSpace.isValid() )
  {
    const Qgis::ColorModel colorModel = QgsColorUtils::toColorModel( mColorSpace.colorModel() );
    mColorModel->setCurrentIndex( mColorModel->findData( QVariant::fromValue( colorModel ) ) );
  }

  mColorModel->setEnabled( !mColorSpace.isValid() );
}

#endif

QListWidgetItem *QgsProjectProperties::addScaleToScaleList( const double newScaleDenominator )
{
  // TODO QGIS3: Rework the scale list widget to be a reusable piece of code, see PR #2558
  QListWidgetItem *newItem = new QListWidgetItem( u"1:%1"_s.arg( QLocale().toString( newScaleDenominator, 'f', 0 ) ) );
  newItem->setData( Qt::UserRole, newScaleDenominator );
  addScaleToScaleList( newItem );
  return newItem;
}

void QgsProjectProperties::addScaleToScaleList( QListWidgetItem *newItem )
{
  // If the new scale already exists, delete it.
  QListWidgetItem *duplicateItem = lstScales->findItems( newItem->text(), Qt::MatchExactly ).value( 0 );
  delete duplicateItem;

  const int newDenominator = newItem->data( Qt::UserRole ).toInt();
  int i;
  for ( i = 0; i < lstScales->count(); i++ )
  {
    const int denominator = lstScales->item( i )->data( Qt::UserRole ).toInt();
    if ( newDenominator > denominator )
      break;
  }

  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  lstScales->insertItem( i, newItem );
}

void QgsProjectProperties::scaleItemChanged( QListWidgetItem *changedScaleItem )
{
  // Check if the new value is valid, restore the old value if not.
  const QStringList parts { changedScaleItem->text().split( ':' ) };
  bool valid { parts.count() == 2 };
  double newDenominator { -1 }; // invalid

  if ( valid )
  {
    newDenominator = QgsDoubleValidator::toDouble( parts.at( 1 ), &valid );
  }

  if ( valid )
  {
    changedScaleItem->setText( u"1:%1"_s.arg( QLocale().toString( newDenominator ) ) );
    changedScaleItem->setData( Qt::UserRole, newDenominator );
  }
  else
  {
    QMessageBox::warning( this, tr( "Set Scale" ), tr( "The text you entered is not a valid scale." ) );
    changedScaleItem->setText( u"1:%1"_s.arg( QLocale().toString( changedScaleItem->data( Qt::UserRole ).toDouble() ) ) );
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
  QString link = u"introduction/qgis_configuration.html#project-properties"_s;

  if ( activeTab == mTabRelations )
  {
    link = u"working_with_vector/attribute_table.html#creating-one-or-many-to-many-relations"_s;
  }
  else if ( activeTab == mTab_Variables )
  {
    link = u"introduction/general_tools.html#variables"_s;
  }
  else if ( activeTab == mProjOptsCRS )
  {
    link = u"working_with_projections/working_with_projections.html"_s;
  }
  else if ( activeTab == mProjOptsOWS )
  {
    link = u"working_with_ogc/server/getting_started.html#prepare-a-project-to-serve"_s;
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
  const QMap<QString, QString> pageNames = QgisApp::instance()->projectPropertiesPagesMap();
  Q_ASSERT_X( pageNames.count() == mOptionsListWidget->count(), "QgsProjectProperties::checkPageWidgetNameMap()", "QgisApp::projectPropertiesPagesMap() is outdated, contains too many entries" );
  for ( int idx = 0; idx < mOptionsListWidget->count(); ++idx )
  {
    QWidget *currentPage = mOptionsStackedWidget->widget( idx );
    QListWidgetItem *item = mOptionsListWidget->item( idx );
    const QString title = item->text();
    const QString name = currentPage->objectName();
    Q_ASSERT_X( pageNames.contains( title ), "QgsProjectProperties::checkPageWidgetNameMap()", u"QgisApp::projectPropertiesPagesMap() is outdated, please update. Missing %1"_s.arg( title ).toLocal8Bit().constData() );
    Q_ASSERT_X( pageNames.value( title ) == name, "QgsProjectProperties::checkPageWidgetNameMap()", u"QgisApp::projectPropertiesPagesMap() is outdated, please update. %1 should be %2 not %3"_s.arg( title, name, pageNames.value( title ) ).toLocal8Bit().constData() );
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
                                                                            "- you will be redirected to this new project (e.g. aproject_de.qgs)." )
                                                                          .arg( l ) );
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

void QgsProjectProperties::customizeGeographicCoordinateFormat()
{
  QgsGeographicCoordinateNumericFormatDialog dlg( mGeographicCoordinateFormat.get(), true, this );
  dlg.setWindowTitle( tr( "Coordinate Format" ) );
  if ( dlg.exec() )
  {
    mGeographicCoordinateFormat.reset( dlg.format() );
  }
}
