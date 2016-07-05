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
#include "qgscomposer.h"
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgssnappingdialog.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsscaleutils.h"
#include "qgsgenericprojectionselector.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgsstylev2managerdialog.h"
#include "qgsvectorcolorrampv2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgsrelationmanagerdialog.h"
#include "qgsrelationmanager.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerv2utils.h"
#include "qgscolordialog.h"
#include "qgsexpressioncontext.h"
#include "qgsmapoverviewcanvas.h"
#include "qgslayertreenode.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertreemodel.h"
#include "qgsunittypes.h"
#include "qgstablewidgetitem.h"

#include "qgsmessagelog.h"

//qt includes
#include <QInputDialog>
#include <QFileDialog>
#include <QHeaderView>  // Qt 4.4
#include <QMessageBox>

const char * QgsProjectProperties::GEO_NONE_DESC = QT_TRANSLATE_NOOP( "QgsOptions", "None / Planimetric" );

//stdc++ includes

QgsProjectProperties::QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent, Qt::WindowFlags fl )
    : QgsOptionsDialogBase( "ProjectProperties", parent, fl )
    , mMapCanvas( mapCanvas )
    , mEllipsoidList()
    , mEllipsoidIndex( 0 )

{
  setupUi( this );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  mCoordinateDisplayComboBox->addItem( tr( "Map units" ), MapUnits );
  mCoordinateDisplayComboBox->addItem( tr( "Decimal degrees" ), DecimalDegrees );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, minutes" ), DegreesMinutes );
  mCoordinateDisplayComboBox->addItem( tr( "Degrees, minutes, seconds" ), DegreesMinutesSeconds );

  mDistanceUnitsCombo->addItem( tr( "Meters" ), QGis::Meters );
  mDistanceUnitsCombo->addItem( tr( "Kilometers" ), QGis::Kilometers );
  mDistanceUnitsCombo->addItem( tr( "Feet" ), QGis::Feet );
  mDistanceUnitsCombo->addItem( tr( "Yards" ), QGis::Yards );
  mDistanceUnitsCombo->addItem( tr( "Miles" ), QGis::Miles );
  mDistanceUnitsCombo->addItem( tr( "Nautical miles" ), QGis::NauticalMiles );
  mDistanceUnitsCombo->addItem( tr( "Degrees" ), QGis::Degrees );
  mDistanceUnitsCombo->addItem( tr( "Map units" ), QGis::UnknownUnit );

  mAreaUnitsCombo->addItem( tr( "Square meters" ), QgsUnitTypes::SquareMeters );
  mAreaUnitsCombo->addItem( tr( "Square kilometers" ), QgsUnitTypes::SquareKilometers );
  mAreaUnitsCombo->addItem( tr( "Square feet" ), QgsUnitTypes::SquareFeet );
  mAreaUnitsCombo->addItem( tr( "Square yards" ), QgsUnitTypes::SquareYards );
  mAreaUnitsCombo->addItem( tr( "Square miles" ), QgsUnitTypes::SquareMiles );
  mAreaUnitsCombo->addItem( tr( "Hectares" ), QgsUnitTypes::Hectares );
  mAreaUnitsCombo->addItem( tr( "Acres" ), QgsUnitTypes::Acres );
  mAreaUnitsCombo->addItem( tr( "Square nautical miles" ), QgsUnitTypes::SquareNauticalMiles );
  mAreaUnitsCombo->addItem( tr( "Square degrees" ), QgsUnitTypes::SquareDegrees );
  mAreaUnitsCombo->addItem( tr( "Map units" ), QgsUnitTypes::UnknownAreaUnit );

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( projectionSelector, SIGNAL( sridSelected( QString ) ), this, SLOT( srIdUpdated() ) );
  connect( projectionSelector, SIGNAL( initialized() ), this, SLOT( projectionSelectorInitialized() ) );

  connect( cmbEllipsoid, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateEllipsoidUI( int ) ) );

  connect( radAutomatic, SIGNAL( toggled( bool ) ), spinBoxDP, SLOT( setDisabled( bool ) ) );
  connect( radAutomatic, SIGNAL( toggled( bool ) ), labelDP, SLOT( setDisabled( bool ) ) );
  connect( radManual, SIGNAL( toggled( bool ) ), spinBoxDP, SLOT( setEnabled( bool ) ) );
  connect( radManual, SIGNAL( toggled( bool ) ), labelDP, SLOT( setEnabled( bool ) ) );

  QSettings settings;

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  // we need to initialize it, since the on_cbxProjectionEnabled_toggled()
  // slot triggered by setChecked() might use it.
  mProjectSrsId = mMapCanvas->mapSettings().destinationCrs().srsid();

  QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( mProjectSrsId );
  updateGuiForMapUnits( srs.mapUnits() );

  QgsDebugMsg( "Read project CRSID: " + QString::number( mProjectSrsId ) );
  projectionSelector->setSelectedCrsId( mProjectSrsId );

  mMapTileRenderingCheckBox->setChecked( mMapCanvas->mapSettings().testFlag( QgsMapSettings::RenderMapTile ) );

  // see end of constructor for updating of projection selector

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  Q_FOREACH ( QgsVectorLayer* layer, QgsMapLayerRegistry::instance()->layers<QgsVectorLayer*>() )
  {
    if ( layer->isEditable() )
    {
      mAutoTransaction->setEnabled( false );
      mAutoTransaction->setToolTip( tr( "Layers are in edit mode. Stop edit mode on all layers to toggle transactional editing." ) );
    }
  }

  mAutoTransaction->setChecked( QgsProject::instance()->autoTransaction() );
  title( QgsProject::instance()->title() );
  projectFileName->setText( QgsProject::instance()->fileName() );

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic", true );
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
  int dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );
  spinBoxDP->setValue( dp );

  cbxAbsolutePath->setCurrentIndex( QgsProject::instance()->readBoolEntry( "Paths", "/Absolute", true ) ? 0 : 1 );

  // populate combo box with ellipsoids
  // selection of the ellipsoid from settings is defferred to a later point, because it would
  // be overridden in the meanwhile by the projection selector
  populateEllipsoidList();

  QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "MU" );
  if ( format == "MU" )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( MapUnits ) );
  else if ( format == "DM" )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DegreesMinutes ) );
  else if ( format == "DMS" )
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DegreesMinutesSeconds ) );
  else
    mCoordinateDisplayComboBox->setCurrentIndex( mCoordinateDisplayComboBox->findData( DecimalDegrees ) );

  mDistanceUnitsCombo->setCurrentIndex( mDistanceUnitsCombo->findData( QgsProject::instance()->distanceUnits() ) );
  mAreaUnitsCombo->setCurrentIndex( mAreaUnitsCombo->findData( QgsProject::instance()->areaUnits() ) );

  //get the color selections and set the button color accordingly
  int myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorRedPart", 255 );
  int myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorGreenPart", 255 );
  int myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorBluePart", 0 );
  int myAlphaInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorAlphaPart", 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );
  myRedInt = settings.value( "/qgis/default_selection_color_red", 255 ).toInt();
  myGreenInt = settings.value( "/qgis/default_selection_color_green", 255 ).toInt();
  myBlueInt = settings.value( "/qgis/default_selection_color_blue", 0 ).toInt();
  myAlphaInt = settings.value( "/qgis/default_selection_color_alpha", 255 ).toInt();
  QColor defaultSelectionColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );
  pbnSelectionColor->setContext( "gui" );
  pbnSelectionColor->setColor( myColor );
  pbnSelectionColor->setDefaultColor( defaultSelectionColor );
  pbnSelectionColor->setColorDialogTitle( tr( "Selection color" ) );
  pbnSelectionColor->setAllowAlpha( true );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  myRedInt = settings.value( "/qgis/default_canvas_color_red", 255 ).toInt();
  myGreenInt = settings.value( "/qgis/default_canvas_color_green", 255 ).toInt();
  myBlueInt = settings.value( "/qgis/default_canvas_color_blue", 255 ).toInt();
  QColor defaultCanvasColor = QColor( myRedInt, myGreenInt, myBlueInt );

  pbnCanvasColor->setContext( "gui" );
  pbnCanvasColor->setColor( myColor );
  pbnCanvasColor->setDefaultColor( defaultCanvasColor );

  //get project scales
  QStringList myScales = QgsProject::instance()->readListEntry( "Scales", "/ScalesList" );
  if ( !myScales.isEmpty() )
  {
    Q_FOREACH ( const QString& scale, myScales )
    {
      addScaleToScaleList( scale );
    }
  }
  connect( lstScales, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SLOT( scaleItemChanged( QListWidgetItem* ) ) );

  grpProjectScales->setChecked( QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" ) );

  QgsMapLayer* currentLayer = nullptr;

  QStringList noIdentifyLayerIdList = QgsProject::instance()->nonIdentifiableLayers();

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();

  if ( mMapCanvas->currentLayer() )
  {
    mLayerSrsId = mMapCanvas->currentLayer()->crs().srsid();
  }
  else if ( !mapLayers.isEmpty() )
  {
    mLayerSrsId = mapLayers.begin().value()->crs().srsid();
  }
  else
  {
    mLayerSrsId = mProjectSrsId;
  }

  twIdentifyLayers->setColumnCount( 4 );
  twIdentifyLayers->horizontalHeader()->setVisible( true );
  twIdentifyLayers->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Layer" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Type" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Identifiable" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 3, new QTableWidgetItem( tr( "Read Only" ) ) );
  twIdentifyLayers->setRowCount( mapLayers.size() );
  twIdentifyLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  int i = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
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

      if ( rl && rl->providerType() == "wms" )
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

  grpOWSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( "WMSServiceCapabilities", "/", false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( "WMSServiceTitle", "/" ) );
  mWMSName->setText( QgsProject::instance()->readEntry( "WMSRootName", "/" ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( "WMSContactOrganization", "/", "" ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( "WMSContactPerson", "/", "" ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( "WMSContactMail", "/", "" ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( "WMSContactPhone", "/", "" ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( "WMSServiceAbstract", "/", "" ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( "WMSOnlineResource", "/", "" ) );
  mWMSUrlLineEdit->setText( QgsProject::instance()->readEntry( "WMSUrl", "/", "" ) );
  mWMSKeywordList->setText( QgsProject::instance()->readListEntry( "WMSKeywordList", "/" ).join( ", " ) );

  // WMS Name validator
  QValidator *shortNameValidator = new QRegExpValidator( QgsApplication::shortNameRegExp(), this );
  mWMSName->setValidator( shortNameValidator );

  // WMS Contact Position
  QString contactPositionText = QgsProject::instance()->readEntry( "WMSContactPosition", "/", "" );
  mWMSContactPositionCb->addItem( "" );
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
  QString feesText = QgsProject::instance()->readEntry( "WMSFees", "/", "" );
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
  QString accessConstraintsText = QgsProject::instance()->readEntry( "WMSAccessConstraints", "/", "" );
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

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "bg" ) ), QLocale( "bg" ).nativeLanguageName(), "bul" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cs" ) ), QLocale( "cs" ).nativeLanguageName(), "cze" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "da" ) ), QLocale( "da" ).nativeLanguageName(), "dan" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "nl" ) ), QLocale( "nl" ).nativeLanguageName(), "dut" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "en_GB" ) ), QLocale( "en_GB" ).nativeLanguageName(), "eng" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "et" ) ), QLocale( "et" ).nativeLanguageName(), "est" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fi" ) ), QLocale( "fi" ).nativeLanguageName(), "fin" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "fr" ) ), QLocale( "fr" ).nativeLanguageName(), "fre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "de" ) ), QLocale( "de" ).nativeLanguageName(), "ger" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ga" ) ), QLocale( "ga" ).nativeLanguageName(), "gle" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "el" ) ), QLocale( "el" ).nativeLanguageName(), "gre" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "hu" ) ), QLocale( "hu" ).nativeLanguageName(), "hun" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "it" ) ), QLocale( "it" ).nativeLanguageName(), "ita" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lv" ) ), QLocale( "lv" ).nativeLanguageName(), "lav" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "lt" ) ), QLocale( "lt" ).nativeLanguageName(), "lit" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "mt" ) ), QLocale( "mt" ).nativeLanguageName(), "mlt" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pl" ) ), QLocale( "pl" ).nativeLanguageName(), "pol" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "pt_PT" ) ), QLocale( "pt_PT" ).nativeLanguageName(), "por" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ro" ) ), QLocale( "ro" ).nativeLanguageName(), "rum" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sk" ) ), QLocale( "sk" ).nativeLanguageName(), "slo" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sl" ) ), QLocale( "sl" ).nativeLanguageName(), "slv" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "es" ) ), QLocale( "es" ).nativeLanguageName(), "spa" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "sv" ) ), QLocale( "sv" ).nativeLanguageName(), "swe" );

  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "eu" ) ), QLocale( "eu" ).nativeLanguageName(), "eus" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "ca" ) ), QLocale( "ca" ).nativeLanguageName(), "cat" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gl" ) ), QLocale( "gl" ).nativeLanguageName(), "gal" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "gd" ) ), QLocale( "gd" ).nativeLanguageName(), "gla" );
  mWMSInspireLanguage->addItem( QIcon( QString( ":/images/flags/%1.png" ).arg( "cy" ) ), QLocale( "cy" ).nativeLanguageName(), "cym" );
  mWMSInspireLanguage->setCurrentIndex(
    mWMSInspireLanguage->findText(
      QLocale::system().nativeLanguageName()
    )
  );

  bool addWMSInspire = QgsProject::instance()->readBoolEntry( "WMSInspire", "/activated" );
  if ( addWMSInspire )
  {
    mWMSInspire->setChecked( addWMSInspire );
    QString inspireLanguage = QgsProject::instance()->readEntry( "WMSInspire", "/language", "" );
    int inspireLanguageIndex = mWMSInspireLanguage->findData( inspireLanguage );
    mWMSInspireLanguage->setCurrentIndex( inspireLanguageIndex );

    QString inspireMetadataUrl = QgsProject::instance()->readEntry( "WMSInspire", "/metadataUrl", "" );
    if ( !inspireMetadataUrl.isEmpty() )
    {
      mWMSInspireScenario1->setChecked( true );
      mWMSInspireMetadataUrl->setText( inspireMetadataUrl );
      mWMSInspireMetadataUrlType->setCurrentIndex(
        mWMSInspireMetadataUrlType->findText(
          QgsProject::instance()->readEntry( "WMSInspire", "/metadataUrlType", "" )
        )
      );
    }
    else
    {
      QString inspireTemporalReference = QgsProject::instance()->readEntry( "WMSInspire", "/temporalReference", "" );
      if ( !inspireTemporalReference.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireTemporalReference->setDate( QDate::fromString( inspireTemporalReference, "yyyy-MM-dd" ) );
      }
      QString inspireMetadataDate = QgsProject::instance()->readEntry( "WMSInspire", "/metadataDate", "" );
      if ( !inspireMetadataDate.isEmpty() )
      {
        mWMSInspireScenario2->setChecked( true );
        mWMSInspireMetadataDate->setDate( QDate::fromString( inspireMetadataDate, "yyyy-MM-dd" ) );
      }
    }
  }

  // WMS GetFeatureInfo precision
  int WMSprecision = QgsProject::instance()->readNumEntry( "WMSPrecision", "/", -1 );
  if ( WMSprecision != -1 )
  {
    mWMSPrecisionSpinBox->setValue( WMSprecision );
  }

  bool ok;
  QStringList values;

  mWMSExtMinX->setValidator( new QDoubleValidator( mWMSExtMinX ) );
  mWMSExtMinY->setValidator( new QDoubleValidator( mWMSExtMinY ) );
  mWMSExtMaxX->setValidator( new QDoubleValidator( mWMSExtMaxX ) );
  mWMSExtMaxY->setValidator( new QDoubleValidator( mWMSExtMaxY ) );

  values = QgsProject::instance()->readListEntry( "WMSExtent", "/", QStringList(), &ok );
  grpWMSExt->setChecked( ok && values.size() == 4 );
  if ( grpWMSExt->isChecked() )
  {
    mWMSExtMinX->setText( values[0] );
    mWMSExtMinY->setText( values[1] );
    mWMSExtMaxX->setText( values[2] );
    mWMSExtMaxY->setText( values[3] );
  }

  values = QgsProject::instance()->readListEntry( "WMSCrsList", "/", QStringList(), &ok );
  grpWMSList->setChecked( ok && !values.isEmpty() );
  if ( grpWMSList->isChecked() )
  {
    mWMSList->addItems( values );
  }
  else
  {
    values = QgsProject::instance()->readListEntry( "WMSEpsgList", "/", QStringList(), &ok );
    grpWMSList->setChecked( ok && !values.isEmpty() );
    if ( grpWMSList->isChecked() )
    {
      QStringList list;
      Q_FOREACH ( const QString& value, values )
      {
        list << QString( "EPSG:%1" ).arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  //composer restriction for WMS
  values = QgsProject::instance()->readListEntry( "WMSRestrictedComposers", "/", QStringList(), &ok );
  mWMSComposerGroupBox->setChecked( ok );
  if ( ok )
  {
    mComposerListWidget->addItems( values );
  }

  //layer restriction for WMS
  values = QgsProject::instance()->readListEntry( "WMSRestrictedLayers", "/", QStringList(), &ok );
  mLayerRestrictionsGroupBox->setChecked( ok );
  if ( ok )
  {
    mLayerRestrictionsListWidget->addItems( values );
  }

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( "WMSAddWktGeometry", "/" );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  bool useLayerIDs = QgsProject::instance()->readBoolEntry( "WMSUseLayerIDs", "/" );
  mWmsUseLayerIDs->setChecked( useLayerIDs );

  //WMS maxWidth / maxHeight
  mMaxWidthLineEdit->setValidator( new QIntValidator( mMaxWidthLineEdit ) );
  int maxWidth = QgsProject::instance()->readNumEntry( "WMSMaxWidth", "/", -1 );
  if ( maxWidth != -1 )
  {
    mMaxWidthLineEdit->setText( QString::number( maxWidth ) );
  }
  mMaxHeightLineEdit->setValidator( new QIntValidator( mMaxHeightLineEdit ) );
  int maxHeight = QgsProject::instance()->readNumEntry( "WMSMaxHeight", "/", -1 );
  if ( maxHeight != -1 )
  {
    mMaxHeightLineEdit->setText( QString::number( maxHeight ) );
  }

  // WMS imageQuality
  int imageQuality = QgsProject::instance()->readNumEntry( "WMSImageQuality", "/", -1 );
  if ( imageQuality != -1 )
  {
    mWMSImageQualitySpinBox->setValue( imageQuality );
  }

  mWFSUrlLineEdit->setText( QgsProject::instance()->readEntry( "WFSUrl", "/", "" ) );
  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( "WFSLayers", "/" );
  QStringList wfstUpdateLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Update" );
  QStringList wfstInsertLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Insert" );
  QStringList wfstDeleteLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Delete" );

  QSignalMapper *smPublied = new QSignalMapper( this );
  connect( smPublied, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSPubliedStateChanged( int ) ) );

  twWFSLayers->setColumnCount( 6 );
  twWFSLayers->horizontalHeader()->setVisible( true );
  twWFSLayers->setRowCount( mapLayers.size() );

  i = 0;
  int j = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
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

      QCheckBox* cbp = new QCheckBox();
      cbp->setChecked( wfsLayerIdList.contains( currentLayer->id() ) );
      twWFSLayers->setCellWidget( j, 1, cbp );

      smPublied->setMapping( cbp, j );
      connect( cbp, SIGNAL( stateChanged( int ) ), smPublied, SLOT( map() ) );

      QSpinBox* psb = new QSpinBox();
      psb->setValue( QgsProject::instance()->readNumEntry( "WFSLayersPrecision", "/" + currentLayer->id(), 8 ) );
      twWFSLayers->setCellWidget( j, 2, psb );

      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( currentLayer );
      QgsVectorDataProvider* provider = vlayer->dataProvider();
      if (( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) && ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) )
      {
        QCheckBox* cbu = new QCheckBox();
        cbu->setChecked( wfstUpdateLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 3, cbu );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
      {
        QCheckBox* cbi = new QCheckBox();
        cbi->setChecked( wfstInsertLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 4, cbi );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
      {
        QCheckBox* cbd = new QCheckBox();
        cbd->setChecked( wfstDeleteLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 5, cbd );
      }

      j++;
    }
  }
  twWFSLayers->setRowCount( j );
  twWFSLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  mWCSUrlLineEdit->setText( QgsProject::instance()->readEntry( "WCSUrl", "/", "" ) );
  QStringList wcsLayerIdList = QgsProject::instance()->readListEntry( "WCSLayers", "/" );

  QSignalMapper *smWcsPublied = new QSignalMapper( this );
  connect( smWcsPublied, SIGNAL( mapped( int ) ), this, SLOT( cbxWCSPubliedStateChanged( int ) ) );

  twWCSLayers->setColumnCount( 2 );
  twWCSLayers->horizontalHeader()->setVisible( true );
  twWCSLayers->setRowCount( mapLayers.size() );

  i = 0;
  j = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it, i++ )
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

      QCheckBox* cbp = new QCheckBox();
      cbp->setChecked( wcsLayerIdList.contains( currentLayer->id() ) );
      twWCSLayers->setCellWidget( j, 1, cbp );

      smWcsPublied->setMapping( cbp, j );
      connect( cbp, SIGNAL( stateChanged( int ) ), smWcsPublied, SLOT( map() ) );

      j++;
    }
  }
  twWCSLayers->setRowCount( j );
  twWCSLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  // Default Styles
  mStyle = QgsStyleV2::defaultStyle();
  populateStyles();

  // Color palette
  connect( mButtonCopyColors, SIGNAL( clicked() ), mTreeProjectColors, SLOT( copyColors() ) );
  connect( mButtonRemoveColor, SIGNAL( clicked() ), mTreeProjectColors, SLOT( removeSelection() ) );
  connect( mButtonPasteColors, SIGNAL( clicked() ), mTreeProjectColors, SLOT( pasteColors() ) );

  QList<QgsProjectColorScheme *> projectSchemes;
  QgsColorSchemeRegistry::instance()->schemes( projectSchemes );
  if ( projectSchemes.length() > 0 )
  {
    mTreeProjectColors->setScheme( projectSchemes.at( 0 ) );
  }


  // Project macros
  QString pythonMacros = QgsProject::instance()->readEntry( "Macros", "/pythonCode", QString::null );
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

  QList<QgsVectorLayer*> vectorLayers;
  Q_FOREACH ( QgsMapLayer* mapLayer, mapLayers )
  {
    if ( QgsMapLayer::VectorLayer == mapLayer->type() )
    {
      vectorLayers.append( qobject_cast<QgsVectorLayer*>( mapLayer ) );
    }
  }
  mRelationManagerDlg->setLayers( vectorLayers );

  // Update projection selector (after mLayerSrsId is set)
  bool myProjectionEnabled = mMapCanvas->mapSettings().hasCrsTransformEnabled();
  bool onFlyChecked = cbxProjectionEnabled->isChecked();
  cbxProjectionEnabled->setChecked( myProjectionEnabled );

  if ( onFlyChecked == myProjectionEnabled )
  {
    // ensure selector is updated if cbxProjectionEnabled->toggled signal not sent
    on_cbxProjectionEnabled_toggled( myProjectionEnabled );
  }

  mAutoTransaction->setChecked( QgsProject::instance()->autoTransaction() );
  mEvaluateDefaultValues->setChecked( QgsProject::instance()->evaluateDefaultValues() );

  // Variables editor
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::globalScope() );
  mVariableEditor->context()->appendScope( QgsExpressionContextUtils::projectScope() );
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

