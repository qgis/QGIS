
/***************************************************************************
                          qgsmaplayerdependency.h  -  description
                             -------------------
    begin                : September 2016
    copyright            : (C) 2016 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERDEPENDENCY_H
#define QGSMAPLAYERDEPENDENCY_H

#include "qgis_core.h"
#include <QString>

/**
 * \ingroup core
 * This class models dependencies with or between map layers.
 * A dependency is defined by a layer ID, a type and an origin.
 * The two combinations of type/origin that are currently supported are:
 *  - PresenceDependency && FromProvider: virtual layers for instance which may depend on other layers already loaded to work
 *  - DataDependency && FromUser: dependencies given by the user, mainly to represent database triggers
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMapLayerDependency
{
  public:
    //! Type of dependency
    enum Type
    {
      PresenceDependency = 1, //< The layer must be already present (in the registry) for this dependency to be resolved
      DataDependency     = 2  //< The layer may be invalidated by data changes on another layer
    };

    //! Origin of the dependency
    enum Origin
    {
      FromProvider = 0,  //< Dependency given by the provider, the user cannot change it
      FromUser     = 1   //< Dependency given by the user
    };

    //! Standard constructor
    QgsMapLayerDependency( const QString &layerId, Type type = DataDependency, Origin origin = FromUser )
      : mType( type )
      , mOrigin( origin )
      , mLayerId( layerId )
    {}

    //! Returns the dependency type
    Type type() const { return mType; }

    //! Returns the dependency origin
    Origin origin() const { return mOrigin; }

    //! Returns the ID of the layer this dependency depends on
    QString layerId() const { return mLayerId; }

    //! Comparison operator
    bool operator==( const QgsMapLayerDependency &other ) const
    {
      return layerId() == other.layerId() && origin() == other.origin() && type() == other.type();
    }

#ifdef SIP_RUN
    //! hash operator
    long __hash__() const;
    % MethodCode
    sipRes = qHash( *sipCpp );
    % End
#endif
  private:
    Type mType;
    Origin mOrigin;
    QString mLayerId;
};

#ifndef SIP_RUN

/**
 * global qHash function for QgsMapLayerDependency, so that it can be used in a QSet
 */
inline uint qHash( const QgsMapLayerDependency &dep )
{
  return qHash( dep.layerId() ) + dep.origin() + dep.type();
}
#endif

#endif
