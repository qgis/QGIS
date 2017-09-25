#ifndef QGSTERRAINTEXTUREIMAGE_P_H
#define QGSTERRAINTEXTUREIMAGE_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DRender/QAbstractTextureImage>

#include "qgsrectangle.h"

class QgsTerrainTextureGenerator;

/** \ingroup 3d
 * Class that stores an image with a rendered map. The image is used as a texture for one map tile.
 *
 * The texture is provided to Qt 3D through the implementation of dataGenerator() method.
 *
 * \since QGIS 3.0
 */
class QgsTerrainTextureImage : public Qt3DRender::QAbstractTextureImage
{
    Q_OBJECT
  public:
    //! Constructs the object with given image and map extent
    QgsTerrainTextureImage( const QImage &image, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent = nullptr );
    ~QgsTerrainTextureImage();

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

/// @endcond

#endif // QGSTERRAINTEXTUREIMAGE_P_H
