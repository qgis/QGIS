/***************************************************************************
                         qgstiledmeshrequest.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHREQUEST_H
#define QGSTILEDMESHREQUEST_H

#include "qgis_core.h"
#include "qgis.h"

class QgsFeedback;

/**
 * \ingroup core
 *
 * \brief Tiled mesh data request.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshRequest
{
  public:

    QgsTiledMeshRequest();

    /**
     * Returns the required geometric error treshold for the returned nodes, in
     * mesh CRS units.
     *
     * If the error is 0 then no geometric error filtering will be applied.
     *
     * \see setRequiredGeometricError()
     */
    double requiredGeometricError() const { return mRequiredGeometricError; }

    /**
     * Sets the required geometric \a error treshold for the returned nodes, in
     * mesh CRS units.
     *
     * If the \a error is 0 then no geometric error filtering will be applied.
     *
     * \see requiredGeometricError()
     */
    void setRequiredGeometricError( double error ) { mRequiredGeometricError = error; }

    /**
     * Attach a \a feedback object that can be queried regularly by the request to check
     * if it should be canceled.
     *
     * Ownership of \a feedback is NOT transferred, and the caller must take care that it exists
     * for the lifetime of the request.
     *
     * \see feedback()
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the feedback object that can be queried regularly by the request to check
     * if it should be canceled, if set.
     *
     * \see setFeedback()
     */
    QgsFeedback *feedback() const;

  private:

    QgsFeedback *mFeedback = nullptr;
    double mRequiredGeometricError = 0;
};


#endif // QGSTILEDMESHREQUEST_H
