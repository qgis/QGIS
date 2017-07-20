#include "maptextureimage.h"

#include <Qt3DRender/QTextureImageDataGenerator>

#include "maptexturegenerator.h"

class MapTextureImageDataGenerator : public Qt3DRender::QTextureImageDataGenerator
{
  public:
    QgsRectangle extent;
    QString debugText;
    QImage img;

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

    MapTextureImageDataGenerator( const QgsRectangle &extent, const QString &debugText, const QImage &img )
      : extent( extent ), debugText( debugText ), img( img ) {}

    virtual Qt3DRender::QTextureImageDataPtr operator()() override
    {
      Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
      dataPtr->setImage( img.isNull() ? placeholderImage() : img ); // will copy image data to the internal byte array
      return dataPtr;
    }

    virtual bool operator ==( const QTextureImageDataGenerator &other ) const override
    {
      const MapTextureImageDataGenerator *otherFunctor = functor_cast<MapTextureImageDataGenerator>( &other );
      return otherFunctor != nullptr && otherFunctor->img.isNull() == img.isNull() &&
             extent == otherFunctor->extent;
    }

    QT3D_FUNCTOR( MapTextureImageDataGenerator )
};


MapTextureImage::MapTextureImage( MapTextureGenerator *mapGen, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent )
  : Qt3DRender::QAbstractTextureImage( parent )
  , mapGen( mapGen )
  , extent( extent )
  , debugText( debugText )
  , jobDone( false )
{
  connect( mapGen, &MapTextureGenerator::tileReady, this, &MapTextureImage::onTileReady );

  // request image
  jobId = mapGen->render( extent, debugText );
}


MapTextureImage::MapTextureImage( const QImage &image, const QgsRectangle &extent, const QString &debugText, Qt3DCore::QNode *parent )
  : Qt3DRender::QAbstractTextureImage( parent )
  , extent( extent )
  , debugText( debugText )
  , img( image )
  , jobDone( true )
{
}


MapTextureImage::~MapTextureImage()
{
  if ( !jobDone )
    mapGen->cancelJob( jobId );
}

Qt3DRender::QTextureImageDataGeneratorPtr MapTextureImage::dataGenerator() const
{
  return Qt3DRender::QTextureImageDataGeneratorPtr( new MapTextureImageDataGenerator( extent, debugText, img ) );
}

void MapTextureImage::onTileReady( int jobId, const QImage &img )
{
  if ( jobId == this->jobId )
  {
    this->img = img;
    this->jobDone = true;
    notifyDataGeneratorChanged();
    emit textureReady();
  }
}
