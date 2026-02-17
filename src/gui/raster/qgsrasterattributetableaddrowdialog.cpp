/***************************************************************************
  qgsrasterattributetableaddrowdialog.cpp - QgsRasterAttributeTableAddRowDialog

 ---------------------
 begin                : 18.10.2022
 copyright            : (C) 2022 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetableaddrowdialog.h"

#include "qgsgui.h"
#include "qgshelp.h"

#include <QString>

#include "moc_qgsrasterattributetableaddrowdialog.cpp"

using namespace Qt::StringLiterals;

QgsRasterAttributeTableAddRowDialog::QgsRasterAttributeTableAddRowDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [] {
    QgsHelp::openHelp( u"working_with_raster/raster_properties.html#raster-attribute-tables"_s );
  } );
  QgsGui::enableAutoGeometryRestore( this );
}

bool QgsRasterAttributeTableAddRowDialog::insertAfter() const
{
  return mAfter->isChecked();
}
