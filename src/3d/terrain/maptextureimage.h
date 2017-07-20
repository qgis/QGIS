#ifndef MAPTEXTUREIMAGE_H
#define MAPTEXTUREIMAGE_H

#include <Qt3DRender/QAbstractTextureImage>

#include "qgsrectangle.h"

class MapTextureGenerator;

//! texture image with a rendered map
class MapTextureImage : public Qt3DRender::QAbstractTextureImage
{
    Q_OBJECT
  public:
    //! constructor that will generate image asynchronously
    MapTextureImage( MapTextureGenerator *mapGen, const QgsRectangle &extent, const QString &debugText = QString(), Qt3DCore::QNode *parent = nullptr );
    //! constructor that uses already prepared image
    MapTextureImage( const QImage &image, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent = nullptr );
    ~MapTextureImage();

    virtual Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

  private slots:
    void onTileReady( int jobId, const QImage &img );

  signals:
    void textureReady();

  private:
    MapTextureGenerator *mapGen;
    QgsRectangle extent;
    QString debugText;
    QImage img;
    int jobId;
    bool jobDone;
};

#endif // MAPTEXTUREIMAGE_H
