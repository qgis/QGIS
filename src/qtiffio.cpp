// Read TIFF image files with Qt
// Copyright (c) 1999 by Markus L. Noga <markus@noga.de>
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

#include <stdio.h>
#include <sys/mman.h>

#include <tiffio.h>
#include <qimage.h>
#include <qfile.h>


//! QIODevice / libtiff read wrapper
//
static tsize_t tiff_read(thandle_t handle,tdata_t data,tsize_t size) {
  QIODevice *iod=(QIODevice*) handle;
  return (tsize_t) iod->readBlock((char*) data,size);
}

//! QIODevice / libtiff write wrapper
//
static tsize_t tiff_write(thandle_t handle,tdata_t data,tsize_t size) {
  QIODevice *iod=(QIODevice*) handle;
  return (tsize_t) iod->writeBlock((const char*) data,size);
}

//! QIODevice / libtiff seek wrapper
/*! \returns the current file position. libtiff wants that.
*/
static toff_t tiff_seek(thandle_t handle, toff_t offset, int whence) {
  QIODevice *iod=(QIODevice*) handle;
  if(whence==SEEK_SET)
    iod->at(offset);
  else if(whence==SEEK_CUR)
    iod->at(iod->at()+offset);
  else if(whence==SEEK_END)
    iod->at(iod->size()+offset);
  else
    return -1;
  
  return iod->at();
}

//! QIODevice / libtiff close wrapper
/*! This is a dummy, the IO device's owner will close it.
*/
static int tiff_close(thandle_t handle) {
  return 0;
}

//! QIODevice / libtiff size wrapper
//
static toff_t tiff_size(thandle_t handle) {
  QIODevice *iod=(QIODevice*) handle;
  return iod->size();
}

//! QIODevice / libtiff mmap wrapper
/*! \warning always returns MAP_FAILED.
*/
static int tiff_mmap(thandle_t handle,tdata_t* data,toff_t* size) {
  return (int) MAP_FAILED;
}

//! QIODevice / libtiff write wrapper
/*! \warning because you can't mmap, this is a dummy.
*/
static void tiff_unmap(thandle_t handle, tdata_t data, toff_t size) {
}

//! QImageIO read handler for TIFF files.
//
static void read_tiff_image(QImageIO *iio) {
  QImage img;
  int state=-1;

  // pass on file name if known
  //
  const char *fileName;
  QFile *f=dynamic_cast<QFile*>(iio->ioDevice());
  fileName=f ? f->name() : "QIODevice";

  // open without memory mapping.
  //
  TIFF *tif=TIFFClientOpen(fileName,"rm",
                           (thandle_t) (iio->ioDevice()),
                           tiff_read,
                           tiff_write,
                           tiff_seek,
                           tiff_close,
                           tiff_size,
                           tiff_mmap,
                           tiff_unmap );
  
  if(tif) {
    unsigned width, height,size;
    
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    size=width*height;
    uint32 *bits=(uint32*) _TIFFmalloc(size * sizeof (uint32));
    
    if(bits) {
      if (TIFFReadRGBAImage(tif, width, height, bits, 0)) {
        // successfully read. now convert.
        //
        img.create(width,height,32);

        // no #ifdef because qRed is inline,
        // but the compiler will optimize this out.
        //        
        if(TIFFGetR(0x1234567)==qRed  (0x1234567) &&
           TIFFGetG(0x1234567)==qGreen(0x1234567) &&
           TIFFGetB(0x1234567)==qBlue (0x1234567)    ) {
          // just mirror on x axis.
          //
          
          for(unsigned y=0; y<height; y++)
            memcpy(img.scanLine(height-1-y),bits+y*width,width*4);
                
        } else {
          // swap bytes
          //
                              
          uint32 *inp=bits;
          for(unsigned y=0; y<height; y++) {
            QRgb *row=(QRgb*) (img.scanLine(height-1-y));
            for(unsigned x=0; x<width; x++) {
              const uint32 col=*(inp++);
              row[x]=qRgb(TIFFGetR(col),
                          TIFFGetG(col),
                          TIFFGetB(col) ) |
                         (TIFFGetA(col)<<24);
            }
          }
          
        }
        iio->setImage(img);
        state=0;

      }
      _TIFFfree(bits);
    }
    TIFFClose(tif);
  }
  iio->setStatus(state);
}
       
//! QImageIO write handler for TIFF files.
//
static void write_tiff_image(QImageIO *iio) {
  int state=-1;
  
  // ... to be continued.
  
  iio->setStatus(state);
}
       
//! Add TIFF format capability to QImage.
//
void qInitTiffIO() {
  // Qt regexps don't cut it - we're missing an OR-Operator.
  //
  QImageIO::defineIOHandler("TIFF", "^[MI][MI][\\x01*][\\x01*]", 0,
                            read_tiff_image,
                            write_tiff_image);
}
