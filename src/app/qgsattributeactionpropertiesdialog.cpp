/***************************************************************************
  qgsattributeactionpropertiesdialog.cpp - QgsAttributeActionPropertiesDialog

 ---------------------
 begin                : 18.4.2016
 copyright            : (C) 2016 by mku
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeactionpropertiesdialog.h"
#include "qgsfieldexpressionwidget.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QFileDialog>
#include <QImageWriter>

QgsAttributeActionPropertiesDialog::QgsAttributeActionPropertiesDialog( QgsAction::ActionType type, const QString& description, const QString& shortTitle, const QString& iconPath, const QString& actionText, bool capture, bool showInAttributeTable, QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
{
  setupUi( this );

  mActionType->setCurrentIndex( type );
  mActionName->setText( description );
  mShortTitle->setText( shortTitle );
  mActionIcon->setText( iconPath );
  mIconPreview->setPixmap( QPixmap( iconPath ) );
  mActionText->setText( actionText );
  mCaptureOutput->setChecked( capture );
  mShowInAttributeTable->setChecked( showInAttributeTable );

  // display the expression builder
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  mFieldExpression->setLayer( mLayer );
  mFieldExpression->setGeomCalculator( myDa );

  connect( mBrowseButton, SIGNAL( clicked( bool ) ), this, SLOT( browse() ) );
  connect( mInsertFieldOrExpression, SIGNAL( clicked( bool ) ), this, SLOT( insertExpressionOrField() ) );
  connect( mActionName, SIGNAL( textChanged( QString ) ), this, SLOT( updateButtons() ) );
  connect( mActionText, SIGNAL( textChanged() ), this, SLOT( updateButtons() ) );
  connect( mChooseIconButton, SIGNAL( clicked( bool ) ), this, SLOT( chooseIcon() ) );

  updateButtons();
}

QgsAttributeActionPropertiesDialog::QgsAttributeActionPropertiesDialog( QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent )
    , mLayer( layer )
{
  setupUi( this );

  // display the expression builder
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapSettings().hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  mFieldExpression->setLayer( mLayer );
  mFieldExpression->setGeomCalculator( myDa );

  connect( mBrowseButton, SIGNAL( clicked( bool ) ), this, SLOT( browse() ) );
  connect( mInsertFieldOrExpression, SIGNAL( clicked( bool ) ), this, SLOT( insertExpressionOrField() ) );
  connect( mActionName, SIGNAL( textChanged( QString ) ), this, SLOT( updateButtons() ) );
  connect( mActionText, SIGNAL( textChanged() ), this, SLOT( updateButtons() ) );

  updateButtons();
}

QgsAction::ActionType QgsAttributeActionPropertiesDialog::type() const
{
  return static_cast<QgsAction::ActionType>( mActionType->currentIndex() );
}

QString QgsAttributeActionPropertiesDialog::description() const
{
  return mActionName->text();
}

QString QgsAttributeActionPropertiesDialog::shortTitle() const
{
  return mShortTitle->text();
}

QString QgsAttributeActionPropertiesDialog::iconPath() const
{
  return mActionIcon->text();
}

QString QgsAttributeActionPropertiesDialog::actionText() const
{
  return mActionText->text();
}

bool QgsAttributeActionPropertiesDialog::showInAttributeTable() const
{
  return mShowInAttributeTable->isChecked();
}

bool QgsAttributeActionPropertiesDialog::capture() const
{
  return mCaptureOutput->isChecked();
}

void QgsAttributeActionPropertiesDialog::browse()
{
  // Popup a file browser and place the results into the action widget
  QString action = QFileDialog::getOpenFileName(
                     this, tr( "Select an action", "File dialog window title" ), QDir::homePath() );

  if ( !action.isNull() )
    mActionText->insertText( action );
}

void QgsAttributeActionPropertiesDialog::insertExpressionOrField()
{
  QString selText = mActionText->selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
    selText = selText.mid( 2, selText.size() - 4 );

  mActionText->insertText( "[%" + mFieldExpression->currentField() + "%]" );
}

void QgsAttributeActionPropertiesDialog::chooseIcon()
{
  QList<QByteArray> list = QImageWriter::supportedImageFormats();
  QStringList formatList;
  Q_FOREACH ( const QByteArray& format, list )
    formatList << QString( "*.%1" ).arg( QString( format ) );

  QString filter = tr( "Images( %1 ); All( *.* )" ).arg( formatList.join( " " ) );
  QString icon = QFileDialog::getOpenFileName( this, tr( "Choose Icon..." ), mActionIcon->text(), filter );

  if ( !icon.isNull() )
  {
    mActionIcon->setText( icon );
    mIconPreview->setPixmap( QPixmap( icon ) );
  }
}

void QgsAttributeActionPropertiesDialog::updateButtons()
{
  if ( mActionName->text().isEmpty() || mActionText->text().isEmpty() )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  }
  else
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
}
