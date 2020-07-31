#ifndef QGSSKYBOXENTITY_H
#define QGSSKYBOXENTITY_H

#include <Qt3DCore/QEntity>
#include <QVector3D>
#include <Qt3DRender/QTexture>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QRenderPass>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DRender/QParameter>

class QgsSkyboxTexturesLoader : public Qt3DCore::QEntity
{
  public:
    QgsSkyboxTexturesLoader( Qt3DCore::QNode *parent = nullptr ) : Qt3DCore::QEntity( parent ) {  }
    virtual QVariant getTextureParameter() { return QVariant(); }
};

class QgsDDSSkyboxLoader : public QgsSkyboxTexturesLoader
{
  public:
    QgsDDSSkyboxLoader( const QString &baseName, const QString &extension, Qt3DCore::QNode *parent = nullptr )
      : QgsSkyboxTexturesLoader( parent )
    {
      mLoadedTexture = new Qt3DRender::QTextureLoader( this );
      mLoadedTexture->setGenerateMipMaps( false );
      mLoadedTexture->setSource( QUrl( baseName + extension ) );
    }

    QVariant getTextureParameter() override { return QVariant::fromValue( mLoadedTexture ); }
  private:
    Qt3DRender::QTextureLoader *mLoadedTexture;
};

class QgsSkyboxTextureColloectionLoader : public QgsSkyboxTexturesLoader
{
  public:
    QgsSkyboxTextureColloectionLoader( const QString &baseName, const QString &extension, Qt3DCore::QNode *parent = nullptr )
      : QgsSkyboxTexturesLoader( parent )
    {
      mPosXImage = new Qt3DRender::QTextureImage( this );
      mPosYImage = new Qt3DRender::QTextureImage( this );
      mPosZImage = new Qt3DRender::QTextureImage( this );
      mNegXImage = new Qt3DRender::QTextureImage( this );
      mNegYImage = new Qt3DRender::QTextureImage( this );
      mNegZImage = new Qt3DRender::QTextureImage( this );

      mPosXImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveX );
      mPosXImage->setMirrored( false );
      mPosYImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveY );
      mPosYImage->setMirrored( false );
      mPosZImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapPositiveZ );
      mPosZImage->setMirrored( false );
      mNegXImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeX );
      mNegXImage->setMirrored( false );
      mNegYImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeY );
      mNegYImage->setMirrored( false );
      mNegZImage->setFace( Qt3DRender::QTextureCubeMap::CubeMapNegativeZ );
      mNegZImage->setMirrored( false );

      mPosXImage->setSource( QUrl( baseName + QStringLiteral( "_posx" ) + extension ) );
      mPosYImage->setSource( QUrl( baseName + QStringLiteral( "_posy" ) + extension ) );
      mPosZImage->setSource( QUrl( baseName + QStringLiteral( "_posz" ) + extension ) );
      mNegXImage->setSource( QUrl( baseName + QStringLiteral( "_negx" ) + extension ) );
      mNegYImage->setSource( QUrl( baseName + QStringLiteral( "_negy" ) + extension ) );
      mNegZImage->setSource( QUrl( baseName + QStringLiteral( "_negz" ) + extension ) );

      mCubeMap = new Qt3DRender::QTextureCubeMap( this );
      mCubeMap->setMagnificationFilter( Qt3DRender::QTextureCubeMap::Linear );
      mCubeMap->setMinificationFilter( Qt3DRender::QTextureCubeMap::Linear );
      mCubeMap->setGenerateMipMaps( false );
      mCubeMap->setWrapMode( Qt3DRender::QTextureWrapMode( Qt3DRender::QTextureWrapMode::Repeat ) );

      mCubeMap->addTextureImage( mPosXImage );
      mCubeMap->addTextureImage( mPosYImage );
      mCubeMap->addTextureImage( mPosZImage );
      mCubeMap->addTextureImage( mNegXImage );
      mCubeMap->addTextureImage( mNegYImage );
      mCubeMap->addTextureImage( mNegZImage );
    }
    QVariant getTextureParameter() { return QVariant::fromValue( mCubeMap ); }
  private:
    Qt3DRender::QTextureCubeMap *mCubeMap = nullptr;
    Qt3DRender::QTextureImage *mPosXImage = nullptr;
    Qt3DRender::QTextureImage *mPosYImage = nullptr;
    Qt3DRender::QTextureImage *mPosZImage = nullptr;
    Qt3DRender::QTextureImage *mNegXImage = nullptr;
    Qt3DRender::QTextureImage *mNegYImage = nullptr;
    Qt3DRender::QTextureImage *mNegZImage = nullptr;
};

class QgsSkyboxEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    QgsSkyboxEntity( const QString &baseName, const QString &extension, Qt3DCore::QNode *parent = nullptr );

    QString baseName() const { return mBaseName; };
    QString extension() const { return mExtension; };
    bool isGammaCorrectEnabled() const { return !qFuzzyIsNull( mGammaStrengthParameter->value().toFloat() ); }

  public slots:
    void setBaseName( const QString &path );
    void setExtension( const QString &extension );
    void setGammaCorrectEnabled( bool enabled );

  signals:
    void baseNameChanged( const QString &path );
    void extensionChanged( const QString &extension );
    void gammaCorrectEnabledChanged( bool enabled );

  private:
    void reloadTexture();
  private:
    QgsSkyboxTexturesLoader *mSkyboxTextureLoader = nullptr;
    Qt3DRender::QEffect *mEffect;
    Qt3DRender::QMaterial *mMaterial;
    Qt3DRender::QShaderProgram *mGl3Shader;
    Qt3DRender::QTechnique *mGl3Technique;
    Qt3DRender::QFilterKey *mFilterKey;
    Qt3DRender::QRenderPass *mGl3RenderPass;
    Qt3DExtras::QCuboidMesh *mMesh;
    Qt3DRender::QParameter *mGammaStrengthParameter;
    Qt3DRender::QParameter *mTextureParameter;
    QString mExtension;
    QString mBaseName;
    QVector3D mPosition;
};

#endif // QGSSKYBOXENTITY_H
