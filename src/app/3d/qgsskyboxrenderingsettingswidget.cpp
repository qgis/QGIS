#include "qgsskyboxrenderingsettingswidget.h"

#include <QCheckBox>
#include <QLineEdit>
#include "qgs3dmapsettings.h"

QgsSkyboxRenderingSettingsWidget::QgsSkyboxRenderingSettingsWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  QObject::connect( skyboxEnabledCheckBox, &QCheckBox::stateChanged, [&]( int state ) -> void
  {
    mIsSkyboxEnabled = state == 1;
    emit skyboxSettingsChanged( toSkyboxSettings() );
  } );
  QObject::connect( skyboxPrefixLineEdit, &QLineEdit::textChanged, [&]( const QString & newPrefix ) -> void
  {
    mSkyboxBaseName = newPrefix;
    emit skyboxSettingsChanged( toSkyboxSettings() );
  } );
  connect( skyboxExtensionLineEdit, &QLineEdit::textChanged, [&]( const QString & newExt ) -> void
  {
    mSkyboxExtension = newExt;
    emit skyboxSettingsChanged( toSkyboxSettings() );
  } );
}
