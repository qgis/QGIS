/***************************************************************************
                         qgsprocessingoutputs.cpp
                         -------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsprocessingoutputs.h"

QgsProcessingOutputDefinition::QgsProcessingOutputDefinition( const QString &name, const QString &description )
  : mName( name )
  , mDescription( description )
{}

QgsProcessingOutputVectorLayer::QgsProcessingOutputVectorLayer( const QString &name, const QString &description, QgsProcessing::SourceType type )
  : QgsProcessingOutputDefinition( name, description )
  , mDataType( type )
{}

QgsProcessing::SourceType QgsProcessingOutputVectorLayer::dataType() const
{
  return mDataType;
}

void QgsProcessingOutputVectorLayer::setDataType( QgsProcessing::SourceType type )
{
  mDataType = type;
}

QgsProcessingOutputRasterLayer::QgsProcessingOutputRasterLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputPointCloudLayer::QgsProcessingOutputPointCloudLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputHtml::QgsProcessingOutputHtml( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputNumber::QgsProcessingOutputNumber( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputString::QgsProcessingOutputString( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputBoolean::QgsProcessingOutputBoolean( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputFolder::QgsProcessingOutputFolder( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputFile::QgsProcessingOutputFile( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QgsProcessingOutputMapLayer::QgsProcessingOutputMapLayer( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputMapLayer::type() const
{
  return typeName();
}

QgsProcessingOutputMultipleLayers::QgsProcessingOutputMultipleLayers( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}

QString QgsProcessingOutputMultipleLayers::type() const
{
  return typeName();
}

QgsProcessingOutputConditionalBranch::QgsProcessingOutputConditionalBranch( const QString &name, const QString &description )
  : QgsProcessingOutputDefinition( name, description )
{}
