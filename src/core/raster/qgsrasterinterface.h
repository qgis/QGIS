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

#include <QObject>
#include <QImage>

#include "qgsrectangle.h"

/** \ingroup core
 * Base class for processing modules.
 */
// TODO: inherit from QObject? QgsDataProvider inherits already from QObject, multiple inheritance from QObject is not allowed
class CORE_EXPORT QgsRasterInterface //: public QObject
{

    //Q_OBJECT

  public:

    enum Role
    {
      UnknownRole   = 0,
      ProviderRole  = 1,
      RendererRole  = 2,
      ResamplerRole = 3,
      ProjectorRole = 4
    };

    Role role() { return mRole; }

    // This is modified copy of GDALDataType
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

    int typeSize( int dataType ) const
    {
      // modified copy from GDAL
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

    QgsRasterInterface( QgsRasterInterface * input = 0, Role role = UnknownRole );

    virtual ~QgsRasterInterface();

    /** Returns data type for the band specified by number */
    virtual int dataType( int bandNo ) const
    {
      Q_UNUSED( bandNo );
      return UnknownDataType;
    }

    /** Get number of bands */
    virtual int bandCount() const
    {
      return 1;
    }

    // TODO
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

    /** Create a new image with extraneous data, such data may be used
     *  after the image is destroyed. The memory is not initialized.
     */
    QImage * createImage( int width, int height, QImage::Format format );

    // Clear last rendering time
    void clearTime() { mTime.clear(); if ( mInput ) mInput->clearTime(); }
    
    // Last time consumed by block()
    // Returns total time (for all bands) if bandNo is 0
    double time( int bandNo );
    double avgTime();

    //protected:

    Role mRole;

    // QgsRasterInterface from used as input, data are read from it
    QgsRasterInterface* mInput;

    // Last rendering time
    QVector<double> mTime;

    // minimum block size to record time (to ignore thumbnails etc)
    int mTimeMinSize;
};

#endif


