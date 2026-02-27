/***************************************************************************
  qgs3dsymbolbutton.cpp
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsymbolbutton.h"

#include "qgs3dsymbolutils.h"
#include "qgsvectorlayer3drendererwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QString>
#include <QVBoxLayout>

#include "moc_qgs3dsymbolbutton.cpp"

using namespace Qt::StringLiterals;

Qgs3DSymbolButton::Qgs3DSymbolButton( QWidget *parent )
  : QToolButton( parent )
{
  connect( this, &QAbstractButton::clicked, this, &Qgs3DSymbolButton::showSettingsDialog );
}

void Qgs3DSymbolButton::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void Qgs3DSymbolButton::setDialogTitle( const QString &title )
{
  mDialogTitle = title;
}

void Qgs3DSymbolButton::setSymbol( std::unique_ptr<QgsAbstract3DSymbol> symbol )
{
  mSymbol = std::move( symbol );

  const int fontHeight = static_cast<int>( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.4 );
  const QSize size = QToolButton::minimumSizeHint();

  int symbolBtnHeight;
  if ( mSymbol && mSymbol->type() == "point"_L1 )
  {
    symbolBtnHeight = std::max( size.height(), 3 * fontHeight );
    setMaximumWidth( static_cast<int>( 1.5 * symbolBtnHeight ) );
    setMinimumWidth( maximumWidth() );
  }
  else
  {
    symbolBtnHeight = fontHeight;
    setMaximumWidth( 999999 );
  }

  setMinimumHeight( symbolBtnHeight );
  updatePreview();
}

QgsAbstract3DSymbol *Qgs3DSymbolButton::symbol() const
{
  return mSymbol.get();
}

void Qgs3DSymbolButton::resizeEvent( QResizeEvent *event )
{
  QToolButton::resizeEvent( event );
  updatePreview();
}

void Qgs3DSymbolButton::updatePreview()
{
  if ( !mSymbol )
  {
    return;
  }

#ifdef Q_OS_WIN
  QSize iconSize = QSize( width() - 10, height() - 6 );
#else
  QSize iconSize = QSize( width() - 10, height() - 12 );
#endif

  const QIcon symbolIcon = Qgs3DSymbolUtils::vectorSymbolPreviewIcon( mSymbol.get(), iconSize, QgsScreenProperties( screen() ), 0 );
  setIcon( symbolIcon );
  setIconSize( iconSize );
}

void Qgs3DSymbolButton::updateSymbolFromWidget( QgsSingleSymbol3DRendererWidget *widget )
{
  setSymbol( widget->symbol() );
  emit changed();
}

void Qgs3DSymbolButton::showSettingsDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  QgsSingleSymbol3DRendererWidget *widget = new QgsSingleSymbol3DRendererWidget( mLayer, this );
  widget->setSymbol( mSymbol.get() );
  widget->setPanelTitle( mDialogTitle );
  if ( panel && panel->dockMode() )
  {
    widget->setDockMode( true );
    connect( widget, &QgsPanelWidget::widgetChanged, this, [this, widget] { updateSymbolFromWidget( widget ); } );
    panel->openPanel( widget );
  }
  else
  {
    widget->setDockMode( false );

    // Show the dialog version if not in a panel
    QDialog *dialog = new QDialog( this );

    QgsSettings settings;
    const QString key = u"/UI/paneldialog/%1"_s.arg( widget->panelTitle() );
    dialog->restoreGeometry( settings.value( key ).toByteArray() );

    dialog->setWindowTitle( widget->panelTitle() );
    dialog->setLayout( new QVBoxLayout() );

    dialog->layout()->addWidget( widget );

    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
    connect( buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject );
    dialog->layout()->addWidget( buttonBox );

    if ( dialog->exec() == QDialog::Accepted )
    {
      updateSymbolFromWidget( widget );
    }
    settings.setValue( key, dialog->saveGeometry() );
  }
}
