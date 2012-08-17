#include "qgscomposerhtmlwidget.h"
#include "qgscomposerframe.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermultiframecommand.h"
#include "qgscomposerhtml.h"
#include <QFileDialog>
#include <QSettings>

QgsComposerHtmlWidget::QgsComposerHtmlWidget( QgsComposerHtml* html, QgsComposerFrame* frame ): mHtml( html ), mFrame( frame )
{
  setupUi( this );

  blockSignals( true );
  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat on every page" ), QgsComposerMultiFrame::RepeatOnEveryPage );
  mResizeModeComboBox->addItem( tr( "Repeat until finished" ), QgsComposerMultiFrame::RepeatUntilFinished );
  blockSignals( false );
  setGuiElementValues();

  if ( mHtml )
  {
    QObject::connect( mHtml, SIGNAL( changed() ), this, SLOT( setGuiElementValues() ) );
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
    mToolBox->addItem( itemPropertiesWidget, tr( "General options" ) );
  }
}

QgsComposerHtmlWidget::QgsComposerHtmlWidget()
{
}

QgsComposerHtmlWidget::~QgsComposerHtmlWidget()
{
}

void QgsComposerHtmlWidget::blockSignals( bool block )
{
  mUrlLineEdit->blockSignals( block );
  mFileToolButton->blockSignals( block );
  mResizeModeComboBox->blockSignals( block );
}

void QgsComposerHtmlWidget::on_mUrlLineEdit_editingFinished()
{
  if ( mHtml )
  {
    QUrl newUrl( mUrlLineEdit->text() );
    if ( newUrl == mHtml->url() )
    {
      return;
    }

    QgsComposition* composition = mHtml->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mHtml, tr( "Change html url" ) );
      mHtml->setUrl( newUrl );
      mHtml->update();
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerHtmlWidget::on_mFileToolButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/UI/lastHtmlDir", "" ).toString();
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), lastDir, "HTML (*.html)" );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    mUrlLineEdit->setText( url.toString() );
    on_mUrlLineEdit_editingFinished();
    mHtml->update();
    s.setValue( "/UI/lastHtmlDir", QFileInfo( file ).absolutePath() );
  }
}

void QgsComposerHtmlWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mHtml, tr( "Change resize mode" ) );
    mHtml->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }
}

void QgsComposerHtmlWidget::setGuiElementValues()
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mUrlLineEdit->setText( mHtml->url().toString() );
  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mHtml->resizeMode() ) );
  blockSignals( false );
}


