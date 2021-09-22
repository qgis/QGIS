/***************************************************************************
  qgsdecorationtitledialog.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationtitledialog.h"
#include "qgsdecorationtitle.h"

#include "qgisapp.h"
#include "qgsexpression.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontext.h"
#include "qgshelp.h"
#include "qgsmapcanvas.h"
#include "qgsgui.h"

#include <QColorDialog>
#include <QColor>
#include <QFont>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationTitleDialog::QgsDecorationTitleDialog( QgsDecorationTitle &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationTitleDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationTitleDialog::buttonBox_rejected );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsDecorationTitleDialog::mInsertExpressionButton_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationTitleDialog::showHelp );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationTitleDialog::apply );

  grpEnable->setChecked( mDeco.enabled() );

  // label text
  txtTitleText->setAcceptRichText( false );
  if ( !mDeco.enabled() && mDeco.mLabelText.isEmpty() )
  {
    const QString defaultString = QgsProject::instance()->metadata().title();
    txtTitleText->setPlainText( defaultString );
  }
  else
  {
    txtTitleText->setPlainText( mDeco.mLabelText );
  }

  // background bar color
  pbnBackgroundColor->setAllowOpacity( true );
  pbnBackgroundColor->setColor( mDeco.mBackgroundColor );
  pbnBackgroundColor->setContext( QStringLiteral( "gui" ) );
  pbnBackgroundColor->setColorDialogTitle( tr( "Select Background Bar Color" ) );

  // placement
  cboPlacement->addItem( tr( "Top Left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top Center" ), QgsDecorationItem::TopCenter );
  cboPlacement->addItem( tr( "Top Right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom Left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom Center" ), QgsDecorationItem::BottomCenter );
  cboPlacement->addItem( tr( "Bottom Right" ), QgsDecorationItem::BottomRight );
  connect( cboPlacement, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    spnHorizontal->setMinimum( cboPlacement->currentData() == QgsDecorationItem::TopCenter || cboPlacement->currentData() == QgsDecorationItem::BottomCenter ? -100 : 0 );
  } );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );

  spnHorizontal->setClearValue( 0 );
  spnHorizontal->setValue( mDeco.mMarginHorizontal );
  spnVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPercentage << QgsUnitTypes::RenderPixels );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  // font settings
  mButtonFontStyle->setDialogTitle( tr( "Title Label Text Format" ) );
  mButtonFontStyle->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mButtonFontStyle->setTextFormat( mDeco.textFormat() );
}

void QgsDecorationTitleDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationTitleDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationTitleDialog::mInsertExpressionButton_clicked()
{
  QString selText = txtTitleText->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  selText = selText.replace( QChar( 0x2029 ), QChar( '\n' ) );

  QgsExpressionBuilderDialog exprDlg( nullptr, selText, this, QStringLiteral( "generic" ), QgisApp::instance()->mapCanvas()->mapSettings().expressionContext() );

  exprDlg.setWindowTitle( QObject::tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    const QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      txtTitleText->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

void QgsDecorationTitleDialog::apply()
{
  mDeco.setTextFormat( mButtonFontStyle->textFormat() );
  mDeco.mLabelText = txtTitleText->toPlainText();
  mDeco.mBackgroundColor = pbnBackgroundColor->color();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.update();
}

void QgsDecorationTitleDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#title_label_decoration" ) );
}
