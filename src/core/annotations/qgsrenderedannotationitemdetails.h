/***************************************************************************
    qgsrenderedannotationitemdetails.h
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

#ifndef QGSRENDEREDANNOTATIONITEMDETAILS_H
#define QGSRENDEREDANNOTATIONITEMDETAILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrendereditemdetails.h"

/**
 * \ingroup core
 * \brief Contains information about a rendered annotation item.
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsRenderedAnnotationItemDetails : public QgsRenderedItemDetails
{
  public:

    /**
     * Constructor for QgsRenderedAnnotationItemDetails.
     */
    QgsRenderedAnnotationItemDetails( const QString &layerId, const QString &itemId );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRenderedAnnotationItemDetails: %1 - %2>" ).arg( sipCpp->layerId(), sipCpp->itemId() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the item ID of the associated annotation item.
     */
    QString itemId() const { return mItemId; }

  private:

    QString mItemId;

};

#endif // QGSRENDEREDANNOTATIONITEMDETAILS_H