// return the map units
QGis::UnitType QgsProjectProperties::mapUnits() const
{
  return mMapCanvas->mapSettings().mapUnits();
}

void QgsProjectProperties::setMapUnits( QGis::UnitType unit )
{
  // select the button
  if ( unit == QGis::UnknownUnit )
  {
    unit = QGis::Meters;
  }

  mMapCanvas->setMapUnits( unit );
}

QString QgsProjectProperties::title() const
{
  return titleEdit->text();
} //  QgsProjectPropertires::title() const

void QgsProjectProperties::title( QString const & title )
{
  titleEdit->setText( title );
  QgsProject::instance()->setTitle( title );
} // QgsProjectProperties::title( QString const & title )

//when user clicks apply button
void QgsProjectProperties::apply()
{
  mMapCanvas->setCrsTransformEnabled( cbxProjectionEnabled->isChecked() );

  mMapCanvas->enableMapTileRendering( mMapTileRenderingCheckBox->isChecked() );

  // Only change the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  long myCRSID = projectionSelector->selectedCrsId();
  if ( myCRSID )
  {
    QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( myCRSID );
    mMapCanvas->setDestinationCrs( srs );
    QgsDebugMsg( QString( "Selected CRS " ) + srs.description() );
    // write the currently selected projections _proj string_ to project settings
    QgsDebugMsg( QString( "SpatialRefSys/ProjectCRSProj4String: %1" ).arg( projectionSelector->selectedProj4String() ) );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", projectionSelector->selectedProj4String() );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) projectionSelector->selectedCrsId() );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", projectionSelector->selectedAuthId() );

    // Set the map units to the projected coordinates if we are projecting
    if ( isProjected() )
    {
      // If we couldn't get the map units, default to the value in the
      // projectproperties dialog box (set above)
      if ( srs.mapUnits() != QGis::UnknownUnit )
        mMapCanvas->setMapUnits( srs.mapUnits() );
    }

    if ( cbxProjectionEnabled->isChecked() )
    {
      // mark selected projection for push to front
      projectionSelector->pushProjectionToFront();
    }
  }

  // Set the project title
  QgsProject::instance()->setTitle( title() );
  QgsProject::instance()->setAutoTransaction( mAutoTransaction->isChecked() );
  QgsProject::instance()->setEvaluateDefaultValues( mEvaluateDefaultValues->isChecked() );

  // set the mouse display precision method and the
  // number of decimal places for the manual option
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  QgsProject::instance()->writeEntry( "PositionPrecision", "/Automatic", radAutomatic->isChecked() );
  QgsProject::instance()->writeEntry( "PositionPrecision", "/DecimalPlaces", spinBoxDP->value() );
  QString degreeFormat;
  switch ( static_cast< CoordinateFormat >( mCoordinateDisplayComboBox->itemData( mCoordinateDisplayComboBox->currentIndex() ).toInt() ) )
  {
    case DegreesMinutes:
      degreeFormat = "DM";
      break;
    case DegreesMinutesSeconds:
      degreeFormat = "DMS";
      break;
    case MapUnits:
      degreeFormat = "MU";
      break;
    case DecimalDegrees:
    default:
      degreeFormat = "D";
      break;
  }
  QgsProject::instance()->writeEntry( "PositionPrecision", "/DegreeFormat", degreeFormat );

  // Announce that we may have a new display precision setting
  emit displayPrecisionChanged();

  QGis::UnitType distanceUnits = static_cast< QGis::UnitType >( mDistanceUnitsCombo->itemData( mDistanceUnitsCombo->currentIndex() ).toInt() );
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( distanceUnits ) );

  QgsUnitTypes::AreaUnit areaUnits = static_cast< QgsUnitTypes::AreaUnit >( mAreaUnitsCombo->itemData( mAreaUnitsCombo->currentIndex() ).toInt() );
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( areaUnits ) );

  QgsProject::instance()->writeEntry( "Paths", "/Absolute", cbxAbsolutePath->currentIndex() == 0 );

  if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( "PARAMETER" ) )
  {
    double major = mEllipsoidList.at( mEllipsoidIndex ).semiMajor;
    double minor = mEllipsoidList.at( mEllipsoidIndex ).semiMinor;
    // If the user fields have changed, use them instead.
    if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
    {
      QgsDebugMsg( "Using paramteric major/minor" );
      major = QLocale::system().toDouble( leSemiMajor->text() );
      minor = QLocale::system().toDouble( leSemiMinor->text() );
    }
    QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "PARAMETER:%1:%2" )
                                        .arg( major, 0, 'g', 17 )
                                        .arg( minor, 0, 'g', 17 ) );
  }
  else
  {
    QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", mEllipsoidList[ mEllipsoidIndex ].acronym );
  }

  //set the color for selections
  QColor myColor = pbnSelectionColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorBluePart", myColor.blue() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorAlphaPart", myColor.alpha() );
  mMapCanvas->setSelectionColor( myColor );

  //set the color for canvas
  myColor = pbnCanvasColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorBluePart", myColor.blue() );
  mMapCanvas->setCanvasColor( myColor );
  QgisApp::instance()->mapOverviewCanvas()->setBackgroundColor( myColor );
  QgisApp::instance()->mapOverviewCanvas()->refresh();

  //save project scales
  QStringList myScales;
  myScales.reserve( lstScales->count() );
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( lstScales->item( i )->text() );
  }

  if ( !myScales.isEmpty() )
  {
    QgsProject::instance()->writeEntry( "Scales", "/ScalesList", myScales );
    QgsProject::instance()->writeEntry( "Scales", "/useProjectScales", grpProjectScales->isChecked() );
  }
  else
  {
    QgsProject::instance()->removeEntry( "Scales", "/" );
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
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( id ) );
    if ( vl )
      vl->setReadOnly( readonly );
  }

  QgsProject::instance()->setNonIdentifiableLayers( noIdentifyLayerList );

  QgsProject::instance()->writeEntry( "WMSServiceCapabilities", "/", grpOWSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( "WMSServiceTitle", "/", mWMSTitle->text() );

  if ( !mWMSName->text().isEmpty() )
    QgsProject::instance()->writeEntry( "WMSRootName", "/", mWMSName->text() );

  QgsProject::instance()->writeEntry( "WMSContactOrganization", "/", mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( "WMSContactPerson", "/", mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( "WMSContactMail", "/", mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( "WMSContactPhone", "/", mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( "WMSServiceAbstract", "/", mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( "WMSOnlineResource", "/", mWMSOnlineResourceLineEdit->text() );
  QgsProject::instance()->writeEntry( "WMSUrl", "/", mWMSUrlLineEdit->text() );

  // WMS Contact Position
  int contactPositionIndex = mWMSContactPositionCb->currentIndex();
  QString contactPositionText = mWMSContactPositionCb->currentText();
  if ( !contactPositionText.isEmpty() && contactPositionText == mWMSContactPositionCb->itemText( contactPositionIndex ) )
  {
    QgsProject::instance()->writeEntry( "WMSContactPosition", "/", mWMSContactPositionCb->itemData( contactPositionIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSContactPosition", "/", contactPositionText );
  }

  // WMS Fees
  int feesIndex = mWMSFeesCb->currentIndex();
  QString feesText = mWMSFeesCb->currentText();
  if ( !feesText.isEmpty() && feesText == mWMSFeesCb->itemText( feesIndex ) )
  {
    QgsProject::instance()->writeEntry( "WMSFees", "/", mWMSFeesCb->itemData( feesIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSFees", "/", feesText );
  }

  // WMS Access Constraints
  int accessConstraintsIndex = mWMSAccessConstraintsCb->currentIndex();
  QString accessConstraintsText = mWMSAccessConstraintsCb->currentText();
  if ( !accessConstraintsText.isEmpty() && accessConstraintsText == mWMSAccessConstraintsCb->itemText( accessConstraintsIndex ) )
  {
    QgsProject::instance()->writeEntry( "WMSAccessConstraints", "/", mWMSAccessConstraintsCb->itemData( accessConstraintsIndex ).toString() );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSAccessConstraints", "/", accessConstraintsText );
  }

  //WMS keyword list
  QStringList keywordStringList = mWMSKeywordList->text().split( ',' );
  if ( !keywordStringList.isEmpty() )
  {
    keywordStringList.replaceInStrings( QRegExp( "^\\s+" ), "" ).replaceInStrings( QRegExp( "\\s+$" ), "" );
    QgsProject::instance()->writeEntry( "WMSKeywordList", "/", keywordStringList );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSKeywordList", "/" );
  }

  // WMS INSPIRE configuration
  QgsProject::instance()->removeEntry( "WMSInspire", "/" );
  if ( mWMSInspire->isChecked() )
  {
    QgsProject::instance()->writeEntry( "WMSInspire", "/activated", mWMSInspire->isChecked() );

    int inspireLanguageIndex = mWMSInspireLanguage->currentIndex();
    QgsProject::instance()->writeEntry( "WMSInspire", "/language", mWMSInspireLanguage->itemData( inspireLanguageIndex ).toString() );

    if ( mWMSInspireScenario1->isChecked() )
    {
      QgsProject::instance()->writeEntry( "WMSInspire", "/metadataUrl", mWMSInspireMetadataUrl->text() );
      QgsProject::instance()->writeEntry( "WMSInspire", "/metadataUrlType", mWMSInspireMetadataUrlType->currentText() );
    }
    else if ( mWMSInspireScenario2->isChecked() )
    {
      QgsProject::instance()->writeEntry( "WMSInspire", "/temporalReference", mWMSInspireTemporalReference->date().toString( "yyyy-MM-dd" ) );
      QgsProject::instance()->writeEntry( "WMSInspire", "/metadataDate", mWMSInspireMetadataDate->date().toString( "yyyy-MM-dd" ) );
    }
  }

  // WMS GetFeatureInfo geometry precision (decimal places)
  QgsProject::instance()->writeEntry( "WMSPrecision", "/", mWMSPrecisionSpinBox->text() );

  if ( grpWMSExt->isChecked() )
  {
    QgsProject::instance()->writeEntry( "WMSExtent", "/",
                                        QStringList()
                                        << mWMSExtMinX->text()
                                        << mWMSExtMinY->text()
                                        << mWMSExtMaxX->text()
                                        << mWMSExtMaxY->text() );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSExtent", "/" );
  }

  if ( grpWMSList->isChecked() && mWMSList->count() == 0 )
  {
    QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "No coordinate systems selected. Disabling restriction." ) );
    grpWMSList->setChecked( false );
  }

  QgsProject::instance()->removeEntry( "WMSEpsgList", "/" );

  if ( grpWMSList->isChecked() )
  {
    QStringList crslist;
    for ( int i = 0; i < mWMSList->count(); i++ )
    {
      crslist << mWMSList->item( i )->text();
    }

    QgsProject::instance()->writeEntry( "WMSCrsList", "/", crslist );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSCrsList", "/" );
  }

  //WMS composer restrictions
  if ( mWMSComposerGroupBox->isChecked() )
  {
    QStringList composerTitles;
    for ( int i = 0; i < mComposerListWidget->count(); ++i )
    {
      composerTitles << mComposerListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( "WMSRestrictedComposers", "/", composerTitles );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSRestrictedComposers", "/" );
  }

  //WMS layer restrictions
  if ( mLayerRestrictionsGroupBox->isChecked() )
  {
    QStringList layerNames;
    for ( int i = 0; i < mLayerRestrictionsListWidget->count(); ++i )
    {
      layerNames << mLayerRestrictionsListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( "WMSRestrictedLayers", "/", layerNames );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSRestrictedLayers", "/" );
  }

  QgsProject::instance()->writeEntry( "WMSAddWktGeometry", "/", mAddWktGeometryCheckBox->isChecked() );
  QgsProject::instance()->writeEntry( "WMSUseLayerIDs", "/", mWmsUseLayerIDs->isChecked() );

  QString maxWidthText = mMaxWidthLineEdit->text();
  if ( maxWidthText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( "WMSMaxWidth", "/" );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSMaxWidth", "/", maxWidthText.toInt() );
  }
  QString maxHeightText = mMaxHeightLineEdit->text();
  if ( maxHeightText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( "WMSMaxHeight", "/" );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSMaxHeight", "/", maxHeightText.toInt() );
  }

  // WMS Image quality
  int imageQualityValue = mWMSImageQualitySpinBox->value();
  if ( imageQualityValue == 0 )
  {
    QgsProject::instance()->removeEntry( "WMSImageQuality", "/" );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSImageQuality", "/", imageQualityValue );
  }

  QgsProject::instance()->writeEntry( "WFSUrl", "/", mWFSUrlLineEdit->text() );
  QStringList wfsLayerList;
  QStringList wfstUpdateLayerList;
  QStringList wfstInsertLayerList;
  QStringList wfstDeleteLayerList;
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QString id = twWFSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
    QCheckBox* cb;
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      wfsLayerList << id;

      QSpinBox* sb = qobject_cast<QSpinBox *>( twWFSLayers->cellWidget( i, 2 ) );
      QgsProject::instance()->writeEntry( "WFSLayersPrecision", "/" + id, sb->value() );

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
  QgsProject::instance()->writeEntry( "WFSLayers", "/", wfsLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Update", wfstUpdateLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Insert", wfstInsertLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Delete", wfstDeleteLayerList );

  QgsProject::instance()->writeEntry( "WCSUrl", "/", mWCSUrlLineEdit->text() );
  QStringList wcsLayerList;
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QString id = twWCSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
    QCheckBox* cb;
    cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      wcsLayerList << id;
    }
  }
  QgsProject::instance()->writeEntry( "WCSLayers", "/", wcsLayerList );

  // Default Styles
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Marker", cboStyleMarker->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Line", cboStyleLine->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Fill", cboStyleFill->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/ColorRamp", cboStyleColorRamp->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/AlphaInt", ( int )( 255 - ( mTransparencySlider->value() * 2.55 ) ) );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/RandomColors", cbxStyleRandomColors->isChecked() );
  if ( mTreeProjectColors->isDirty() )
  {
    mTreeProjectColors->saveColorsToScheme();
  }

  // store project macros
  QString pythonMacros = ptePythonMacros->text();
  if ( !grpPythonMacros->isChecked() || pythonMacros.isEmpty() )
  {
    pythonMacros = QString::null;
    resetPythonMacros();
  }
  QgsProject::instance()->writeEntry( "Macros", "/pythonCode", pythonMacros );

  QgsProject::instance()->relationManager()->setRelations( mRelationManagerDlg->relations() );

  //save variables
  QgsExpressionContextUtils::setProjectVariables( mVariableEditor->variablesInActiveScope() );

  emit refresh();
}

bool QgsProjectProperties::isProjected()
{
  return cbxProjectionEnabled->isChecked();
}

void QgsProjectProperties::showProjectionsTab()
{
  mOptionsListWidget->setCurrentRow( 1 );
}

void QgsProjectProperties::on_cbxProjectionEnabled_toggled( bool onFlyEnabled )
{
  if ( !onFlyEnabled )
  {
    // reset projection to default
    const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();

    if ( mMapCanvas->currentLayer() )
    {
      mLayerSrsId = mMapCanvas->currentLayer()->crs().srsid();
    }
    else if ( !mapLayers.isEmpty() )
    {
      mLayerSrsId = mapLayers.begin().value()->crs().srsid();
    }
    else
    {
      mLayerSrsId = mProjectSrsId;
    }
    mProjectSrsId = mLayerSrsId;
    projectionSelector->setSelectedCrsId( mLayerSrsId );

    // unset ellipsoid
    mEllipsoidIndex = 0;
  }
  else
  {
    if ( !mLayerSrsId )
    {
      mLayerSrsId = projectionSelector->selectedCrsId();
    }
    projectionSelector->setSelectedCrsId( mProjectSrsId );
  }

  srIdUpdated();

  // Enable/Disable selector and update tool-tip
  updateEllipsoidUI( mEllipsoidIndex ); // maybe already done by setMapUnitsToCurrentProjection
}

void QgsProjectProperties::cbxWFSPubliedStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 1 ) );
  if ( cb && !cb->isChecked() )
  {
    QCheckBox* cbUpdate = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 3 ) );
    if ( cbUpdate )
    {
      cbUpdate->setChecked( false );
    }
    QCheckBox* cbInsert = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 4 ) );
    if ( cbInsert )
    {
      cbInsert->setChecked( false );
    }
    QCheckBox* cbDelete = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 5 ) );
    if ( cbDelete )
    {
      cbDelete->setChecked( false );
    }
  }
}

