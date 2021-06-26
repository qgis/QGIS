/***************************************************************************
  qgsvectortilemvtutils.h
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEMVTUTILS_H
#define QGSVECTORTILEMVTUTILS_H

#define SIP_NO_FILE

class QgsLineString;


/**
 * \ingroup core
 * Assorted utility functions used during MVT decoding and encoding.
 *
 * \since QGIS 3.14
 */
class QgsVectorTileMVTUtils
{
  public:

    /**
     * Returns whether this linear ring forms an exterior ring according to MVT spec
     * (depending on the orientation - clockwise or counter-clockwise)
     */
    static bool isExteriorRing( const QgsLineString *lineString );
};

#endif // QGSVECTORTILEMVTUTILS_H
