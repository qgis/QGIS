#include "qgsrastergrayscalerenderer.h"

#include <QPainter>
#include <QImage>
#include <QString>

#include <qgsrasterviewport.h>
#include <qgsmaptopixel.h>

#include <gdal_priv.h>
#include <iostream>
void QgsRasterGrayscaleRenderer::draw(const QPainter * theQPainter, const QgsRasterViewPort * theRasterViewPort,                                                                             const QgsMapToPixel * theQgsMapToPixel)
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::drawSingleBandGray called for layer " << theBandNoInt << std::endl;
#endif
  QgsRasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  GDALDataType myDataType = myGdalBand->GetRasterDataType();
  void *myGdalScanData = readData ( myGdalBand, theRasterViewPort );

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);

  double myRangeDouble = myRasterBandStats.rangeDouble;

  // print each point in myGdalScanData with equal parts R, G ,B o make it show as gray
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      double myGrayValDouble = readValue ( myGdalScanData, myDataType,
                                           myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt );

      if ( myGrayValDouble == noDataValueDouble ) 
      {

        myQImage.setPixel(myRowInt, myColumnInt, qRgba(255,255,255,0 ));
        continue;
      }

      int myGrayValInt = static_cast < int >( (myGrayValDouble-myRasterBandStats.minValDouble)
                                              * (255/myRangeDouble));

      if (invertHistogramFlag)
      {
        myGrayValDouble = 255 - myGrayValDouble;
      }
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValInt, myGrayValInt, myGrayValInt, transparencyLevelInt));
    }
  }

  // Set up the initial offset into the myQImage we want to copy to the map canvas
  // This is useful when the source image pixels are larger than the screen image.
  int paintXoffset = 0;
  int paintYoffset = 0;
  
  if (theQgsMapToPixel)
  {
    paintXoffset = static_cast<int>( 
                                     (theRasterViewPort->rectXOffsetFloat - 
                                      theRasterViewPort->rectXOffsetInt)
                                    / theQgsMapToPixel->mapUnitsPerPixel() 
                                    * fabs(adfGeoTransform[1])
                                   ); 

    paintYoffset = static_cast<int>( 
                                     (theRasterViewPort->rectYOffsetFloat - 
                                      theRasterViewPort->rectYOffsetInt)
                                    / theQgsMapToPixel->mapUnitsPerPixel() 
                                    * fabs(adfGeoTransform[5])
                                   ); 
  }                                   

#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawSingleBandGray: painting image to canvas from " 
            << paintXoffset << ", " << paintYoffset
            << " to "
            << static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5) << ", " 
            << static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5)
            << "." << std::endl;
#endif

  theQPainter->drawImage(static_cast<int>(theRasterViewPort->topLeftPoint.x() + 0.5),
                         static_cast<int>(theRasterViewPort->topLeftPoint.y() + 0.5),
                         myQImage,
                         paintXoffset,
                         paintYoffset);

} // QgsRasterLayer::drawSingleBandGray
