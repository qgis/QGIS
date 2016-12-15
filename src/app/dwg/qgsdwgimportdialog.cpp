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

#include <QSettings>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFileDialog>

#include "qgisapp.h"
#include "qgsdwgimporter.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"
#include "qgsfeatureiterator.h"
#include "qgslayertreeview.h"
#include "qgslayertreemodel.h"
#include "qgslayertreegroup.h"
#include "qgsrendererv2.h"
#include "qgsdatadefined.h"
#include "qgsnullsymbolrenderer.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgspallabeling.h"
#include "qgsmapcanvas.h"
#include "qgscrscache.h"
#include "qgsgenericprojectionselector.h"
#include "qgsmessagelog.h"


struct CursorOverride
{
  CursorOverride()
  {
    QApplication::setOverrideCursor( Qt::BusyCursor );
  }

  ~CursorOverride()
  {
    QApplication::restoreOverrideCursor();
  }
};


struct SkipCrsValidation
{
  SkipCrsValidation() : savedValidation( QgsCoordinateReferenceSystem::customSrsValidation() )
  {
    QgsCoordinateReferenceSystem::setCustomSrsValidation( nullptr );
  }

  ~SkipCrsValidation()
  {
    QgsCoordinateReferenceSystem::setCustomSrsValidation( savedValidation );
  }

private:
  CUSTOM_CRS_VALIDATION savedValidation;
};


QgsDwgImportDialog::QgsDwgImportDialog( QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
{
  setupUi( this );

  QSettings s;
  leDatabase->setText( s.value( "/DwgImport/lastDatabase", "" ).toString() );
  cbExpandInserts->setChecked( s.value( "/DwgImport/lastExpandInserts", true ).toBool() );
  cbMergeLayers->setChecked( s.value( "/DwgImport/lastMergeLayers", false ).toBool() );
  cbUseCurves->setChecked( s.value( "/DwgImport/lastUseCurves", true ).toBool() );

#if !defined(GDAL_COMPUTE_VERSION) || GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(2,0,0)
  cbUseCurves->setChecked( false );
  cbUseCurves->setHidden( true );
#endif

  leDrawing->setReadOnly( true );
  pbImportDrawing->setHidden( true );
  lblMessage->setHidden( true );

  int crsid = s.value( "/DwgImport/lastCrs", QString::number( QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs().srsid() ) ).toInt();

  QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsBySrsId( crsid );
  mCrsSelector->setCrs( crs );
  mCrsSelector->setLayerCrs( crs );
  mCrsSelector->dialog()->setMessage( tr( "Select the coordinate reference system for the dxf file. "
                                          "The data points will be transformed from the layer coordinate reference system." ) );

  on_pbLoadDatabase_clicked();
  updateUI();

  restoreGeometry( s.value( "/Windows/DwgImport/geometry" ).toByteArray() );
}

QgsDwgImportDialog::~QgsDwgImportDialog()
{
  QSettings s;
  s.setValue( "/DwgImport/lastDatabase", leDatabase->text() );
  s.setValue( "/DwgImport/lastExpandInserts", cbExpandInserts->isChecked() );
  s.setValue( "/DwgImport/lastMergeLayers", cbMergeLayers->isChecked() );
  s.setValue( "/DwgImport/lastUseCurves", cbUseCurves->isChecked() );
  s.setValue( "/Windows/DwgImport/geometry", saveGeometry() );
}

void QgsDwgImportDialog::updateUI()
{
  bool dbAvailable = false;
  bool dbReadable = false;
  bool dwgReadable = false;

  if ( !leDatabase->text().isEmpty() )
  {
    QFileInfo fi( leDatabase->text() );
    dbAvailable = fi.exists() ? fi.isWritable() : QFileInfo( fi.path() ).isWritable();
    dbReadable = fi.exists() && fi.isReadable();
  }

  if ( !leDrawing->text().isEmpty() )
  {
    QFileInfo fi( leDrawing->text() );
    dwgReadable = fi.exists() && fi.isReadable();
  }

  pbImportDrawing->setEnabled( dbAvailable && dwgReadable );
  pbImportDrawing->setVisible( dbAvailable && dwgReadable );
  pbLoadDatabase->setEnabled( dbReadable );
  pbBrowseDrawing->setEnabled( dbAvailable );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mLayers->rowCount() > 0 && !leLayerGroup->text().isEmpty() );
}

