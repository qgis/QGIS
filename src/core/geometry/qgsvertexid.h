/***************************************************************************
                        qgsvertexid.h
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVERTEXID_H
#define QGSVERTEXID_H

#include "qgis_core.h"
#include "qgis.h"

class QgsAbstractGeometry;

/**
 * \ingroup core
 * \class QgsVertexId
 * \brief Utility class for identifying a unique vertex within a geometry.
 * \since QGIS 2.10
 */
struct CORE_EXPORT QgsVertexId
{

  /**
   * Constructor for QgsVertexId.
   */
  explicit QgsVertexId( int _part = -1, int _ring = -1, int _vertex = -1, Qgis::VertexType _type = Qgis::VertexType::Segment ) SIP_HOLDGIL
: part( _part )
  , ring( _ring )
  , vertex( _vertex )
  , type( _type )
  {}

  /**
   * Returns TRUE if the vertex id is valid
   */
  bool isValid() const  SIP_HOLDGIL { return part >= 0 && ring >= 0 && vertex >= 0; }

  bool operator==( QgsVertexId other ) const SIP_HOLDGIL
  {
    return part == other.part && ring == other.ring && vertex == other.vertex;
  }
  bool operator!=( QgsVertexId other ) const SIP_HOLDGIL
  {
    return part != other.part || ring != other.ring || vertex != other.vertex;
  }

  /**
   * Returns TRUE if this vertex ID belongs to the same part as another vertex ID.
   */
  bool partEqual( QgsVertexId o ) const SIP_HOLDGIL
  {
    return part >= 0 && o.part == part;
  }

  /**
   * Returns TRUE if this vertex ID belongs to the same ring as another vertex ID (i.e. the part
   * and ring number are equal).
   */
  bool ringEqual( QgsVertexId o ) const SIP_HOLDGIL
  {
    return partEqual( o ) && ( ring >= 0 && o.ring == ring );
  }

  /**
   * Returns TRUE if this vertex ID corresponds to the same vertex as another vertex ID (i.e. the part,
   * ring number and vertex number are equal).
   */
  bool vertexEqual( QgsVertexId o ) const SIP_HOLDGIL
  {
    return ringEqual( o ) && ( vertex >= 0 && o.ring == ring );
  }

  /**
   * Returns TRUE if this vertex ID is valid for the specified \a geom.
   */
  bool isValid( const QgsAbstractGeometry *geom ) const SIP_HOLDGIL;

  //! Part number
  int part = -1;

  //! Ring number
  int ring = -1;

  //! Vertex number
  int vertex = -1;

  //! Vertex type
  Qgis::VertexType type = Qgis::VertexType::Segment;

#ifdef SIP_RUN
  SIP_PYOBJECT __repr__();
  % MethodCode
  QString str = QStringLiteral( "<QgsVertexId: %1,%2,%3 %4>" ).arg( sipCpp->part ).arg( sipCpp->ring ).arg( sipCpp->vertex ).arg( qgsEnumValueToKey( sipCpp->type ) );
  sipRes = PyUnicode_FromString( str.toUtf8().data() );
  % End
#endif

};

#endif //QGSVERTEXID_H
