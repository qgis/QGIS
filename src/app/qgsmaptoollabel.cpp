/***************************************************************************
                          qgsmaptoollabel.cpp
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoollabel.h"
#include "qgsdatadefined.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsdiagramrendererv2.h"
#include <QMouseEvent>

QgsMapToolLabel::QgsMapToolLabel( QgsMapCanvas* canvas ): QgsMapTool( canvas ), mLabelRubberBand( 0 ), mFeatureRubberBand( 0 ), mFixPointRubberBand( 0 )
{
}

QgsMapToolLabel::~QgsMapToolLabel()
{
  delete mLabelRubberBand;
  delete mFeatureRubberBand;
  delete mFixPointRubberBand;
}

bool QgsMapToolLabel::labelAtPosition( QMouseEvent* e, QgsLabelPosition& p )
{
  QgsPoint pt = toMapCoordinates( e->pos() );
  QgsLabelingEngineInterface* labelingEngine = mCanvas->mapRenderer()->labelingEngine();
  if ( labelingEngine )
  {
    QList<QgsLabelPosition> labelPosList = labelingEngine->labelsAtPosition( pt );
    QList<QgsLabelPosition>::const_iterator posIt = labelPosList.constBegin();
    if ( posIt != labelPosList.constEnd() )
    {
      p = *posIt;
      return true;
    }
  }

  return false;
}

void QgsMapToolLabel::createRubberBands( )
{
  delete mLabelRubberBand;
  delete mFeatureRubberBand;

  //label rubber band
  QgsRectangle rect = mCurrentLabelPos.labelRect;
  mLabelRubberBand = new QgsRubberBand( mCanvas, QGis::Line );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  mLabelRubberBand->setColor( QColor( 0, 255, 0, 65 ) );
  mLabelRubberBand->setWidth( 3 );
  mLabelRubberBand->show();

  //feature rubber band
  QgsVectorLayer* vlayer = currentLayer();
  if ( vlayer )
  {
    QgsFeature f;
    if ( currentFeature( f, true ) )
    {
      QgsGeometry* geom = f.geometry();
      if ( geom )
      {
        mFeatureRubberBand = new QgsRubberBand( mCanvas, geom->type() );
        mFeatureRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
        mFeatureRubberBand->setToGeometry( geom, vlayer );
        mFeatureRubberBand->show();
      }
    }

    //fixpoint rubber band
    QgsPoint fixPoint;
    if ( rotationPoint( fixPoint, false, false ) )
    {
      if ( mCanvas )
      {
        QgsMapRenderer* r = mCanvas->mapRenderer();
        if ( r && r->hasCrsTransformEnabled() )
        {
          fixPoint = r->mapToLayerCoordinates( vlayer, fixPoint );
        }
      }

      QgsGeometry* pointGeom = QgsGeometry::fromPoint( fixPoint );
      mFixPointRubberBand = new QgsRubberBand( mCanvas, QGis::Line );
      mFixPointRubberBand->setColor( QColor( 0, 0, 255, 65 ) );
      mFixPointRubberBand->setToGeometry( pointGeom, vlayer );
      mFixPointRubberBand->show();
      delete pointGeom;
    }
  }
}

void QgsMapToolLabel::deleteRubberBands()
{
  delete mLabelRubberBand; mLabelRubberBand = 0;
  delete mFeatureRubberBand; mFeatureRubberBand = 0;
  delete mFixPointRubberBand; mFixPointRubberBand = 0;
}

QgsVectorLayer* QgsMapToolLabel::currentLayer()
{
  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( mCurrentLabelPos.layerID ) );
  return vlayer;
}

QgsPalLayerSettings& QgsMapToolLabel::currentLabelSettings( bool* ok )
{
  //QgsDebugMsg( "entered" );
  QgsVectorLayer* vlayer = currentLayer();
  if ( vlayer )
  {
    //QgsDebugMsg( "has vlayer" );
    QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mCanvas->mapRenderer()->labelingEngine() );
    if ( labelEngine )
    {
      //QgsDebugMsg( "has labelEngine" );
      if ( ok )
      {
        *ok = true;
      }
      return labelEngine->layer( mCurrentLabelPos.layerID );
    }
  }

  if ( ok )
  {
    *ok = false;
  }

  return const_cast<QgsPalLayerSettings&>( mInvalidLabelSettings );
}

QString QgsMapToolLabel::currentLabelText( int trunc )
{
  bool settingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &settingsOk );
  if ( !settingsOk )
  {
    return "";
  }

  if ( labelSettings.isExpression )
  {
    QString labelText = mCurrentLabelPos.labelText;

    if ( trunc > 0 && labelText.length() > trunc )
    {
      labelText.truncate( trunc );
      labelText += "...";
    }
    return labelText;
  }
  else
  {
    QgsVectorLayer* vlayer = currentLayer();
    if ( !vlayer )
    {
      return "";
    }

    QString labelField = vlayer->customProperty( "labeling/fieldName" ).toString();
    if ( !labelField.isEmpty() )
    {
      int labelFieldId = vlayer->fieldNameIndex( labelField );
      QgsFeature f;
      if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabelPos.featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
      {
        QString labelText = f.attribute( labelFieldId ).toString();
        if ( trunc > 0 && labelText.length() > trunc )
        {
          labelText.truncate( trunc );
          labelText += "...";
        }
        return labelText;
      }
    }
  }
  return "";
}

void QgsMapToolLabel::currentAlignment( QString& hali, QString& vali )
{
  hali = "Left";
  vali = "Bottom";

  QgsVectorLayer* vlayer = currentLayer();
  if ( !vlayer )
  {
    return;
  }

  QgsFeature f;
  if ( !currentFeature( f ) )
  {
    return;
  }

  int haliIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Hali, vlayer );
  if ( haliIndx != -1 )
  {
    hali = f.attribute( haliIndx ).toString();
  }

  int valiIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Vali, vlayer );
  if ( valiIndx != -1 )
  {
    vali = f.attribute( valiIndx ).toString();
  }
}

bool QgsMapToolLabel::currentFeature( QgsFeature& f, bool fetchGeom )
{
  QgsVectorLayer* vlayer = currentLayer();
  if ( !vlayer )
  {
    return false;
  }
  return vlayer->getFeatures( QgsFeatureRequest()
                              .setFilterFid( mCurrentLabelPos.featureId )
                              .setFlags( fetchGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                            ).nextFeature( f );
}

QFont QgsMapToolLabel::labelFontCurrentFeature()
{
  QFont font;
  QgsVectorLayer* vlayer = currentLayer();

  bool labelSettingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &labelSettingsOk );

  if ( labelSettingsOk && vlayer )
  {
    font = labelSettings.textFont;

    QgsFeature f;
    if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabelPos.featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
    {
      //size
      int sizeIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Size, vlayer );
      if ( sizeIndx != -1 )
      {
        if ( labelSettings.fontSizeInMapUnits )
        {
          font.setPixelSize( labelSettings.sizeToPixel( f.attribute( sizeIndx ).toDouble(),
                             QgsRenderContext(), QgsPalLayerSettings::MapUnits, true ) );
        }
        else
        {
          font.setPointSizeF( f.attribute( sizeIndx ).toDouble() );
        }
      }

      //family
      int fmIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Family, vlayer );
      if ( fmIndx != -1 )
      {
        font.setFamily( f.attribute( fmIndx ).toString() );
      }

      //underline
      int ulIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Underline, vlayer );
      if ( ulIndx != -1 )
      {
        font.setUnderline( f.attribute( ulIndx ).toBool() );
      }

      //strikeout
      int soIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Strikeout, vlayer );
      if ( soIndx != -1 )
      {
        font.setStrikeOut( f.attribute( soIndx ).toBool() );
      }

      //bold
      int boIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Bold, vlayer );
      if ( boIndx != -1 )
      {
        font.setBold( f.attribute( boIndx ).toBool() );
      }

      //italic
      int itIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Italic, vlayer );
      if ( itIndx != -1 )
      {
        font.setItalic( f.attribute( itIndx ).toBool() );
      }

      // TODO: Add other font data defined values (word spacing, etc.)
    }
  }

  return font;
}

bool QgsMapToolLabel::preserveRotation()
{
  bool labelSettingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &labelSettingsOk );

  if ( labelSettingsOk )
  {
    return labelSettings.preserveRotation;
  }

  return true; // default, so there is no accidental data loss
}

bool QgsMapToolLabel::rotationPoint( QgsPoint& pos, bool ignoreUpsideDown, bool rotatingUnpinned )
{
  QVector<QgsPoint> cornerPoints = mCurrentLabelPos.cornerPoints;
  if ( cornerPoints.size() < 4 )
  {
    return false;
  }

  if ( mCurrentLabelPos.upsideDown && !ignoreUpsideDown )
  {
    pos = mCurrentLabelPos.cornerPoints.at( 2 );
  }
  else
  {
    pos = mCurrentLabelPos.cornerPoints.at( 0 );
  }

  //alignment always center/center and rotation 0 for diagrams
  if ( mCurrentLabelPos.isDiagram )
  {
    pos.setX( pos.x() + mCurrentLabelPos.labelRect.width() / 2.0 );
    pos.setY( pos.y() + mCurrentLabelPos.labelRect.height() / 2.0 );
    return true;
  }

  //adapt pos depending on data defined alignment
  QString haliString, valiString;
  currentAlignment( haliString, valiString );

  // rotate unpinned labels (i.e. no hali/vali settings) as if hali/vali was Center/Half
  if ( rotatingUnpinned )
  {
    haliString = "Center";
    valiString = "Half";
  }

//  QFont labelFont = labelFontCurrentFeature();
  QFontMetricsF labelFontMetrics( mCurrentLabelPos.labelFont );

  //label text?
  QString labelText = currentLabelText();

  bool labelSettingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &labelSettingsOk );
  if ( !labelSettingsOk )
  {
    return false;
  }

  double labelSizeX, labelSizeY;
  QgsFeature f;
  if ( !currentFeature( f ) )
  {
    return false;
  }
  labelSettings.calculateLabelSize( &labelFontMetrics, labelText, labelSizeX, labelSizeY, &f );

  double xdiff = 0;
  double ydiff = 0;

  if ( haliString.compare( "Center", Qt::CaseInsensitive ) == 0 )
  {
    xdiff = labelSizeX / 2.0;
  }
  else if ( haliString.compare( "Right", Qt::CaseInsensitive ) == 0 )
  {
    xdiff = labelSizeX;
  }

  if ( valiString.compare( "Top", Qt::CaseInsensitive ) == 0 || valiString.compare( "Cap", Qt::CaseInsensitive ) == 0 )
  {
    ydiff = labelSizeY;
  }
  else
  {
    double descentRatio = 1 / labelFontMetrics.ascent() / labelFontMetrics.height();
    if ( valiString.compare( "Base", Qt::CaseInsensitive ) == 0 )
    {
      ydiff = labelSizeY * descentRatio;
    }
    else if ( valiString.compare( "Half", Qt::CaseInsensitive ) == 0 )
    {
      ydiff = labelSizeY * descentRatio;
      ydiff = labelSizeY * 0.5 * ( 1 - descentRatio );
    }
  }

  double angle = mCurrentLabelPos.rotation;
  double xd = xdiff * cos( angle ) - ydiff * sin( angle );
  double yd = xdiff * sin( angle ) + ydiff * cos( angle );
  if ( mCurrentLabelPos.upsideDown && !ignoreUpsideDown )
  {
    pos.setX( pos.x() - xd );
    pos.setY( pos.y() - yd );
  }
  else
  {
    pos.setX( pos.x() + xd );
    pos.setY( pos.y() + yd );
  }
  return true;
}

int QgsMapToolLabel::dataDefinedColumnIndex( QgsPalLayerSettings::DataDefinedProperties p, const QgsVectorLayer* vlayer ) const
{

  QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mCanvas->mapRenderer()->labelingEngine() );
  if ( !labelEngine )
  {
    return -1;
  }
  QgsDebugMsg( QString( "dataDefinedProperties layer id:%1" ).arg( vlayer->id() ) );
  QgsPalLayerSettings& labelSettings = labelEngine->layer( vlayer->id() );

  QgsDebugMsg( QString( "dataDefinedProperties count:%1" ).arg( labelSettings.dataDefinedProperties.size() ) );

  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator dIt = labelSettings.dataDefinedProperties.find( p );
  if ( dIt != labelSettings.dataDefinedProperties.constEnd() )
  {
    //QgsDebugMsg( "found data defined" );
    QgsDataDefined* dd = dIt.value();

    QString ddField = dd->field();
    //QgsDebugMsg( "testing for active" );

    // can only modify attributes that are data defined with a mapped field
    if ( dd->isActive() && !dd->useExpression() && !ddField.isEmpty() )
    {
      //QgsDebugMsg( "looking up index" );
      return vlayer->fieldNameIndex( ddField );
    }
  }

  return -1;
}

bool QgsMapToolLabel::dataDefinedPosition( QgsVectorLayer* vlayer, int featureId, double& x, bool& xSuccess, double& y, bool& ySuccess, int& xCol, int& yCol ) const
{
  xSuccess = false;
  ySuccess = false;

  if ( !vlayer )
  {
    return false;
  }

  if ( mCurrentLabelPos.isDiagram )
  {
    if ( !diagramMoveable( vlayer, xCol, yCol ) )
    {
      return false;
    }
  }
  else if ( !labelMoveable( vlayer, xCol, yCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  const QgsAttributes& attributes = f.attributes();
  x = attributes[xCol].toDouble( &xSuccess );
  y = attributes[yCol].toDouble( &ySuccess );

  return true;
}

bool QgsMapToolLabel::layerIsRotatable( const QgsMapLayer* layer, int& rotationCol ) const
{
  const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>( layer );
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  int rotCol = dataDefinedColumnIndex( QgsPalLayerSettings::Rotation, vlayer );
  if ( rotCol != -1 )
  {
    rotationCol = rotCol;
    return true;
  }

  return false;
}

bool QgsMapToolLabel::dataDefinedRotation( QgsVectorLayer* vlayer, int featureId, double& rotation, bool& rotationSuccess, bool ignoreXY ) const
{
  rotationSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  int rotationCol;
  if ( !layerIsRotatable( vlayer, rotationCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  //test, if data defined x- and y- values are not null. Otherwise, the position is determined by PAL and the rotation cannot be fixed
  if ( !ignoreXY )
  {
    int xCol, yCol;
    double x, y;
    bool xSuccess, ySuccess;
    if ( !dataDefinedPosition( vlayer, featureId, x, xSuccess, y, ySuccess, xCol, yCol ) || !xSuccess || !ySuccess )
    {
      return false;
    }
  }

  rotation = f.attribute( rotationCol ).toDouble( &rotationSuccess );
  return true;
}

bool QgsMapToolLabel::dataDefinedShowHide( QgsVectorLayer* vlayer, int featureId, int& show, bool& showSuccess, int& showCol ) const
{
  showSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  if ( !layerCanShowHide( vlayer, showCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  show = f.attribute( showCol ).toInt( &showSuccess );
  return true;
}

bool QgsMapToolLabel::diagramMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const
{
  const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>( ml );
  if ( vlayer && vlayer->diagramRenderer() )
  {
    const QgsDiagramLayerSettings *dls = vlayer->diagramLayerSettings();
    if ( dls && dls->xPosColumn >= 0 && dls->yPosColumn >= 0 )
    {
      xCol = dls->xPosColumn;
      yCol = dls->yPosColumn;
      return true;
    }
  }
  return false;
}

bool QgsMapToolLabel::labelMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const
{
  const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>( ml );
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  bool xColOk = false;
  bool yColOk = false;

  int xColumn = dataDefinedColumnIndex( QgsPalLayerSettings::PositionX, vlayer );
  if ( xColumn != -1 )
  {
    xCol = xColumn;
    xColOk = true;
  }

  int yColumn = dataDefinedColumnIndex( QgsPalLayerSettings::PositionY, vlayer );
  if ( yColumn != -1 )
  {
    yCol = yColumn;
    yColOk = true;
  }

  if ( xColOk && yColOk )
  {
    return true;
  }

  return false;
}

bool QgsMapToolLabel::layerCanPin( const QgsMapLayer* ml, int& xCol, int& yCol ) const
{
  // currently same as QgsMapToolLabel::labelMoveable, but may change
  bool canPin = labelMoveable( ml, xCol, yCol );
  return canPin;
}

bool QgsMapToolLabel::layerCanShowHide( const QgsMapLayer* ml, int& showCol ) const
{
  //QgsDebugMsg( "entered" );
  const QgsVectorLayer* vlayer = dynamic_cast<const QgsVectorLayer*>( ml );
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  int showColmn = dataDefinedColumnIndex( QgsPalLayerSettings::Show, vlayer );
  if ( showColmn != -1 )
  {
    showCol = showColmn;
    return true;
  }

  return false;
}
