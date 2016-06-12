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
#include "qgsvectorlayerlabeling.h"
#include "qgsdiagramrendererv2.h"
#include <QMouseEvent>

QgsMapToolLabel::QgsMapToolLabel( QgsMapCanvas* canvas )
    : QgsMapTool( canvas )
    , mLabelRubberBand( nullptr )
    , mFeatureRubberBand( nullptr )
    , mFixPointRubberBand( nullptr )
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
  const QgsLabelingResults* labelingResults = mCanvas->labelingResults();
  if ( labelingResults )
  {
    QList<QgsLabelPosition> labelPosList = labelingResults->labelsAtPosition( pt );
    QList<QgsLabelPosition>::const_iterator posIt = labelPosList.constBegin();
    if ( posIt != labelPosList.constEnd() )
    {
      p = *posIt;
      return true;
    }
  }

  return false;
}

void QgsMapToolLabel::createRubberBands()
{
  delete mLabelRubberBand;
  delete mFeatureRubberBand;

  //label rubber band
  QgsRectangle rect = mCurrentLabel.pos.labelRect;
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
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  if ( vlayer )
  {
    QgsFeature f;
    if ( currentFeature( f, true ) )
    {
      const QgsGeometry* geom = f.constGeometry();
      if ( geom )
      {
        QSettings settings;
        int r = settings.value( "/qgis/digitizing/line_color_red", 255 ).toInt();
        int g = settings.value( "/qgis/digitizing/line_color_green", 0 ).toInt();
        int b = settings.value( "/qgis/digitizing/line_color_blue", 0 ).toInt();
        int a = settings.value( "/qgis/digitizing/line_color_alpha", 200 ).toInt();
        mFeatureRubberBand = new QgsRubberBand( mCanvas, geom->type() );
        mFeatureRubberBand->setColor( QColor( r, g, b, a ) );
        mFeatureRubberBand->setToGeometry( geom, vlayer );
        mFeatureRubberBand->show();
      }
    }

    //fixpoint rubber band
    QgsPoint fixPoint;
    if ( currentLabelRotationPoint( fixPoint, false, false ) )
    {
      if ( mCanvas )
      {
        const QgsMapSettings& s = mCanvas->mapSettings();
        if ( s.hasCrsTransformEnabled() )
        {
          fixPoint = s.mapToLayerCoordinates( vlayer, fixPoint );
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
  delete mLabelRubberBand;
  mLabelRubberBand = nullptr;
  delete mFeatureRubberBand;
  mFeatureRubberBand = nullptr;
  delete mFixPointRubberBand;
  mFixPointRubberBand = nullptr;
}


QString QgsMapToolLabel::currentLabelText( int trunc )
{
  if ( !mCurrentLabel.valid )
  {
    return "";
  }
  QgsPalLayerSettings& labelSettings = mCurrentLabel.settings;

  if ( labelSettings.isExpression )
  {
    QString labelText = mCurrentLabel.pos.labelText;

    if ( trunc > 0 && labelText.length() > trunc )
    {
      labelText.truncate( trunc );
      labelText += "...";
    }
    return labelText;
  }
  else
  {
    QgsVectorLayer* vlayer = mCurrentLabel.layer;
    if ( !vlayer )
    {
      return "";
    }

    QString labelField = vlayer->customProperty( "labeling/fieldName" ).toString();
    if ( !labelField.isEmpty() )
    {
      int labelFieldId = vlayer->fieldNameIndex( labelField );
      QgsFeature f;
      if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabel.pos.featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
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

  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return;
  }

  QgsFeature f;
  if ( !currentFeature( f ) )
  {
    return;
  }

  int haliIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Hali, mCurrentLabel.settings, vlayer );
  if ( haliIndx != -1 )
  {
    hali = f.attribute( haliIndx ).toString();
  }

  int valiIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Vali, mCurrentLabel.settings, vlayer );
  if ( valiIndx != -1 )
  {
    vali = f.attribute( valiIndx ).toString();
  }
}

bool QgsMapToolLabel::currentFeature( QgsFeature& f, bool fetchGeom )
{
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  if ( !vlayer )
  {
    return false;
  }
  return vlayer->getFeatures( QgsFeatureRequest()
                              .setFilterFid( mCurrentLabel.pos.featureId )
                              .setFlags( fetchGeom ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry )
                            ).nextFeature( f );
}

QFont QgsMapToolLabel::currentLabelFont()
{
  QFont font;

  QgsPalLayerSettings& labelSettings = mCurrentLabel.settings;
  QgsVectorLayer* vlayer = mCurrentLabel.layer;

  if ( mCurrentLabel.valid && vlayer )
  {
    font = labelSettings.textFont;

    QgsFeature f;
    if ( vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mCurrentLabel.pos.featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
    {
      //size
      int sizeIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Size, mCurrentLabel.settings, vlayer );
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
      int fmIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Family, labelSettings, vlayer );
      if ( fmIndx != -1 )
      {
        font.setFamily( f.attribute( fmIndx ).toString() );
      }

      //underline
      int ulIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Underline, labelSettings, vlayer );
      if ( ulIndx != -1 )
      {
        font.setUnderline( f.attribute( ulIndx ).toBool() );
      }

      //strikeout
      int soIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Strikeout, labelSettings, vlayer );
      if ( soIndx != -1 )
      {
        font.setStrikeOut( f.attribute( soIndx ).toBool() );
      }

      //bold
      int boIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Bold, labelSettings, vlayer );
      if ( boIndx != -1 )
      {
        font.setBold( f.attribute( boIndx ).toBool() );
      }

      //italic
      int itIndx = dataDefinedColumnIndex( QgsPalLayerSettings::Italic, labelSettings, vlayer );
      if ( itIndx != -1 )
      {
        font.setItalic( f.attribute( itIndx ).toBool() );
      }

      // TODO: Add other font data defined values (word spacing, etc.)
    }
  }

  return font;
}

