/***************************************************************************
    qgsannotationpointtextitem.cpp
    ----------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationpointtextitem.h"
#include "qgstextrenderer.h"
#include "qgsannotationitemnode.h"
#include "qgsannotationitemeditoperation.h"
#include "qgsrendercontext.h"
#include "qgsapplication.h"
#include "qgscalloutsregistry.h"

QgsAnnotationPointTextItem::QgsAnnotationPointTextItem( const QString &text, QgsPointXY point )
  : QgsAnnotationItem()
  , mText( text )
  , mPoint( point )
{

}

Qgis::AnnotationItemFlags QgsAnnotationPointTextItem::flags() const
{
  // in truth this should depend on whether the text format is scale dependent or not!
  return Qgis::AnnotationItemFlag::ScaleDependentBoundingBox
         | Qgis::AnnotationItemFlag::SupportsReferenceScale
         | Qgis::AnnotationItemFlag::SupportsCallouts;
}

QgsAnnotationPointTextItem::~QgsAnnotationPointTextItem() = default;

QString QgsAnnotationPointTextItem::type() const
{
  return QStringLiteral( "pointtext" );
}

void QgsAnnotationPointTextItem::render( QgsRenderContext &context, QgsFeedback *feedback )
{
  QPointF pt;
  if ( context.coordinateTransform().isValid() )
  {
    double x = mPoint.x();
    double y = mPoint.y();
    double z = 0.0;
    context.coordinateTransform().transformInPlace( x, y, z );
    pt = QPointF( x, y );
  }
  else
    pt = mPoint.toQPointF();

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  double angle = mAngle;
  switch ( mRotationMode )
  {
    case Qgis::SymbolRotationMode::RespectMapRotation:
      angle += context.mapToPixel().mapRotation();
      break;

    case Qgis::SymbolRotationMode::IgnoreMapRotation:
      break;
  }

  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  if ( callout() )
  {
    const double textWidth = QgsTextRenderer::textWidth(
                               context, mTextFormat, displayText.split( '\n' ) );
    const double textHeight = QgsTextRenderer::textHeight(
                                context, mTextFormat, displayText.split( '\n' ) );

    QgsCallout::QgsCalloutContext calloutContext;
    renderCallout( context, QRectF( pt.x(), pt.y() - textHeight, textWidth, textHeight ), angle, calloutContext, feedback );
  }

  QgsTextRenderer::drawText( pt, - angle * M_PI / 180.0,
                             QgsTextRenderer::convertQtHAlignment( mAlignment ),
                             mTextFormat.allowHtmlFormatting() ? QStringList{displayText }: displayText.split( '\n' ), context, mTextFormat );
}

bool QgsAnnotationPointTextItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "x" ), qgsDoubleToString( mPoint.x() ) );
  element.setAttribute( QStringLiteral( "y" ), qgsDoubleToString( mPoint.y() ) );
  element.setAttribute( QStringLiteral( "text" ), mText );
  element.setAttribute( QStringLiteral( "angle" ), qgsDoubleToString( mAngle ) );
  element.setAttribute( QStringLiteral( "alignment" ), QString::number( mAlignment ) );
  element.setAttribute( QStringLiteral( "rotationMode" ), qgsEnumValueToKey( mRotationMode ) );

  QDomElement textFormatElem = document.createElement( QStringLiteral( "pointTextFormat" ) );
  textFormatElem.appendChild( mTextFormat.writeXml( document, context ) );
  element.appendChild( textFormatElem );

  writeCommonProperties( element, document, context );
  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::create()
{
  return new QgsAnnotationPointTextItem( QString(), QgsPointXY() );
}

bool QgsAnnotationPointTextItem::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const double x = element.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = element.attribute( QStringLiteral( "y" ) ).toDouble();
  mPoint = QgsPointXY( x, y );
  mText = element.attribute( QStringLiteral( "text" ) );
  mAngle = element.attribute( QStringLiteral( "angle" ) ).toDouble();
  mAlignment = static_cast< Qt::Alignment >( element.attribute( QStringLiteral( "alignment" ) ).toInt() );
  mRotationMode = qgsEnumKeyToValue( element.attribute( QStringLiteral( "rotationMode" ) ), Qgis::SymbolRotationMode::IgnoreMapRotation );
  const QDomElement textFormatElem = element.firstChildElement( QStringLiteral( "pointTextFormat" ) );
  if ( !textFormatElem.isNull() )
  {
    const QDomNodeList textFormatNodeList = textFormatElem.elementsByTagName( QStringLiteral( "text-style" ) );
    const QDomElement textFormatElem = textFormatNodeList.at( 0 ).toElement();
    mTextFormat.readXml( textFormatElem, context );
  }

  readCommonProperties( element, context );
  return true;
}

QgsAnnotationPointTextItem *QgsAnnotationPointTextItem::clone() const
{
  std::unique_ptr< QgsAnnotationPointTextItem > item = std::make_unique< QgsAnnotationPointTextItem >( mText, mPoint );
  item->setFormat( mTextFormat );
  item->setAngle( mAngle );
  item->setAlignment( mAlignment );
  item->setRotationMode( mRotationMode );
  item->copyCommonProperties( this );
  return item.release();
}

QgsRectangle QgsAnnotationPointTextItem::boundingBox() const
{
  return QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x(), mPoint.y() );
}

QgsRectangle rotateBoundingBoxAroundPoint( double cx, double cy, const QgsRectangle &original, double angleClockwiseDegrees )
{
  QTransform t;
  t.translate( cx, cy );
  const double angleRadians = -M_PI * angleClockwiseDegrees / 180.0;
  t.rotateRadians( angleRadians );
  t.translate( -cx, -cy );
  const QRectF result = t.mapRect( original.toRectF() );
  return QgsRectangle( result );
}

QgsRectangle QgsAnnotationPointTextItem::boundingBox( QgsRenderContext &context ) const
{
  const QString displayText = QgsExpression::replaceExpressionText( mText, &context.expressionContext(), &context.distanceArea() );

  const double widthInPixels = QgsTextRenderer::textWidth( context, mTextFormat, mTextFormat.allowHtmlFormatting() ? QStringList{displayText }: displayText.split( '\n' ) );
  const double heightInPixels = QgsTextRenderer::textHeight( context, mTextFormat, mTextFormat.allowHtmlFormatting() ? QStringList{displayText }: displayText.split( '\n' ) );

  // text size has already been calculated using any symbology reference scale factor above -- we need
  // to temporarily remove the reference scale here or we'll be undoing the scaling
  QgsScopedRenderContextReferenceScaleOverride resetScaleFactor( context, -1.0 );
  const double widthInMapUnits = context.convertToMapUnits( widthInPixels, Qgis::RenderUnit::Pixels );
  const double heightInMapUnits = context.convertToMapUnits( heightInPixels, Qgis::RenderUnit::Pixels );

  double angle = mAngle;
  switch ( mRotationMode )
  {
    case Qgis::SymbolRotationMode::RespectMapRotation:
      angle += context.mapToPixel().mapRotation();
      break;

    case Qgis::SymbolRotationMode::IgnoreMapRotation:
      break;
  }

  QgsRectangle unrotatedRect;
  switch ( mAlignment & Qt::AlignHorizontal_Mask )
  {
    case Qt::AlignRight:
      unrotatedRect = QgsRectangle( mPoint.x() - widthInMapUnits, mPoint.y(), mPoint.x(), mPoint.y() + heightInMapUnits );
      break;

    case Qt::AlignHCenter:
      unrotatedRect = QgsRectangle( mPoint.x() - widthInMapUnits * 0.5, mPoint.y(), mPoint.x() + widthInMapUnits * 0.5, mPoint.y() + heightInMapUnits );
      break;

    default:
      unrotatedRect = QgsRectangle( mPoint.x(), mPoint.y(), mPoint.x() + widthInMapUnits, mPoint.y() + heightInMapUnits );
      break;
  }

  QgsRectangle textRect;
  if ( !qgsDoubleNear( angle, 0 ) )
  {
    textRect = rotateBoundingBoxAroundPoint( mPoint.x(), mPoint.y(), unrotatedRect, angle );
  }
  else
  {
    textRect = unrotatedRect;
  }

  if ( callout() && !calloutAnchor().isEmpty() )
  {
    QgsGeometry anchor = calloutAnchor();
    textRect.combineExtentWith( anchor.boundingBox() );
  }
  return textRect;
}

QList<QgsAnnotationItemNode> QgsAnnotationPointTextItem::nodesV2( const QgsAnnotationItemEditContext &context ) const
{
  QList<QgsAnnotationItemNode> res = { QgsAnnotationItemNode( QgsVertexId( 0, 0, 0 ), mPoint, Qgis::AnnotationItemNodeType::VertexHandle )};

  QgsPointXY calloutNodePoint;
  if ( !calloutAnchor().isEmpty() )
  {
    calloutNodePoint = calloutAnchor().asPoint();
  }
  else
  {
    calloutNodePoint = context.currentItemBounds().center();
  }
  res.append( QgsAnnotationItemNode( QgsVertexId( 0, 0, 1 ), calloutNodePoint, Qgis::AnnotationItemNodeType::CalloutHandle ) );

  return res;
}

Qgis::AnnotationItemEditOperationResult QgsAnnotationPointTextItem::applyEditV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      if ( moveOperation->nodeId().vertex == 0 )
      {
        mPoint = moveOperation->after();
      }
      else if ( moveOperation->nodeId().vertex == 1 )
      {
        setCalloutAnchor( QgsGeometry::fromPoint( moveOperation->after() ) );
        if ( !callout() )
        {
          setCallout( QgsApplication::calloutRegistry()->defaultCallout() );
        }
      }
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    {
      return Qgis::AnnotationItemEditOperationResult::ItemCleared;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      mPoint.setX( mPoint.x() + moveOperation->translationX() );
      mPoint.setY( mPoint.y() + moveOperation->translationY() );
      return Qgis::AnnotationItemEditOperationResult::Success;
    }

    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }

  return Qgis::AnnotationItemEditOperationResult::Invalid;
}

QgsAnnotationItemEditOperationTransientResults *QgsAnnotationPointTextItem::transientEditResultsV2( QgsAbstractAnnotationItemEditOperation *operation, const QgsAnnotationItemEditContext & )
{
  switch ( operation->type() )
  {
    case QgsAbstractAnnotationItemEditOperation::Type::MoveNode:
    {
      QgsAnnotationItemEditOperationMoveNode *moveOperation = dynamic_cast< QgsAnnotationItemEditOperationMoveNode * >( operation );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( moveOperation->after().clone() ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::TranslateItem:
    {
      QgsAnnotationItemEditOperationTranslateItem *moveOperation = qgis::down_cast< QgsAnnotationItemEditOperationTranslateItem * >( operation );
      return new QgsAnnotationItemEditOperationTransientResults( QgsGeometry( new QgsPoint( mPoint.x() + moveOperation->translationX(), mPoint.y() + moveOperation->translationY() ) ) );
    }

    case QgsAbstractAnnotationItemEditOperation::Type::DeleteNode:
    case QgsAbstractAnnotationItemEditOperation::Type::AddNode:
      break;
  }
  return nullptr;
}

QgsTextFormat QgsAnnotationPointTextItem::format() const
{
  return mTextFormat;
}

void QgsAnnotationPointTextItem::setFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

Qt::Alignment QgsAnnotationPointTextItem::alignment() const
{
  return mAlignment;
}

void QgsAnnotationPointTextItem::setAlignment( Qt::Alignment alignment )
{
  mAlignment = alignment;
}

Qgis::SymbolRotationMode QgsAnnotationPointTextItem::rotationMode() const
{
  return mRotationMode;
}

void QgsAnnotationPointTextItem::setRotationMode( Qgis::SymbolRotationMode mode )
{
  mRotationMode = mode;
}
