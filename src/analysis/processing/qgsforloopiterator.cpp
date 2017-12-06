/***************************************************************************
                         qgsalgorithmbuffer.cpp
                         ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Arnaud Morvan
    email                : arnaud dot morvan at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsforloopiterator.h"

///@cond PRIVATE

QString QgsForLoopIterator::name() const
{
  return QStringLiteral( "forloop" );
}

QString QgsForLoopIterator::displayName() const
{
  return QObject::tr( "For loop iterator" );
}

QStringList QgsForLoopIterator::tags() const
{
  return QObject::tr( "for,loop,iterator" ).split( ',' );
}

QString QgsForLoopIterator::group() const
{
  return QObject::tr( "Iterators" );
}

void QgsForLoopIterator::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "FROM" ), QObject::tr( "From" ), QgsProcessingParameterNumber::Integer ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TO" ), QObject::tr( "To" ), QgsProcessingParameterNumber::Integer ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "STEP" ), QObject::tr( "Step" ), QgsProcessingParameterNumber::Integer ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "CURRENT_VALUE" ), QObject::tr( "Current value" ) ) );
}

QString QgsForLoopIterator::shortHelpString() const
{
  return QObject::tr( "This algorithm iterate over a for loop." );
}

QgsForLoopIterator *QgsForLoopIterator::createInstance() const
{
  return new QgsForLoopIterator();
}

bool QgsForLoopIterator::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mCurrentValue = parameterAsInt( parameters, "FROM", context );
  mStep = parameterAsInt( parameters, "STEP", context );
  mTo = parameterAsInt( parameters, "TO", context );
  return true;
}

QVariantMap QgsForLoopIterator::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "CURRENT_VALUE" ), mCurrentValue );
  return outputs;
}

bool QgsForLoopIterator::next()
{
  mCurrentValue += mStep;
  return mCurrentValue != mTo;
}

///@endcond
