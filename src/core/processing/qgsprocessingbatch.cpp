/***************************************************************************
                         qgsprocessingbatch.cpp
                         ------------------------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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

#include "qgsprocessingbatch.h"

QgsProcessingBatchFeedback::QgsProcessingBatchFeedback( int tasks, QgsProcessingFeedback *feedback )
  :  QgsProcessingMultiStepFeedback( tasks, feedback )
{

}

void QgsProcessingBatchFeedback::reportError( const QString &error, bool fatalError )
{
  mErrors.append( error );
  QgsProcessingMultiStepFeedback::reportError( error, fatalError );
}

QStringList QgsProcessingBatchFeedback::popErrors()
{
  QStringList res = mErrors;
  mErrors.clear();
  return res;
}
