/***************************************************************************
                    qgsmssqlgeometryparser.h  -  description
                             -------------------
    begin                : 2014-03-16
    copyright            : (C) 2014 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLGEOMETRYPARSER_H
#define QGSMSSQLGEOMETRYPARSER_H


/**
\class QgsMssqlGeometryParser
\brief Geometry parser for SqlGeometry/SqlGeography.
*
*/

class QgsMssqlGeometryParser
{

  protected:
    unsigned char* pszData;
    unsigned char* pszWkb;
    int nWkbLen;
    int nWkbMaxLen;
    /* byte order */
    char chByteOrder;
    /* serialization properties */
    char chProps;
    /* point array */
    int nPointSize;
    int nPointPos;
    int nNumPoints;
    /* figure array */
    int nFigurePos;
    int nNumFigures;
    /* shape array */
    int nShapePos;
    int nNumShapes;
    int nSRSId;

  protected:
    void CopyBytes( void* src, int len );
    void CopyCoordinates( int iPoint );
    void CopyPoint( int iPoint );
    void ReadPoint( int iShape );
    void ReadMultiPoint( int iShape );
    void ReadLineString( int iShape );
    void ReadMultiLineString( int iShape );
    void ReadPolygon( int iShape );
    void ReadMultiPolygon( int iShape );
    void ReadGeometryCollection( int iShape );

  public:
    QgsMssqlGeometryParser();
    unsigned char* ParseSqlGeometry( unsigned char* pszInput, int nLen );
    int GetSRSId() { return nSRSId; }
    int GetWkbLen() { return nWkbLen; }
    void DumpMemoryToLog( const char* pszMsg, unsigned char* pszInput, int nLen );
    /* sql geo type */
    bool IsGeography;
};


#endif // QGSMSSQLGEOMETRYPARSER_H
