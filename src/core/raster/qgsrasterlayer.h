/***************************************************************************
                        qgsrasterlayer.h  -  description
                              -------------------
 begin                : Fri Jun 28 2002
 copyright            : (C) 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
 email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */



#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

//
// Includes
//

#include <QColor>
#include <QDateTime>
#include <QVector>
#include <QList>
#include <QMap>

#include "qgis.h"
#include "qgspoint.h"
#include "qgsmaplayer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgsrastershader.h"
#include "qgscolorrampshader.h"
#include "qgsrastershaderfunction.h"
#include "qgsrasterdataprovider.h"


#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>
/** \ingroup core
 * A call back function for showing progress of gdal operations.
 */
int CPL_STDCALL progressCallback( double dfComplete,
                                  const char *pszMessage,
                                  void * pProgressArg );


//
// Forward declarations
//
class QgsMapToPixel;
class QgsRect;
class QgsRasterBandStats;
class QgsRasterPyramid;
class QgsRasterLayerProperties;
struct QgsRasterViewPort;
class QImage;
class QPixmap;
class QSlider;
class QLibrary;


/** \ingroup core
 *  This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas.
 *
 *  The qgsrasterlayer class makes use of gdal for data io, and thus supports
 *  any gdal supported format. The constructor attempts to infer what type of
 *  file (RASTER_LAYER_TYPE) is being opened - not in terms of the file format (tif, ascii grid etc.)
 *  but rather in terms of whether the image is a GRAYSCALE, PALETTED or MULTIBAND,
 *
 *  Within the three allowable raster layer types, there are 8 permutations of
 *  how a layer can actually be rendered. These are defined in the DRAWING_STYLE enum
 *  and consist of:
 *
 *  SINGLE_BAND_GRAY -> a GRAYSCALE layer drawn as a range of gray colors (0-255)
 *  SINGLE_BAND_PSEUDO_COLOR -> a GRAYSCALE layer drawn using a pseudocolor algorithm
 *  PALETTED_SINGLE_BAND_GRAY -> a PALETTED layer drawn in gray scale (using only one of the color components)
 *  PALETTED_SINGLE_BAND_PSEUDO_COLOR -> a PALETTED layer having only one of its color components rendered as psuedo color
 *  PALETTED_MULTI_BAND_COLOR -> a PALETTED image where the bands contains 24bit color info and 8 bits is pulled out per color
 *  MULTI_BAND_SINGLE_BAND_GRAY -> a layer containing 2 or more bands, but using only one band to produce a grayscale image
 *  MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR -> a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
 *  MULTI_BAND_COLOR -> a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
 *
 *  Each of the above mentioned drawing styles is implemented in its own draw* function.
 *  Some of the drawing styles listed above require statistics about the layer such
 *  as the min / max / mean / stddev etc. statistics for a band can be gathered using the
 *  getRasterBandStats function. Note that statistics gathering is a slow process and
 *  every effort should be made to call this function as few times as possible. For this
 *  reason, qgsraster has a vector class member to store stats for each band. The
 *  constructor initialises this vector on startup, but only populates the band name and
 *  number fields.
 *
 *  Note that where bands are of gdal 'undefined' type, their values may exceed the
 *  renderable range of 0-255. Because of this a linear scaling histogram stretch is
 *  applied to undefined layers to normalise the data into the 0-255 range.
 *
 *  A qgsrasterlayer band can be referred to either by name or by number (base=1). It
 *  should be noted that band names as stored in datafiles may not be unique, and
 *  so the rasterlayer class appends the band number in brackets behind each band name.
 *
 *  Sample usage of the QgsRasterLayer class:
 *
 * \code
 *     QString myFileNameQString = "/path/to/file";
 *     QFileInfo myFileInfo(myFileNameQString);
 *     QString myBaseNameQString = myFileInfo.baseName();
 *     QgsRasterLayer *myRasterLayer = new QgsRasterLayer(myFileNameQString, myBaseNameQString);
 *
 * \endcode
 *
 *  In order to automate redrawing of a raster layer, you should like it to a map canvas like this :
 *
 * \code
 *     QObject::connect( myRasterLayer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()) );
 * \endcode
 *
 *  A raster layer can also export its legend as a pixmap:
 *
 * \code
 *     QPixmap myQPixmap = myRasterLayer->legendPixmap();
 * \endcode
 *
 * Once a layer has been created you can find out what type of layer it is (GRAY_OR_UNDEFINED, PALETTE or MULTIBAND):
 *
 * \code
 *    if (rasterLayer->getRasterLayerType()==QgsRasterLayer::MULTIBAND)
 *    {
 *      //do something
 *    }
 *    else if (rasterLayer->getRasterLayerType()==QgsRasterLayer::PALETTE)
 *    {
 *      //do something
 *    }
 *    else // QgsRasterLayer::GRAY_OR_UNDEFINED
 *    {
 *      //do something.
 *    }
 * \endcode
 *
 * You can combine layer type detection with the setDrawingStyle method to override the default drawing style assigned
 * when a layer is loaded:
 *
  * \code
 *    if (rasterLayer->getRasterLayerType()==QgsRasterLayer::MULTIBAND)
 *    {
 *       myRasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR);
 *    }
 *    else if (rasterLayer->getRasterLayerType()==QgsRasterLayer::PALETTE)
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR);
 *    }
 *    else // QgsRasterLayer::GRAY_OR_UNDEFINED
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
 *    }
 * \endcode
 *
 *  Raster layers can also have an arbitrary level of transparency defined, and have their
 *  color palettes inverted using the setTransparency and setInvertHistogramFlag methods.
 *
 *  Pseudocolor images can have their output adjusted to a given number of standard
 *  deviations using the setStdDevsToPlot method.
 *
 *  The final area of functionality you may be interested in is band mapping. Band mapping
 *  allows you to choose arbitrary band -> color mappings and is applicable only to PALETTE
 *  and MULTIBAND rasters, There are four mappings that can be made: red, green, blue and gray.
 *  Mappings are non-exclusive. That is a given band can be assigned to no, some or all
 *  color mappings. The constructor sets sensible defaults for band mappings but these can be
 *  overridden at run time using the setRedBandName, setGreenBandName, setBlueBandName and setGrayBandName
 *  methods.
 */

