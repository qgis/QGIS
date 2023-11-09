/***************************************************************************
  qgsalignrasterdata.h
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALIGNRASTERDATA_H
#define QGSALIGNRASTERDATA_H

#include "qgis.h"

/**
 * \ingroup core
 * \brief The QgsAlignRasterData class provides data structures and enums for align raster tool.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAlignRasterData
{
  public:

    //! Definition of one raster layer for alignment
    struct RasterItem
    {
      RasterItem( const QString &input, const QString &output )
        : inputFilename( input )
        , outputFilename( output )
        , resampleMethod( Qgis::GdalResampleAlgorithm::RA_NearestNeighbour )
        , rescaleValues( false )
        , srcCellSizeInDestCRS( 0.0 )
      {}

      virtual ~RasterItem() = default;

      //! filename of the source raster
      QString inputFilename;
      //! filename of the newly created aligned raster (will be overwritten if exists already)
      QString outputFilename;
      //! resampling method to be used
      Qgis::GdalResampleAlgorithm resampleMethod;
      //! rescaling of values according to the change of pixel size
      bool rescaleValues;

      // private part

      //! used for rescaling of values (if necessary)
      double srcCellSizeInDestCRS;
    };
    typedef QList<QgsAlignRasterData::RasterItem> RasterItemList;
};


#endif // QGSALIGNRASTERDATA_H
