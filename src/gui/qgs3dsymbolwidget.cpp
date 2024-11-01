/***************************************************************************
  qgs3dsymbolwidget.cpp
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsymbolwidget.h"
#include "moc_qgs3dsymbolwidget.cpp"
#include "qgsapplication.h"
#include "qgs3dsymbolregistry.h"
#include "qgsabstract3dsymbol.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>

Qgs3DSymbolWidget::Qgs3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
{}

Qgs3DSymbolDialog::Qgs3DSymbolDialog( const QgsAbstract3DSymbol *symbol, QWidget *parent )
  : QDialog( parent )
{
  Q_ASSERT( symbol );

  QgsGui::enableAutoGeometryRestore( this );

  QVBoxLayout *vLayout = new QVBoxLayout();

  if ( Qgs3DSymbolAbstractMetadata *metadata = QgsApplication::symbol3DRegistry()->symbolMetadata( symbol->type() ) )
  {
    mWidget = metadata->createSymbolWidget( nullptr );
    vLayout->addWidget( mWidget );
    mWidget->setSymbol( symbol, nullptr );
  }

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "style_library/3d_symbols.html" ) );
  } );
  vLayout->addStretch();
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "3D Symbol" ) );
}

QgsAbstract3DSymbol *Qgs3DSymbolDialog::symbol() const
{
  return mWidget ? mWidget->symbol() : nullptr;
}

QDialogButtonBox *Qgs3DSymbolDialog::buttonBox() const
{
  return mButtonBox;
}
