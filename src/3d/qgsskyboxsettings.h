#ifndef QGSSKYBOXSETTINGS_H
#define QGSSKYBOXSETTINGS_H

#include <QString>
#include <QMap>
#include <QDomDocument>

#include "qgsreadwritecontext.h"
#include "qgssymbollayerutils.h"

class QgsSkyboxSettings
{
  public:

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context )
    {
      Q_UNUSED( context );
      mIsSkyboxEnabled = element.attribute( QStringLiteral( "skybox-enabled" ) ).toInt();
      mSkyboxType = element.attribute( QStringLiteral( "skybox-type" ) );
      mSkyboxBaseName = element.attribute( QStringLiteral( "base-name" ) );
      mSkyboxExt = element.attribute( QStringLiteral( "extension" ) );
      mHDRTexturePath = element.attribute( QStringLiteral( "HDR-texture-path" ) );
      mCubeMapFacesPaths.clear();
      mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] = element.attribute( QStringLiteral( "posX-texture-path" ) );
      mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] = element.attribute( QStringLiteral( "posY-texture-path" ) );
      mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] = element.attribute( QStringLiteral( "posZ-texture-path" ) );
      mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] = element.attribute( QStringLiteral( "negX-texture-path" ) );
      mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] = element.attribute( QStringLiteral( "negY-texture-path" ) );
      mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] = element.attribute( QStringLiteral( "negZ-texture-path" ) );
    }

    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const
    {
      Q_UNUSED( context );
      element.setAttribute( QStringLiteral( "skybox-enabled" ), mIsSkyboxEnabled );
      element.setAttribute( QStringLiteral( "skybox-type" ), mSkyboxType );
      element.setAttribute( QStringLiteral( "base-name" ), mSkyboxBaseName );
      element.setAttribute( QStringLiteral( "extension" ), mSkyboxExt );
      element.setAttribute( QStringLiteral( "HDR-texture-path" ), mHDRTexturePath );
      element.setAttribute( QStringLiteral( "posX-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posX" ) ] );
      element.setAttribute( QStringLiteral( "posY-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posY" ) ] );
      element.setAttribute( QStringLiteral( "posZ-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "posZ" ) ] );
      element.setAttribute( QStringLiteral( "negX-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negX" ) ] );
      element.setAttribute( QStringLiteral( "negY-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negY" ) ] );
      element.setAttribute( QStringLiteral( "negZ-texture-path" ), mCubeMapFacesPaths[ QStringLiteral( "negZ" ) ] );
    }

    bool isSkyboxEnabled() const { return mIsSkyboxEnabled; }
    void setIsSkyboxEnabled( bool enabled ) { mIsSkyboxEnabled = enabled; }

    QString skyboxType() const { return mSkyboxType; }
    void setSkyboxType( const QString &type ) { mSkyboxType = type; }

    QString skyboxBaseName() const { return mSkyboxBaseName; }
    void setSkyboxBaseName( const QString &baseName ) { mSkyboxBaseName = baseName; }

    QString skyboxExtension() const { return mSkyboxExt; }
    void setSkyboxExtension( const QString &extension ) { mSkyboxExt = extension; }

    QString hdrTexturePath() const { return mHDRTexturePath; }
    void setHDRTexturePath( const QString &texturePath ) { mHDRTexturePath = texturePath; }

    QMap<QString, QString> cubeMapFacesPaths() const { return mCubeMapFacesPaths; }
    void setCubeMapFace( const QString &face, const QString &path ) { mCubeMapFacesPaths[face] = path; }

  private:
    bool mIsSkyboxEnabled = false;
    QString mSkyboxType;
    //
    QString mSkyboxBaseName;
    QString mSkyboxExt;
    //
    QString mHDRTexturePath;
    //
    QMap<QString, QString> mCubeMapFacesPaths;
};

#endif // QGSSKYBOXSETTINGS_H
