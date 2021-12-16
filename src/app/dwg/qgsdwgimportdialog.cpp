/***************************************************************************
                         qgsdwgimportdialog.cpp
                         ----------------------
    begin                : May 2016
    copyright            : (C) 2016 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdwgimportdialog.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsdwgimporter.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsfeatureiterator.h"
#include "qgslayertreeview.h"
#include "qgslayertreemodel.h"
#include "qgslayertreegroup.h"
#include "qgsrenderer.h"
#include "qgsnullsymbolrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgspallabeling.h"
#include "qgsmapcanvas.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgsproperty.h"
#include "qgslayertree.h"
#include "qgsguiutils.h"
#include "qgsfilewidget.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

QgsDwgImportDialog::QgsDwgImportDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  mDatabaseFileWidget->setStorageMode( QgsFileWidget::SaveFile );
  mDatabaseFileWidget->setConfirmOverwrite( false );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDwgImportDialog::buttonBox_accepted );
  connect( mDatabaseFileWidget, &QgsFileWidget::fileChanged, this, &QgsDwgImportDialog::mDatabaseFileWidget_textChanged );
  connect( pbBrowseDrawing, &QPushButton::clicked, this, &QgsDwgImportDialog::pbBrowseDrawing_clicked );
  connect( pbImportDrawing, &QPushButton::clicked, this, &QgsDwgImportDialog::pbImportDrawing_clicked );
  connect( pbLoadDatabase, &QPushButton::clicked, this, &QgsDwgImportDialog::pbLoadDatabase_clicked );
  connect( pbSelectAll, &QPushButton::clicked, this, &QgsDwgImportDialog::pbSelectAll_clicked );
  connect( pbDeselectAll, &QPushButton::clicked, this, &QgsDwgImportDialog::pbDeselectAll_clicked );
  connect( leLayerGroup, &QLineEdit::textChanged, this, &QgsDwgImportDialog::leLayerGroup_textChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDwgImportDialog::showHelp );

  const QgsSettings s;
  cbExpandInserts->setChecked( s.value( QStringLiteral( "/DwgImport/lastExpandInserts" ), true ).toBool() );
  cbMergeLayers->setChecked( s.value( QStringLiteral( "/DwgImport/lastMergeLayers" ), false ).toBool() );
  cbUseCurves->setChecked( s.value( QStringLiteral( "/DwgImport/lastUseCurves" ), true ).toBool() );
  mDatabaseFileWidget->setDefaultRoot( s.value( QStringLiteral( "/DwgImport/lastDirDatabase" ), QDir::homePath() ).toString() );

  leDrawing->setReadOnly( true );
  pbImportDrawing->setHidden( true );
  lblMessage->setHidden( true );

  const int crsid = s.value( QStringLiteral( "/DwgImport/lastCrs" ), QString::number( QgsProject::instance()->crs().srsid() ) ).toInt();

  mCrsSelector->setShowAccuracyWarnings( true );
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrsId( crsid );
  mCrsSelector->setCrs( crs );
  mCrsSelector->setLayerCrs( crs );
  mCrsSelector->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                "The data points will be transformed from the layer coordinate reference system." ) );


  if ( ! QgsVectorFileWriter::supportedFormatExtensions().contains( QStringLiteral( "gpkg" ) ) )
  {
    bar->pushMessage( tr( "GDAL/OGR not built with GPKG (sqlite3) support. You will not be able to export the DWG in a GPKG." ), Qgis::MessageLevel::Critical );
  }
  pbLoadDatabase_clicked();
  updateUI();
}

QgsDwgImportDialog::~QgsDwgImportDialog()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/DwgImport/lastExpandInserts" ), cbExpandInserts->isChecked() );
  s.setValue( QStringLiteral( "/DwgImport/lastMergeLayers" ), cbMergeLayers->isChecked() );
  s.setValue( QStringLiteral( "/DwgImport/lastUseCurves" ), cbUseCurves->isChecked() );
}

void QgsDwgImportDialog::updateUI()
{
  bool dbAvailable = false;
  bool dbReadable = false;
  bool dwgReadable = false;

  if ( !mDatabaseFileWidget->filePath().isEmpty() )
  {
    const QFileInfo fi( mDatabaseFileWidget->filePath() );
    dbAvailable = fi.exists() ? fi.isWritable() : QFileInfo( fi.path() ).isWritable();
    dbReadable = fi.exists() && fi.isReadable();
  }

  if ( !leDrawing->text().isEmpty() )
  {
    const QFileInfo fi( leDrawing->text() );
    dwgReadable = fi.exists() && fi.isReadable();
  }

  pbImportDrawing->setEnabled( dbAvailable && dwgReadable );
  pbImportDrawing->setVisible( dbAvailable && dwgReadable );
  pbLoadDatabase->setEnabled( dbReadable );
  pbBrowseDrawing->setEnabled( dbAvailable );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mLayers->rowCount() > 0 && !leLayerGroup->text().isEmpty() );
}

void QgsDwgImportDialog::mDatabaseFileWidget_textChanged( const QString &filename )
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/DwgImport/lastDirDatabase" ), QFileInfo( filename ).canonicalPath() );
  updateUI();
}

void QgsDwgImportDialog::leLayerGroup_textChanged( const QString &text )
{
  Q_UNUSED( text )
  updateUI();
}

void QgsDwgImportDialog::pbLoadDatabase_clicked()
{
  if ( !QFileInfo::exists( mDatabaseFileWidget->filePath() ) )
    return;

  const QgsTemporaryCursorOverride waitCursor( Qt::BusyCursor );

  bool lblVisible = false;

  QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  options.loadDefaultStyle = false;
  std::unique_ptr<QgsVectorLayer> d( new QgsVectorLayer( QStringLiteral( "%1|layername=drawing" ).arg( mDatabaseFileWidget->filePath() ), QStringLiteral( "layers" ), QStringLiteral( "ogr" ), options ) );
  if ( d && d->isValid() )
  {
    const int idxPath = d->fields().lookupField( QStringLiteral( "path" ) );
    const int idxLastModified = d->fields().lookupField( QStringLiteral( "lastmodified" ) );
    const int idxCrs = d->fields().lookupField( QStringLiteral( "crs" ) );

    QgsFeature f;
    if ( d->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << idxPath << idxLastModified << idxCrs ) ).nextFeature( f ) )
    {
      leDrawing->setText( f.attribute( idxPath ).toString() );

      QgsCoordinateReferenceSystem crs;
      crs.createFromSrsId( f.attribute( idxCrs ).toInt() );
      mCrsSelector->setCrs( crs );
      mCrsSelector->setLayerCrs( crs );

      const QFileInfo fi( leDrawing->text() );
      if ( fi.exists() )
      {
        if ( fi.lastModified() > f.attribute( idxLastModified ).toDateTime() )
        {
          lblMessage->setText( tr( "Drawing file was meanwhile updated (%1 > %2)." ).arg( fi.lastModified().toString(), f.attribute( idxLastModified ).toDateTime().toString() ) );
          lblVisible = true;
        }
      }
      else
      {
        lblMessage->setText( tr( "Drawing file unavailable." ) );
        lblVisible = true;
      }
    }
  }

  lblMessage->setVisible( lblVisible );

  std::unique_ptr<QgsVectorLayer> l( new QgsVectorLayer( QStringLiteral( "%1|layername=layers" ).arg( mDatabaseFileWidget->filePath() ), QStringLiteral( "layers" ), QStringLiteral( "ogr" ), options ) );
  if ( l && l->isValid() )
  {
    const int idxName = l->fields().lookupField( QStringLiteral( "name" ) );
    const int idxColor = l->fields().lookupField( QStringLiteral( "ocolor" ) );
    const int idxFlags = l->fields().lookupField( QStringLiteral( "flags" ) );

    QgsDebugMsg( QStringLiteral( "idxName:%1 idxColor:%2 idxFlags:%3" ).arg( idxName ).arg( idxColor ).arg( idxFlags ) );

    QgsFeatureIterator fit = l->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << idxName << idxColor << idxFlags ) );
    QgsFeature f;

    mLayers->setRowCount( 0 );

    while ( fit.nextFeature( f ) )
    {
      const int row = mLayers->rowCount();
      mLayers->setRowCount( row + 1 );

      QgsDebugMsg( QStringLiteral( "name:%1 color:%2 flags:%3" ).arg( f.attribute( idxName ).toString() ).arg( f.attribute( idxColor ).toInt() ).arg( f.attribute( idxFlags ).toString(), 0, 16 ) );

      QTableWidgetItem *item = nullptr;
      item = new QTableWidgetItem( f.attribute( idxName ).toString() );
      item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      item->setCheckState( Qt::Checked );
      mLayers->setItem( row, 0, item );

      item = new QTableWidgetItem();
      item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      item->setCheckState( ( f.attribute( idxColor ).toInt() >= 0 && ( f.attribute( idxFlags ).toInt() & 1 ) == 0 ) ? Qt::Checked : Qt::Unchecked );
      mLayers->setItem( row, 1, item );
    }

    mLayers->resizeColumnsToContents();

    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mLayers->rowCount() > 0 && !leLayerGroup->text().isEmpty() );
  }
  else
  {
    bar->pushMessage( tr( "Could not open layer list" ), Qgis::MessageLevel::Critical );
  }
}

void QgsDwgImportDialog::pbBrowseDrawing_clicked()
{
  const QString dir( leDrawing->text().isEmpty() ? QDir::homePath() : QFileInfo( leDrawing->text() ).canonicalPath() );
  const QString filename = QFileDialog::getOpenFileName( nullptr, tr( "Select DWG/DXF file" ), dir, tr( "DXF/DWG files" ) + " (*.dwg *.DWG *.dxf *.DXF)" );
  if ( filename.isEmpty() )
    return;

  leDrawing->setText( filename );

  pbImportDrawing_clicked();
}

void QgsDwgImportDialog::pbImportDrawing_clicked()
{
  const QgsTemporaryCursorOverride waitCursor( Qt::BusyCursor );

  QgsDwgImporter importer( mDatabaseFileWidget->filePath(), mCrsSelector->crs() );

  lblMessage->setVisible( true );

  QString error;
  if ( importer.import( leDrawing->text(), error, cbExpandInserts->isChecked(), cbUseCurves->isChecked(), lblMessage ) )
  {
    bar->pushMessage( tr( "Drawing import completed." ), Qgis::MessageLevel::Info );
  }
  else
  {
    bar->pushMessage( tr( "Drawing import failed (%1)" ).arg( error ), Qgis::MessageLevel::Critical );
  }

  pbLoadDatabase_clicked();
}

QgsVectorLayer *QgsDwgImportDialog::layer( QgsLayerTreeGroup *layerGroup, const QString &layerFilter, const QString &table )
{
  QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  options.loadDefaultStyle = false;
  QgsVectorLayer *l = new QgsVectorLayer( QStringLiteral( "%1|layername=%2" ).arg( mDatabaseFileWidget->filePath(), table ), table, QStringLiteral( "ogr" ), options );
  l->setSubsetString( QStringLiteral( "%1space=0 AND block=-1" ).arg( layerFilter ) );

  if ( l->featureCount() == 0 )
  {
    delete l;
    return nullptr;
  }

  QgsProject::instance()->addMapLayer( l, false );
  layerGroup->addLayer( l );
  return l;
}

void QgsDwgImportDialog::createGroup( QgsLayerTreeGroup *group, const QString &name, const QStringList &layers, bool visible )
{
  QgsLayerTreeGroup *layerGroup = group->addGroup( name );
  QgsDebugMsg( QStringLiteral( " %1" ).arg( name ) ) ;
  Q_ASSERT( layerGroup );

  QString layerFilter;
  if ( !layers.isEmpty() )
  {
    QStringList exprlist;
    const auto constLayers = layers;
    for ( QString layer : constLayers )
    {
      exprlist.append( QStringLiteral( "'%1'" ).arg( layer.replace( QLatin1String( "'" ), QLatin1String( "''" ) ) ) );
    }
    layerFilter = QStringLiteral( "layer IN (%1) AND " ).arg( exprlist.join( QLatin1Char( ',' ) ) );
  }

  QgsVectorLayer *l = nullptr;
  QgsSymbol *sym = nullptr;

  l = layer( layerGroup, layerFilter, QStringLiteral( "hatches" ) );
  if ( l )
  {
    QgsSimpleFillSymbolLayer *sfl = new QgsSimpleFillSymbolLayer();
    sfl->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromField( QStringLiteral( "color" ) ) );
    sfl->setStrokeStyle( Qt::NoPen );
    sym = new QgsFillSymbol();
    sym->changeSymbolLayer( 0, sfl );
    l->setRenderer( new QgsSingleSymbolRenderer( sym ) );
  }

  l = layer( layerGroup, layerFilter, QStringLiteral( "lines" ) );
  if ( l )
  {
    QgsSimpleLineSymbolLayer *sll = new QgsSimpleLineSymbolLayer();
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromField( QStringLiteral( "color" ) ) );
    sll->setPenJoinStyle( Qt::MiterJoin );
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty::fromField( QStringLiteral( "linewidth" ) ) );
    // sll->setUseCustomDashPattern( true );
    // sll->setCustomDashPatternUnit( QgsSymbolV2::MapUnit );
    // sll->setDataDefinedProperty( QgsSymbolLayer::PropertyCustomDash, QgsProperty::fromField( "linetype" ) );
    sym = new QgsLineSymbol();
    sym->changeSymbolLayer( 0, sll );
    sym->setOutputUnit( QgsUnitTypes::RenderMillimeters );
    l->setRenderer( new QgsSingleSymbolRenderer( sym ) );
  }

  l = layer( layerGroup, layerFilter, QStringLiteral( "polylines" ) );
  if ( l )
  {
    sym = new QgsLineSymbol();

    QgsSimpleLineSymbolLayer *sll = new QgsSimpleLineSymbolLayer();
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromField( QStringLiteral( "color" ) ) );
    sll->setPenJoinStyle( Qt::MiterJoin );
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty::fromField( QStringLiteral( "width" ) ) );
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyLayerEnabled, QgsProperty::fromExpression( QStringLiteral( "width>0" ) ) );
    sll->setOutputUnit( QgsUnitTypes::RenderMapUnits );
    // sll->setUseCustomDashPattern( true );
    // sll->setCustomDashPatternUnit( QgsSymbolV2::MapUnit );
    // sll->setDataDefinedProperty( QgsSymbolLayer::PropertyCustomDash, QgsProperty::fromField( "linetype" ) );
    sym->changeSymbolLayer( 0, sll );

    sll = new QgsSimpleLineSymbolLayer();
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromField( QStringLiteral( "color" ) ) );
    sll->setPenJoinStyle( Qt::MiterJoin );
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty::fromField( QStringLiteral( "linewidth" ) ) );
    sll->setDataDefinedProperty( QgsSymbolLayer::PropertyLayerEnabled, QgsProperty::fromExpression( QStringLiteral( "width=0" ) ) );
    sll->setOutputUnit( QgsUnitTypes::RenderMillimeters );
    sym->appendSymbolLayer( sll );

    l->setRenderer( new QgsSingleSymbolRenderer( sym ) );
  }

  l = layer( layerGroup, layerFilter, QStringLiteral( "texts" ) );
  if ( l )
  {
    l->setRenderer( new QgsNullSymbolRenderer() );

    QgsTextFormat tf;
    tf.setSizeUnit( QgsUnitTypes::RenderMapUnits );

    QgsPalLayerSettings pls;
    pls.setFormat( tf );

    pls.drawLabels = true;
    pls.fieldName = QStringLiteral( "text" );
    pls.wrapChar = QStringLiteral( "\\P" );

    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::Size, QgsProperty::fromField( QStringLiteral( "height" ) ) );
    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::Color, QgsProperty::fromField( QStringLiteral( "color" ) ) );
    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::MultiLineHeight, QgsProperty::fromExpression( QStringLiteral( "CASE WHEN interlin<0 THEN 1 ELSE interlin*1.5 END" ) ) );
    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromExpression( QStringLiteral( "$x" ) ) );
    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromExpression( QStringLiteral( "$y" ) ) );

    // DXF TEXT
    // vertical: 0 = Base, 1 = Bottom, 2 = Middle, 3 = Top,  default Base
    // horizontal: 0 = Left, 1 = Center, 2 = Right, 3 = Aligned (if Base), 4 = Middle (if Base), default Left

    // DXF MTEXT
    // 1 = Top left;    2 = Top center;    3 = Top right
    // 4 = Middle left; 5 = Middle center; 6 = Middle right
    // 7 = Bottom left; 8 = Bottom center; 9 = Bottom right

    // QGIS Quadrant
    // 0 QuadrantAboveLeft, 1 QuadrantAbove, 2 QuadrantAboveRight,
    // 3 QuadrantLeft,      4 QuadrantOver,  5 QuadrantRight,
    // 6 QuadrantBelowLeft, 7 QuadrantBelow, 8 QuadrantBelowRight,

    pls.dataDefinedProperties().setProperty(
      QgsPalLayerSettings::Hali,
      QgsProperty::fromExpression( QStringLiteral(
                                     "CASE"
                                     " WHEN etype=%1 THEN"
                                     " CASE"
                                     " WHEN textgen % 3=2 THEN 'Center'"
                                     " WHEN textgen % 3=0 THEN 'Right'"
                                     " ELSE 'Left'"
                                     " END"
                                     " ELSE"
                                     " CASE"
                                     " WHEN alignh=1 THEN 'Center'"
                                     " WHEN alignh=2 THEN 'Right'"
                                     " ELSE 'Left'"
                                     " END"
                                     " END"
                                   ).arg( DRW::MTEXT )
                                 )
    );

    pls.dataDefinedProperties().setProperty(
      QgsPalLayerSettings::Vali,
      QgsProperty::fromExpression( QStringLiteral(
                                     "CASE"
                                     " WHEN etype=%1 THEN"
                                     " CASE"
                                     " WHEN textgen<4 THEN 'Top'"
                                     " WHEN textgen<7 THEN 'Half'"
                                     " ELSE 'Bottom'"
                                     " END"
                                     " ELSE"
                                     " CASE"
                                     " WHEN alignv=1 THEN 'Bottom'"
                                     " WHEN alignv=2 THEN 'Half'"
                                     " WHEN alignv=3 THEN 'Top'"
                                     " ELSE 'Base'"
                                     " END"
                                     " END"
                                   ).arg( DRW::MTEXT )
                                 )
    );

    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromExpression( QStringLiteral( "360-angle" ) ) );
    pls.dataDefinedProperties().setProperty( QgsPalLayerSettings::AlwaysShow, QgsProperty::fromExpression( QStringLiteral( "1" ) ) );

    l->setLabeling( new QgsVectorLayerSimpleLabeling( pls ) );
    l->setLabelsEnabled( true );
  }

  l = layer( layerGroup, layerFilter, QStringLiteral( "points" ) );
  if ( l )
  {
    // FIXME: use PDMODE?
    l->setRenderer( new QgsNullSymbolRenderer() );
  }

  if ( !cbExpandInserts->isChecked() )
  {
    l = layer( layerGroup, layerFilter, QStringLiteral( "inserts" ) );
    if ( l && l->renderer() )
    {
      QgsSingleSymbolRenderer *ssr = dynamic_cast<QgsSingleSymbolRenderer *>( l->renderer() );
      if ( ssr && ssr->symbol() && ssr->symbol()->symbolLayer( 0 ) )
        ssr->symbol()->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromExpression( QStringLiteral( "180-angle*180.0/pi()" ) ) );
    }
  }

  if ( !layerGroup->children().isEmpty() )
  {
    layerGroup->setExpanded( false );
    layerGroup->setItemVisibilityChecked( visible );
  }
  else
  {
    layerGroup->parent()->takeChild( layerGroup );
    delete layerGroup;
  }
}

void QgsDwgImportDialog::updateCheckState( Qt::CheckState state )
{
  for ( int i = 0; i < mLayers->rowCount(); i++ )
    mLayers->item( i, 0 )->setCheckState( state );
}

void QgsDwgImportDialog::pbSelectAll_clicked()
{
  updateCheckState( Qt::Checked );
}

void QgsDwgImportDialog::pbDeselectAll_clicked()
{
  updateCheckState( Qt::Unchecked );
}

void QgsDwgImportDialog::buttonBox_accepted()
{
  const QgsTemporaryCursorOverride waitCursor( Qt::BusyCursor );

  QMap<QString, bool> layers;
  bool allLayers = true;
  for ( int i = 0; i < mLayers->rowCount(); i++ )
  {
    QTableWidgetItem *item = mLayers->item( i, 0 );
    if ( item->checkState() == Qt::Unchecked )
    {
      allLayers = false;
      continue;
    }

    layers.insert( item->text(), mLayers->item( i, 1 )->checkState() == Qt::Checked );
  }

  if ( cbMergeLayers->isChecked() )
  {
    if ( allLayers )
      layers.clear();

    createGroup( QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup(), leLayerGroup->text(), layers.keys(), true );
  }
  else
  {
    QgsLayerTreeGroup *dwgGroup = QgisApp::instance()->layerTreeView()->layerTreeModel()->rootGroup()->addGroup( leLayerGroup->text() );
    Q_ASSERT( dwgGroup );

    const auto constKeys = layers.keys();
    for ( const QString &layer : constKeys )
    {
      createGroup( dwgGroup, layer, QStringList( layer ), layers[layer] );
    }

    dwgGroup->setExpanded( false );
  }
}

void QgsDwgImportDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#importing-a-dxf-or-dwg-file" ) );
}
