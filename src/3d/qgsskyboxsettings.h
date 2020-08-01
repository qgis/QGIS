#ifndef QGSSKYBOXSETTINGS_H
#define QGSSKYBOXSETTINGS_H

#include <QString>
#include <QMap>

class QgsSkyboxSettings
{
  public:
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