void QgsProjectProperties::cbxWCSPubliedStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( aIdx, 1 ) );
  if ( cb && !cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( aIdx, 2 ) );
    if ( cbn )
      cbn->setChecked( false );
  }
}

void QgsProjectProperties::updateGuiForMapUnits( QGis::UnitType units )
{
  //make sure map units option is shown in coordinate display combo
  int idx = mCoordinateDisplayComboBox->findData( MapUnits );
  QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( units ) );
  mCoordinateDisplayComboBox->setItemText( idx, mapUnitString );

  //also update unit combo boxes
  idx = mDistanceUnitsCombo->findData( QGis::UnknownUnit );
  if ( idx >= 0 )
  {
    QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( units ) );
    mDistanceUnitsCombo->setItemText( idx, mapUnitString );
  }
  idx = mAreaUnitsCombo->findData( QgsUnitTypes::UnknownAreaUnit );
  if ( idx >= 0 )
  {
    QString mapUnitString = tr( "Map units (%1)" ).arg( QgsUnitTypes::toString( QgsUnitTypes::distanceToAreaUnit( units ) ) );
    mAreaUnitsCombo->setItemText( idx, mapUnitString );
  }
}

void QgsProjectProperties::srIdUpdated()
{
  long myCRSID = projectionSelector->selectedCrsId();
  if ( !isProjected() || !myCRSID )
    return;

  QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( myCRSID );
  //set radio button to crs map unit type
  QGis::UnitType units = srs.mapUnits();

  updateGuiForMapUnits( units );

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

/*!
 * Function to save non-base dialog states
 */
void QgsProjectProperties::saveState()
{
}

/*!
 * Function to restore non-base dialog states
 */
void QgsProjectProperties::restoreState()
{
}

/*!
 * Set WMS default extent to current canvas extent
 */
void QgsProjectProperties::on_pbnWMSExtCanvas_clicked()
{
  QgsRectangle ext = mMapCanvas->extent();
  mWMSExtMinX->setText( qgsDoubleToString( ext.xMinimum() ) );
  mWMSExtMinY->setText( qgsDoubleToString( ext.yMinimum() ) );
  mWMSExtMaxX->setText( qgsDoubleToString( ext.xMaximum() ) );
  mWMSExtMaxY->setText( qgsDoubleToString( ext.yMaximum() ) );
}

void QgsProjectProperties::on_pbnWMSAddSRS_clicked()
{
  QgsGenericProjectionSelector *mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  if ( mWMSList->count() > 0 )
  {
    mySelector->setSelectedAuthId( mWMSList->item( mWMSList->count() - 1 )->text() );
  }
  if ( mySelector->exec() && mySelector->selectedCrsId() != 0 )
  {
    QString authid = mySelector->selectedAuthId();

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

void QgsProjectProperties::on_pbnWMSRemoveSRS_clicked()
{
  Q_FOREACH ( QListWidgetItem *item, mWMSList->selectedItems() )
  {
    delete item;
  }
}

void QgsProjectProperties::on_pbnWMSSetUsedSRS_clicked()
{
  if ( mWMSList->count() > 1 )
  {
    if ( QMessageBox::question( this,
                                tr( "Coordinate System Restrictions" ),
                                tr( "The current selection of coordinate systems will be lost.\nProceed?" ) ) == QMessageBox::No )
      return;
  }

  QSet<QString> crsList;

  if ( cbxProjectionEnabled->isChecked() )
  {
    QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( projectionSelector->selectedCrsId() );
    crsList << srs.authid();
  }

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    crsList << it.value()->crs().authid();
  }

  mWMSList->clear();
  mWMSList->addItems( crsList.values() );
}

void QgsProjectProperties::on_mAddWMSComposerButton_clicked()
{
  QSet<QgsComposer*> projectComposers = QgisApp::instance()->printComposers();
  QStringList composerTitles;
  QSet<QgsComposer*>::const_iterator cIt = projectComposers.constBegin();
  for ( ; cIt != projectComposers.constEnd(); ++cIt )
  {
    composerTitles << ( *cIt )->title();
  }

  bool ok;
  QString name = QInputDialog::getItem( this, tr( "Select print composer" ), tr( "Composer Title" ), composerTitles, 0, false, &ok );
  if ( ok )
  {
    if ( mComposerListWidget->findItems( name, Qt::MatchExactly ).size() < 1 )
    {
      mComposerListWidget->addItem( name );
    }
  }
}

void QgsProjectProperties::on_mRemoveWMSComposerButton_clicked()
{
  QListWidgetItem* currentItem = mComposerListWidget->currentItem();
  if ( currentItem )
  {
    delete mComposerListWidget->takeItem( mComposerListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::on_mAddLayerRestrictionButton_clicked()
{
  QgsProjectLayerGroupDialog d( this, QgsProject::instance()->fileName() );
  d.setWindowTitle( tr( "Select restricted layers and groups" ) );
  if ( d.exec() == QDialog::Accepted )
  {
    QStringList layerNames = d.selectedLayerNames();
    QStringList::const_iterator layerIt = layerNames.constBegin();
    for ( ; layerIt != layerNames.constEnd(); ++layerIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *layerIt, Qt::MatchExactly ).size() < 1 )
      {
        mLayerRestrictionsListWidget->addItem( *layerIt );
      }
    }

    QStringList groups = d.selectedGroups();
    QStringList::const_iterator groupIt = groups.constBegin();
    for ( ; groupIt != groups.constEnd(); ++groupIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *groupIt, Qt::MatchExactly ).size() < 1 )
      {
        mLayerRestrictionsListWidget->addItem( *groupIt );
      }
    }
  }
}

void QgsProjectProperties::on_mRemoveLayerRestrictionButton_clicked()
{
  QListWidgetItem* currentItem = mLayerRestrictionsListWidget->currentItem();
  if ( currentItem )
  {
    delete mLayerRestrictionsListWidget->takeItem( mLayerRestrictionsListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::on_pbnWFSLayersSelectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( true );
  }
}

void QgsProjectProperties::on_mWMSInspireScenario1_toggled( bool on )
{
  mWMSInspireScenario2->blockSignals( true );
  mWMSInspireScenario2->setChecked( !on );
  mWMSInspireScenario2->blockSignals( false );
}

void QgsProjectProperties::on_mWMSInspireScenario2_toggled( bool on )
{
  mWMSInspireScenario1->blockSignals( true );
  mWMSInspireScenario1->setChecked( !on );
  mWMSInspireScenario1->blockSignals( false );
}

void QgsProjectProperties::on_pbnWFSLayersUnselectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( false );
  }
}

