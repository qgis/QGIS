/***************************************************************************
                              qgswmsutils.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts fron qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodule.h"
#include "qgswmsutils.h"
#include "qgsconfigcache.h"

namespace QgsWms
{

  typedef QList< QPair<QRgb, int> > QgsColorBox; //Color / number of pixels
  typedef QMultiMap< int, QgsColorBox > QgsColorBoxMap; // sum of pixels / color box

  QString ImplementationVersion()
  {
    return QStringLiteral( "1.3.0" );
  }

  // Return the wms config parser (Transitional)
  QgsWmsConfigParser* getConfigParser( QgsServerInterface* serverIface )
  {
    QString configFilePath = serverIface->configFilePath();
    return QgsConfigCache::instance()->wmsConfiguration( configFilePath, serverIface->accessControls() );
  }

  // Output a wms standard error
  void writeError( QgsServerResponse& response, const QString& code, const QString& message )
  {
    // WMS errors return erros with http 200
    // XXX Do we really need to use a QDomDocument here ?
    QDomDocument doc;
    QDomElement root = doc.createElement( QStringLiteral( "ServiceExceptionReport" ) );
    root.setAttribute( QStringLiteral( "version" ), ImplementationVersion() );
    root.setAttribute( QStringLiteral( "xmlns" ) , QStringLiteral( "http://www.opengis.net/ogc" ) );
    doc.appendChild( root );

    QDomElement elem = doc.createElement( QStringLiteral( "ServiceException" ) );
    elem.setAttribute( QStringLiteral( "code" ), code );
    QDomText messageText = doc.createTextNode( message );
    elem.appendChild( messageText );
    root.appendChild( elem );

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( doc.toByteArray() );
  }

  ImageOutputFormat parseImageFormat( const QString& format )
  {
    if ( format.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 ||
         format.compare( QLatin1String( "image/png" ), Qt::CaseInsensitive ) == 0 )
    {
      return PNG;
    }
    else if ( format.compare( QLatin1String( "jpg " ) , Qt::CaseInsensitive ) == 0  ||
              format.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
    {
      return JPEG;
    }
    else
    {
      // lookup for png with mode
      QRegularExpression modeExpr = QRegularExpression( QStringLiteral( "image/png\\s*;\\s*mode=([^;]+)" ),
                                    QRegularExpression::CaseInsensitiveOption );

      QRegularExpressionMatch match = modeExpr.match( format );
      QString mode = match.captured();
      if ( mode.compare( QLatin1String( "16bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG16;
      if ( mode.compare( QLatin1String( "8bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG8;
      if ( mode.compare( QLatin1String( "1bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG1;
    }

    return UNKN;
  }

  //forward declaration
  namespace
  {
    void medianCut( QVector<QRgb>& colorTable, int nColors, const QImage& inputImage );
  }

  // Write image response
  void writeImage( QgsServerResponse& response, QImage& img, const QString& formatStr,
                   int imageQuality )
  {
    ImageOutputFormat outputFormat = parseImageFormat( formatStr );
    QImage  result;
    QString saveFormat;
    QString contentType;
    switch ( outputFormat )
    {
      case PNG:
        result = img;
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG8:
      {
        QVector<QRgb> colorTable;
        medianCut( colorTable, 256, img );
        result = img.convertToFormat( QImage::Format_Indexed8, colorTable,
                                      Qt::ColorOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
      }
      contentType = "image/png";
      saveFormat = "PNG";
      break;
      case PNG16:
        result = img.convertToFormat( QImage::Format_ARGB4444_Premultiplied );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG1:
        result = img.convertToFormat( QImage::Format_Mono,
                                      Qt::MonoOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case JPEG:
        result = img;
        contentType = "image/jpeg";
        saveFormat = "JPEG";
        break;
      default:
        QgsMessageLog::logMessage( QString( "Unsupported format string %1" ).arg( formatStr ) );
        saveFormat = UNKN;
        break;
    }

    if ( outputFormat != UNKN )
    {
      response.setHeader( "Content-Type", contentType );
      result.save( response.io(), qPrintable( saveFormat ), imageQuality );
    }
    else
    {
      writeError( response, "InvalidFormat",
                  QString( "Output format '%1' is not supported in the GetMap request" ).arg( formatStr ) );
    }
  }

  namespace
  {
    //
    // Median cut implementation used when reducing RGB colors to palletized colors
    // This code has been ported directly from qgswmshandler.cpp
    //

    void imageColors( QHash<QRgb, int>& colors, const QImage& image )
    {
      colors.clear();
      int width = image.width();
      int height = image.height();

      const QRgb* currentScanLine = nullptr;
      QHash<QRgb, int>::iterator colorIt;
      for ( int i = 0; i < height; ++i )
      {
        currentScanLine = ( const QRgb* )( image.scanLine( i ) );
        for ( int j = 0; j < width; ++j )
        {
          colorIt = colors.find( currentScanLine[j] );
          if ( colorIt == colors.end() )
          {
            colors.insert( currentScanLine[j], 1 );
          }
          else
          {
            colorIt.value()++;
          }
        }
      }
    }

    bool minMaxRange( const QgsColorBox& colorBox, int& redRange, int& greenRange, int& blueRange, int& alphaRange )
    {
      if ( colorBox.size() < 1 )
      {
        return false;
      }

      int rMin = INT_MAX;
      int gMin = INT_MAX;
      int bMin = INT_MAX;
      int aMin = INT_MAX;
      int rMax = INT_MIN;
      int gMax = INT_MIN;
      int bMax = INT_MIN;
      int aMax = INT_MIN;

      int currentRed = 0;
      int currentGreen = 0;
      int currentBlue = 0;
      int currentAlpha = 0;

      QgsColorBox::const_iterator colorBoxIt = colorBox.constBegin();
      for ( ; colorBoxIt != colorBox.constEnd(); ++colorBoxIt )
      {
        currentRed = qRed( colorBoxIt->first );
        if ( currentRed > rMax )
        {
          rMax = currentRed;
        }
        if ( currentRed < rMin )
        {
          rMin = currentRed;
        }

        currentGreen = qGreen( colorBoxIt->first );
        if ( currentGreen > gMax )
        {
          gMax = currentGreen;
        }
        if ( currentGreen < gMin )
        {
          gMin = currentGreen;
        }

        currentBlue = qBlue( colorBoxIt->first );
        if ( currentBlue > bMax )
        {
          bMax = currentBlue;
        }
        if ( currentBlue < bMin )
        {
          bMin = currentBlue;
        }

        currentAlpha = qAlpha( colorBoxIt->first );
        if ( currentAlpha > aMax )
        {
          aMax = currentAlpha;
        }
        if ( currentAlpha < aMin )
        {
          aMin = currentAlpha;
        }
      }

      redRange = rMax - rMin;
      greenRange = gMax - gMin;
      blueRange = bMax - bMin;
      alphaRange = aMax - aMin;
      return true;
    }

    bool redCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 )
    {
      return qRed( c1.first ) < qRed( c2.first );
    }

    bool greenCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 )
    {
      return qGreen( c1.first ) < qGreen( c2.first );
    }

    bool blueCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 )
    {
      return qBlue( c1.first ) < qBlue( c2.first );
    }

    bool alphaCompare( QPair<QRgb, int> c1, QPair<QRgb, int> c2 )
    {
      return qAlpha( c1.first ) < qAlpha( c2.first );
    }

    QRgb boxColor( const QgsColorBox& box, int boxPixels )
    {
      double avRed = 0;
      double avGreen = 0;
      double avBlue = 0;
      double avAlpha = 0;
      QRgb currentColor;
      int currentPixel;

      double weight;

      QgsColorBox::const_iterator colorBoxIt = box.constBegin();
      for ( ; colorBoxIt != box.constEnd(); ++colorBoxIt )
      {
        currentColor = colorBoxIt->first;
        currentPixel = colorBoxIt->second;
        weight = ( double )currentPixel / boxPixels;
        avRed   += ( qRed( currentColor ) * weight );
        avGreen += ( qGreen( currentColor ) * weight );
        avBlue  += ( qBlue( currentColor ) * weight );
        avAlpha += ( qAlpha( currentColor ) * weight );
      }

      return qRgba( avRed, avGreen, avBlue, avAlpha );
    }


    void splitColorBox( QgsColorBox& colorBox, QgsColorBoxMap& colorBoxMap,
                        QMap<int, QgsColorBox>::iterator colorBoxMapIt )
    {

      if ( colorBox.size() < 2 )
      {
        return; //need at least two colors for a split
      }

      //a,r,g,b ranges
      int redRange = 0;
      int greenRange = 0;
      int blueRange = 0;
      int alphaRange = 0;

      if ( !minMaxRange( colorBox, redRange, greenRange, blueRange, alphaRange ) )
      {
        return;
      }

      //sort color box for a/r/g/b
      if ( redRange >= greenRange && redRange >= blueRange && redRange >= alphaRange )
      {
        qSort( colorBox.begin(), colorBox.end(), redCompare );
      }
      else if ( greenRange >= redRange && greenRange >= blueRange && greenRange >= alphaRange )
      {
        qSort( colorBox.begin(), colorBox.end(), greenCompare );
      }
      else if ( blueRange >= redRange && blueRange >= greenRange && blueRange >= alphaRange )
      {
        qSort( colorBox.begin(), colorBox.end(), blueCompare );
      }
      else
      {
        qSort( colorBox.begin(), colorBox.end(), alphaCompare );
      }

      //get median
      double halfSum = colorBoxMapIt.key() / 2.0;
      int currentSum = 0;
      int currentListIndex = 0;

      QgsColorBox::iterator colorBoxIt = colorBox.begin();
      for ( ; colorBoxIt != colorBox.end(); ++colorBoxIt )
      {
        currentSum += colorBoxIt->second;
        if ( currentSum >= halfSum )
        {
          break;
        }
        ++currentListIndex;
      }

      if ( currentListIndex > ( colorBox.size() - 2 ) ) //if the median is contained in the last color, split one item before that
      {
        --currentListIndex;
        currentSum -= colorBoxIt->second;
      }
      else
      {
        ++colorBoxIt; //the iterator needs to point behind the last item to remove
      }

      //do split: replace old color box, insert new one
      QgsColorBox newColorBox1 = colorBox.mid( 0, currentListIndex + 1 );
      colorBoxMap.insert( currentSum, newColorBox1 );

      colorBox.erase( colorBox.begin(), colorBoxIt );
      QgsColorBox newColorBox2 = colorBox;
      colorBoxMap.erase( colorBoxMapIt );
      colorBoxMap.insert( halfSum * 2.0 - currentSum, newColorBox2 );
    }


    void medianCut( QVector<QRgb>& colorTable, int nColors, const QImage& inputImage )
    {
      QHash<QRgb, int> inputColors;
      imageColors( inputColors, inputImage );

      if ( inputColors.size() <= nColors ) //all the colors in the image can be mapped to one palette color
      {
        colorTable.resize( inputColors.size() );
        int index = 0;
        QHash<QRgb, int>::const_iterator inputColorIt = inputColors.constBegin();
        for ( ; inputColorIt != inputColors.constEnd(); ++inputColorIt )
        {
          colorTable[index] = inputColorIt.key();
          ++index;
        }
        return;
      }

      //create first box
      QgsColorBox firstBox; //QList< QPair<QRgb, int> >
      int firstBoxPixelSum = 0;
      QHash<QRgb, int>::const_iterator inputColorIt = inputColors.constBegin();
      for ( ; inputColorIt != inputColors.constEnd(); ++inputColorIt )
      {
        firstBox.push_back( qMakePair( inputColorIt.key(), inputColorIt.value() ) );
        firstBoxPixelSum += inputColorIt.value();
      }

      QgsColorBoxMap colorBoxMap; //QMultiMap< int, ColorBox >
      colorBoxMap.insert( firstBoxPixelSum, firstBox );
      QMap<int, QgsColorBox>::iterator colorBoxMapIt = colorBoxMap.end();

      //split boxes until number of boxes == nColors or all the boxes have color count 1
      bool allColorsMapped = false;
      while ( colorBoxMap.size() < nColors )
      {
        //start at the end of colorBoxMap and pick the first entry with number of colors < 1
        colorBoxMapIt = colorBoxMap.end();
        while ( true )
        {
          --colorBoxMapIt;
          if ( colorBoxMapIt.value().size() > 1 )
          {
            splitColorBox( colorBoxMapIt.value(), colorBoxMap, colorBoxMapIt );
            break;
          }
          if ( colorBoxMapIt == colorBoxMap.begin() )
          {
            allColorsMapped = true;
            break;
          }
        }

        if ( allColorsMapped )
        {
          break;
        }
      }

      //get representative colors for the boxes
      int index = 0;
      colorTable.resize( colorBoxMap.size() );
      QgsColorBoxMap::const_iterator colorBoxIt = colorBoxMap.constBegin();
      for ( ; colorBoxIt != colorBoxMap.constEnd(); ++colorBoxIt )
      {
        colorTable[index] = boxColor( colorBoxIt.value(), colorBoxIt.key() );
        ++index;
      }
    }


  } // namespace

  QgsRectangle parseBbox( const QString& bboxStr )
  {
    QStringList lst = bboxStr.split( QStringLiteral( "," ) );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( QLatin1String( " " ), QLatin1String( "+" ) );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

} // namespace QgsWms


