#include "qgscomposerhtmlwidget.h"
#include "qgscomposerhtml.h"
#include <QFileDialog>

QgsComposerHtmlWidget::QgsComposerHtmlWidget( QgsComposerHtml* html ): mHtml( html )
{
  setupUi( this );
  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
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
    //mHtml->beginCommand( tr( "Url changed" ) );
    mHtml->setUrl( QUrl( mUrlLineEdit->text() ) );
    mHtml->update();
    //mHtmlItem->endCommand();
  }
}

void QgsComposerHtmlWidget::on_mFileToolButton_clicked()
{
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), QString(), "HTML (*.html)" );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    //mHtmlItem->beginCommand( tr( "Url changed" ) );
    mHtml->setUrl( url );
    //mHtmlItem->endCommand();
    mUrlLineEdit->setText( url.toString() );
  }
}

void QgsComposerHtmlWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
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