class CORE_EXPORT QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
  public:
    //
    // Static methods:
    //
    static void buildSupportedRasterFileFilter( QString & fileFilters );
    static void registerGdalDrivers();

    /** This helper checks to see whether the file name appears to be a valid
       raster file name.  If the file name looks like it could be valid,
       but some sort of error occurs in processing the file, the error is
       returned in retError. */
    static bool isValidRasterFileName( const QString & theFileNameQString,
                                       QString &retError );

    static bool isValidRasterFileName( const QString & theFileNameQString );

    //
    // Non Static methods:
    //
    /** \brief This is the constructor for the RasterLayer class.
     *
     * The main tasks carried out by the constructor are:
     *
     * -Load the rasters default style (.qml) file if it exists
     *
     * -Populate the RasterStatsVector with initial values for each band.
     *
     * -Calculate the layer extents
     *
     * -Determine whether the layer is gray, paletted or multiband.
     *
     * -Assign sensible defaults for the red, green, blue and gray bands.
     *
     * -
     * */
    QgsRasterLayer( const QString & path = QString::null,
                    const QString &  baseName = QString::null,
                    bool loadDefaultStyleFlag = true );

    /** \brief The destructor.  */
    ~QgsRasterLayer();

    /** \brief  A list containing one RasterBandStats struct per raster band in this raster layer.
     * Note that while every RasterBandStats element will have the name and number of its associated
     * band populated, any additional stats are calculated on a need to know basis.*/
    typedef QList<QgsRasterBandStats> RasterStatsList;


    /** \brief  A list containing one RasterPyramid struct per raster band in this raster layer.
     * POTENTIAL pyramid layer. How this works is we divide the height
     * and width of the raster by an incrementing number. As soon as the result
     * of the division is <=256 we stop allowing RasterPyramid structs
     * to be added to the list. Each time a RasterPyramid is created
     * we will check to see if a pyramid matching these dimensions already exists
     * in the raster layer, and if so mark the exists flag as true. */

    typedef QList<QgsRasterPyramid> RasterPyramidList;

    /** \brief A list containing on ContrastEnhancement object per raster band in this raster layer. */
    typedef QList<QgsContrastEnhancement> ContrastEnhancementList;

    /** \brief This typedef is used when the showProgress function is passed to gdal as a function
    pointer. */
    //  typedef  int (QgsRasterLayer::*showTextProgress)( double theProgress,
    //                                      const char *theMessageCharArray,
    //                                      void *theData);

    /** \brief Identify raster value(s) found on the point position
     *
     * \param point[in]  a coordinate in the CRS of this layer.
     */
    void identify( const QgsPoint & point, QMap<QString, QString>& results );

    /** \brief Identify arbitrary details from the WMS server found on the point position
     *
     * \param point[in]  an image pixel coordinate in the last requested extent of layer.
     *
     * \return  A text document containing the return from the WMS server
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     */
    QString identifyAsText( const QgsPoint & point );

    /** \brief Query gdal to find out the WKT projection string for this layer. This implements the virtual method of the same name defined in QgsMapLayer*/
    QString getProjectionWKT();

    /** \brief Returns the number of raster units per each raster pixel. For rasters with world file, this is normally the first row (without the sign) in that file */
    double rasterUnitsPerPixel();

    /** \brief Draws a thumbnail of the rasterlayer into the supplied pixmap pointer */
    void drawThumbnail( QPixmap * theQPixmap );

    /** \brief Get an 8x8 pixmap of the color palette. If the layer has no palette a white pixmap will be returned. */
    QPixmap getPaletteAsPixmap( int theBand = 1 );

    /** \brief This is called when the view on the raster layer needs to be refreshed (redrawn).
     */
    bool draw( QgsRenderContext& rendererContext );

    /** \brief This is an overloaded version of the above function that is called by both draw above and drawThumbnail */
    void draw( QPainter * theQPainter, QgsRasterViewPort * myRasterViewPort,
               const QgsMapToPixel* theQgsMapToPixel = 0 );

    //
    // Accessors for image height and width
    //
    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int getRasterXDim() {return mRasterXDim;}

    /** \brief Accessor that returns the height of the (unclipped) raster  */
    int getRasterYDim() {return mRasterYDim;}

    //
    // Accessor and mutator for no data double
    //
    /** \brief Is the NoDataValue Valid */
    bool isNoDataValueValid() {return mValidNoDataValue;}

    /** \brief Accessor that returns the NO_DATA entry for this raster. */
    double getNoDataValue( bool* isValid = 0 ) { if ( isValid ) { *isValid = mValidNoDataValue;} return mNoDataValue;}

    /** \brief Mutator that allows the  NO_DATA entry for this raster to be overridden. */
    void setNoDataValue( double theNoData );

    /** \brief Simple reset function that set the noDataValue back to the value stored in the first raster band */
    void resetNoDataValue()
    {
      mNoDataValue = -9999;
      if ( mGdalDataset != NULL && GDALGetRasterCount( mGdalDataset ) > 0 )
      {
        int myRequestValid;
        double myValue = GDALGetRasterNoDataValue(
                           GDALGetRasterBand( mGdalDataset, 1 ), &myRequestValid );

        if ( 0 != myRequestValid )
        {
          setNoDataValue( myValue );
        }
        else
        {
          setNoDataValue( myValue );
          mValidNoDataValue = false;
        }
      }
    }
    //
    // Accessor and mutator for mInvertPixelsFlag
    //
    /** \brief Accessor to find out whether the histogram should be inverted.   */
    bool getInvertHistogramFlag()
    {
      return mInvertPixelsFlag;
    }
    /** \brief Mutator to alter the state of the invert histogram flag.  */
    void setInvertHistogramFlag( bool theFlag )
    {
      mInvertPixelsFlag = theFlag;
    }
    //
    // Accessor and mutator for mStandardDeviations
    //
    /** \brief Accessor to find out how many standard deviations are being plotted.  */
    double getStdDevsToPlot()
    {
      return mStandardDeviations;
    }
    /** \brief Mutator to alter the number of standard deviations that should be plotted.  */
    void setStdDevsToPlot( double theStdDevsToPlot )
    {
      mStandardDeviations = theStdDevsToPlot;
    }
    /** \brief Get the number of bands in this layer  */
    unsigned int getBandCount();
    /** \brief Get RasterBandStats for a band given its number (read only)  */
    const  QgsRasterBandStats getRasterBandStats( int );
    /** \brief  Check whether a given band number has stats associated with it */
    bool hasStats( int theBandNoInt );
    /** \brief Overloaded method that also returns stats for a band, but uses the band color name
    *    Note this approach is not recommended because it is possible for two gdal raster
    *    bands to have the same name!
    */
    const  QgsRasterBandStats getRasterBandStats( const QString & );
    /** \brief Get the number of a band given its name. Note this will be the rewritten name set
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    int getRasterBandNumber( const QString & theBandName );
    /** \brief Get the name of a band given its number.  */
    const  QString getRasterBandName( int theBandNoInt );
    /** \brief Find out whether a given band exists.    */
    bool hasBand( const QString &  theBandName );
    /** \brief Call any inline image manipulation filters */
    void filterLayer( QImage * theQImage );
    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red color). */
    QString getRedBandName()
    {
      return mRedBandName;
    }
    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red color). */
    void setRedBandName( const QString & theBandName );
    //
    // Accessor and mutator for green band name
    //
    /** \brief Accessor for green band name mapping.  */
    QString getGreenBandName()
    {
      return mGreenBandName;
    }
    /** \brief Mutator for green band name mapping.  */
    void setGreenBandName( const QString & theBandName );
    //
    // Accessor and mutator for blue band name
    //
    /** \brief  Accessor for blue band name mapping. */
    QString getBlueBandName()
    {
      return mBlueBandName;
    }
    /** \brief Mutator for blue band name mapping.  */
    void setBlueBandName( const QString & theBandName );

    //
    // Accessor raster transparency object
    //
    /** \brief Returns a pointer to the transparency object */
    QgsRasterTransparency* getRasterTransparency() { return &mRasterTransparency; }

    //
    // Accessor and mutator for transparent band name
    //
    /** \brief  Accessor for transparent band name mapping. */
    QString getTransparentBandName()
    {
      return mTransparencyBandName;
    }
    /** \brief Mutator for transparent band name mapping.  */
    void setTransparentBandName( const QString & theBandName );

    //
    // Accessor and mutator for gray band name
    //
    /** \brief Accessor for gray band name mapping.  */
    QString getGrayBandName()
    {
      return mGrayBandName;
    }
    /** \brief Mutator for gray band name mapping.  */
    void setGrayBandName( const QString & theBandName );

    // Accessor and mutator for minimum maximum values
    //TODO: Move these out of the header file...
    /** \brief Accessor for minimum value user for contrast enhancement */
    double getMinimumValue( unsigned int theBand )
    {
      if ( 0 < theBand && theBand <= getBandCount() )
      {
        return mContrastEnhancementList[theBand - 1].getMinimumValue();
      }

      return 0.0;
    }

    /** \brief Accessor for minimum value user for contrast enhancement */
    double getMinimumValue( QString theBand )
    {
      return getMinimumValue( getRasterBandNumber( theBand ) );
    }

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    void setMinimumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true )
    {
      if ( 0 < theBand && theBand <= getBandCount() )
      {
        mContrastEnhancementList[theBand - 1].setMinimumValue( theValue, theGenerateLookupTableFlag );
      }
    }

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    void setMinimumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true )
    {
      if ( theBand != tr( "Not Set" ) )
      {
        setMinimumValue( getRasterBandNumber( theBand ), theValue, theGenerateLookupTableFlag );
      }

    }

    /** \brief Accessor for maximum value user for contrast enhancement */
    double getMaximumValue( unsigned int theBand )
    {
      if ( 0 < theBand && theBand <= getBandCount() )
      {
        return mContrastEnhancementList[theBand - 1].getMaximumValue();
      }

      return 0.0;
    }

    /** \brief Accessor for maximum value user for contrast enhancement */
    double getMaximumValue( QString theBand )
    {
      if ( theBand != tr( "Not Set" ) )
      {
        return getMaximumValue( getRasterBandNumber( theBand ) );
      }

      return 0.0;
    }

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true )
    {
      if ( 0 < theBand && theBand <= getBandCount() )
      {
        mContrastEnhancementList[theBand - 1].setMaximumValue( theValue, theGenerateLookupTableFlag );
      }
    }

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true )
    {
      if ( theBand != tr( "Not Set" ) )
      {
        setMaximumValue( getRasterBandNumber( theBand ), theValue, theGenerateLookupTableFlag );
      }
    }

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    void computeMinimumMaximumEstimates( int theBand, double* theMinMax )
    {
      if ( 0 < theBand && theBand <= ( int ) getBandCount() )
      {
        GDALRasterBandH myGdalBand = GDALGetRasterBand( mGdalDataset, theBand );
        GDALComputeRasterMinMax( myGdalBand, 1, theMinMax );
      }
    }

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    void computeMinimumMaximumEstimates( QString theBand, double* theMinMax )
    {
      computeMinimumMaximumEstimates( getRasterBandNumber( theBand ), theMinMax );
    }

    QgsContrastEnhancement* getContrastEnhancement( unsigned int theBand )
    {
      return &mContrastEnhancementList[theBand - 1];
    }

    //
    // Accessor and mutator for the contrast enhancement algorithm
    //

    /** \brief Accessor for contrast enhancement algorithm. */
    QgsContrastEnhancement::CONTRAST_ENHANCEMENT_ALGORITHM getContrastEnhancementAlgorithm()
    {
      return mContrastEnhancementAlgorithm;
    }

    /** \brief Accessor for contrast enhancement algorithm. */
    QString getContrastEnhancementAlgorithmAsQString();

    /** \brief Mutator for contrast enhancement algorithm. */
    void setContrastEnhancementAlgorithm( QgsContrastEnhancement::CONTRAST_ENHANCEMENT_ALGORITHM theAlgorithm, bool theGenerateLookupTableFlag = true )
    {
      QList<QgsContrastEnhancement>::iterator myIterator = mContrastEnhancementList.begin();
      while ( myIterator !=  mContrastEnhancementList.end() )
      {
        ( *myIterator ).setContrastEnhancementAlgorithm( theAlgorithm, theGenerateLookupTableFlag );
        ++myIterator;
      }
      mContrastEnhancementAlgorithm = theAlgorithm;
    }

    /** \brief Mutator for contrast enhancement algorithm. */
    void setContrastEnhancementAlgorithm( QString theAlgorithm, bool theGenerateLookupTableFlag = true );

    //
    // Mutator for the contrast enhancement function
    //
    /** \brief Mutator for contrast enhancement function. */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction )
    {
      if ( theFunction )
      {
        QList<QgsContrastEnhancement>::iterator myIterator = mContrastEnhancementList.begin();
        while ( myIterator !=  mContrastEnhancementList.end() )
        {
          ( *myIterator ).setContrastEnhancementFunction( theFunction );
          ++myIterator;
        }
      }
    }

    /** \brief This enumerator describes the types of shading that can be used.  */
    enum COLOR_SHADING_ALGORITHM
    {
      UNDEFINED_SHADING_ALGORITHM,
      PSEUDO_COLOR,
      FREAK_OUT, //it will scare your granny!
      COLOR_RAMP,
      USER_DEFINED
    };
    //
    // Accessor and mutator for the color shader algorithm
    //
    /** \brief Accessor for color shader algorithm. */
    QgsRasterLayer::COLOR_SHADING_ALGORITHM getColorShadingAlgorithm()
    {
      return mColorShadingAlgorithm;
    }

    /** \brief Accessor for color shader algorithm. */
    QString getColorShadingAlgorithmAsQString();

    /** \brief Mutator for color shader algorithm. */
    void setColorShadingAlgorithm( QgsRasterLayer::COLOR_SHADING_ALGORITHM theShaderAlgorithm );

    /** \brief Mutator for color shader algorithm. */
    void setColorShadingAlgorithm( QString theShaderAlgorithm );

    /** \brief Accessor for raster shader */
    QgsRasterShader* getRasterShader()
    {
      return mRasterShader;
    }

    /** \brief Set the raster shader function to a user defined function */
    void setRasterShaderFunction( QgsRasterShaderFunction* theFunction )
    {
      if ( theFunction )
      {
        mRasterShader->setRasterShaderFunction( theFunction );
        mColorShadingAlgorithm = QgsRasterLayer::USER_DEFINED;
      }
      else
      {
        //If NULL as passed in, set a default shader function to prevent segfaults
        mRasterShader->setRasterShaderFunction( new QgsRasterShaderFunction() );
        mColorShadingAlgorithm = QgsRasterLayer::USER_DEFINED;
      }
    }

    /** \brief Read color table from GDAL raster band */
    bool readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList );

    /** \brief This enumerator describes the different kinds of drawing we can do.  */
    enum DRAWING_STYLE
    {
      UNDEFINED_DRAWING_STYLE,
      SINGLE_BAND_GRAY, // a "Gray" or "Undefined" layer drawn as a range of gray colors
      SINGLE_BAND_PSEUDO_COLOR,// a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
      PALETTED_COLOR, //a "Palette" image drawn using color table
      PALETTED_SINGLE_BAND_GRAY,// a "Palette" layer drawn in gray scale (using only one of the color components)
      PALETTED_SINGLE_BAND_PSEUDO_COLOR, // a "Palette" layer having only one of its color components rendered as psuedo color
      PALETTED_MULTI_BAND_COLOR, // currently not supported
      // as multiband
      MULTI_BAND_SINGLE_BAND_GRAY, // a layer containing 2 or more bands, but using only one band to produce a grayscale image
      MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR, //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
      MULTI_BAND_COLOR //a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
    } drawingStyle;
    //
    // Accessor and mutator for drawing style.
    //
    /** \brief Accessor for drawing style.  */
    DRAWING_STYLE getDrawingStyle() {return drawingStyle;}
    /** \brief Returns a string representation of drawing style.
     *
     * Implementaed mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future!. Use alternate implementation above rather.
     * */
    QString getDrawingStyleAsQString();
    /** \brief Mutator for drawing style.  */
    void setDrawingStyle( const DRAWING_STYLE &  theDrawingStyle ) {drawingStyle = theDrawingStyle;}
    /** \brief Overloaded version of the above function for convenience when restoring from xml.
     *
     * Implemented mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future! Use alternate implementation above rather.
     * */
    void setDrawingStyle( const QString & theDrawingStyleQString );

    /** \brief This enumerator describes the type of raster layer.  */
    enum RASTER_LAYER_TYPE
    {
      GRAY_OR_UNDEFINED,
      PALETTE,
      MULTIBAND
    } rasterLayerType;
    //
    // Accessor and for raster layer type (READ ONLY)
    //
    /** \brief  Accessor for raster layer type (which is a read only property) */
    RASTER_LAYER_TYPE getRasterLayerType() { return rasterLayerType; }
    /** \brief Accessor for hasPyramidsFlag (READ ONLY) */
    bool getHasPyramidsFlag() {return hasPyramidsFlag;}

    /** \brief Get a legend image for this layer.  */
    QPixmap getLegendQPixmap();
    /** \brief  Overloaded version of above function that can print layer name onto legend. */
    QPixmap getLegendQPixmap( bool );

    /** \brief Use this method when you want an annotated legend suitable for print output etc.
     * @param int theLabelCountInt Number of vertical labels to display (defaults to 3)
     * */
    QPixmap getDetailedLegendQPixmap( int theLabelCount );

    /**
     * Returns the sublayers of this layer
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    QStringList subLayers() const;

    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    virtual void setLayerOrder( const QStringList & layers );

    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( const QString & name, bool vis );

    /** \brief Emit a signal asking for a repaint. (inherited from maplayer) */
    void triggerRepaint();
    /** \brief Obtain GDAL Metadata for this layer */
    QString getMetadata();
    /** \brief Accessor for ths raster layers pyramid list. A pyramid list defines the
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the
     * list.*/
    RasterPyramidList buildRasterPyramidList();
    /** \brief Helper method to retrieve the nth pyramid layer struct from the PyramidList.
     * If the nth layer does not exist, NULL will be returned. */
