/***************************************************************************
                         qgslayoutpdfexportoptionsdialog.cpp
                         -------------------------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutpdfexportoptionsdialog.h"
#include "qgis.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgshelp.h"

#include <QCheckBox>
#include <QPushButton>

QgsLayoutPdfExportOptionsDialog::QgsLayoutPdfExportOptionsDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setupUi( this );

  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Paths (Recommended)" ), QgsRenderContext::TextFormatAlwaysOutlines );
  mTextRenderFormatComboBox->addItem( tr( "Always Export Text as Text Objects" ), QgsRenderContext::TextFormatAlwaysText );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsLayoutPdfExportOptionsDialog::showHelp );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsLayoutPdfExportOptionsDialog::setTextRenderFormat( QgsRenderContext::TextRenderFormat format )
{
  mTextRenderFormatComboBox->setCurrentIndex( mTextRenderFormatComboBox->findData( format ) );
}

QgsRenderContext::TextRenderFormat QgsLayoutPdfExportOptionsDialog::textRenderFormat() const
{
  return static_cast< QgsRenderContext::TextRenderFormat >( mTextRenderFormatComboBox->currentData().toInt() );
}

void QgsLayoutPdfExportOptionsDialog::setForceVector( bool force )
{
  mForceVectorCheckBox->setChecked( force );
}

bool QgsLayoutPdfExportOptionsDialog::forceVector() const
{
  return mForceVectorCheckBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::enableGeoreferencingOptions( bool enabled )
{
  mAppendGeoreferenceCheckbox->setEnabled( enabled );
}

void QgsLayoutPdfExportOptionsDialog::setGeoreferencingEnabled( bool enabled )
{
  mAppendGeoreferenceCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::georeferencingEnabled() const
{
  return mAppendGeoreferenceCheckbox->isChecked();;
}

void QgsLayoutPdfExportOptionsDialog::setMetadataEnabled( bool enabled )
{
  mIncludeMetadataCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::metadataEnabled() const
{
  return mIncludeMetadataCheckbox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setRasterTilingDisabled( bool disabled )
{
  mDisableRasterTilingCheckBox->setChecked( disabled );
}

bool QgsLayoutPdfExportOptionsDialog::rasterTilingDisabled() const
{
  return mDisableRasterTilingCheckBox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::setGeometriesSimplified( bool enabled )
{
  mSimplifyGeometriesCheckbox->setChecked( enabled );
}

bool QgsLayoutPdfExportOptionsDialog::geometriesSimplified() const
{
  return mSimplifyGeometriesCheckbox->isChecked();
}

void QgsLayoutPdfExportOptionsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "print_composer/create_output.html" ) );
}