bool QgsMapToolLabel::currentLabelPreserveRotation()
{
  if ( mCurrentLabel.valid )
  {
    return mCurrentLabel.settings.preserveRotation;
  }

  return true; // default, so there is no accidental data loss
}

bool QgsMapToolLabel::currentLabelRotationPoint( QgsPoint& pos, bool ignoreUpsideDown, bool rotatingUnpinned )
{
  QVector<QgsPoint> cornerPoints = mCurrentLabel.pos.cornerPoints;
  if ( cornerPoints.size() < 4 )
  {
    return false;
  }

  if ( mCurrentLabel.pos.upsideDown && !ignoreUpsideDown )
  {
    pos = cornerPoints.at( 2 );
  }
  else
  {
    pos = cornerPoints.at( 0 );
  }

  //alignment always center/center and rotation 0 for diagrams
  if ( mCurrentLabel.pos.isDiagram )
  {
    pos.setX( pos.x() + mCurrentLabel.pos.labelRect.width() / 2.0 );
    pos.setY( pos.y() + mCurrentLabel.pos.labelRect.height() / 2.0 );
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
  QFontMetricsF labelFontMetrics( mCurrentLabel.pos.labelFont );

  // NOTE: this assumes the label corner points comprise a rectangle and that the
  //       CRS supports equidistant measurements to accurately determine hypotenuse
  QgsPoint cp_0 = cornerPoints.at( 0 );
  QgsPoint cp_1 = cornerPoints.at( 1 );
  QgsPoint cp_3 = cornerPoints.at( 3 );
  //  QgsDebugMsg( QString( "cp_0: x=%1, y=%2" ).arg( cp_0.x() ).arg( cp_0.y() ) );
  //  QgsDebugMsg( QString( "cp_1: x=%1, y=%2" ).arg( cp_1.x() ).arg( cp_1.y() ) );
  //  QgsDebugMsg( QString( "cp_3: x=%1, y=%2" ).arg( cp_3.x() ).arg( cp_3.y() ) );
  double labelSizeX = qSqrt( cp_0.sqrDist( cp_1 ) );
  double labelSizeY = qSqrt( cp_0.sqrDist( cp_3 ) );

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
      ydiff = labelSizeY * 0.5 * ( 1 - descentRatio );
    }
  }

  double angle = mCurrentLabel.pos.rotation;
  double xd = xdiff * cos( angle ) - ydiff * sin( angle );
  double yd = xdiff * sin( angle ) + ydiff * cos( angle );
  if ( mCurrentLabel.pos.upsideDown && !ignoreUpsideDown )
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

