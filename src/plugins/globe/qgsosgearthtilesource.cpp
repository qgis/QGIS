
#include <osgEarth/TileSource>
#include <osgEarth/Registry>
#include <osgEarth/ImageUtils>

#include <osg/Notify>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <sstream>

#include "qgsosgearthtilesource.h"

#include "qgsapplication.h"
#include <qgslogger.h>
#include <qgisinterface.h>
#include <qgsmapcanvas.h>
#include "qgsproviderregistry.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgspallabeling.h"
#include "qgsproject.h"

#include <QFile>
#include <QPainter>

using namespace osgEarth;
using namespace osgEarth::Drivers;


QgsOsgEarthTileSource::QgsOsgEarthTileSource( QgisInterface* theQgisInterface ) : TileSource(), mQGisIface(theQgisInterface), mCoordTranform(0)
{
}

void QgsOsgEarthTileSource::initialize( const std::string& referenceURI, const Profile* overrideProfile)
{
    setProfile( osgEarth::Registry::instance()->getGlobalGeodeticProfile() );
    QgsMapRenderer* mainRenderer = mQGisIface->mapCanvas()->mapRenderer();
    mMapRenderer = new QgsMapRenderer();

    long epsgGlobe = 4326;
    if (mainRenderer->destinationSrs().epsg() != epsgGlobe)
    {
      QgsCoordinateReferenceSystem srcCRS;
      srcCRS.createFromEpsg(mainRenderer->destinationSrs().epsg()); //FIXME: crs from canvas or first layer?
      QgsCoordinateReferenceSystem destCRS;
      destCRS.createFromEpsg(epsgGlobe);
      //QgsProject::instance()->writeEntry("SpatialRefSys","/ProjectionsEnabled",1);
      mMapRenderer->setDestinationSrs(destCRS);
      mMapRenderer->setProjectionsEnabled(true);
      mCoordTranform = new QgsCoordinateTransform( srcCRS, destCRS );
    }
    mMapRenderer->setOutputUnits(mainRenderer->outputUnits());
    mMapRenderer->setMapUnits( QGis::Degrees );

    mMapRenderer->setLabelingEngine( new QgsPalLabeling() );
}

osg::Image* QgsOsgEarthTileSource::createImage( const TileKey* key,
                          ProgressCallback* progress )
{
    osg::ref_ptr<osg::Image> image;
    if (intersects(key))
    {
        //Get the extents of the tile
        double xmin, ymin, xmax, ymax;
        key->getGeoExtent().getBounds(xmin, ymin, xmax, ymax);

        int tileSize = getPixelsPerTile();
        int target_width = tileSize;
        int target_height = tileSize;

        QgsDebugMsg("QGIS: xmin:" + QString::number(xmin) + " ymin:" + QString::number(ymin) + " ymax:" + QString::number(ymax) + " ymax " + QString::number(ymax));

        //Return if parameters are out of range.
        if (target_width <= 0 || target_height <= 0)
        {
            return 0;
        }

        QImage* qImage = createImage(target_width, target_height);
        if(!qImage)
        {
            return 0;
        }

        QgsMapRenderer* mainRenderer = mQGisIface->mapCanvas()->mapRenderer();
        mMapRenderer->setLayerSet(mainRenderer->layerSet());

        mMapRenderer->setOutputSize(QSize(qImage->width(), qImage->height()), qImage->logicalDpiX());

        QgsRectangle mapExtent(xmin, ymin, xmax, ymax);
        mMapRenderer->setExtent(mapExtent);

        QPainter thePainter(qImage);
        //thePainter.setRenderHint(QPainter::Antialiasing); //make it look nicer
        mMapRenderer->render(&thePainter);

        unsigned char* data = qImage->bits();

        image = new osg::Image;
        //The pixel format is always RGBA to support transparency
        image->setImage(qImage->width(), qImage->height(), 1,
                    4,
                    GL_BGRA,GL_UNSIGNED_BYTE, //Why not GL_RGBA - QGIS bug?
                    data,
                    osg::Image::NO_DELETE, 1);
        image->flipVertical();
    }

    //Create a transparent image if we don't have an image
    if (!image.valid())
    {
        return ImageUtils::createEmptyImage();
    }
    return image.release();
}

QImage* QgsOsgEarthTileSource::createImage( int width, int height ) const
{
  if( width < 0 || height < 0 )
  {
    return 0;
  }

  QImage* qImage = 0;

  //is format jpeg?
  bool jpeg = false;
  //transparent parameter
  bool transparent = true;

  //use alpha channel only if necessary because it slows down performance
  if( transparent && !jpeg )
  {
    qImage = new QImage( width, height, QImage::Format_ARGB32_Premultiplied );
    qImage->fill( 0 );
  }
  else
  {
    qImage = new QImage( width, height, QImage::Format_RGB32 );
    qImage->fill( qRgb(255, 255, 255) );
  }

  if(!qImage)
    {
      return 0;
    }

  //apply DPI parameter if present.
  /*
        int dpm = dpi / 0.0254;
        qImage->setDotsPerMeterX(dpm);
        qImage->setDotsPerMeterY(dpm);
        */
  return qImage;
}

bool QgsOsgEarthTileSource::intersects(const TileKey* key)
{
    //Get the native extents of the tile
    double xmin, ymin, xmax, ymax;
    key->getGeoExtent().getBounds(xmin, ymin, xmax, ymax);
    QgsRectangle extent = mQGisIface->mapCanvas()->fullExtent();
    if (mCoordTranform) extent = mCoordTranform->transformBoundingBox(extent);
    return ! ( xmin >= extent.xMaximum() || xmax <= extent.xMinimum() || ymin >= extent.yMaximum() || ymax <= extent.yMinimum() );
}