void QgsProjectProperties::on_pbnWCSLayersSelectAll_clicked()
{
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    cb->setChecked( true );
  }
}

void QgsProjectProperties::on_pbnWCSLayersUnselectAll_clicked()
{
  for ( int i = 0; i < twWCSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWCSLayers->cellWidget( i, 1 ) );
    cb->setChecked( false );
  }
}

void QgsProjectProperties::on_pbnLaunchOWSChecker_clicked()
{
  QString myStyle = QgsApplication::reportStyleSheet();
  teOWSChecker->clear();
  teOWSChecker->document()->setDefaultStyleSheet( myStyle );
  teOWSChecker->setHtml( "<h1>" + tr( "Start checking QGIS Server" ) + "</h1>" );

  QStringList owsNames, encodingMessages;
  checkOWS( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup(), owsNames, encodingMessages );

  QStringList duplicateNames, regExpMessages;
  QRegExp snRegExp = QgsApplication::shortNameRegExp();
  Q_FOREACH ( const QString& name, owsNames )
  {
    if ( !snRegExp.exactMatch( name ) )
      regExpMessages << tr( "Use short name for \"%1\"" ).arg( name );
    if ( duplicateNames.contains( name ) )
      continue;
    if ( owsNames.count( name ) > 1 )
      duplicateNames << name;
  }

  if ( duplicateNames.size() != 0 )
  {
    QString nameMessage = "<h1>" + tr( "Some layers and groups have the same name or short name" ) + "</h1>";
    nameMessage += "<h2>" + tr( "Duplicate names:" ) + "</h2>";
    nameMessage += duplicateNames.join( "</li><li>" ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + nameMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All names and short names of layer and group are unique" ) + "</h1>" );
  }

  if ( regExpMessages.size() != 0 )
  {
    QString encodingMessage = "<h1>" + tr( "Some layer short names have to be updated:" ) + "</h1><ul><li>" + regExpMessages.join( "</li><li>" ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + encodingMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All layer short names are well formed" ) + "</h1>" );
  }

  if ( encodingMessages.size() != 0 )
  {
    QString encodingMessage = "<h1>" + tr( "Some layer encodings are not set:" ) + "</h1><ul><li>" + encodingMessages.join( "</li><li>" ) + "</li></ul>";
    teOWSChecker->setHtml( teOWSChecker->toHtml() + encodingMessage );
  }
  else
  {
    teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "All layer encodings are set" ) + "</h1>" );
  }

  teOWSChecker->setHtml( teOWSChecker->toHtml() + "<h1>" + tr( "Start checking QGIS Server" ) + "</h1>" );
}

