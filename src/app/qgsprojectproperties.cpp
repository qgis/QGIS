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
#include "qgscoordinatetransform.h"
#include "qgsdatumtransformtablewidget.h"
#include "qgslayoutmanager.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
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
#include "qgslayertree.h"

#include "qgsmessagelog.h"

//qt includes
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>  // Qt 4.4
#include <QMessageBox>
#include <QDesktopServices>

const char *QgsProjectProperties::GEO_NONE_DESC = QT_TRANSLATE_NOOP( "QgsOptions", "None / Planimetric" );

//stdc++ includes

QgsProjectProperties::QgsProjectProperties( QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "ProjectProperties" ), parent, fl )
  , mMapCanvas( mapCanvas )
  , mEllipsoidIndex( 0 )
{
  setupUi( this );
  connect( pbnAddScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnAddScale_clicked );
  connect( pbnRemoveScale, &QToolButton::clicked, this, &QgsProjectProperties::pbnRemoveScale_clicked );
  connect( pbnImportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnImportScales_clicked );
  connect( pbnExportScales, &QToolButton::clicked, this, &QgsProjectProperties::pbnExportScales_clicked );
  connect( pbnWMSExtCanvas, &QPushButton::clicked, this, &QgsProjectProperties::pbnWMSExtCanvas_clicked );
  connect( pbnWMSAddSRS, &QToolButton::clicked, this, &QgsProjectProperties::pbnWMSAddSRS_clicked );
  connect( pbnWMSRemoveSRS, &QToolButton::clicked, this, &QgsProjectProperties::pbnWMSRemoveSRS_clicked );
  connect( pbnWMSSetUsedSRS, &QPushButton::clicked, this, &QgsProjectProperties::pbnWMSSetUsedSRS_clicked );
  connect( mAddWMSComposerButton, &QToolButton::clicked, this, &QgsProjectProperties::mAddWMSComposerButton_clicked );
  connect( mRemoveWMSComposerButton, &QToolButton::clicked, this, &QgsProjectProperties::mRemoveWMSComposerButton_clicked );
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

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mCoordinateDisplayComboBox->addItem( tr( "Map units" ), MapUnits );
  mCoordinateDisplayComboBox->addItem( tr( "Decimal degrees" ), DecimalDegrees );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, minutes" ), DegreesMinutes );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, minutes, seconds" ), DegreesMinutesSeconds );

  mDistanceUnitsCombo->addItem( tr( "Meters" ), QgsUnitTypes::DistanceMeters );
  mDistanceUnitsCombo->addItem( tr( "Kilometers" ), QgsUnitTypes::DistanceKilometers );
  mDistanceUnitsCombo->addItem( tr( "Feet" ), QgsUnitTypes::DistanceFeet );
  mDistanceUnitsCombo->addItem( tr( "Yards" ), QgsUnitTypes::DistanceYards );
  mDistanceUnitsCombo->addItem( tr( "Miles" ), QgsUnitTypes::DistanceMiles );
  mDistanceUnitsCombo->addItem( tr( "Nautical miles" ), QgsUnitTypes::DistanceNauticalMiles );
  mDistanceUnitsCombo->addItem( tr( "Degrees" ), QgsUnitTypes::DistanceDegrees );
  mDistanceUnitsCombo->addItem( tr( "Map units" ), QgsUnitTypes::DistanceUnknownUnit );

  mAreaUnitsCombo->addItem( tr( "Square meters" ), QgsUnitTypes::AreaSquareMeters );
  mAreaUnitsCombo->addItem( tr( "Square kilometers" ), QgsUnitTypes::AreaSquareKilometers );
  mAreaUnitsCombo->addItem( tr( "Square feet" ), QgsUnitTypes::AreaSquareFeet );
  mAreaUnitsCombo->addItem( tr( "Square yards" ), QgsUnitTypes::AreaSquareYards );
  mAreaUnitsCombo->addItem( tr( "Square miles" ), QgsUnitTypes::AreaSquareMiles );
  mAreaUnitsCombo->addItem( tr( "Hectares" ), QgsUnitTypes::AreaHectares );
  mAreaUnitsCombo->addItem( tr( "Acres" ), QgsUnitTypes::AreaAcres );
  mAreaUnitsCombo->addItem( tr( "Square nautical miles" ), QgsUnitTypes::AreaSquareNauticalMiles );
  mAreaUnitsCombo->addItem( tr( "Square degrees" ), QgsUnitTypes::AreaSquareDegrees );
  mAreaUnitsCombo->addItem( tr( "Map units" ), QgsUnitTypes::AreaUnknownUnit );

  projectionSelector->setShowNoProjection( true );

  connect( buttonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &QgsProjectProperties::apply );
  connect( this, &QDialog::accepted, this, &QgsProjectProperties::apply );
  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::crsSelected, this, &QgsProjectProperties::srIdUpdated );
  connect( projectionSelector, &QgsProjectionSelectionTreeWidget::initialized, this, &QgsProjectProperties::projectionSelectorInitialized );

  connect( cmbEllipsoid, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsProjectProperties::updateEllipsoidUI );

  connect( radAutomatic, &QAbstractButton::toggled, spinBoxDP, &QWidget::setDisabled );
  connect( radAutomatic, &QAbstractButton::toggled, labelDP, &QWidget::setDisabled );
  connect( radManual, &QAbstractButton::toggled, spinBoxDP, &QWidget::setEnabled );
  connect( radManual, &QAbstractButton::toggled, labelDP, &QWidget::setEnabled );

  QgsSettings settings;

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  updateGuiForMapUnits( QgsProject::instance()->crs().mapUnits() );
  projectionSelector->setCrs( QgsProject::instance()->crs() );

  // Datum transforms
  QgsCoordinateTransformContext context = QgsProject::instance()->transformContext();
  mDatumTransformTableWidget->setTransformContext( context );

  bool show = settings.value( QStringLiteral( "/Projections/showDatumTransformDialog" ), false ).toBool();
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

  mMapTileRenderingCheckBox->setChecked( mMapCanvas->mapSettings().testFlag( QgsMapSettings::RenderMapTile ) );

  // see end of constructor for updating of projection selector

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  Q_FOREACH ( QgsVectorLayer *layer, QgsProject::instance()->layers<QgsVectorLayer *>() )
  {
    if ( layer->isEditable() )
    {
      mAutoTransaction->setEnabled( false );
      mAutoTransaction->setToolTip( tr( "Layers are in edit mode. Stop edit mode on all layers to toggle transactional editing." ) );
    }
  }

  mAutoTransaction->setChecked( QgsProject::instance()->autoTransaction() );
  title( QgsProject::instance()->title() );
  mProjectFileLineEdit->setText( QDir::toNativeSeparators( QgsProject::instance()->fileName() ) );

  connect( mButtonOpenProjectFolder, &QToolButton::clicked, this, [ = ]
  {
    QFileInfo fi( QgsProject::instance()->fileName() );
    QString folder = fi.path();
    QDesktopServices::openUrl( QUrl::fromLocalFile( folder ) );
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

  cbxAbsolutePath->setCurrentIndex( QgsProject::instance()->readBoolEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), true ) ? 0 : 1 );

  // populate combo box with ellipsoids
  // selection of the ellipsoid from settings is defferred to a later point, because it would
  // be overridden in the meanwhile by the projection selector
  populateEllipsoidList();

  if ( !QgsProject::instance()->crs().isValid() )
  {
    cmbEllipsoid->setCurrentIndex( 0 );
    cmbEllipsoid->setEnabled( false );
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
  int myRedInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), 255 );
  int myGreenInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), 255 );
  int myBlueInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), 0 );
  int myAlphaInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );
  myRedInt = settings.value( QStringLiteral( "qgis/default_selection_color_red" ), 255 ).toInt();
  myGreenInt = settings.value( QStringLiteral( "qgis/default_selection_color_green" ), 255 ).toInt();
  myBlueInt = settings.value( QStringLiteral( "qgis/default_selection_color_blue" ), 0 ).toInt();
  myAlphaInt = settings.value( QStringLiteral( "qgis/default_selection_color_alpha" ), 255 ).toInt();
  QColor defaultSelectionColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );
  pbnSelectionColor->setContext( QStringLiteral( "gui" ) );
  pbnSelectionColor->setColor( myColor );
  pbnSelectionColor->setDefaultColor( defaultSelectionColor );
  pbnSelectionColor->setColorDialogTitle( tr( "Selection Color" ) );
  pbnSelectionColor->setAllowOpacity( true );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), 255 );
  myGreenInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), 255 );
  myBlueInt = QgsProject::instance()->readNumEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), 255 );
  myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  myRedInt = settings.value( QStringLiteral( "qgis/default_canvas_color_red" ), 255 ).toInt();
  myGreenInt = settings.value( QStringLiteral( "qgis/default_canvas_color_green" ), 255 ).toInt();
  myBlueInt = settings.value( QStringLiteral( "qgis/default_canvas_color_blue" ), 255 ).toInt();
  QColor defaultCanvasColor = QColor( myRedInt, myGreenInt, myBlueInt );

  pbnCanvasColor->setContext( QStringLiteral( "gui" ) );
  pbnCanvasColor->setColor( myColor );
  pbnCanvasColor->setDefaultColor( defaultCanvasColor );

  //get project scales
  QStringList myScales = QgsProject::instance()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) );
  if ( !myScales.isEmpty() )
  {
    Q_FOREACH ( const QString &scale, myScales )
    {
      addScaleToScaleList( scale );
    }
  }
  connect( lstScales, &QListWidget::itemChanged, this, &QgsProjectProperties::scaleItemChanged );

  grpProjectScales->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );

  QgsMapLayer *currentLayer = nullptr;

  QStringList noIdentifyLayerIdList = QgsProject::instance()->nonIdentifiableLayers();

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();

  twIdentifyLayers->setColumnCount( 4 );
  twIdentifyLayers->horizontalHeader()->setVisible( true );
  twIdentifyLayers->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Layer" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Type" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Identifiable" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Read Only" ) ) );
  twIdentifyLayers->setRowCount( mapLayers.size() );
  twIdentifyLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  int i = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();

    QgsTableWidgetItem *twi = new QgsTableWidgetItem( QString::number( i ) );
    twIdentifyLayers->setVerticalHeaderItem( i, twi );

    twi = new QgsTableWidgetItem( currentLayer->name() );
    twi->setData( Qt::UserRole, it.key() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twIdentifyLayers->setItem( i, 0, twi );

    QString type;
    if ( currentLayer->type() == QgsMapLayer::VectorLayer )
    {
      type = tr( "Vector" );
    }
    else if ( currentLayer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( currentLayer );

      if ( rl && rl->providerType() == QLatin1String( "wms" ) )
      {
        type = tr( "WMS" );
      }
      else
      {
        type = tr( "Raster" );
      }
    }

    twi = new QgsTableWidgetItem( type );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twIdentifyLayers->setItem( i, 1, twi );

    twi = new QgsTableWidgetItem();
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twi->setFlags( twi->flags() | Qt::ItemIsUserCheckable );
    twi->setCheckState( noIdentifyLayerIdList.contains( currentLayer->id() ) ? Qt::Unchecked : Qt::Checked );
    twi->setSortRole( Qt::CheckStateRole );
    twIdentifyLayers->setItem( i, 2, twi );

    twi = new QgsTableWidgetItem();
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twi->setFlags( twi->flags() | Qt::ItemIsUserCheckable );
    twi->setCheckState( currentLayer->readOnly() ? Qt::Checked : Qt::Unchecked );
    twi->setSortRole( Qt::CheckStateRole );
    twIdentifyLayers->setItem( i, 3, twi );
  }

  grpOWSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ) ) );
  mWMSName->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSRootName" ), QStringLiteral( "/" ) ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  mWMSKeywordList->setText( QgsProject::instance()->readListEntry( QStringLiteral( "WMSKeywordList" ), QStringLiteral( "/" ) ).join( QStringLiteral( ", " ) ) );

  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
  mWMSName->setValidator( shortNameValidator );

  // WMS Contact Position
  QString contactPositionText = QgsProject::instance()->readEntry( QStringLiteral( "WMSContactPosition" ), QStringLiteral( "/" ), QLatin1String( "" ) );
  mWMSContactPositionCb->addItem( QLatin1String( "" ) );
  mWMSContactPositionCb->addItem( tr( "Custodian" ), "custodian" );
  mWMSContactPositionCb->addItem( tr( "Owner" ), "owner" );
  mWMSContactPositionCb->addItem( tr( "User" ), "user" );
  mWMSContactPositionCb->addItem( tr( "Distributor" ), "distributor" );
  mWMSContactPositionCb->addItem( tr( "Originator" ), "originator" );
  mWMSContactPositionCb->addItem( tr( "Point of contact" ), "pointOfContact" );
  mWMSContactPositionCb->addItem( tr( "Principal investigator" ), "principalInvestigator" );
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
  QString feesText = QgsProject::instance()->readEntry( QStringLiteral( "WMSFees" ), QStringLiteral( "/" ), QLatin1String( "" ) );
  mWMSFeesCb->addItem( tr( "Conditions unknown" ), "conditions unknown" );
  mWMSFeesCb->addItem( tr( "No conditions apply" ), "no conditions apply" );
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
  QString accessConstraintsText = QgsProject::instance()->readEntry( QStringLiteral( "WMSAccessConstraints" ), QStringLiteral( "/" ), QLatin1String( "" ) );
  mWMSAccessConstraintsCb->addItem( tr( "None" ), "None" );
  mWMSAccessConstraintsCb->addItem( tr( "Copyright" ), "copyright" );
  mWMSAccessConstraintsCb->addItem( tr( "Patent" ), "patent" );
  mWMSAccessConstraintsCb->addItem( tr( "Patent pending" ), "patentPending" );
  mWMSAccessConstraintsCb->addItem( tr( "Trademark" ), "trademark" );
  mWMSAccessConstraintsCb->addItem( tr( "License" ), "license" );
  mWMSAccessConstraintsCb->addItem( tr( "Intellectual property rights" ), "intellectualPropertyRights" );
  mWMSAccessConstraintsCb->addItem( tr( "Restricted" ), "restricted" );
  mWMSAccessConstraintsCb->addItem( tr( "Other restrictions" ), "otherRestrictions" );
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
      QLocale::system().nativeLanguageName()
    )
  );

  bool addWMSInspire = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/activated" ) );
  if ( addWMSInspire )
  {
    mWMSInspire->setChecked( addWMSInspire );
    QString inspireLanguage = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/language" ), QLatin1String( "" ) );
    int inspireLanguageIndex = mWMSInspireLanguage->findData( inspireLanguage );
    mWMSInspireLanguage->setCurrentIndex( inspireLanguageIndex );

    QString inspireMetadataUrl = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrl" ), QLatin1String( "" ) );
    if ( !inspireMetadataUrl.isEmpty() )
    {
      mWMSInspireScenario1->setChecked( true );
      mWMSInspireMetadataUrl->setText( inspireMetadataUrl );
      mWMSInspireMetadataUrlType->setCurrentIndex(
        mWMSInspireMetadataUrlType->findText(
          QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataUrlType" ), QLatin1String( "" ) )
        )
      );
    }
    else
    {
      QString inspireTemporalReference = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/temporalReference" ), QLatin1String( "" ) );
      if ( !inspireTemporalReference.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireTemporalReference->setDate( QDate::fromString( inspireTemporalReference, QStringLiteral( "yyyy-MM-dd" ) ) );
      }
      QString inspireMetadataDate = QgsProject::instance()->readEntry( QStringLiteral( "WMSInspire" ), QStringLiteral( "/metadataDate" ), QLatin1String( "" ) );
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
      Q_FOREACH ( const QString &value, values )
      {
        list << QStringLiteral( "EPSG:%1" ).arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  //composer restriction for WMS
  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSRestrictedComposers" ), QStringLiteral( "/" ), QStringList(), &ok );
  mWMSComposerGroupBox->setChecked( ok );
  if ( ok )
  {
    mComposerListWidget->addItems( values );
  }

  //layer restriction for WMS
  values = QgsProject::instance()->readListEntry( QStringLiteral( "WMSRestrictedLayers" ), QStringLiteral( "/" ), QStringList(), &ok );
  mLayerRestrictionsGroupBox->setChecked( ok );
  if ( ok )
  {
    mLayerRestrictionsListWidget->addItems( values );
  }

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSAddWktGeometry" ), QStringLiteral( "/" ) );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  bool requestDefinedSources = QgsProject::instance()->readBoolEntry( QStringLiteral( "WMSRequestDefinedDataSources" ), QStringLiteral( "/" ), false );
  mAllowRequestDefinedDataSourcesBox->setChecked( requestDefinedSources );

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

  mWFSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WFSUrl" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSLayers" ), QStringLiteral( "/" ) );
  QStringList wfstUpdateLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Update" ) );
  QStringList wfstInsertLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Insert" ) );
  QStringList wfstDeleteLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WFSTLayers" ), QStringLiteral( "Delete" ) );

  QSignalMapper *smPublied = new QSignalMapper( this );
  connect( smPublied, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSPubliedStateChanged( int ) ) );

  twWFSLayers->setColumnCount( 6 );
  twWFSLayers->horizontalHeader()->setVisible( true );
  twWFSLayers->setRowCount( mapLayers.size() );

  i = 0;
  int j = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == QgsMapLayer::VectorLayer )
    {

      QTableWidgetItem *twi = new QTableWidgetItem( QString::number( j ) );
      twWFSLayers->setVerticalHeaderItem( j, twi );

      twi = new QTableWidgetItem( currentLayer->name() );
      twi->setData( Qt::UserRole, it.key() );
      twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
      twWFSLayers->setItem( j, 0, twi );

      QCheckBox *cbp = new QCheckBox();
      cbp->setChecked( wfsLayerIdList.contains( currentLayer->id() ) );
      twWFSLayers->setCellWidget( j, 1, cbp );

      smPublied->setMapping( cbp, j );
      connect( cbp, SIGNAL( stateChanged( int ) ), smPublied, SLOT( map() ) );

      QSpinBox *psb = new QSpinBox();
      psb->setValue( QgsProject::instance()->readNumEntry( QStringLiteral( "WFSLayersPrecision" ), "/" + currentLayer->id(), 8 ) );
      twWFSLayers->setCellWidget( j, 2, psb );

      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      QgsVectorDataProvider *provider = vlayer->dataProvider();
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
  twWFSLayers->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );

  mWCSUrlLineEdit->setText( QgsProject::instance()->readEntry( QStringLiteral( "WCSUrl" ), QStringLiteral( "/" ), QLatin1String( "" ) ) );
  QStringList wcsLayerIdList = QgsProject::instance()->readListEntry( QStringLiteral( "WCSLayers" ), QStringLiteral( "/" ) );

  QSignalMapper *smWcsPublied = new QSignalMapper( this );
  connect( smWcsPublied, SIGNAL( mapped( int ) ), this, SLOT( cbxWCSPubliedStateChanged( int ) ) );

  twWCSLayers->setColumnCount( 2 );
  twWCSLayers->horizontalHeader()->setVisible( true );
  twWCSLayers->setRowCount( mapLayers.size() );

  i = 0;
  j = 0;
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == QgsMapLayer::RasterLayer )
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

      smWcsPublied->setMapping( cbp, j );
      connect( cbp, SIGNAL( stateChanged( int ) ), smWcsPublied, SLOT( map() ) );

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
  Q_FOREACH ( QgsMapLayer *mapLayer, mapLayers )
  {
    if ( QgsMapLayer::VectorLayer == mapLayer->type() )
    {
      vectorLayers.append( qobject_cast<QgsVectorLayer *>( mapLayer ) );
    }
  }
  mRelationManagerDlg->setLayers( vectorLayers );

  mAutoTransaction->setChecked( QgsProject::instance()->autoTransaction() );
  mEvaluateDefaultValues->setChecked( QgsProject::instance()->evaluateDefaultValues() );
  mTrustProjectCheckBox->setChecked( QgsProject::instance()->trustLayerMetadata() );

  // Variables editor
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  mVariableEditor->reloadContext();
  mVariableEditor->setEditableScopeIndex( 1 );

  projectionSelectorInitialized();
  restoreOptionsBaseUi();
  restoreState();
}