void QgsDwgImportDialog::on_pbBrowseDatabase_clicked()
{
  QString dir( leDatabase->text().isEmpty() ? QDir::homePath() : QFileInfo( leDatabase->text() ).canonicalPath() );
  QString filename = QFileDialog::getSaveFileName( this, tr( "Specify GeoPackage database" ), dir, tr( "GeoPackage database" ) + " (*.gpkg *.GPKG)", nullptr, QFileDialog::DontConfirmOverwrite );
  if ( filename.isEmpty() )
    return;
  leDatabase->setText( filename );
  updateUI();
}

void QgsDwgImportDialog::on_leDatabase_textChanged( const QString &text )
{
  Q_UNUSED( text );
  updateUI();
}

void QgsDwgImportDialog::on_leLayerGroup_textChanged( const QString &text )
{
  Q_UNUSED( text );
  updateUI();
}

void QgsDwgImportDialog::on_pbLoadDatabase_clicked()
{
  if ( !QFileInfo( leDatabase->text() ).exists() )
    return;

  CursorOverride waitCursor;

  bool lblVisible = false;

  QScopedPointer<QgsVectorLayer> d( new QgsVectorLayer( QString( "%1|layername=drawing" ).arg( leDatabase->text() ), "layers", "ogr", false ) );
  if ( d && d->isValid() )
  {
    int idxPath = d->fieldNameIndex( "path" );
    int idxLastModified = d->fieldNameIndex( "lastmodified" );
    int idxCrs = d->fieldNameIndex( "crs" );

    QgsFeature f;
    if ( d->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << idxPath << idxLastModified << idxCrs ) ).nextFeature( f ) )
    {
      leDrawing->setText( f.attribute( idxPath ).toString() );

      QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsBySrsId( f.attribute( idxCrs ).toInt() );
      mCrsSelector->setCrs( crs );
      mCrsSelector->setLayerCrs( crs );

      QFileInfo fi( leDrawing->text() );
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

  QScopedPointer<QgsVectorLayer> l( new QgsVectorLayer( QString( "%1|layername=layers" ).arg( leDatabase->text() ), "layers", "ogr", false ) );
  if ( l && l->isValid() )
  {
    int idxName = l->fieldNameIndex( "name" );
    int idxColor = l->fieldNameIndex( "ocolor" );
    int idxFlags = l->fieldNameIndex( "flags" );

    QgsDebugMsg( QString( "idxName:%1 idxColor:%2 idxFlags:%3" ).arg( idxName ).arg( idxColor ).arg( idxFlags ) );

    QgsFeatureIterator fit = l->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << idxName << idxColor << idxFlags ) );
    QgsFeature f;

    mLayers->setRowCount( 0 );

    while ( fit.nextFeature( f ) )
    {
      int row = mLayers->rowCount();
      mLayers->setRowCount( row + 1 );

      QgsDebugMsg( QString( "name:%1 color:%2 flags:%3" ).arg( f.attribute( idxName ).toString() ).arg( f.attribute( idxColor ).toInt() ).arg( f.attribute( idxFlags ).toString(), 0, 16 ) );

      QTableWidgetItem *item;
      item = new QTableWidgetItem( f.attribute( idxName ).toString() );
      item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      item->setCheckState( Qt::Checked );
      mLayers->setItem( row, 0, item );

      item = new QTableWidgetItem();
      item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
      item->setCheckState(( f.attribute( idxColor ).toInt() >= 0 && ( f.attribute( idxFlags ).toInt() & 1 ) == 0 ) ? Qt::Checked : Qt::Unchecked );
      mLayers->setItem( row, 1, item );
    }

    mLayers->resizeColumnsToContents();

    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( mLayers->rowCount() > 0 && !leLayerGroup->text().isEmpty() );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Could not open layer list" ), QgsMessageBar::CRITICAL, 4 );
  }
}

void QgsDwgImportDialog::on_pbBrowseDrawing_clicked()
{
  QString dir( leDrawing->text().isEmpty() ? QDir::homePath() : QFileInfo( leDrawing->text() ).canonicalPath() );
  QString filename = QFileDialog::getOpenFileName( nullptr, tr( "Select DWG/DXF file" ), dir, tr( "DXF/DWG files" ) + " (*.dwg *.DWG *.dxf *.DXF)" );
  if ( filename.isEmpty() )
    return;

  leDrawing->setText( filename );

  on_pbImportDrawing_clicked();
}

