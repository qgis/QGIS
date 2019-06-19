/***************************************************************************
                         qgssymbolsavedialog.cpp
                         ---------------------------------------
    begin                : November 2016
    copyright            : (C) 2016 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstylesavedialog.h"

#include "qgis.h"
#include "qgsstyle.h"
#include "qgsgui.h"

#include <QLineEdit>
#include <QCheckBox>

QgsStyleSaveDialog::QgsStyleSaveDialog( QWidget *parent, QgsStyle::StyleEntity type )
  : QDialog( parent )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  switch ( type )
  {
    case QgsStyle::SymbolEntity:
      this->setWindowTitle( tr( "Save New Symbol" ) );
      break;

    case QgsStyle::ColorrampEntity:
      this->setWindowTitle( tr( "Save New Color Ramp" ) );
      break;

    case QgsStyle::TextFormatEntity:
      this->setWindowTitle( tr( "Save New Text Format" ) );
      break;

    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
      break;

  }
}

QString QgsStyleSaveDialog::name() const
{
  return mName->text();
}

QString QgsStyleSaveDialog::tags() const
{
  return mTags->text();
}

bool QgsStyleSaveDialog::isFavorite() const
{
  return mFavorite->isChecked();
}