QgsProjectProperties::~QgsProjectProperties()
{
  saveState();
}

QString QgsProjectProperties::title() const
{
  return titleEdit->text();
} //  QgsProjectPropertires::title() const

void QgsProjectProperties::title( QString const &title )
{
  titleEdit->setText( title );
  QgsProject::instance()->setTitle( title );
} // QgsProjectProperties::title( QString const & title )

//when user clicks apply button
void QgsProjectProperties::apply()
{
  mMapCanvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );

  if ( projectionSelector->hasValidSelection() )
  {
    QgsCoordinateReferenceSystem srs = projectionSelector->crs();
    QgsProject::instance()->setCrs( srs );
    if ( srs.isValid() )
    {
      QgsDebugMsgLevel( QString( "Selected CRS " ) + srs.description(), 4 );
      // write the currently selected projections _proj string_ to project settings
      QgsDebugMsgLevel( QString( "SpatialRefSys/ProjectCRSProj4String: %1" ).arg( srs.toProj4() ), 4 );
    }
    else
    {
      QgsDebugMsgLevel( QString( "CRS set to no projection!" ), 4 );
    }

    // mark selected projection for push to front
    projectionSelector->pushProjectionToFront();
  }

  QgsCoordinateTransformContext transformContext = mDatumTransformTableWidget->transformContext();
  QgsProject::instance()->setTransformContext( transformContext );

  // Set the project title
  QgsProject::instance()->setTitle( title() );
  QgsProject::instance()->setAutoTransaction( mAutoTransaction->isChecked() );
  QgsProject::instance()->setEvaluateDefaultValues( mEvaluateDefaultValues->isChecked() );
  QgsProject::instance()->setTrustLayerMetadata( mTrustProjectCheckBox->isChecked() );

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
    default:
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

  QgsProject::instance()->writeEntry( QStringLiteral( "Paths" ), QStringLiteral( "/Absolute" ), cbxAbsolutePath->currentIndex() == 0 );

  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( QLatin1String( "PARAMETER" ) ) )
  {
    double major = mEllipsoidList.at( mEllipsoidIndex ).semiMajor;
    double minor = mEllipsoidList.at( mEllipsoidIndex ).semiMinor;
    // If the user fields have changed, use them instead.
    if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
    {
      QgsDebugMsgLevel( "Using parameteric major/minor", 4 );
      major = QLocale::system().toDouble( leSemiMajor->text() );
      minor = QLocale::system().toDouble( leSemiMinor->text() );
    }
    QgsProject::instance()->setEllipsoid( QStringLiteral( "PARAMETER:%1:%2" )
                                          .arg( major, 0, 'g', 17 )
                                          .arg( minor, 0, 'g', 17 ) );
  }
  else
  {
    QgsProject::instance()->setEllipsoid( mEllipsoidList[ mEllipsoidIndex ].acronym );
  }

  //set the color for selections
  QColor selectionColor = pbnSelectionColor->color();
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorRedPart" ), selectionColor.red() );
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorGreenPart" ), selectionColor.green() );
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorBluePart" ), selectionColor.blue() );
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/SelectionColorAlphaPart" ), selectionColor.alpha() );


  //set the color for canvas
  QColor canvasColor = pbnCanvasColor->color();
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorRedPart" ), canvasColor.red() );
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorGreenPart" ), canvasColor.green() );
  QgsProject::instance()->writeEntry( QStringLiteral( "Gui" ), QStringLiteral( "/CanvasColorBluePart" ), canvasColor.blue() );

  Q_FOREACH ( QgsMapCanvas *canvas, QgisApp::instance()->mapCanvases() )
  {
    canvas->setCanvasColor( canvasColor );
    canvas->setSelectionColor( selectionColor );
    canvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );
  }
  QgisApp::instance()->mapOverviewCanvas()->setBackgroundColor( canvasColor );

  //save project scales
  QStringList myScales;
  myScales.reserve( lstScales->count() );
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( lstScales->item( i )->text() );
  }

  if ( !myScales.isEmpty() )
  {
    QgsProject::instance()->writeEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ), myScales );
    QgsProject::instance()->writeEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ), grpProjectScales->isChecked() );
  }
  else
  {
    QgsProject::instance()->removeEntry( QStringLiteral( "Scales" ), QStringLiteral( "/" ) );
  }

  //use global or project scales depending on checkbox state
  if ( grpProjectScales->isChecked() )
  {
    emit scalesChanged( myScales );
  }
  else
  {
    emit scalesChanged();
  }

  QStringList noIdentifyLayerList;
  for ( int i = 0; i < twIdentifyLayers->rowCount(); i++ )
  {
    QString id = twIdentifyLayers->item( i, 0 )->data( Qt::UserRole ).toString();

    if ( twIdentifyLayers->item( i, 2 )->checkState() == Qt::Unchecked )
    {
      noIdentifyLayerList << id;
    }
    bool readonly = twIdentifyLayers->item( i, 3 )->checkState() == Qt::Checked;
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( id ) );
    if ( vl )
      vl->setReadOnly( readonly );
  }

  QgsProject::instance()->setNonIdentifiableLayers( noIdentifyLayerList );

  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceCapabilities" ), QStringLiteral( "/" ), grpOWSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceTitle" ), QStringLiteral( "/" ), mWMSTitle->text() );

  if ( !mWMSName->text().isEmpty() )
    QgsProject::instance()->writeEntry( QStringLiteral( "WMSRootName" ), QStringLiteral( "/" ), mWMSName->text() );

  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactOrganization" ), QStringLiteral( "/" ), mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPerson" ), QStringLiteral( "/" ), mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactMail" ), QStringLiteral( "/" ), mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSContactPhone" ), QStringLiteral( "/" ), mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSServiceAbstract" ), QStringLiteral( "/" ), mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSOnlineResource" ), QStringLiteral( "/" ), mWMSOnlineResourceLineEdit->text() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSUrl" ), QStringLiteral( "/" ), mWMSUrlLineEdit->text() );

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
    keywordStringList.replaceInStrings( QRegExp( "^\\s+" ), QLatin1String( "" ) ).replaceInStrings( QRegExp( "\\s+$" ), QLatin1String( "" ) );
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
  if ( mWMSComposerGroupBox->isChecked() )
  {
    QStringList composerTitles;
    for ( int i = 0; i < mComposerListWidget->count(); ++i )
    {
      composerTitles << mComposerListWidget->item( i )->text();
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

  QgsProject::instance()->writeEntry( QStringLiteral( "WMSAddWktGeometry" ), QStringLiteral( "/" ), mAddWktGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( QStringLiteral( "WMSRequestDefinedDataSources" ), QStringLiteral( "/" ), mAllowRequestDefinedDataSourcesBox->isChecked() );
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

  //save variables
  QgsProject::instance()->setCustomVariables( mVariableEditor->variablesInActiveScope() );

  //refresh canvases to reflect new properties, eg background color and scale bar after changing display units.
  Q_FOREACH ( QgsMapCanvas *canvas, QgisApp::instance()->mapCanvases() )
  {
    canvas->refresh();
  }
  QgisApp::instance()->mapOverviewCanvas()->refresh();
}

void QgsProjectProperties::showProjectionsTab()
{
  mOptionsListWidget->setCurrentRow( 1 );
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

void QgsProjectProperties::updateGuiForMapUnits( QgsUnitTypes::DistanceUnit units )
{
  if ( !projectionSelector->crs().isValid() )
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

void QgsProjectProperties::srIdUpdated()
{
  if ( !projectionSelector->hasValidSelection() )
    return;

  QgsCoordinateReferenceSystem srs = projectionSelector->crs();

  //set radio button to crs map unit type
  QgsUnitTypes::DistanceUnit units = srs.mapUnits();

  updateGuiForMapUnits( units );

  if ( srs.isValid() )
  {
    cmbEllipsoid->setEnabled( true );
    // attempt to reset the projection ellipsoid according to the srs
    int myIndex = 0;
    for ( int i = 0; i < mEllipsoidList.length(); i++ )
    {
      if ( mEllipsoidList[ i ].acronym == srs.ellipsoidAcronym() )
      {
        myIndex = i;
        break;
      }
    }
    updateEllipsoidUI( myIndex );
  }
  else
  {
    cmbEllipsoid->setCurrentIndex( 0 );
    cmbEllipsoid->setEnabled( false );
  }
}

void QgsProjectProperties::saveState()
{
}

void QgsProjectProperties::restoreState()
{
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
  mySelector->setMessage( QString() );
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
  Q_FOREACH ( QListWidgetItem *item, mWMSList->selectedItems() )
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

  QgsCoordinateReferenceSystem srs = projectionSelector->crs();
  if ( srs.isValid() )
    crsList << srs.authid();

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    crsList << it.value()->crs().authid();
  }

  mWMSList->clear();
  mWMSList->addItems( crsList.values() );
}

void QgsProjectProperties::mAddWMSComposerButton_clicked()
{
  QList<QgsComposition *> projectComposers = QgsProject::instance()->layoutManager()->compositions();
  QStringList composerTitles;
  QList<QgsComposition *>::const_iterator cIt = projectComposers.constBegin();
  for ( ; cIt != projectComposers.constEnd(); ++cIt )
  {
    composerTitles << ( *cIt )->name();
  }

  bool ok;
  QString name = QInputDialog::getItem( this, tr( "Select print composer" ), tr( "Composer Title" ), composerTitles, 0, false, &ok );
  if ( ok )
  {
    if ( mComposerListWidget->findItems( name, Qt::MatchExactly ).empty() )
    {
      mComposerListWidget->addItem( name );
    }
  }
}

void QgsProjectProperties::mRemoveWMSComposerButton_clicked()
{
  QListWidgetItem *currentItem = mComposerListWidget->currentItem();
  if ( currentItem )
  {
    delete mComposerListWidget->takeItem( mComposerListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::mAddLayerRestrictionButton_clicked()
{
  QgsProjectLayerGroupDialog d( this, QgsProject::instance()->fileName() );
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
  QString myStyle = QgsApplication::reportStyleSheet();
  teOWSChecker->clear();
  teOWSChecker->document()->setDefaultStyleSheet( myStyle );
  teOWSChecker->setHtml( "<h1>" + tr( "Start checking QGIS Server" ) + "</h1>" );

  QStringList owsNames, encodingMessages;
  checkOWS( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup(), owsNames, encodingMessages );

  QStringList duplicateNames, regExpMessages;
  QRegExp snRegExp = QgsApplication::shortNameRegExp();
  Q_FOREACH ( const QString &name, owsNames )
  {
    if ( !snRegExp.exactMatch( name ) )
      regExpMessages << tr( "Use short name for \"%1\"" ).arg( name );
    if ( duplicateNames.contains( name ) )
      continue;
    if ( owsNames.count( name ) > 1 )
      duplicateNames << name;
  }

  if ( !duplicateNames.empty() )
  {
    QString nameMessage = "<h1>" + tr( "Some layers and groups have the same name or short name" ) + "</h1>";
    nameMessage += "<h2>" + tr( "Duplicate names:" ) + "</h2>";
    nameMessage += duplicateNames.join( QStringLiteral( "</li><li>" ) ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + nameMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All names and short names of layer and group are unique" ) + "</h1>" );
  }

  if ( !regExpMessages.empty() )
  {
    QString encodingMessage = "<h1>" + tr( "Some layer short names have to be updated:" ) + "</h1><ul><li>" + regExpMessages.join( QStringLiteral( "</li><li>" ) ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + encodingMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All layer short names are well formed" ) + "</h1>" );
  }

  if ( !encodingMessages.empty() )
  {
    QString encodingMessage = "<h1>" + tr( "Some layer encodings are not set:" ) + "</h1><ul><li>" + encodingMessages.join( QStringLiteral( "</li><li>" ) ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + encodingMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All layer encodings are set" ) + "</h1>" );
  }

  teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "Start checking QGIS Server" ) + "</h1>" );
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

  Q_FOREACH ( const QString &scale, myScales )
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
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Marker" ), QLatin1String( "" ) );
  cboList << cboStyleLine;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Line" ), QLatin1String( "" ) );
  cboList << cboStyleFill;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/Fill" ), QLatin1String( "" ) );
  cboList << cboStyleColorRamp;
  prefList << QgsProject::instance()->readEntry( QStringLiteral( "DefaultStyles" ), QStringLiteral( "/ColorRamp" ), QLatin1String( "" ) );
  for ( int i = 0; i < cboList.count(); i++ )
  {
    cboList[i]->clear();
    cboList[i]->addItem( QLatin1String( "" ) );
  }

  // populate symbols
  QStringList symbolNames = mStyle->symbolNames();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    QgsSymbol *symbol = mStyle->symbol( name );
    QComboBox *cbo = nullptr;
    switch ( symbol->type() )
    {
      case QgsSymbol::Marker :
        cbo = cboStyleMarker;
        break;
      case QgsSymbol::Line :
        cbo = cboStyleLine;
        break;
      case QgsSymbol::Fill :
        cbo = cboStyleFill;
        break;
      case QgsSymbol::Hybrid:
        // Shouldn't get here
        break;
    }
    if ( cbo )
    {
      QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, cbo->iconSize() );
      cbo->addItem( icon, name );
    }
    delete symbol;
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
    QMessageBox::information( this, QLatin1String( "" ), tr( "Select a valid symbol" ) );
    return;
  }
  QgsSymbol *symbol = mStyle->symbol( symbolName );
  if ( ! symbol )
  {
    QMessageBox::warning( this, QLatin1String( "" ), tr( "Invalid symbol : " ) + symbolName );
    return;
  }

  // let the user edit the symbol and update list when done
  QgsSymbolSelectorDialog dlg( symbol, mStyle, nullptr, this );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return;
  }

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol );

  // update icon
  QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( symbol, cbo->iconSize() );
  cbo->setItemIcon( cbo->currentIndex(), icon );
}

void QgsProjectProperties::resetPythonMacros()
{
  grpPythonMacros->setChecked( false );
  ptePythonMacros->setText( "def openProject():\n    pass\n\n" \
                            "def saveProject():\n    pass\n\n" \
                            "def closeProject():\n    pass\n" );
}

void QgsProjectProperties::checkOWS( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages )
{
  QList< QgsLayerTreeNode * > treeGroupChildren = treeGroup->children();
  for ( int i = 0; i < treeGroupChildren.size(); ++i )
  {
    QgsLayerTreeNode *treeNode = treeGroupChildren.at( i );
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      QString shortName = treeGroupChild->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
      if ( shortName.isEmpty() )
        owsNames << treeGroupChild->name();
      else
        owsNames << shortName;
      checkOWS( treeGroupChild, owsNames, encodingMessages );
    }
    else
    {
      QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer *l = treeLayer->layer();
      QString shortName = l->shortName();
      if ( shortName.isEmpty() )
        owsNames << l->name();
      else
        owsNames << shortName;
      if ( l->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( l );
        if ( vl->dataProvider()->encoding() == QLatin1String( "System" ) )
          encodingMessages << tr( "Update layer \"%1\" encoding" ).arg( l->name() );
      }
    }
  }
}

void QgsProjectProperties::populateEllipsoidList()
{
  //
  // Populate the ellipsoid list
  //
  EllipsoidDefs myItem;

  myItem.acronym = GEO_NONE;
  myItem.description = tr( GEO_NONE_DESC );
  myItem.semiMajor = 0.0;
  myItem.semiMinor = 0.0;
  mEllipsoidList.append( myItem );

  myItem.acronym = QStringLiteral( "PARAMETER:6370997:6370997" );
  myItem.description = tr( "Custom" );
  myItem.semiMajor = 6370997.0;
  myItem.semiMinor = 6370997.0;
  mEllipsoidList.append( myItem );

  Q_FOREACH ( const QgsEllipsoidUtils::EllipsoidDefinition &def, QgsEllipsoidUtils::definitions() )
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

  Q_FOREACH ( const EllipsoidDefs &i, mEllipsoidList )
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
    QgsDebugMsgLevel( "Saving major/minor", 4 );
    mEllipsoidList[ mEllipsoidIndex ].semiMajor = QLocale::system().toDouble( leSemiMajor->text() );
    mEllipsoidList[ mEllipsoidIndex ].semiMinor = QLocale::system().toDouble( leSemiMinor->text() );
  }

  mEllipsoidIndex = newIndex;
  leSemiMajor->setEnabled( false );
  leSemiMinor->setEnabled( false );
  leSemiMajor->clear();
  leSemiMinor->clear();

  cmbEllipsoid->setEnabled( projectionSelector->crs().isValid() );
  cmbEllipsoid->setToolTip( QLatin1String( "" ) );
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
  if ( mEllipsoidList[ mEllipsoidIndex ].acronym != GEO_NONE )
  {
    leSemiMajor->setText( QLocale::system().toString( myMajor, 'f', 3 ) );
    leSemiMinor->setText( QLocale::system().toString( myMinor, 'f', 3 ) );
  }

  if ( projectionSelector->crs().isValid() )
    cmbEllipsoid->setCurrentIndex( mEllipsoidIndex ); // Not always necessary
}

void QgsProjectProperties::projectionSelectorInitialized()
{
  QgsDebugMsgLevel( "Setting up ellipsoid", 4 );

  // Reading ellipsoid from settings
  QStringList mySplitEllipsoid = QgsProject::instance()->ellipsoid().split( ':' );

  int myIndex = 0;
  for ( int i = 0; i < mEllipsoidList.length(); i++ )
  {
    if ( mEllipsoidList.at( i ).acronym.startsWith( mySplitEllipsoid[ 0 ] ) )
    {
      myIndex = i;
      break;
    }
  }

  // Update parameters if present.
  if ( mySplitEllipsoid.length() >= 3 )
  {
    mEllipsoidList[ myIndex ].semiMajor = mySplitEllipsoid[ 1 ].toDouble();
    mEllipsoidList[ myIndex ].semiMinor = mySplitEllipsoid[ 2 ].toDouble();
  }

  updateEllipsoidUI( myIndex );
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
    QMessageBox::warning( this, tr( "Invalid scale" ), tr( "The text you entered is not a valid scale." ) );
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
  QgsHelp::openHelp( link );
}