void QgsProjectProperties::on_pbnAddScale_clicked()
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
    lstScales->setCurrentItem( newItem );
  }
}

void QgsProjectProperties::on_pbnRemoveScale_clicked()
{
  int currentRow = lstScales->currentRow();
  QListWidgetItem* itemToRemove = lstScales->takeItem( currentRow );
  delete itemToRemove;
}

void QgsProjectProperties::on_pbnImportScales_clicked()
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

void QgsProjectProperties::on_pbnExportScales_clicked()
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
  // Styles - taken from qgsstylev2managerdialog

  // use QComboBox and QString lists for shorter code
  QStringList prefList;
  QList<QComboBox*> cboList;
  cboList << cboStyleMarker;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Marker", "" );
  cboList << cboStyleLine;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Line", "" );
  cboList << cboStyleFill;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Fill", "" );
  cboList << cboStyleColorRamp;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/ColorRamp", "" );
  for ( int i = 0; i < cboList.count(); i++ )
  {
    cboList[i]->clear();
    cboList[i]->addItem( "" );
  }

  // populate symbols
  QStringList symbolNames = mStyle->symbolNames();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    QgsSymbolV2* symbol = mStyle->symbol( name );
    QComboBox* cbo = nullptr;
    switch ( symbol->type() )
    {
      case QgsSymbolV2::Marker :
        cbo = cboStyleMarker;
        break;
      case QgsSymbolV2::Line :
        cbo = cboStyleLine;
        break;
      case QgsSymbolV2::Fill :
        cbo = cboStyleFill;
        break;
      case QgsSymbolV2::Hybrid:
        // Shouldn't get here
        break;
    }
    if ( cbo )
    {
      QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, cbo->iconSize() );
      cbo->addItem( icon, name );
    }
    delete symbol;
  }

  // populate color ramps
  QStringList colorRamps = mStyle->colorRampNames();
  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, cboStyleColorRamp->iconSize() );
    cboStyleColorRamp->addItem( icon, name );
    delete ramp;
  }

  // set current index if found
  for ( int i = 0; i < cboList.count(); i++ )
  {
    int index = cboList[i]->findText( prefList[i], Qt::MatchCaseSensitive );
    if ( index >= 0 )
      cboList[i]->setCurrentIndex( index );
  }

  // random colors
  cbxStyleRandomColors->setChecked( QgsProject::instance()->readBoolEntry( "DefaultStyles", "/RandomColors", true ) );

  // alpha transparency
  int transparencyInt = ( 255 - QgsProject::instance()->readNumEntry( "DefaultStyles", "/AlphaInt", 255 ) ) / 2.55;
  mTransparencySlider->setValue( transparencyInt );
}