void QgsDwgImportDialog::on_pbImportDrawing_clicked()
{
  CursorOverride waitCursor;

  QgsDwgImporter importer( leDatabase->text(), mCrsSelector->crs() );

  QString error;
  if ( importer.import( leDrawing->text(), error, cbExpandInserts->isChecked(), cbUseCurves->isChecked() ) )
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Drawing import completed." ), QgsMessageBar::INFO, 4 );
  }
  else
  {
    QgisApp::instance()->messageBar()->pushMessage( tr( "Drawing import failed (%1)" ).arg( error ), QgsMessageBar::CRITICAL, 4 );
  }

  on_pbLoadDatabase_clicked();
}

QgsVectorLayer *QgsDwgImportDialog::layer( QgsLayerTreeGroup *layerGroup, QString layerFilter, QString table )
{
  QgsVectorLayer *l = new QgsVectorLayer( QString( "%1|layername=%2" ).arg( leDatabase->text() ).arg( table ), table, "ogr", false );
  l->setSubsetString( QString( "%1space=0 AND block=-1" ).arg( layerFilter ) );

  if ( l->featureCount() == 0 )
  {
    delete l;
    return nullptr;
  }

  QgsMapLayerRegistry::instance()->addMapLayer( l, false );
  layerGroup->addLayer( l );
  return l;
}

