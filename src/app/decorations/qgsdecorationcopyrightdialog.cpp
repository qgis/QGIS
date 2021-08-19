/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qgsdecorationcopyrightdialog.h"
#include "qgsdecorationcopyright.h"

#include "qgisapp.h"
#include "qgsexpression.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontext.h"
#include "qgshelp.h"
#include "qgsmapcanvas.h"
#include "qgsgui.h"

//qt includes
#include <QColorDialog>
#include <QColor>
#include <QFont>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationCopyrightDialog::QgsDecorationCopyrightDialog( QgsDecorationCopyright &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationCopyrightDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationCopyrightDialog::buttonBox_rejected );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsDecorationCopyrightDialog::mInsertExpressionButton_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationCopyrightDialog::showHelp );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationCopyrightDialog::apply );

  grpEnable->setChecked( mDeco.enabled() );

  // label text
  txtCopyrightText->setAcceptRichText( false );
  if ( !mDeco.enabled() && mDeco.mLabelText.isEmpty() )
  {
    const QDate now = QDate::currentDate();
    const QString defaultString = QStringLiteral( "%1 %2 %3" ).arg( QChar( 0x00A9 ), QgsProject::instance()->metadata().author(), now.toString( QStringLiteral( "yyyy" ) ) );
    txtCopyrightText->setPlainText( defaultString );
  }
  else
  {
    txtCopyrightText->setPlainText( mDeco.mLabelText );
  }

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
  mButtonFontStyle->setDialogTitle( tr( "Copyright Label Text Format" ) );
  mButtonFontStyle->setMapCanvas( QgisApp::instance()->mapCanvas() );
  mButtonFontStyle->setTextFormat( mDeco.textFormat() );
}

void QgsDecorationCopyrightDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationCopyrightDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationCopyrightDialog::mInsertExpressionButton_clicked()
{
  QString selText = txtCopyrightText->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  QgsExpressionBuilderDialog exprDlg( nullptr, selText, this, QStringLiteral( "generic" ), QgisApp::instance()->mapCanvas()->mapSettings().expressionContext() );

  exprDlg.setWindowTitle( QObject::tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    const QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      txtCopyrightText->insertPlainText( "[%" + expression + "%]" );
    }
  }
}

void QgsDecorationCopyrightDialog::apply()
{
  mDeco.setTextFormat( mButtonFontStyle->textFormat() );
  mDeco.mLabelText = txtCopyrightText->toPlainText();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.mMarginHorizontal = spnHorizontal->value();
  mDeco.mMarginVertical = spnVertical->value();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.update();
}

void QgsDecorationCopyrightDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#copyright_decoration" ) );
}