//   RasterPyramid getRasterPyramid(int thePyramidNo);

    /**Currently returns always false*/
    bool isEditable() const;

    /** Return time stamp for given file name */
    static QDateTime lastModified( const QString &  name );

    /**Copies the symbology settings from another layer. Returns true in case of success*/
    bool copySymbologySettings( const QgsMapLayer& other )
    {
      //preventwarnings
      if ( other.type() < 0 )
      {
        return false;
      }
      return false;
    } //todo

    bool isSymbologyCompatible( const QgsMapLayer& other ) const
    {
      //preventwarnings
      if ( other.type() < 0 )
      {
        return false;
      }
      return false;
    } //todo

    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorCaptionString();

    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorString();

    /** Returns the data provider
     *
     *  \retval 0 if not using the data provider model (i.e. directly using GDAL)
     */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
     *
     *  \retval 0 if not using the data provider model (i.e. directly using GDAL)
     */
    const QgsRasterDataProvider* dataProvider() const;

    /** \brief Mutator for mUserDefinedRGBMinMaxFlag */
    void setUserDefinedRGBMinMax( bool theBool )
    {
      mUserDefinedRGBMinMaxFlag = theBool;
    }

    /** \brief Accessor for mUserDefinedRGBMinMaxFlag.  */
    bool getUserDefinedRGBMinMax()
    {
      return mUserDefinedRGBMinMaxFlag;
    }

    /** \brief Mutator for mRGBActualMinimumMaximum */
    void setActualRGBMinMaxFlag( bool theBool )
    {
      mRGBActualMinimumMaximum = theBool;
    }

    /** \brief Accessor for mRGBActualMinimumMaximum.  */
    bool getActualRGBMinMaxFlag()
    {
      return mRGBActualMinimumMaximum;
    }


    /** \brief Mutator for mUserDefinedGrayMinMaxFlag */
    void setUserDefinedGrayMinMax( bool theBool )
    {
      mUserDefinedGrayMinMaxFlag = theBool;
    }

    /** \brief Accessor for mUserDefinedGrayMinMaxFlag.  */
    bool getUserDefinedGrayMinMax()
    {
      return mUserDefinedGrayMinMaxFlag;
    }

    /** \brief Mutator for mGrayActualMinimumMaximum */
    void setActualGrayMinMaxFlag( bool theBool )
    {
      mGrayActualMinimumMaximum = theBool;
    }

    /** \brief Accessor for mGrayActualMinimumMaximum.  */
    bool getActualGrayMinMaxFlag()
    {
      return mGrayActualMinimumMaximum;
    }

  public slots:
    /**
     * Convert this raster to another format
     */
    //void const convertTo();
    /**
     * Mainly intended for use in propagating progress updates from gdal up to the parent app.
     **/
    void updateProgress( int, int );

    /** \brief Create  gdal pyramid overviews  for this layer.
    * This will speed up performance at the expense of hard drive space.
    * Also, write access to the file is required for creating internal pyramids,
    * and to the directory in which the files exists if external
    * pyramids (.ovr) are to be created. If no paramter is passed in
    * it will default to nearest neighbor resampling.
    * @param theTryInternalFlag - Try to make the pyramids internal to
    * the raster file if supported (e.g. geotiff). If not supported it
    * will revert to creating external .ovr file anyway.
    * \return null string on success, otherwise a string specifying error
    */
    QString buildPyramids( const RasterPyramidList &,
                           const QString &  theResamplingMethod = "NEAREST",
                           bool theTryInternalFlag = false );
    /** \brief Used at the moment by the above function but hopefully will later
    be useable by any operation that needs to notify the user of its progress. */
    /*
        int showTextProgress( double theProgress,
                              const char *theMessageCharArray,
                              void *theData);
    */

    /** Populate the histogram vector for a given layer
    * @param theBandNoInt - which band to compute the histogram for
    * @param theBinCountInt - how many 'bins' to categorise the data into
    * @param theIgnoreOutOfRangeFlag - whether to ignore values that are out of range (default=true)
    * @param theThoroughBandScanFlag - whether to visit each cell when computing the histogram (default=false)
    */
    void populateHistogram( int theBandNoInt,
                            int theBinCountInt = 256,
                            bool theIgnoreOutOfRangeFlag = true,
                            bool theThoroughBandScanFlag = false );

    /** \brief Color table
     *  \param band number
     *  \return pointer to color table
     */
    QList<QgsColorRampShader::ColorRampItem>* getColorTable( int theBandNoInt );
  protected:

    /** reads vector layer specific state from project file Dom node.

        @note

        Called by QgsMapLayer::readXML().

    */
    /* virtual */ bool readXml( QDomNode & layer_node );



    /** write vector layer specific state to project file Dom node.

        @note

        Called by QgsMapLayer::writeXML().

    */
    /* virtual */ bool writeXml( QDomNode & layer_node, QDomDocument & doc );

  private:

    //
    // Private methods
    //

    //
    // Grayscale Imagery
    //

    /** \brief Drawing routine for single band grayscale image.  */
    void drawSingleBandGray( QPainter * theQPainter,
                             QgsRasterViewPort * theRasterViewPort,
                             const QgsMapToPixel* theQgsMapToPixel,
                             int theBandNoInt );

    /** \brief Drawing routine for single band grayscale image, rendered in pseudocolor.  */
    void drawSingleBandPseudoColor( QPainter * theQPainter,
                                    QgsRasterViewPort * theRasterViewPort,
                                    const QgsMapToPixel* theQgsMapToPixel,
                                    int theBandNoInt );


    //
    // Paletted Layers
    //

    /** \brief Drawing routine for single band with a color map.  */
    void drawPalettedSingleBandColor( QPainter * theQPainter,
                                      QgsRasterViewPort * theRasterViewPort,
                                      const QgsMapToPixel* theQgsMapToPixel,
                                      int theBandNoInt );

    /** \brief Drawing routine for paletted image, rendered as a single band image in grayscale.  */
    void drawPalettedSingleBandGray( QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort,
                                     const QgsMapToPixel* theQgsMapToPixel,
                                     int theBandNoInt );

    /** \brief Drawing routine for paletted image, rendered as a single band image in pseudocolor.  */
    void drawPalettedSingleBandPseudoColor( QPainter * theQPainter,
                                            QgsRasterViewPort * theRasterViewPort,
                                            const QgsMapToPixel* theQgsMapToPixel,
                                            int theBandNoInt,
                                            const QString &  theColorQString );

    /** \brief Drawing routine for paletted multiband image.  */
    void drawPalettedMultiBandColor( QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort,
                                     const QgsMapToPixel* theQgsMapToPixel,
                                     int theBandNoInt );

    //
    // Multiband Layers
    //

    /** \brief Drawing routine for multiband image, rendered as a single band image in grayscale.  */
    void drawMultiBandSingleBandGray( QPainter * theQPainter,
                                      QgsRasterViewPort * theRasterViewPort,
                                      const QgsMapToPixel* theQgsMapToPixel,
                                      int theBandNoInt );

    /** \brief Drawing routine for multiband image, rendered as a single band image in pseudocolor.  */
    void drawMultiBandSingleBandPseudoColor( QPainter * theQPainter,
        QgsRasterViewPort * theRasterViewPort,
        const QgsMapToPixel* theQgsMapToPixel,
        int theBandNoInt );

    /** \brief Drawing routine for multiband image  */
    void drawMultiBandColor( QPainter * theQPainter,
                             QgsRasterViewPort * theRasterViewPort,
                             const QgsMapToPixel* theQgsMapToPixel );

    /** \brief Places the rendered image onto the canvas */
    void paintImageToCanvas( QPainter* theQPainter, QgsRasterViewPort * theRasterViewPort,
                             const QgsMapToPixel* theQgsMapToPixel, QImage* theImage );

    /** \brief Allocate memory and load data to that allocated memory, data type is the same
     *         as raster band. The memory must be released later!
     *  \return pointer to the memory
     */
    void *readData( GDALRasterBandH gdalBand, QgsRasterViewPort *viewPort );

    /** \brief Read a raster value on given position from memory block created by readData()
     *  \param index index in memory block
     */
    inline double readValue( void *data, GDALDataType type, int index );


    /**
       Load the given raster file

       @returns true if successfully read file

       @note

       Called from ctor if a raster image given there
     */
    bool readFile( const QString & fileName );

    /** \brief Close data set and release related data */
    void closeDataset();

    /** \brief Update the layer if it is outdated */
    bool update();

    /** \brief Verify and transform band name for internal consistency. Return 'Not Set' on any type of failure */
    QString validateBandName( const QString & theBandName );

    //
    // Private member vars
    //
    /** \brief  Constant defining flag for XML and a constant that signals property not used */
    const QString QSTRING_NOT_SET;
    const QString TRSTRING_NOT_SET;

    /** \brief  Raster width. */
    int mRasterXDim;
    /** \brief  Raster height. */
    int mRasterYDim;
    /** \brief Cell value representing no data. e.g. -9999  */
    double mNoDataValue;
    /** \brief Flag indicating if the nodatavalue is valid*/
    bool mValidNoDataValue;
    /** \brief Pointer to the gdaldataset.  */
    GDALDatasetH mGdalBaseDataset;
    /** \brief Pointer to the gdaldataset (possibly warped vrt).  */
    GDALDatasetH mGdalDataset;
    /** \brief Values for mapping pixel to world coordinates. Contents of
     * this array are the same as the gdal adfGeoTransform */
    double mGeoTransform[6];
    /** \brief Flag indicating whether the color of pixels should be inverted or not.  */
    bool mInvertPixelsFlag;
    /** \brief Number of stddev to plot (0) to ignore. Not applicable to all layer types.  */
    double mStandardDeviations;
    /** \brief A collection of stats - one for each band in the layer.
     * The typedef for this is defined above before class declaration
     */
    RasterStatsList mRasterStatsList;
    /** \brief List containing the contrast enhancements for each band */
    ContrastEnhancementList mContrastEnhancementList;
    /** \brief The contrast enhancement algorithm being used */
    QgsContrastEnhancement::CONTRAST_ENHANCEMENT_ALGORITHM mContrastEnhancementAlgorithm;
    /** \brief The raster shading algorithm being used */
    COLOR_SHADING_ALGORITHM mColorShadingAlgorithm;
    /** \brief The raster shader for the layer */
    QgsRasterShader* mRasterShader;
    /** \brief The band to be associated with the color red - usually 1.  */
    QString mRedBandName;
    /** \brief The band to be associated with the color green - usually 2.  */
    QString mGreenBandName;
    /** \brief The band to be associated with the color blue - usually 3.  */
    QString mBlueBandName;
    /** \brief The transparency container */
    QgsRasterTransparency mRasterTransparency;
    /** \brief The band to be associated with transparency.  */
    QString mTransparencyBandName;
    /** \brief The band to be associated with the grayscale only output - usually 1.  */
    QString mGrayBandName;
    /** \brief Whether this raster has overviews / pyramids or not */
    bool hasPyramidsFlag;
    /** \brief Flag to indicate if the user entered custom min max values */
    bool mUserDefinedRGBMinMaxFlag;
    /** \brief Flag to indicate of the min max values are actual or estimates/user defined */
    bool mRGBActualMinimumMaximum;
    /** \brief Flag to indicate if the user entered custom min max values */
    bool mUserDefinedGrayMinMaxFlag;
    /** \brief Flag to indicate of the min max values are actual or estimates/user defined */
    bool mGrayActualMinimumMaximum;
    /** \brief This list holds a series of RasterPyramid structs
     * which store information for each potential pyramid level for this raster.*/
    RasterPyramidList mPyramidList;

    /*
     *
     * New functions that will convert this class to a data provider interface
     * (B Morley)
     *
     */

  public:

    //! Constructor in provider mode
    // TODO Rename into a general constructor when the old raster interface is retired
    // \param  dummy  is just there to distinguish this function signature from the old non-provider one.
    QgsRasterLayer( int dummy,
                    const QString & baseName = QString(),
                    const QString & path = QString(),
                    const QString & providerLib = QString(),
                    const QStringList & layers = QStringList(),
                    const QStringList & styles = QStringList(),
                    const QString & format = QString(),
                    const QString & crs = QString()
                  );

    void setDataProvider( const QString & provider,
                          const QStringList & layers,
                          const QStringList & styles,
                          const QString & format,
                          const QString & crs );

    //! Does this layer use a provider for setting/retrieving data?
    bool usesProvider();

    //! Which provider is being used for this Raster Layer?
    QString providerKey();

    /** A wrapper function to emit a progress update signal.
     * For example used by gdal callback to show pyramid building progress.
     */
    void showProgress( int theValue );

  public slots:

    void showStatusMessage( const QString & theMessage );

  signals:
    //for notifying listeners of long running processes
    void progressUpdate( int theValue );


  private:

    //! Data provider key
    QString mProviderKey;

    //! pointer for loading the provider library
    QLibrary *mLib;

    //! Pointer to data provider derived from the abstract base class QgsDataProvider
    QgsRasterDataProvider *mDataProvider;

    /**Flag indicating wheter the layer is in editing mode or not*/
    bool mEditable;

    /**Flag indicating whether the layer has been modified since the last commit*/
    bool mModified;

    //! Timestamp, the last modified time of the data source when the layer was created
    QDateTime mLastModified;

    /**
     * The error caption associated with the last error.
     */
    QString mErrorCaption;

    /**
     * The error message associated with the last error.
     */
    QString mError;

};

#endif
