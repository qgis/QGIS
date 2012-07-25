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

#include <QImage>

#include "qgsrectangle.h"

/** \ingroup core
 * Base class for processing modules.
 */
// TODO: inherit from QObject? It would be probably better but QgsDataProvider inherits already from QObject and multiple inheritance from QObject is not allowed
class CORE_EXPORT QgsRasterInterface
{
  public:

    /** Data types.
     *  This is modified and extended copy of GDALDataType.
     */
    enum DataType
    {
      /*! Unknown or unspecified type */          UnknownDataType = 0,
      /*! Eight bit unsigned integer */           Byte = 1,
      /*! Sixteen bit unsigned integer */         UInt16 = 2,
      /*! Sixteen bit signed integer */           Int16 = 3,
      /*! Thirty two bit unsigned integer */      UInt32 = 4,
      /*! Thirty two bit signed integer */        Int32 = 5,
      /*! Thirty two bit floating point */        Float32 = 6,
      /*! Sixty four bit floating point */        Float64 = 7,
      /*! Complex Int16 */                        CInt16 = 8,
      /*! Complex Int32 */                        CInt32 = 9,
      /*! Complex Float32 */                      CFloat32 = 10,
      /*! Complex Float64 */                      CFloat64 = 11,
      /*! Color, alpha, red, green, blue, 4 bytes the same as
          QImage::Format_ARGB32 */                ARGB32 = 12,
      /*! Color, alpha, red, green, blue, 4 bytes  the same as
          QImage::Format_ARGB32_Premultiplied */  ARGB32_Premultiplied = 13,

      TypeCount = 14          /* maximum type # + 1 */
    };

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    int typeSize( int dataType ) const
    {
      // Modified and extended copy from GDAL
      switch ( dataType )
      {
        case Byte:
          return 8;

        case UInt16:
        case Int16:
          return 16;

        case UInt32:
        case Int32:
        case Float32:
        case CInt16:
          return 32;

        case Float64:
        case CInt32:
        case CFloat32:
          return 64;

        case CFloat64:
          return 128;

        case ARGB32:
        case ARGB32_Premultiplied:
          return 32;

        default:
          return 0;
      }
    }

    int dataTypeSize( int bandNo ) const
    {
      return typeSize( dataType( bandNo ) );
    }

    /** Returns true if data type is numeric */
    bool typeIsNumeric( DataType type ) const;

    /** Returns true if data type is color */
    bool typeIsColor( DataType type ) const;

    /** Returns data type for the band specified by number */
    virtual DataType dataType( int bandNo ) const
    {
      Q_UNUSED( bandNo );
      return UnknownDataType;
    }

    /** Get number of bands */
    virtual int bandCount() const
    {
      return 1;
    }

    /** Retruns value representing 'no data' (NULL) */
    virtual double noDataValue() const { return 0; }

    /** Read block of data using given extent and size.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    void * block( int bandNo, QgsRectangle  const & extent, int width, int height );

    /** Read block of data using given extent and size.
     *  Method to be implemented by subclasses.
     *  Returns pointer to data.
     *  Caller is responsible to free the memory returned.
     */
    virtual void * readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
    {
      Q_UNUSED( bandNo ); Q_UNUSED( extent ); Q_UNUSED( width ); Q_UNUSED( height );
      return 0;
    }

    /** Set input.
      * Returns true if set correctly, false if cannot use that input */
    virtual bool setInput( QgsRasterInterface* input ) { mInput = input; return true; }

    /** Is on/off */
    virtual bool on( ) { return mOn; }

    /** Set on/off */
    virtual void setOn( bool on ) { mOn = on; }

    /** Get source / raw input, the first in pipe, usually provider.
     *  It may be used to get info about original data, e.g. resolution to decide
     *  resampling etc.
     */
    virtual QgsRasterInterface * srcInput() { return mInput ? mInput->srcInput() : 0; }

    /** Create a new image with extraneous data, such data may be used
     *  after the image is destroyed. The memory is not initialized.
     */
    QImage * createImage( int width, int height, QImage::Format format );

    /** Switch on (and clear old statistics) or off collection of statistics */
    void setStatsOn( bool on );

    /** Last total time (for allbands) consumed by this interface for call to block()
     * If cumulative is true, the result includes also time spent in all preceding
     * interfaces. If cumulative is false, only time consumed by this interface is
     * returned. */
    double time( bool cumulative = false );

  protected:
    // QgsRasterInterface used as input
    QgsRasterInterface* mInput;

    // On/off state, if off, it does not do anything, replicates input
    bool mOn;

  private:
    // Last rendering cumulative (this and all preceding interfaces) times, from index 1
    QVector<double> mTime;

    // Collect statistics
    int mStatsOn;
};

#endif


