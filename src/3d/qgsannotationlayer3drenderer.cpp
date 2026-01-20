/***************************************************************************
  qgsannotationlayer3drenderer.cpp
  --------------------------------------
  Date                 : January 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayer3drenderer.h"

#include "qgis.h"
#include "qgsannotationlayer.h"
#include "qgsannotationlayerchunkloader_p.h"
#include "qgsstyle.h"

//
// QgsAnnotationLayer3DRendererMetadata
//

QgsAnnotationLayer3DRendererMetadata::QgsAnnotationLayer3DRendererMetadata()
  : Qgs3DRendererAbstractMetadata( u"annotation"_s )
{
}

QgsAbstract3DRenderer *QgsAnnotationLayer3DRendererMetadata::createRenderer( QDomElement &elem, const QgsReadWriteContext &context )
{
  auto r = std::make_unique< QgsAnnotationLayer3DRenderer >();
  r->readXml( elem, context );
  return r.release();
}


//
// QgsAnnotationLayer3DRenderer
//

QgsAnnotationLayer3DRenderer::QgsAnnotationLayer3DRenderer()
{
  mTextFormat = QgsStyle::defaultStyle()->defaultTextFormat();
  mTextFormat.setNamedStyle( u"Bold"_s );
  mTextFormat.setSize( 20 );
}

void QgsAnnotationLayer3DRenderer::setLayer( QgsAnnotationLayer *layer )
{
  mLayerRef = QgsMapLayerRef( layer );
}

QgsAnnotationLayer *QgsAnnotationLayer3DRenderer::layer() const
{
  return qobject_cast<QgsAnnotationLayer *>( mLayerRef.layer );
}

void QgsAnnotationLayer3DRenderer::resolveReferences( const QgsProject &project )
{
  mLayerRef.resolve( &project );
}

bool QgsAnnotationLayer3DRenderer::showCalloutLines() const
{
  return mShowCalloutLines;
}

void QgsAnnotationLayer3DRenderer::setShowCalloutLines( bool show )
{
  mShowCalloutLines = show;
}

void QgsAnnotationLayer3DRenderer::setCalloutLineColor( const QColor &color )
{
  mCalloutLineColor = color;
}

QColor QgsAnnotationLayer3DRenderer::calloutLineColor() const
{
  return mCalloutLineColor;
}

void QgsAnnotationLayer3DRenderer::setCalloutLineWidth( double width )
{
  mCalloutLineWidth = width;
}

double QgsAnnotationLayer3DRenderer::calloutLineWidth() const
{
  return mCalloutLineWidth;
}

QgsTextFormat QgsAnnotationLayer3DRenderer::textFormat() const
{
  return mTextFormat;
}

void QgsAnnotationLayer3DRenderer::setTextFormat( const QgsTextFormat &format )
{
  mTextFormat = format;
}

QString QgsAnnotationLayer3DRenderer::type() const
{
  return "annotation";
}

QgsAnnotationLayer3DRenderer *QgsAnnotationLayer3DRenderer::clone() const
{
  auto r = std::make_unique< QgsAnnotationLayer3DRenderer >();
  r->mLayerRef = mLayerRef;
  r->mAltClamping = mAltClamping;
  r->mZOffset = mZOffset;
  r->mShowCalloutLines = mShowCalloutLines;
  r->mCalloutLineColor = mCalloutLineColor;
  r->mCalloutLineWidth = mCalloutLineWidth;
  r->mTextFormat = mTextFormat;
  return r.release();
}

Qt3DCore::QEntity *QgsAnnotationLayer3DRenderer::createEntity( Qgs3DMapSettings *map ) const
{
  QgsAnnotationLayer *l = layer();

  if ( !l )
    return nullptr;

  // For some cases we start with a maximal z range because we can't know this upfront, as it potentially involves terrain heights.
  // This range will be refined after populating the nodes to the actual z range of the generated chunks nodes.
  // Assuming the vertical height is in meter, then it's extremely unlikely that a real vertical
  // height will exceed this amount!
  constexpr double MINIMUM_ANNOTATION_Z_ESTIMATE = -100000;
  constexpr double MAXIMUM_ANNOTATION_Z_ESTIMATE = 100000;

  double minimumZ = MINIMUM_ANNOTATION_Z_ESTIMATE;
  double maximumZ = MAXIMUM_ANNOTATION_Z_ESTIMATE;
  switch ( mAltClamping )
  {
    case Qgis::AltitudeClamping::Absolute:
      // special case where we DO know the exact z range upfront!
      minimumZ = mZOffset;
      maximumZ = mZOffset;
      break;

    case Qgis::AltitudeClamping::Relative:
    case Qgis::AltitudeClamping::Terrain:
      break;
  }

  return new QgsAnnotationLayerChunkedEntity( map, l, mAltClamping, mZOffset, mShowCalloutLines, mCalloutLineColor, mCalloutLineWidth, mTextFormat, minimumZ, maximumZ );
}

void QgsAnnotationLayer3DRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( u"layer"_s, mLayerRef.layerId );
  elem.setAttribute( u"clamping"_s, qgsEnumValueToKey( mAltClamping ) );
  elem.setAttribute( u"offset"_s, mZOffset );
  if ( mShowCalloutLines )
    elem.setAttribute( u"callouts"_s, u"1"_s );

  if ( mTextFormat.isValid() )
  {
    elem.appendChild( mTextFormat.writeXml( doc, context ) );
  }
}

void QgsAnnotationLayer3DRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mLayerRef = QgsMapLayerRef( elem.attribute( u"layer"_s ) );
  mAltClamping = qgsEnumKeyToValue( elem.attribute( u"clamping"_s ), Qgis::AltitudeClamping::Relative );
  mZOffset = elem.attribute( u"offset"_s, QString::number( DEFAULT_Z_OFFSET ) ).toDouble();
  mShowCalloutLines = elem.attribute( u"callouts"_s, u"0"_s ).toInt();
  if ( !elem.firstChildElement( u"text-style"_s ).isNull() )
  {
    mTextFormat = QgsTextFormat();
    mTextFormat.readXml( elem.firstChildElement( u"text-style"_s ), context );
  }
}
