/***************************************************************************
     qgsfeaturestore.h
     --------------------------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Radim Blazek
    Email                : radim.blazek@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATURESTORE_H
#define QGSFEATURESTORE_H

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include <QList>
#include <QMetaType>
#include <QVariant>

/** \ingroup core
 * Container for features with the same fields and crs.
 */
class CORE_EXPORT QgsFeatureStore
{
  public:
    //! Constructor
    QgsFeatureStore();

    //! Constructor
    QgsFeatureStore( const QgsFeatureStore &rhs );

    //! Constructor
    QgsFeatureStore( const QgsFields& fields, const QgsCoordinateReferenceSystem& crs );

    //! Destructor
    ~QgsFeatureStore();

    /** Get fields list */
    QgsFields& fields() { return mFields; }

    /** Set fields. Resets feauters fields to pointer to new internal fields. */
    void setFields( const QgsFields & fields );

    /** Get crs */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /** Set crs */
    void setCrs( const QgsCoordinateReferenceSystem& crs ) { mCrs = crs; }

    /** Add feature. Feature's fields will be set to pointer to the store fields.
     * @param feature
     * @note added in 2.1
     */
    void addFeature( const QgsFeature& feature );

    /** Get features list reference */
    QgsFeatureList& features() { return mFeatures; }

    /** Set map of optional parameters */
    void setParams( const QMap<QString, QVariant> &theParams ) { mParams = theParams; }

    /** Get map of optional parameters */
    QMap<QString, QVariant> params() const { return mParams; }

  private:
    QgsFields mFields;

    QgsCoordinateReferenceSystem mCrs;

    QgsFeatureList mFeatures;

    // Optional parameters
    QMap<QString, QVariant> mParams;
};

typedef QList<QgsFeatureStore> QgsFeatureStoreList;

Q_DECLARE_METATYPE( QgsFeatureStore );

Q_DECLARE_METATYPE( QgsFeatureStoreList );

#endif
