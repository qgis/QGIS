/***************************************************************************
    qgsrasterface.h - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERINTERFACE_H
#define QGSRASTERINTERFACE_H

#include <limits>

#include <QImage>

#include "qgslogger.h"
#include "qgsrasterblock.h"
#include "qgsrectangle.h"

#include "gdal.h"

/** \ingroup core
 * Base class for processing modules.
 */
// TODO: inherit from QObject? It would be probably better but QgsDataProvider inherits already from QObject and multiple inheritance from QObject is not allowed
class CORE_EXPORT QgsRasterInterface
{
  public:

#if 0
    struct Range
    {
      double min;
      double max;
      inline bool operator==( const Range &o ) const
      {
        return min == o.min && max == o.max;
      }
    };
#endif

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    /** Clone itself, create deep copy */
    virtual QgsRasterInterface *clone() const = 0;

    /** Returns data type for the band specified by number */
    virtual QGis::DataType dataType( int bandNo ) const = 0;
#if 0
    {
      Q_UNUSED( bandNo );
      QgsDebugMsg( "Entered" );
      return UnknownDataType;
    }
#endif

    int dataTypeSize( int bandNo ) { return QgsRasterBlock::typeSize( dataType( bandNo ) ); }

    /** Get number of bands */
    virtual int bandCount() const = 0;

    /** Return no data value for specific band. Each band/provider must have
     * no data value, if there is no one set in original data, provider decides one
     * possibly using wider data type.
     * @param bandNo band number
     * @return No data value */
    virtual double noDataValue( int bandNo ) const { Q_UNUSED( bandNo ); return std::numeric_limits<double>::quiet_NaN(); }

    /** Test if value is nodata for specific band
     * @param bandNo band number
     * @param value tested value
     * @return true if value is nodata */
    virtual bool isNoDataValue( int bandNo, double value ) const;

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    //void *block( int bandNo, const QgsRectangle &extent, int width, int height );
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) = 0;

    /** Read block of data using given extent and size.
     *  Method to be implemented by subclasses.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    //virtual void *readBlock( int bandNo, const QgsRectangle &extent, int width, int height )
    //virtual QgsRasterBlock *readBlock( int bandNo, const QgsRectangle &extent, int width, int height ) const = 0;

    //{
    //  Q_UNUSED( bandNo ); Q_UNUSED( extent ); Q_UNUSED( width ); Q_UNUSED( height );
    //  return new QgsRasterBlock();
    //}

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input ) { mInput = input; return true; }

    /** Current input */
    virtual QgsRasterInterface * input() const { return mInput; }

    /** Is on/off */
    virtual bool on() const { return mOn; }

    /** Set on/off */
    virtual void setOn( bool on ) { mOn = on; }

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual const QgsRasterInterface *srcInput() const
    {
      QgsDebugMsg( "Entered" );
      return mInput ? mInput->srcInput() : this;
    }
    virtual QgsRasterInterface * srcInput()
    {
      QgsDebugMsg( "Entered" );
      return mInput ? mInput->srcInput() : this;
    }

    /** Switch on (and clear old statistics) or off collection of statistics */
    //void setStatsOn( bool on );

    /** Last total time (for allbands) consumed by this interface for call to block()
     * If cumulative is true, the result includes also time spent in all preceding
     * interfaces. If cumulative is false, only time consumed by this interface is
     * returned. */
    //double time( bool cumulative = false );

  protected:
    // QgsRasterInterface used as input
    QgsRasterInterface* mInput;

    // On/off state, if off, it does not do anything, replicates input
    bool mOn;

  private:
    // Last rendering cumulative (this and all preceding interfaces) times, from index 1
    //QVector<double> mTime;

    // Collect statistics
    //int mStatsOn;
};

#endif


