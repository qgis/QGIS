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
  mLabelRubberBand = new QgsRubberBand( mCanvas, false );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  mLabelRubberBand->addPoint( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  mLabelRubberBand->setColor( Qt::green );
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
        mFeatureRubberBand = new QgsRubberBand( mCanvas, geom->type() == QGis::Polygon );
        mFeatureRubberBand->setColor( Qt::red );
        mFeatureRubberBand->setToGeometry( geom, vlayer );
        mFeatureRubberBand->show();
      }
    }

    //fixpoint rubber band
    QgsPoint fixPoint;
    if ( rotationPoint( fixPoint ) )
    {
      QgsGeometry* pointGeom = QgsGeometry::fromPoint( fixPoint );
      mFixPointRubberBand = new QgsRubberBand( mCanvas, false );
      mFixPointRubberBand->setColor( Qt::blue );
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
  QgsVectorLayer* vlayer = currentLayer();
  if ( vlayer )
  {
    QgsPalLabeling* labelEngine = dynamic_cast<QgsPalLabeling*>( mCanvas->mapRenderer()->labelingEngine() );
    if ( labelEngine )
    {
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
  return mInvalidLabelSettings;
}

QString QgsMapToolLabel::currentLabelText()
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
    if ( vlayer->featureAtId( mCurrentLabelPos.featureId, f, false, true ) )
    {
      return f.attributeMap()[labelFieldId].toString();
    }
  }
  return "";
}

void QgsMapToolLabel::currentAlignment( QString& hali, QString& vali )
{
  hali = "Left";
  vali = "Bottom";

  QgsFeature f;
  if ( !currentFeature( f ) )
  {
    return;
  }
  const QgsAttributeMap& featureAttributes = f.attributeMap();

  bool settingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &settingsOk );
  if ( settingsOk )
  {
    QMap< QgsPalLayerSettings::DataDefinedProperties, int > ddProperties = labelSettings.dataDefinedProperties;

    QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator haliIter = ddProperties.find( QgsPalLayerSettings::Hali );
    if ( haliIter != ddProperties.constEnd() )
    {
      hali = featureAttributes[*haliIter].toString();
    }

    QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator valiIter = ddProperties.find( QgsPalLayerSettings::Vali );
    if ( valiIter != ddProperties.constEnd() )
    {
      vali = featureAttributes[*valiIter].toString();
    }
  }
}

bool QgsMapToolLabel::currentFeature( QgsFeature& f, bool fetchGeom )
{
  QgsVectorLayer* vlayer = currentLayer();
  if ( !vlayer )
  {
    return false;
  }
  return vlayer->featureAtId( mCurrentLabelPos.featureId, f, fetchGeom, true );
}

QFont QgsMapToolLabel::labelFontCurrentFeature()
{
  QFont font;
  QgsVectorLayer* vlayer = currentLayer();

  bool labelSettingsOk;
  QgsPalLayerSettings& layerSettings = currentLabelSettings( &labelSettingsOk );

  if ( labelSettingsOk && vlayer )
  {
    font = layerSettings.textFont;

    QgsFeature f;
    if ( vlayer->featureAtId( mCurrentLabelPos.featureId, f, false, true ) )
    {
      const QgsAttributeMap& attributes = f.attributeMap();
      QMap< QgsPalLayerSettings::DataDefinedProperties, int > ddProperties = layerSettings.dataDefinedProperties;

      //size
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator sizeIt = ddProperties.find( QgsPalLayerSettings::Size );
      if ( sizeIt != ddProperties.constEnd() )
      {
        font.setPointSizeF( attributes[*sizeIt].toDouble() );
      }

      //family
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator familyIt = ddProperties.find( QgsPalLayerSettings::Family );
      if ( familyIt != ddProperties.constEnd() )
      {
        font.setFamily( attributes[*sizeIt].toString() );
      }

      //underline
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator underlineIt = ddProperties.find( QgsPalLayerSettings::Underline );
      if ( familyIt != ddProperties.constEnd() )
      {
        font.setUnderline( attributes[*underlineIt].toBool() );
      }

      //strikeout
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator strikeoutIt = ddProperties.find( QgsPalLayerSettings::Strikeout );
      if ( strikeoutIt != ddProperties.constEnd() )
      {
        font.setStrikeOut( attributes[*strikeoutIt].toBool() );
      }

      //bold
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator boldIt = ddProperties.find( QgsPalLayerSettings::Bold );
      if ( boldIt != ddProperties.constEnd() )
      {
        font.setBold( attributes[*boldIt].toBool() );
      }

      //italic
      QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator italicIt = ddProperties.find( QgsPalLayerSettings::Italic );
      if ( italicIt != ddProperties.constEnd() )
      {
        font.setItalic( attributes[*italicIt].toBool() );
      }
    }
  }

  return font;
}

bool QgsMapToolLabel::rotationPoint( QgsPoint& pos )
{
  QVector<QgsPoint> cornerPoints = mCurrentLabelPos.cornerPoints;
  if ( cornerPoints.size() < 4 )
  {
    return false;
  }

  if ( mCurrentLabelPos.upsideDown )
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

  QFont labelFont = labelFontCurrentFeature();
  QFontMetricsF labelFontMetrics( labelFont );

  //label text?
  QString labelText = currentLabelText();

  bool labelSettingsOk;
  QgsPalLayerSettings& labelSettings = currentLabelSettings( &labelSettingsOk );
  if ( !labelSettingsOk )
  {
    return false;
  }

  double labelSizeX, labelSizeY;
  labelSettings.calculateLabelSize( &labelFontMetrics, labelText, labelSizeX, labelSizeY );

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
    double descentRatio = labelFontMetrics.descent() / labelFontMetrics.height();
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
  if ( mCurrentLabelPos.upsideDown )
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
  if ( !vlayer->featureAtId( featureId, f, false, true ) )
  {
    return false;
  }

  QgsAttributeMap attributes = f.attributeMap();
  x = attributes[xCol].toDouble( &xSuccess );
  y = attributes[yCol].toDouble( &ySuccess );

  return true;
}

bool QgsMapToolLabel:: diagramMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const
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

  bool xColOk, yColOk;

  QVariant xColumn = ml->customProperty( "labeling/dataDefinedProperty9" );
  if ( !xColumn.isValid() )
  {
    return false;
  }
  xCol = xColumn.toInt( &xColOk );
  if ( !xColOk )
  {
    return false;
  }

  QVariant yColumn = ml->customProperty( "labeling/dataDefinedProperty10" );
  if ( !yColumn.isValid() )
  {
    return false;
  }
  yCol = yColumn.toInt( &yColOk );
  if ( !yColOk )
  {
    return false;
  }

  return true;
}
