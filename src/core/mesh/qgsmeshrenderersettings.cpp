/***************************************************************************
                         qgsmeshrenderersettings.cpp
                         ---------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderersettings.h"

#include "qgssymbollayerutils.h"


bool QgsMeshRendererMeshSettings::isEnabled() const
{
  return mEnabled;
}

void QgsMeshRendererMeshSettings::setEnabled( bool on )
{
  mEnabled = on;
}

double QgsMeshRendererMeshSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererMeshSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererMeshSettings::color() const
{
  return mColor;
}

void QgsMeshRendererMeshSettings::setColor( const QColor &color )
{
  mColor = color;
}

QDomElement QgsMeshRendererMeshSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( "mesh-settings" );
  elem.setAttribute( "enabled", mEnabled ? "1" : "0" );
  elem.setAttribute( "line-width", mLineWidth );
  elem.setAttribute( "color", QgsSymbolLayerUtils::encodeColor( mColor ) );
  return elem;
}

void QgsMeshRendererMeshSettings::readXml( const QDomElement &elem )
{
  mEnabled = elem.attribute( "enabled" ).toInt();
  mLineWidth = elem.attribute( "line-width" ).toDouble();
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( "color" ) );
}

// ---------------------------------------------------------------------

QgsColorRampShader QgsMeshRendererScalarSettings::colorRampShader() const
{
  return mColorRampShader;
}

void QgsMeshRendererScalarSettings::setColorRampShader( const QgsColorRampShader &shader )
{
  mColorRampShader = shader;
}

void QgsMeshRendererScalarSettings::setClassificationMinMax( double vMin, double vMax )
{
  mClassificationMin = vMin;
  mClassificationMax = vMax;
}

QDomElement QgsMeshRendererScalarSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( "scalar-settings" );
  elem.setAttribute( "min-val", mClassificationMin );
  elem.setAttribute( "max-val", mClassificationMax );
  QDomElement elemShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemShader );
  return elem;
}

void QgsMeshRendererScalarSettings::readXml( const QDomElement &elem )
{
  mClassificationMin = elem.attribute( "min-val" ).toDouble();
  mClassificationMax = elem.attribute( "max-val" ).toDouble();
  QDomElement elemShader = elem.firstChildElement( QStringLiteral( "colorrampshader" ) );
  mColorRampShader.readXml( elemShader );
}

// ---------------------------------------------------------------------

double QgsMeshRendererVectorSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererVectorSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererVectorSettings::color() const
{
  return mColor;
}

void QgsMeshRendererVectorSettings::setColor( const QColor &vectorColor )
{
  mColor = vectorColor;
}

double QgsMeshRendererVectorSettings::filterMin() const
{
  return mFilterMin;
}

void QgsMeshRendererVectorSettings::setFilterMin( double vectorFilterMin )
{
  mFilterMin = vectorFilterMin;
}

double QgsMeshRendererVectorSettings::filterMax() const
{
  return mFilterMax;
}

void QgsMeshRendererVectorSettings::setFilterMax( double vectorFilterMax )
{
  mFilterMax = vectorFilterMax;
}

QgsMeshRendererVectorSettings::ArrowScalingMethod QgsMeshRendererVectorSettings::shaftLengthMethod() const
{
  return mShaftLengthMethod;
}

void QgsMeshRendererVectorSettings::setShaftLengthMethod( QgsMeshRendererVectorSettings::ArrowScalingMethod shaftLengthMethod )
{
  mShaftLengthMethod = shaftLengthMethod;
}

double QgsMeshRendererVectorSettings::minShaftLength() const
{
  return mMinShaftLength;
}

void QgsMeshRendererVectorSettings::setMinShaftLength( double minShaftLength )
{
  mMinShaftLength = minShaftLength;
}

double QgsMeshRendererVectorSettings::maxShaftLength() const
{
  return mMaxShaftLength;
}

void QgsMeshRendererVectorSettings::setMaxShaftLength( double maxShaftLength )
{
  mMaxShaftLength = maxShaftLength;
}

double QgsMeshRendererVectorSettings::scaleFactor() const
{
  return mScaleFactor;
}

void QgsMeshRendererVectorSettings::setScaleFactor( double scaleFactor )
{
  mScaleFactor = scaleFactor;
}

double QgsMeshRendererVectorSettings::fixedShaftLength() const
{
  return mFixedShaftLength;
}

void QgsMeshRendererVectorSettings::setFixedShaftLength( double fixedShaftLength )
{
  mFixedShaftLength = fixedShaftLength;
}

double QgsMeshRendererVectorSettings::arrowHeadWidthRatio() const
{
  return mArrowHeadWidthRatio;
}

void QgsMeshRendererVectorSettings::setArrowHeadWidthRatio( double vectorHeadWidthRatio )
{
  mArrowHeadWidthRatio = vectorHeadWidthRatio;
}

double QgsMeshRendererVectorSettings::arrowHeadLengthRatio() const
{
  return mArrowHeadLengthRatio;
}

void QgsMeshRendererVectorSettings::setArrowHeadLengthRatio( double vectorHeadLengthRatio )
{
  mArrowHeadLengthRatio = vectorHeadLengthRatio;
}

QDomElement QgsMeshRendererVectorSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( "vector-settings" );
  elem.setAttribute( "line-width", mLineWidth );
  elem.setAttribute( "color", QgsSymbolLayerUtils::encodeColor( mColor ) );
  elem.setAttribute( "filter-min", mFilterMin );
  elem.setAttribute( "filter-max", mFilterMax );
  elem.setAttribute( "arrow-head-width-ratio", mArrowHeadWidthRatio );
  elem.setAttribute( "arrow-head-length-ratio", mArrowHeadLengthRatio );

  QDomElement elemShaft = doc.createElement( "shaft-length" );
  QString methodTxt;
  switch ( mShaftLengthMethod )
  {
    case MinMax:
      methodTxt = "minmax";
      elemShaft.setAttribute( "min", mMinShaftLength );
      elemShaft.setAttribute( "max", mMaxShaftLength );
      break;
    case Scaled:
      methodTxt = "scaled";
      elemShaft.setAttribute( "scale-factor", mScaleFactor );
      break;
    case Fixed:
      methodTxt = "fixed";
      elemShaft.setAttribute( "fixed-length", mFixedShaftLength );
      break;
  }
  elemShaft.setAttribute( "method", methodTxt );
  elem.appendChild( elemShaft );
  return elem;
}

void QgsMeshRendererVectorSettings::readXml( const QDomElement &elem )
{
  mLineWidth = elem.attribute( "line-width" ).toDouble();
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( "color" ) );
  mFilterMin = elem.attribute( "filter-min" ).toDouble();
  mFilterMax = elem.attribute( "filter-max" ).toDouble();
  mArrowHeadWidthRatio = elem.attribute( "arrow-head-width-ratio" ).toDouble();
  mArrowHeadLengthRatio = elem.attribute( "arrow-head-length-ratio" ).toDouble();

  QDomElement elemShaft = elem.firstChildElement( "shaft-length" );
  QString methodTxt = elemShaft.attribute( "method" );
  if ( methodTxt == "minmax" )
  {
    mShaftLengthMethod = MinMax;
    mMinShaftLength = elemShaft.attribute( "min" ).toDouble();
    mMaxShaftLength = elemShaft.attribute( "max" ).toDouble();
  }
  else if ( methodTxt == "scaled" )
  {
    mShaftLengthMethod = Scaled;
    mScaleFactor = elemShaft.attribute( "scale-factor" ).toDouble();
  }
  else  // fixed
  {
    mShaftLengthMethod = Fixed;
    mFixedShaftLength = elemShaft.attribute( "fixed-length" ).toDouble();
  }
}

// ---------------------------------------------------------------------

QDomElement QgsMeshRendererSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( "mesh-renderer-settings" );

  QDomElement elemActiveDataset = doc.createElement( "active-dataset" );
  if ( mActiveScalarDataset.isValid() )
    elemActiveDataset.setAttribute( "scalar", QString( "%1,%2" ).arg( mActiveScalarDataset.group() ).arg( mActiveScalarDataset.dataset() ) );
  if ( mActiveVectorDataset.isValid() )
    elemActiveDataset.setAttribute( "vector", QString( "%1,%2" ).arg( mActiveVectorDataset.group() ).arg( mActiveVectorDataset.dataset() ) );
  elem.appendChild( elemActiveDataset );

  for ( int groupIndex : mRendererScalarSettings.keys() )
  {
    const QgsMeshRendererScalarSettings &scalarSettings = mRendererScalarSettings[groupIndex];
    QDomElement elemScalar = scalarSettings.writeXml( doc );
    elemScalar.setAttribute( "group", groupIndex );
    elem.appendChild( elemScalar );
  }

  for ( int groupIndex : mRendererVectorSettings.keys() )
  {
    const QgsMeshRendererVectorSettings &vectorSettings = mRendererVectorSettings[groupIndex];
    QDomElement elemVector = vectorSettings.writeXml( doc );
    elemVector.setAttribute( "group", groupIndex );
    elem.appendChild( elemVector );
  }

  QDomElement elemNativeMesh = mRendererNativeMeshSettings.writeXml( doc );
  elemNativeMesh.setTagName( "mesh-settings-native" );
  elem.appendChild( elemNativeMesh );

  QDomElement elemTriangularMesh = mRendererTriangularMeshSettings.writeXml( doc );
  elemTriangularMesh.setTagName( "mesh-settings-triangular" );
  elem.appendChild( elemTriangularMesh );

  return elem;
}

void QgsMeshRendererSettings::readXml( const QDomElement &elem )
{
  mRendererScalarSettings.clear();
  mRendererVectorSettings.clear();

  QDomElement elemActiveDataset = elem.firstChildElement( "active-dataset" );
  if ( elemActiveDataset.hasAttribute( "scalar" ) )
  {
    QStringList lst = elemActiveDataset.attribute( "scalar" ).split( QChar( ',' ) );
    if ( lst.count() == 2 )
      mActiveScalarDataset = QgsMeshDatasetIndex( lst[0].toInt(), lst[1].toInt() );
  }
  if ( elemActiveDataset.hasAttribute( "vector" ) )
  {
    QStringList lst = elemActiveDataset.attribute( "vector" ).split( QChar( ',' ) );
    if ( lst.count() == 2 )
      mActiveVectorDataset = QgsMeshDatasetIndex( lst[0].toInt(), lst[1].toInt() );
  }

  QDomElement elemScalar = elem.firstChildElement( "scalar-settings" );
  while ( !elemScalar.isNull() )
  {
    int groupIndex = elemScalar.attribute( "group" ).toInt();
    QgsMeshRendererScalarSettings scalarSettings;
    scalarSettings.readXml( elemScalar );
    mRendererScalarSettings.insert( groupIndex, scalarSettings );

    elemScalar = elemScalar.nextSiblingElement( "scalar-settings" );
  }

  QDomElement elemVector = elem.firstChildElement( "vector-settings" );
  while ( !elemVector.isNull() )
  {
    int groupIndex = elemVector.attribute( "group" ).toInt();
    QgsMeshRendererVectorSettings vectorSettings;
    vectorSettings.readXml( elemVector );
    mRendererVectorSettings.insert( groupIndex, vectorSettings );

    elemVector = elemVector.nextSiblingElement( "vector-settings" );
  }

  QDomElement elemNativeMesh = elem.firstChildElement( "mesh-settings-native" );
  mRendererNativeMeshSettings.readXml( elemNativeMesh );

  QDomElement elemTriangularMesh = elem.firstChildElement( "mesh-settings-triangular" );
  mRendererTriangularMeshSettings.readXml( elemTriangularMesh );
}