void QgsDwgImportDialog::createGroup( QgsLayerTreeGroup *group, QString name, QStringList layers, bool visible )
{
  QgsLayerTreeGroup *layerGroup = group->addGroup( name );
  QgsDebugMsg( QString( " %1" ).arg( name ) ) ;
  Q_ASSERT( layerGroup );

  QString layerFilter;
  if ( !layers.isEmpty() )
  {
    QStringList exprlist;
    Q_FOREACH ( QString layer, layers )
    {
      exprlist.append( QString( "'%1'" ).arg( layer.replace( "'", "''" ) ) );
    }
    layerFilter = QString( "layer IN (%1) AND " ).arg( exprlist.join( "," ) );
  }

  QgsVectorLayer *l;
  QgsSymbolV2 *sym;

  l = layer( layerGroup, layerFilter, "hatches" );
  if ( l )
  {
    QgsSimpleFillSymbolLayerV2 *sfl = new QgsSimpleFillSymbolLayerV2();
    sfl->setDataDefinedProperty( "color", new QgsDataDefined( true, false, "", "color" ) );
    sfl->setBorderStyle( Qt::NoPen );
    sym = new QgsFillSymbolV2();
    sym->changeSymbolLayer( 0, sfl );
    l->setRendererV2( new QgsSingleSymbolRendererV2( sym ) );
  }

  l = layer( layerGroup, layerFilter, "lines" );
  if ( l )
  {
    QgsSimpleLineSymbolLayerV2 *sll = new QgsSimpleLineSymbolLayerV2();
    sll->setDataDefinedProperty( "color", new QgsDataDefined( true, false, "", "color" ) );
    sll->setPenJoinStyle( Qt::MiterJoin );
    sll->setDataDefinedProperty( "width", new QgsDataDefined( true, false, "", "linewidth" ) );
    // sll->setUseCustomDashPattern( true );
    // sll->setCustomDashPatternUnit( QgsSymbolV2::MapUnit );
    // sll->setDataDefinedProperty( "customdash", new QgsDataDefined( true, false, "", "linetype" ) );
    sym = new QgsLineSymbolV2();
    sym->changeSymbolLayer( 0, sll );
    sym->setOutputUnit( QgsSymbolV2::MM );
    l->setRendererV2( new QgsSingleSymbolRendererV2( sym ) );
  }

  l = layer( layerGroup, layerFilter, "polylines" );
  if ( l )
  {
    QgsSimpleLineSymbolLayerV2 *sll = new QgsSimpleLineSymbolLayerV2();
    sll->setDataDefinedProperty( "color", new QgsDataDefined( true, false, "", "color" ) );
    sll->setPenJoinStyle( Qt::MiterJoin );
    sll->setDataDefinedProperty( "width", new QgsDataDefined( true, false, "", "width" ) );
    // sll->setUseCustomDashPattern( true );
    // sll->setCustomDashPatternUnit( QgsSymbolV2::MapUnit );
    // sll->setDataDefinedProperty( "customdash", new QgsDataDefined( true, false, "", "linetype" ) );
    sym = new QgsLineSymbolV2();
    sym->changeSymbolLayer( 0, sll );
    sym->setOutputUnit( QgsSymbolV2::MapUnit );
    l->setRendererV2( new QgsSingleSymbolRendererV2( sym ) );
  }

  l = layer( layerGroup, layerFilter, "texts" );
  if ( l )
  {
    l->setRendererV2( new QgsNullSymbolRenderer() );

    QgsPalLayerSettings pls;
    pls.readFromLayer( l );

    pls.enabled = true;
    pls.drawLabels = true;
    pls.fieldName = "text";
    pls.fontSizeInMapUnits = true;
    pls.wrapChar = "\\P";
    pls.setDataDefinedProperty( QgsPalLayerSettings::Size, true, false, "", "height" );
    pls.setDataDefinedProperty( QgsPalLayerSettings::Color, true, false, "", "color" );
    pls.setDataDefinedProperty( QgsPalLayerSettings::MultiLineHeight, true, true, "CASE WHEN interlin<0 THEN 1 ELSE interlin*1.5 END", "" );
    pls.placement = QgsPalLayerSettings::OrderedPositionsAroundPoint;
    pls.setDataDefinedProperty( QgsPalLayerSettings::PositionX, true, true, "$x", "" );
    pls.setDataDefinedProperty( QgsPalLayerSettings::PositionY, true, true, "$y", "" );
    pls.setDataDefinedProperty( QgsPalLayerSettings::Hali, true, true, QString(
                                  "CASE"
                                  " WHEN etype=%1 THEN"
                                  " CASE"
                                  " WHEN alignv IN (1,4,7) THEN 'Left'"
                                  " WHEN alignv IN (2,5,6) THEN 'Center'"
                                  " ELSE 'Right'"
                                  " END"
                                  " ELSE"
                                  "  CASE"
                                  " WHEN alignh=0 THEN 'Left'"
                                  " WHEN alignh=1 THEN 'Center'"
                                  " WHEN alignh=2 THEN 'Right'"
                                  " WHEN alignh=3 THEN 'Left'"
                                  " WHEN alignh=4 THEN 'Left'"
                                  " END "
                                  " END" ).arg( DRW::MTEXT ), "" );

    pls.setDataDefinedProperty( QgsPalLayerSettings::Vali, true, true, QString(
                                  "CASE"
                                  " WHEN etype=%1 THEN"
                                  " CASE"
                                  " WHEN alignv < 4 THEN 'Top'"
                                  " WHEN alignv < 7 THEN 'Half'"
                                  " ELSE 'Bottom'"
                                  " END"
                                  " ELSE"
                                  " CASE"
                                  " WHEN alignv=0 THEN 'Base'"
                                  " WHEN alignv=1 THEN 'Bottom'"
                                  " WHEN alignv=2 THEN 'Half'"
                                  " WHEN alignv=3 THEN 'Top'"
                                  " END"
                                  " END" ).arg( DRW::MTEXT ), "" );

    pls.setDataDefinedProperty( QgsPalLayerSettings::Rotation, true, true, "angle*180.0/pi()", "" );

    pls.writeToLayer( l );
  }

  l = layer( layerGroup, layerFilter, "points" );
  if ( l )
  {
    // FIXME: use PDMODE?
    l->setRendererV2( new QgsNullSymbolRenderer() );
  }

  if ( !cbExpandInserts->isChecked() )
    layer( layerGroup, layerFilter, "inserts" );

  if ( !layerGroup->children().isEmpty() )
  {
    layerGroup->setExpanded( false );
    layerGroup->setVisible( visible ? Qt::Checked : Qt::Unchecked );
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

void QgsDwgImportDialog::on_pbSelectAll_clicked()
{
  updateCheckState( Qt::Checked );
}

void QgsDwgImportDialog::on_pbDeselectAll_clicked()
{
  updateCheckState( Qt::Unchecked );
}

void QgsDwgImportDialog::on_buttonBox_accepted()
{
  CursorOverride waitCursor;

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

    Q_FOREACH ( QString layer, layers.keys() )
    {
      createGroup( dwgGroup, layer, QStringList( layer ), layers[layer] );
    }

    dwgGroup->setExpanded( false );
  }
}
