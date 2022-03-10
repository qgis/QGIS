/***************************************************************************
                         qgsprocessingbatch.h
                         ------------------------
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

#ifndef QGSPROCESSINGBATCH_H
#define QGSPROCESSINGBATCH_H

#include "qgis_core.h"
#include "qgis.h"

#include "qgsprocessingfeedback.h"

/**
 * \class QgsProcessingBatchFeedback
 * \ingroup core
 * \brief Processing feedback subclass for use when batch processing.
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProcessingBatchFeedback : public QgsProcessingMultiStepFeedback
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingBatchFeedback, for a process with the specified
     * number of \a tasks. This feedback object will proxy calls
     * to the specified \a feedback object.
     */
    QgsProcessingBatchFeedback( int tasks, QgsProcessingFeedback *feedback );

    void reportError( const QString &error, bool fatalError = false ) override;

    /**
     * Takes the current list of reported errors and clears the stored list of errors.
     */
    QStringList popErrors();

  private:

    QStringList mErrors;
};


#endif // QGSPROCESSINGBATCH_H


