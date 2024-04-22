/***************************************************************************
                         qgsprocessingmodelresult.cpp
                         ----------------------
    begin                : April 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsprocessingmodelresult.h"

//
// QgsProcessingModelChildResult
//

QgsProcessingModelChildAlgorithmResult::QgsProcessingModelChildAlgorithmResult() = default;


//
// QgsProcessingModelResult
//

QgsProcessingModelResult::QgsProcessingModelResult() = default;

void QgsProcessingModelResult::clear()
{
  mChildResults.clear();
  mExecutedChildren.clear();
  mRawChildInputs.clear();
  mRawChildOutputs.clear();
}

void QgsProcessingModelResult::mergeWith( const QgsProcessingModelResult &other )
{
  for ( auto it = other.mChildResults.constBegin(); it != other.mChildResults.constEnd(); ++it )
  {
    mChildResults.insert( it.key(), it.value() );
  }
  mExecutedChildren.unite( other.mExecutedChildren );
  for ( auto it = other.mRawChildInputs.constBegin(); it != other.mRawChildInputs.constEnd(); ++it )
  {
    mRawChildInputs.insert( it.key(), it.value() );
  }
  for ( auto it = other.mRawChildOutputs.constBegin(); it != other.mRawChildOutputs.constEnd(); ++it )
  {
    mRawChildOutputs.insert( it.key(), it.value() );
  }
}