#if 0
bool QgsMapToolLabel::hasDataDefinedColumn( QgsPalLayerSettings::DataDefinedProperties p, QgsVectorLayer* vlayer ) const
{
  Q_FOREACH ( const QString& providerId, vlayer->labeling()->subProviders() )
  {
    if ( QgsPalLayerSettings* settings = vlayer->labeling()->settings( vlayer, providerId ) )
    {
      QString fieldname = dataDefinedColumnName( p, *settings );
      if ( !fieldname.isEmpty() )
        return true;
    }
  }
  return false;
}
#endif

QString QgsMapToolLabel::dataDefinedColumnName( QgsPalLayerSettings::DataDefinedProperties p, const QgsPalLayerSettings& labelSettings ) const
{
  //QgsDebugMsg( QString( "dataDefinedProperties count:%1" ).arg( labelSettings.dataDefinedProperties.size() ) );

  QMap< QgsPalLayerSettings::DataDefinedProperties, QgsDataDefined* >::const_iterator dIt = labelSettings.dataDefinedProperties.constFind( p );
  if ( dIt != labelSettings.dataDefinedProperties.constEnd() )
  {
    QgsDataDefined* dd = dIt.value();

    // can only modify attributes that are data defined with a mapped field
    if ( dd->isActive() && !dd->useExpression() && !dd->field().isEmpty() )
      return dd->field();
  }
  return QString();
}

int QgsMapToolLabel::dataDefinedColumnIndex( QgsPalLayerSettings::DataDefinedProperties p, const QgsPalLayerSettings& labelSettings, const QgsVectorLayer* vlayer ) const
{
  QString fieldname = dataDefinedColumnName( p, labelSettings );
  if ( !fieldname.isEmpty() )
    return vlayer->fieldNameIndex( fieldname );
  return -1;
}

bool QgsMapToolLabel::currentLabelDataDefinedPosition( double& x, bool& xSuccess, double& y, bool& ySuccess, int& xCol, int& yCol ) const
{
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  QgsFeatureId featureId = mCurrentLabel.pos.featureId;

  xSuccess = false;
  ySuccess = false;

  if ( !vlayer )
  {
    return false;
  }

  if ( mCurrentLabel.pos.isDiagram )
  {
    if ( !diagramMoveable( vlayer, xCol, yCol ) )
    {
      return false;
    }
  }
  else if ( !labelMoveable( vlayer, mCurrentLabel.settings, xCol, yCol ) )
  {
    return false;
  }

  QgsFeature f;
  if ( !vlayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setFlags( QgsFeatureRequest::NoGeometry ) ).nextFeature( f ) )
  {
    return false;
  }

  QgsAttributes attributes = f.attributes();
  if ( !attributes.at( xCol ).isNull() )
    x = attributes.at( xCol ).toDouble( &xSuccess );
  if ( !attributes.at( yCol ).isNull() )
    y = attributes.at( yCol ).toDouble( &ySuccess );

  return true;
}

bool QgsMapToolLabel::layerIsRotatable( QgsVectorLayer* vlayer, int& rotationCol ) const
{
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  Q_FOREACH ( const QString& providerId, vlayer->labeling()->subProviders() )
  {
    if ( labelIsRotatable( vlayer, vlayer->labeling()->settings( vlayer, providerId ), rotationCol ) )
      return true;
  }

  return false;
}

bool QgsMapToolLabel::labelIsRotatable( QgsVectorLayer* layer, const QgsPalLayerSettings& settings, int& rotationCol ) const
{
  QString rColName = dataDefinedColumnName( QgsPalLayerSettings::Rotation, settings );
  rotationCol = layer->fieldNameIndex( rColName );
  return rotationCol != -1;
}


bool QgsMapToolLabel::currentLabelDataDefinedRotation( double& rotation, bool& rotationSuccess, int& rCol, bool ignoreXY ) const
{
  QgsVectorLayer* vlayer = mCurrentLabel.layer;
  QgsFeatureId featureId = mCurrentLabel.pos.featureId;

  rotationSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  if ( !labelIsRotatable( vlayer, mCurrentLabel.settings, rCol ) )
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
    if ( !currentLabelDataDefinedPosition( x, xSuccess, y, ySuccess, xCol, yCol ) || !xSuccess || !ySuccess )
    {
      return false;
    }
  }

  rotation = f.attribute( rCol ).toDouble( &rotationSuccess );
  return true;
}

