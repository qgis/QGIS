#ifndef QGSSKYBOXSETTINGS_H
#define QGSSKYBOXSETTINGS_H

#include <QString>

class QgsSkyboxSettings
{
  public:
    bool getIsSkyboxEnabled() const { return mIsSkyboxEnabled; }
    void setIsSkyboxEnabled( bool enabled ) { mIsSkyboxEnabled = enabled; }

    QString getSkyboxBaseName() const { return mSkyboxBaseName; }
    void setSkyboxBaseName( const QString &baseName ) { mSkyboxBaseName = baseName; }

    QString getSkyboxExtension() const { return mSkyboxExt; }
    void setSkyboxExtension( const QString &extension ) { mSkyboxExt = extension; }

  private:
    bool mIsSkyboxEnabled = false;
    QString mSkyboxBaseName;
    QString mSkyboxExt;
};

#endif // QGSSKYBOXSETTINGS_H
