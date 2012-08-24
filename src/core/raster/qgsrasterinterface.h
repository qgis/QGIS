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

#include "gdal.h"

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

    /** Clone itself, create deep copy */
    virtual QgsRasterInterface *clone() const = 0;

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

    /** For given data type returns wider type and sets no data value */
    static DataType typeWithNoDataValue( DataType dataType, double *noDataValue );

    /** Get number of bands */
    virtual int bandCount() const
    {
      return 1;
    }

    /** Retruns value representing 'no data' (NULL) */
    // TODO: Q_DECL_DEPRECATED
    virtual double noDataValue() const { return 0; }

    /** Return no data value for specific band. Each band/provider must have
     * no data value, if there is no one set in original data, provider decides one
     * possibly using wider data type.
     * @param bandNo band number
     * @return No data value */
    virtual double noDataValue( int bandNo ) const { Q_UNUSED( bandNo ); return noDataValue(); }

    /** Test if value is nodata for specific band
     * @param bandNo band number
     * @param value tested value
     * @return true if value is nodata */
    virtual bool isNoDataValue( int bandNo, double value ) const ;

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
    virtual const QgsRasterInterface * srcInput() const { return mInput ? mInput->srcInput() : this; }
    virtual QgsRasterInterface * srcInput() { return mInput ? mInput->srcInput() : this; }

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

    inline double readValue( void *data, QgsRasterInterface::DataType type, int index );
    inline void writeValue( void *data, QgsRasterInterface::DataType type, int index, double value );

  private:
    // Last rendering cumulative (this and all preceding interfaces) times, from index 1
    QVector<double> mTime;

    // Collect statistics
    int mStatsOn;
};

inline double QgsRasterInterface::readValue( void *data, QgsRasterInterface::DataType type, int index )
{
  if ( !mInput )
  {
    return 0;
  }

  if ( !data )
  {
    return mInput->noDataValue();
  }

  switch ( type )
  {
    case QgsRasterInterface::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterInterface::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterInterface::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterInterface::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterInterface::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterInterface::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterInterface::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }

  // TODO: noDataValue is per band
  return mInput->noDataValue();
}

inline void QgsRasterInterface::writeValue( void *data, QgsRasterInterface::DataType type, int index, double value )
{
  if ( !mInput ) return;
  if ( !data ) return;

  switch ( type )
  {
    case QgsRasterInterface::Byte:
      (( GByte * )data )[index] = ( GByte ) value;
      break;
    case QgsRasterInterface::UInt16:
      (( GUInt16 * )data )[index] = ( GUInt16 ) value;
      break;
    case QgsRasterInterface::Int16:
      (( GInt16 * )data )[index] = ( GInt16 ) value;
      break;
    case QgsRasterInterface::UInt32:
      (( GUInt32 * )data )[index] = ( GUInt32 ) value;
      break;
    case QgsRasterInterface::Int32:
      (( GInt32 * )data )[index] = ( GInt32 ) value;
      break;
    case QgsRasterInterface::Float32:
      (( float * )data )[index] = ( float ) value;
      break;
    case QgsRasterInterface::Float64:
      (( double * )data )[index] = value;
      break;
    default:
      //QgsMessageLog::logMessage( tr( "GDAL data type %1 is not supported" ).arg( type ), tr( "Raster" ) );
      break;
  }
}

#endif


