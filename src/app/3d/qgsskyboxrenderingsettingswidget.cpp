#include "qgsskyboxrenderingsettingswidget.h"

#include <QCheckBox>
#include <QLineEdit>
#include "qgs3dmapsettings.h"

QgsSkyboxRenderingSettingsWidget::QgsSkyboxRenderingSettingsWidget( Qgs3DMapSettings *map, QWidget *parent )
  : QWidget( parent ), mMapSettings( map )
{
  setupUi( this );

  QObject::connect( skyboxEnabledCheckBox, &QCheckBox::stateChanged, [&]( int state ) -> void
  {
    mIsSkyboxEnabled = state == 1;
    emit skyboxSettingsChanged();
  } );
  QObject::connect( skyboxPrefixLineEdit, &QLineEdit::textChanged, [&]( const QString & newPrefix ) -> void
  {
    mSkyboxPrefix = newPrefix;
    emit skyboxSettingsChanged();
  } );
  connect( skyboxExtensionLineEdit, &QLineEdit::textChanged, [&]( const QString & newExt ) -> void
  {
    mSkyboxExt = newExt;
    emit skyboxSettingsChanged();
  } );
}