void QgsProjectProperties::on_pbtnStyleManager_clicked()
{
  QgsStyleV2ManagerDialog dlg( mStyle, this );
  dlg.exec();
  populateStyles();
}

void QgsProjectProperties::on_pbtnStyleMarker_clicked()
{
  editSymbol( cboStyleMarker );
}

void QgsProjectProperties::on_pbtnStyleLine_clicked()
{
  editSymbol( cboStyleLine );
}

void QgsProjectProperties::on_pbtnStyleFill_clicked()
{
  editSymbol( cboStyleFill );
}

void QgsProjectProperties::on_pbtnStyleColorRamp_clicked()
{
  // TODO for now just open style manager
  // code in QgsStyleV2ManagerDialog::editColorRamp()
  on_pbtnStyleManager_clicked();
}

void QgsProjectProperties::on_mTransparencySlider_valueChanged( int value )
{
  mTransparencySpinBox->blockSignals( true );
  mTransparencySpinBox->setValue( value );
  mTransparencySpinBox->blockSignals( false );
}

void QgsProjectProperties::on_mTransparencySpinBox_valueChanged( int value )
{
  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value );
  mTransparencySlider->blockSignals( false );
}

void QgsProjectProperties::editSymbol( QComboBox* cbo )
{
  QString symbolName = cbo->currentText();
  if ( symbolName.isEmpty() )
  {
    QMessageBox::information( this, "", tr( "Select a valid symbol" ) );
    return;
  }
  QgsSymbolV2* symbol = mStyle->symbol( symbolName );
  if ( ! symbol )
  {
    QMessageBox::warning( this, "", tr( "Invalid symbol : " ) + symbolName );
    return;
  }

  // let the user edit the symbol and update list when done
  QgsSymbolV2SelectorDialog dlg( symbol, mStyle, nullptr, this );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return;
  }

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol );

  // update icon
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, cbo->iconSize() );
  cbo->setItemIcon( cbo->currentIndex(), icon );
}

