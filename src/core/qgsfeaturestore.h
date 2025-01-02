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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsfeaturesink.h"
#include "qgscoordinatereferencesystem.h"
#include <QList>
#include <QMetaType>
#include <QVariant>

/**
 * \ingroup core
 * \brief A container for features with the same fields and crs.
 */
class CORE_EXPORT QgsFeatureStore : public QgsFeatureSink
{
  public:
    QgsFeatureStore() = default;

    //! Constructor
    QgsFeatureStore( const QgsFields &fields, const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the store's field list.
     * \see setFields()
     */
    QgsFields fields() const { return mFields; }

    /**
     * Sets the store's \a fields. Every contained feature's fields will be reset to match \a fields.
     * \see fields()
     */
    void setFields( const QgsFields &fields );

    /**
     * Returns the store's coordinate reference system.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Sets the store's \a crs.
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; }

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;

    /**
     * Returns the number of features contained in the store.
     */
    int count() const { return mFeatures.size(); }

#ifdef SIP_RUN

    /**
     * Returns the number of features contained in the store.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->count();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End
#endif

    /**
     * Returns the list of features contained in the store.
     */
    QgsFeatureList features() const { return mFeatures; }

    /**
     * Sets a map of optional \a parameters for the store.
     * \see params()
     */
    void setParams( const QMap<QString, QVariant> &parameters ) { mParams = parameters; }

    /**
     * Returns the map of optional parameters.
     * \see setParams()
     */
    QMap<QString, QVariant> params() const { return mParams; }

  private:
    QgsFields mFields;

    QgsCoordinateReferenceSystem mCrs;

    QgsFeatureList mFeatures;

    // Optional parameters
    QMap<QString, QVariant> mParams;
};

#ifndef SIP_RUN
typedef QVector<QgsFeatureStore> QgsFeatureStoreList;
#else
typedef QVector<QgsFeatureStore> QgsFeatureStoreList;

% MappedType QgsFeatureStoreList
{
  % TypeHeaderCode
#include "qgsfeaturestore.h"
  % End

  % ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ( ( l = PyList_New( sipCpp->size() ) ) == NULL )
    return NULL;

  // Set the list elements.
  for ( int i = 0; i < sipCpp->size(); ++i )
  {
    QgsFeatureStore *v = new QgsFeatureStore( sipCpp->at( i ) );
    PyObject *tobj;

    if ( ( tobj = sipConvertFromNewType( v, sipType_QgsFeatureStore, Py_None ) ) == NULL )
    {
      Py_DECREF( l );
      delete v;

      return NULL;
    }

    PyList_SET_ITEM( l, i, tobj );
  }

  return l;
  % End

  % ConvertToTypeCode
  // Check the type if that is all that is required.
  if ( sipIsErr == NULL )
  {
    if ( !PyList_Check( sipPy ) )
      return 0;

    for ( SIP_SSIZE_T i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
      if ( !sipCanConvertToType( PyList_GET_ITEM( sipPy, i ), sipType_QgsFeatureStore, SIP_NOT_NONE ) )
        return 0;

    return 1;
  }

  QgsFeatureStoreList *qv = new QgsFeatureStoreList;
  SIP_SSIZE_T listSize = PyList_GET_SIZE( sipPy );
  qv->reserve( listSize );

  for ( SIP_SSIZE_T i = 0; i < listSize; ++i )
  {
    PyObject *obj = PyList_GET_ITEM( sipPy, i );
    int state;
    QgsFeatureStore *t = reinterpret_cast<QgsFeatureStore *>( sipConvertToType( obj, sipType_QgsFeatureStore, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr ) );

    if ( *sipIsErr )
    {
      sipReleaseType( t, sipType_QgsFeatureStore, state );

      delete qv;
      return 0;
    }

    qv->append( *t );
    sipReleaseType( t, sipType_QgsFeatureStore, state );
  }

  *sipCppPtr = qv;

  return sipGetState( sipTransferObj );
  % End
};
#endif

Q_DECLARE_METATYPE( QgsFeatureStore )

Q_DECLARE_METATYPE( QgsFeatureStoreList )

#endif
