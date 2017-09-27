#include "qgsterraintextureimage_p.h"

#include <Qt3DRender/QTextureImageDataGenerator>

#include "qgsterraintexturegenerator_p.h"

///@cond PRIVATE

class TerrainTextureImageDataGenerator : public Qt3DRender::QTextureImageDataGenerator
{
  public:

    static QImage placeholderImage()
    {
      // simple placeholder image
      QImage i( 2, 2, QImage::Format_RGB32 );
      i.setPixelColor( 0, 0, Qt::darkGray );
      i.setPixelColor( 1, 0, Qt::lightGray );
      i.setPixelColor( 0, 1, Qt::lightGray );
      i.setPixelColor( 1, 1, Qt::darkGray );
      return i;
    }

    TerrainTextureImageDataGenerator( const QgsRectangle &extent, const QString &debugText, const QImage &img, int version )
      : mExtent( extent ), mDebugText( debugText ), mImage( img ), mVersion( version ) {}

    virtual Qt3DRender::QTextureImageDataPtr operator()() override
    {
      Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
      dataPtr->setImage( mImage.isNull() ? placeholderImage() : mImage ); // will copy image data to the internal byte array
      return dataPtr;
    }

    virtual bool operator ==( const QTextureImageDataGenerator &other ) const override
    {
      const TerrainTextureImageDataGenerator *otherFunctor = functor_cast<TerrainTextureImageDataGenerator>( &other );
      return otherFunctor != nullptr && otherFunctor->mVersion == mVersion &&
             mExtent == otherFunctor->mExtent;
    }

    QT3D_FUNCTOR( TerrainTextureImageDataGenerator )

  private:
    QgsRectangle mExtent;
    QString mDebugText;
    QImage mImage;
    int mVersion;
};



////////


QgsTerrainTextureImage::QgsTerrainTextureImage( const QImage &image, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent )
  : Qt3DRender::QAbstractTextureImage( parent )
  , mExtent( extent )
  , mDebugText( debugText )
  , mImage( image )
  , mVersion( 1 )
{
}


QgsTerrainTextureImage::~QgsTerrainTextureImage()
{
}

Qt3DRender::QTextureImageDataGeneratorPtr QgsTerrainTextureImage::dataGenerator() const
{
  return Qt3DRender::QTextureImageDataGeneratorPtr( new TerrainTextureImageDataGenerator( mExtent, mDebugText, mImage, mVersion ) );
}

void QgsTerrainTextureImage::invalidate()
{
  mImage = QImage();
  mVersion++;
  notifyDataGeneratorChanged();
}

void QgsTerrainTextureImage::setImage( const QImage &img )
{
  this->mImage = img;
  mVersion++;
  notifyDataGeneratorChanged();
}

/// @endcond
