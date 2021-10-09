/***************************************************************************
    qgsrendereditemdetails.h
    ----------------
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSRENDEREDITEMDETAILS_H
#define QGSRENDEREDITEMDETAILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"

#ifdef SIP_RUN
// this is needed for the "convert to subclass" code below to compile
% ModuleHeaderCode
#include "qgsrenderedannotationitemdetails.h"
% End
#endif

/**
 * \ingroup core
 * \brief Base class for detailed information about a rendered item.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsRenderedItemDetails
{
  public:

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsRenderedAnnotationItemDetails *>( sipCpp ) )
      sipType = sipType_QgsRenderedAnnotationItemDetails;
    else
      sipType = 0;
    SIP_END
#endif

    /**
     * Constructor for QgsRenderedItemDetails.
     */
    QgsRenderedItemDetails( const QString &layerId );

    virtual ~QgsRenderedItemDetails();

    /**
     * Returns the layer ID of the associated map layer.
     */
    QString layerId() const { return mLayerId; }

    /**
     * Returns the bounding box of the item (in map units).
     *
     * \see setBoundingBox()
     */
    QgsRectangle boundingBox() const { return mBounds; }

    /**
     * Sets the bounding box of the item (in map units).
     *
     * \see boundingBox()
     */
    void setBoundingBox( const QgsRectangle &bounds ) { mBounds = bounds; }

  private:

    QString mLayerId;
    QgsRectangle mBounds;
};

#endif // QGSRENDEREDITEMDETAILS_H
