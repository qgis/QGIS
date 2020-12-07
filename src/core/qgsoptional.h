/***************************************************************************
  qgsoptional.h - QgsOptional

 ---------------------
 begin                : 7.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPTIONAL_H
#define QGSOPTIONAL_H

#include "qgis_core.h"


/**
 * \ingroup core
 *
 * \brief QgsOptional is a container for other classes and adds an additional enabled/disabled flag.
 *
 * Often it is used for configuration options which can be enabled or disabled but also have
 * more internal configuration information that should not be lost when disabling and re-enabling.
 *
 * \note For Python you need to use implementations for specific template classes
 * \note Not available in Python bindings (although SIP file is present for specific implementations).
 *
 * \since QGIS 3.0
 */
template<typename T>
class CORE_EXPORT QgsOptional
{
  public:

    /**
     * A QgsOptional is disabled by default if default constructed.
     */
    QgsOptional() = default;

    /**
     * A QgsOptional is enabled by default if constructed with payload.
     */
    QgsOptional( const T &data )
      : mEnabled( true )
      , mData( data )
    {
    }

    /**
     * A QgsOptional constructed with enabled status and data
     */
    QgsOptional( const T &data, bool enabled )
      : mEnabled( enabled )
      , mData( data )
    {
    }

    /**
     * Compare this QgsOptional to another one.
     *
     * This will compare the enabled flag and call the == operator
     * of the contained class.
     *
     * \since QGIS 3.0
     */
    bool operator== ( const QgsOptional<T> &other ) const
    {
      return mEnabled == other.mEnabled && mData == other.mData;
    }

    /**
     * Boolean operator. Will return TRUE if this optional is enabled.
     */
    operator bool() const
    {
      return mEnabled;
    }

    /**
     * Check if this optional is enabled
     *
     * \since QGIS 3.0
     */
    bool enabled() const
    {
      return mEnabled;
    }

    /**
     * Set if this optional is enabled
     *
     * \since QGIS 3.0
     */
    void setEnabled( bool enabled )
    {
      mEnabled = enabled;
    }

    /**
     * Access the payload data
     *
     * \since QGIS 3.0
     */
    const T *operator->() const
    {
      return &mData;
    }

    /**
     * Access the payload data
     *
     * \since QGIS 3.0
     */
    T data() const
    {
      return mData;
    }

    /**
     * Set the payload data
     *
     * \since QGIS 3.0
     */
    void setData( const T &data )
    {
      mData = data;
    }

  private:
    bool mEnabled = false;
    T mData;
};

#endif // QGSOPTIONAL_H