void QgsProjectProperties::resetPythonMacros()
{
  grpPythonMacros->setChecked( false );
  ptePythonMacros->setText( "def openProject():\n    pass\n\n" \
                            "def saveProject():\n    pass\n\n" \
                            "def closeProject():\n    pass\n" );
}

void QgsProjectProperties::checkOWS( QgsLayerTreeGroup* treeGroup, QStringList& owsNames, QStringList& encodingMessages )
{
  QList< QgsLayerTreeNode * > treeGroupChildren = treeGroup->children();
  for ( int i = 0; i < treeGroupChildren.size(); ++i )
  {
    QgsLayerTreeNode* treeNode = treeGroupChildren.at( i );
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup* treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      QString shortName = treeGroupChild->customProperty( "wmsShortName" ).toString();
      if ( shortName.isEmpty() )
        owsNames << treeGroupChild->name();
      else
        owsNames << shortName;
      checkOWS( treeGroupChild, owsNames, encodingMessages );
    }
    else
    {
      QgsLayerTreeLayer* treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer* l = treeLayer->layer();
      QString shortName = l->shortName();
      if ( shortName.isEmpty() )
        owsNames << l->name();
      else
        owsNames << shortName;
      if ( l->type() == QgsMapLayer::VectorLayer )
      {
        QgsVectorLayer* vl = static_cast<QgsVectorLayer *>( l );
        if ( vl->dataProvider()->encoding() == "System" )
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
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  EllipsoidDefs myItem;

  myItem.acronym = GEO_NONE;
  myItem.description =  tr( GEO_NONE_DESC );
  myItem.semiMajor = 0.0;
  myItem.semiMinor = 0.0;
  mEllipsoidList.append( myItem );

  myItem.acronym = QLatin1String( "PARAMETER:6370997:6370997" );
  myItem.description = tr( "Parameters:" );
  myItem.semiMajor = 6370997.0;
  myItem.semiMinor = 6370997.0;
  mEllipsoidList.append( myItem );

  //check the db is available
  myResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase, SQLITE_OPEN_READONLY, nullptr );
  if ( myResult )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( myResult == 0 );
  }

  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym, name, radius, parameter2 from tbl_ellipsoid order by name";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    while ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString para1, para2;
      myItem.acronym = ( const char * )sqlite3_column_text( myPreparedStatement, 0 );
      myItem.description = ( const char * )sqlite3_column_text( myPreparedStatement, 1 );

      // Copied from QgsDistanecArea. Should perhaps be moved there somehow?
      // No error checking, this values are for show only, never used in calculations.

      // Fall-back values
      myItem.semiMajor = 0.0;
      myItem.semiMinor = 0.0;
      // Crash if no column?
      para1 = ( const char * )sqlite3_column_text( myPreparedStatement, 2 );
      para2 = ( const char * )sqlite3_column_text( myPreparedStatement, 3 );
      myItem.semiMajor = para1.mid( 2 ).toDouble();
      if ( para2.left( 2 ) == "b=" )
      {
        myItem.semiMinor = para2.mid( 2 ).toDouble();
      }
      else if ( para2.left( 3 ) == "rf=" )
      {
        double invFlattening = para2.mid( 3 ).toDouble();
        if ( invFlattening != 0.0 )
        {
          myItem.semiMinor = myItem.semiMajor - ( myItem.semiMajor / invFlattening );
        }
      }
      mEllipsoidList.append( myItem );
    }
  }

  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  // Add all items to selector

  Q_FOREACH ( const EllipsoidDefs& i, mEllipsoidList )
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
  double myMajor =  mEllipsoidList.at( newIndex ).semiMajor;
  double myMinor =  mEllipsoidList.at( newIndex ).semiMinor;

  // If user has modified the radii (only possible if parametric!), before
  // changing ellipsoid, save the modified coordinates
  if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
  {
    QgsDebugMsg( "Saving major/minor" );
    mEllipsoidList[ mEllipsoidIndex ].semiMajor = QLocale::system().toDouble( leSemiMajor->text() );
    mEllipsoidList[ mEllipsoidIndex ].semiMinor = QLocale::system().toDouble( leSemiMinor->text() );
  }

  mEllipsoidIndex = newIndex;
  leSemiMajor->setEnabled( false );
  leSemiMinor->setEnabled( false );
  leSemiMajor->setText( "" );
  leSemiMinor->setText( "" );
  if ( cbxProjectionEnabled->isChecked() )
  {
    cmbEllipsoid->setEnabled( true );
    cmbEllipsoid->setToolTip( "" );
    if ( mEllipsoidList.at( mEllipsoidIndex ).acronym.startsWith( "PARAMETER:" ) )
    {
      leSemiMajor->setEnabled( true );
      leSemiMinor->setEnabled( true );
    }
    else
    {
      leSemiMajor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Parameters:" ) ) );
      leSemiMinor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Parameters:" ) ) );
    }
    if ( mEllipsoidList[ mEllipsoidIndex ].acronym != GEO_NONE )
    {
      leSemiMajor->setText( QLocale::system().toString( myMajor, 'f', 3 ) );
      leSemiMinor->setText( QLocale::system().toString( myMinor, 'f', 3 ) );
    }
  }
  else
  {
    cmbEllipsoid->setEnabled( false );
    cmbEllipsoid->setToolTip( tr( "Can only use ellipsoidal calculations when CRS transformation is enabled" ) );
  }
  cmbEllipsoid->setCurrentIndex( mEllipsoidIndex ); // Not always necessary
}

