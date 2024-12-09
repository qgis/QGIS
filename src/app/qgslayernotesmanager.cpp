/***************************************************************************
  qgslayernotesmanager.cpp
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayernotesmanager.h"
#include "moc_qgslayernotesmanager.cpp"
#include "qgslayernotesutils.h"
#include "qgsmaplayer.h"
#include "qgsrichtexteditor.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include <QDialogButtonBox>
#include <QPushButton>

void QgsLayerNotesManager::editLayerNotes( QgsMapLayer *layer, QWidget *parent )
{
  const QString notes = QgsLayerNotesUtils::layerNotes( layer );
  QgsLayerNotesDialog *editor = new QgsLayerNotesDialog( parent );
  editor->setNotes( notes );
  editor->setWindowTitle( QObject::tr( "Layer Notes — %1" ).arg( layer->name() ) );
  if ( editor->exec() )
  {
    QgsLayerNotesUtils::setLayerNotes( layer, editor->notes() );
    QgsProject::instance()->setDirty( true );
  }
}

//
// QgsLayerNotesDialog
//
QgsLayerNotesDialog::QgsLayerNotesDialog( QWidget *parent )
  : QDialog( parent, Qt::Tool )
{
  QVBoxLayout *layout = new QVBoxLayout();
  mEditor = new QgsRichTextEditor();
  layout->addWidget( mEditor );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Save | QDialogButtonBox::Help | QDialogButtonBox::Cancel );
  connect( buttonBox->button( QDialogButtonBox::Save ), &QPushButton::clicked, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#layer-notes" ) );
  } );
  layout->addWidget( buttonBox );

  layout->setContentsMargins( 3, 0, 3, 3 );
  setLayout( layout );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLayerNotesDialog::setNotes( const QString &notes )
{
  mEditor->setText( notes );
}

QString QgsLayerNotesDialog::notes() const
{
  return mEditor->toHtml();
}
