#include "qgsrasterformatoptionswidget.h"
#include "qgslogger.h"

#include <QSettings>
#include <QInputDialog>

QgsRasterFormatOptionsWidget::QgsRasterFormatOptionsWidget( QWidget* parent, QString format, QString provider ): QWidget( parent ), mFormat( format ), mProvider( provider )

{
  setupUi( this );
  showProfileButtons( false );
  connect( mProfileComboBox, SIGNAL( currentIndexChanged ( const QString & ) ),
           this, SLOT( profileChanged() ) );
  connect( mProfileApplyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );
  update();
}

QgsRasterFormatOptionsWidget::~QgsRasterFormatOptionsWidget()
{
}


void QgsRasterFormatOptionsWidget::showProfileButtons( bool show )
{
  mProfileButtons->setVisible( show );
}

void QgsRasterFormatOptionsWidget::update()
{
  // mCreateOptionsLineEdit->setText( createOption( mProfileComboBox->currentText() ) );  
  mProfileComboBox->blockSignals( true );
  mProfileComboBox->clear();
  foreach ( QString profileName, profiles() )
  {
    mProfileComboBox->addItem( profileName );
  }
  mProfileComboBox->blockSignals( false );
  mProfileComboBox->setCurrentIndex( 0 );
  profileChanged();
}

void QgsRasterFormatOptionsWidget::apply()
{
  setProfiles();
  setCreateOptions( mProfileComboBox->currentText(), mCreateOptionsLineEdit->text() );
  profileChanged();
}

// void QgsRasterFormatOptionsWidget::on_mProfileComboBox_currentIndexChanged( const QString & text )
void QgsRasterFormatOptionsWidget::profileChanged()
{
  QgsDebugMsg("Entered");
  mCreateOptionsLineEdit->setText( createOptions( mProfileComboBox->currentText() ) );
}

void QgsRasterFormatOptionsWidget::on_mProfileNewButton_clicked()
{
  QString profileName = QInputDialog::getText( this, "", tr("Profile name:") );
  if ( ! profileName.isEmpty() )
  {
    profileName = profileName.trimmed();
    profileName.replace( " ", "_" );
    profileName.replace( "=", "_" );
    mProfileComboBox->addItem( profileName );
    mProfileComboBox->setCurrentIndex( mProfileComboBox->count()-1 );
  }
}

void QgsRasterFormatOptionsWidget::on_mProfileDeleteButton_clicked()
{
  int index = mProfileComboBox->currentIndex();
  QString profileName = mProfileComboBox->currentText();
  if ( index != -1 )
  {
    mProfileComboBox->removeItem( index );
    setCreateOptions( profileName, "" );
  }
}

QString QgsRasterFormatOptionsWidget::settingsKey( QString profileName ) const
{ 
  if ( profileName != "" )
    profileName = "/profile_" + profileName;
  else
    profileName = "/profile_default" + profileName;
  return mProvider + "/driverOptions/" + mFormat.toLower() + profileName + "/create"; 
}

QStringList QgsRasterFormatOptionsWidget::createOptions() const
{
  return mCreateOptionsLineEdit->text().trimmed().split( " " ) ;
}

QString QgsRasterFormatOptionsWidget::createOptions( QString profileName ) const
{
  QSettings mySettings;
  return mySettings.value( settingsKey( profileName ), "" ).toString();
}

void QgsRasterFormatOptionsWidget::deleteCreateOptions( QString profileName )
{
  QSettings mySettings;
  mySettings.remove( settingsKey( profileName ) );
}

void QgsRasterFormatOptionsWidget::setCreateOptions( QString profileName, QString value )
{
  QSettings mySettings;
  mySettings.setValue( settingsKey( profileName ), value.trimmed() );
}

QStringList QgsRasterFormatOptionsWidget::profiles() const
{
  QSettings mySettings;
  return mySettings.value( mProvider + "/driverOptions/" + mFormat.toLower() + "/profiles", "" ).toString().split( " " );
}

void QgsRasterFormatOptionsWidget::setProfiles() const
{
  QSettings mySettings;
  QString myProfiles;
  for ( int i = 0 ; i < mProfileComboBox->count() ; i++ )
    myProfiles += mProfileComboBox->itemText( i ) + QString( " " );
  return mySettings.setValue( mProvider + "/driverOptions/" + mFormat.toLower() + "/profiles", myProfiles.trimmed() );
}