void QgsProjectProperties::projectionSelectorInitialized()
{
  QgsDebugMsg( "Setting up ellipsoid" );

  // Reading ellipsoid from setttings
  QStringList mySplitEllipsoid = QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ).split( ':' );

  int myIndex = 0;
  for ( int i = 0; i < mEllipsoidList.length(); i++ )
  {
    if ( mEllipsoidList.at( i ).acronym.startsWith( mySplitEllipsoid[ 0 ] ) )
    {
      myIndex = i;
      break;
    }
  }

  // Update paramaters if present.
  if ( mySplitEllipsoid.length() >= 3 )
  {
    mEllipsoidList[ myIndex ].semiMajor =  mySplitEllipsoid[ 1 ].toDouble();
    mEllipsoidList[ myIndex ].semiMinor =  mySplitEllipsoid[ 2 ].toDouble();
  }

  updateEllipsoidUI( myIndex );
}

void QgsProjectProperties::on_mButtonAddColor_clicked()
{
  QColor newColor = QgsColorDialogV2::getColor( QColor(), this->parentWidget(), tr( "Select Color" ), true );
  if ( !newColor.isValid() )
  {
    return;
  }
  activateWindow();

  mTreeProjectColors->addColor( newColor, QgsSymbolLayerV2Utils::colorToName( newColor ) );
}

void QgsProjectProperties::on_mButtonImportColors_clicked()
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
  bool importOk = mTreeProjectColors->importColorsFromGpl( file );
  if ( !importOk )
  {
    QMessageBox::critical( nullptr, tr( "Invalid file" ), tr( "Error, no colors found in palette file" ) );
    return;
  }
}

void QgsProjectProperties::on_mButtonExportColors_clicked()
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
  bool exportOk = mTreeProjectColors->exportColorsToGpl( file );
  if ( !exportOk )
  {
    QMessageBox::critical( nullptr, tr( "Error exporting" ), tr( "Error writing palette file" ) );
    return;
  }
}

QListWidgetItem* QgsProjectProperties::addScaleToScaleList( const QString &newScale )
{
  // TODO QGIS3: Rework the scale list widget to be a reusable piece of code, see PR #2558
  QListWidgetItem* newItem = new QListWidgetItem( newScale );
  addScaleToScaleList( newItem );
  return newItem;
}

void QgsProjectProperties::addScaleToScaleList( QListWidgetItem* newItem )
{
  // If the new scale already exists, delete it.
  QListWidgetItem* duplicateItem = lstScales->findItems( newItem->text(), Qt::MatchExactly ).value( 0 );
  delete duplicateItem;

  int newDenominator = newItem->text().split( ":" ).value( 1 ).toInt();
  int i;
  for ( i = 0; i < lstScales->count(); i++ )
  {
    int denominator = lstScales->item( i )->text().split( ":" ).value( 1 ).toInt();
    if ( newDenominator > denominator )
      break;
  }

  newItem->setData( Qt::UserRole, newItem->text() );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  lstScales->insertItem( i, newItem );
}

void QgsProjectProperties::scaleItemChanged( QListWidgetItem* changedScaleItem )
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
  int row = lstScales->row( changedScaleItem );
  lstScales->takeItem( row );
  addScaleToScaleList( changedScaleItem );
  lstScales->setCurrentItem( changedScaleItem );
}
