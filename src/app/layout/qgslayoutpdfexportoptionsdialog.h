/***************************************************************************
                         qgslayoutpdfexportoptionsdialog.h
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

#ifndef QGSLAYOUTPDFEXPORTOPTIONSDIALOG_H
#define QGSLAYOUTPDFEXPORTOPTIONSDIALOG_H

#include <QDialog>
#include "ui_qgspdfexportoptions.h"

#include "qgsrendercontext.h"

/**
 * A dialog for customizing the properties of an exported PDF file from a layout.
*/
class QgsLayoutPdfExportOptionsDialog: public QDialog, private Ui::QgsPdfExportOptionsDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutPdfExportOptionsDialog
     * \param parent parent widget
     * \param flags window flags
     */
    QgsLayoutPdfExportOptionsDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

    void setTextRenderFormat( QgsRenderContext::TextRenderFormat format );
    QgsRenderContext::TextRenderFormat textRenderFormat() const;
    void setForceVector( bool force );
    bool forceVector() const;
    void enableGeoreferencingOptions( bool enabled );
    void setGeoreferencingEnabled( bool enabled );
    bool georeferencingEnabled() const;
    void setMetadataEnabled( bool enabled );
    bool metadataEnabled() const;
    void setRasterTilingDisabled( bool disabled );
    bool rasterTilingDisabled() const;
    void setGeometriesSimplified( bool enabled );
    bool geometriesSimplified() const;

    void setExportGeoPdf( bool enabled );
    bool exportGeoPdf() const;
    void setUseOgcBestPracticeFormat( bool enabled );
    bool useOgcBestPracticeFormat() const;

    void setExportGeoPdfFeatures( bool enabled );
    bool exportGeoPdfFeatures() const;

    void setExportThemes( const QStringList &themes );
    QStringList exportThemes() const;

  private slots:

    void showHelp();

  private:

    bool mGeopdfAvailable = true;

};

#endif // QGSLAYOUTPDFEXPORTOPTIONSDIALOG_H