bool QgsMapToolLabel::dataDefinedShowHide( QgsVectorLayer* vlayer, QgsFeatureId featureId, int& show, bool& showSuccess, int& showCol ) const
{
  showSuccess = false;
  if ( !vlayer )
  {
    return false;
  }

  if ( mCurrentLabel.pos.isDiagram )
  {
    if ( ! diagramCanShowHide( vlayer, showCol ) )
    {
      return false;
    }
  }
  else if ( ! labelCanShowHide( vlayer, showCol ) )
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

bool QgsMapToolLabel::diagramMoveable( QgsVectorLayer* vlayer, int& xCol, int& yCol ) const
{
  if ( vlayer && vlayer->diagramsEnabled() )
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

bool QgsMapToolLabel::labelMoveable( QgsVectorLayer *vlayer, int& xCol, int& yCol ) const
{
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  Q_FOREACH ( const QString& providerId, vlayer->labeling()->subProviders() )
  {
    if ( labelMoveable( vlayer, vlayer->labeling()->settings( vlayer, providerId ), xCol, yCol ) )
      return true;
  }

  return false;
}

bool QgsMapToolLabel::labelMoveable( QgsVectorLayer* vlayer, const QgsPalLayerSettings& settings, int& xCol, int& yCol ) const
{
  QString xColName = dataDefinedColumnName( QgsPalLayerSettings::PositionX, settings );
  QString yColName = dataDefinedColumnName( QgsPalLayerSettings::PositionY, settings );
  //return !xColName.isEmpty() && !yColName.isEmpty();
  xCol = vlayer->fieldNameIndex( xColName );
  yCol = vlayer->fieldNameIndex( yColName );
  return ( xCol != -1 && yCol != -1 );
}

bool QgsMapToolLabel::layerCanPin( QgsVectorLayer* vlayer, int& xCol, int& yCol ) const
{
  // currently same as QgsMapToolLabel::labelMoveable, but may change
  bool canPin = labelMoveable( vlayer, xCol, yCol );
  return canPin;
}

bool QgsMapToolLabel::labelCanShowHide( QgsVectorLayer* vlayer, int& showCol ) const
{
  if ( !vlayer || !vlayer->isEditable() )
  {
    return false;
  }

  Q_FOREACH ( const QString& providerId, vlayer->labeling()->subProviders() )
  {
    QString fieldname = dataDefinedColumnName( QgsPalLayerSettings::Show,
                        vlayer->labeling()->settings( vlayer, providerId ) );
    showCol = vlayer->fieldNameIndex( fieldname );
    if ( showCol != -1 )
      return true;
  }

  return false;
}

bool QgsMapToolLabel::isPinned()
{
  bool rc = false;

  if ( ! mCurrentLabel.pos.isDiagram )
  {
    rc = mCurrentLabel.pos.isPinned;
  }
  else
  {
    // for diagrams, the isPinned attribute is not set. So we check directly if
    // there's data defined.
    int xCol, yCol;
    double x, y;
    bool xSuccess, ySuccess;

    if ( currentLabelDataDefinedPosition( x, xSuccess, y, ySuccess, xCol, yCol ) && xSuccess && ySuccess )
      rc = true;
  }

  return rc;
}

bool QgsMapToolLabel::diagramCanShowHide( QgsVectorLayer* vlayer, int& showCol ) const
{
  bool rc = false;

  if ( vlayer && vlayer->isEditable() && vlayer->diagramsEnabled() )
  {
    const QgsDiagramLayerSettings *dls = vlayer->diagramLayerSettings();

    if ( dls && dls->showColumn >= 0 )
    {
      showCol = dls->showColumn;
      rc = true;
    }
  }

  return rc;
}

//

QgsMapToolLabel::LabelDetails::LabelDetails( const QgsLabelPosition& p )
    : valid( false )
    , pos( p )
{
  layer = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( pos.layerID ) );
  if ( layer && layer->labeling() )
  {
    settings = layer->labeling()->settings( layer, pos.providerID );

    if ( p.isDiagram )
      valid = layer->diagramsEnabled();
    else
      valid = settings.enabled;
  }

  if ( !valid )
  {
    layer = nullptr;
    settings = QgsPalLayerSettings();
  }
}
