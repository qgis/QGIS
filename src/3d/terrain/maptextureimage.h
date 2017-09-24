#ifndef MAPTEXTUREIMAGE_H
#define MAPTEXTUREIMAGE_H

#include <Qt3DRender/QAbstractTextureImage>

#include "qgsrectangle.h"

class MapTextureGenerator;

/** \ingroup 3d
 * Class that stores an image with a rendered map. The image is used as a texture for one map tile.
 *
 * The texture is provided to Qt 3D through the implementation of dataGenerator() method.
 *
 * \since QGIS 3.0
 */
class MapTextureImage : public Qt3DRender::QAbstractTextureImage
{
    Q_OBJECT
  public:
    //! Constructs the object with given image and map extent
    MapTextureImage( const QImage &image, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent = nullptr );
    ~MapTextureImage();

    virtual Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

    //! Clears the current map image and emits signal that data generator has changed
    void invalidate();
    //! Stores a new map image and emits signal that data generator has changed
    void setImage( const QImage &img );

    //! Returns extent of the image in map coordinates
    QgsRectangle imageExtent() const { return extent; }
    //! Returns debug information (normally map tile coordinates)
    QString imageDebugText() const { return debugText; }

  private:
    QgsRectangle extent;
    QString debugText;
    QImage img;
    int version;
    int jobId;
};

#endif // MAPTEXTUREIMAGE_H
