
#ifndef OSGEARTH_DRIVER_QGIS_DRIVEROPTIONS
#define OSGEARTH_DRIVER_QGIS_DRIVEROPTIONS 1

#include "qgsmaprenderer.h"

#include <QImage>
class QgisInterface;

#include <osgEarth/Common>
#include <osgEarth/TileSource>
#include <osgEarth/ThreadingUtils>

using namespace osgEarth;

namespace osgEarth { namespace Drivers
{
    class QgsOsgEarthTileSource : public TileSource
    {
    public:
        QgsOsgEarthTileSource( QgisInterface* theQgisInterface );

        void initialize( const std::string& referenceURI, const Profile* overrideProfile);

        osg::Image* createImage( const TileKey* key,
                                ProgressCallback* progress );

        osg::HeightField* createHeightField( const TileKey* key,
                                            ProgressCallback* progress)
        {
            //NI
            OE_WARN << "[QGIS] Driver does not support heightfields" << std::endl;
            return NULL;
        }

        virtual std::string getExtension()  const
        {
            //All QGIS tiles are in JPEG format
            return "jpg";
        }

        virtual bool supportsPersistentCaching() const
        {
            return false;
        }

        Threading::ReadWriteMutex& getRenderMutex() { return mRenderMutex; }

private:

    int configureMapRender( const QPaintDevice* paintDevice ) const;
    QImage* createImage( int width, int height ) const;
    bool intersects(const TileKey* key);

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    QgsMapRenderer* mMapRenderer;
    //! Canvas render Mutex
    Threading::ReadWriteMutex mRenderMutex;

    };
} } // namespace osgEarth::Drivers

#endif // OSGEARTH_DRIVER_QGIS_DRIVEROPTIONS

