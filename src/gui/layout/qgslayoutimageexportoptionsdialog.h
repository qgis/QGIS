/***************************************************************************
                         qgslayoutimageexportoptionsdialog.h
                         -------------------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTIMAGEEXPORTOPTIONSDIALOG_H
#define QGSLAYOUTIMAGEEXPORTOPTIONSDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>
#include "qgis_gui.h"
#include "ui_qgslayoutimageexportoptions.h"


/**
 * \ingroup gui
 * \brief A dialog for customizing the properties of an exported image file.
 *
 * \note This class is not a part of public API
*/
class GUI_EXPORT QgsLayoutImageExportOptionsDialog : public QDialog, private Ui::QgsLayoutImageExportOptionsDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsLayoutImageExportOptionsDialog
     * \param parent parent widget
     * \param fileExtension output image file extension
     * \param flags window flags
     */
    QgsLayoutImageExportOptionsDialog( QWidget *parent = nullptr, const QString &fileExtension = QString(), Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Sets the initial resolution displayed in the dialog.
     * \param resolution default resolution in DPI
     * \see resolution()
     */
    void setResolution( double resolution );

    /**
     * Returns the selected resolution from the dialog.
     * \returns image resolution in DPI
     * \see setResolution()
     */
    double resolution() const;

    /**
     * Sets the target image size. This is used to calculate the default size in pixels
     * and also for determining the image's width to height ratio.
     * \param size image size
     */
    void setImageSize( QSizeF size );

    /**
     * Returns the user-set image width in pixels.
     * \see imageHeight
     */
    int imageWidth() const;

    /**
     * Returns the user-set image height in pixels.
     * \see imageWidth
     */
    int imageHeight() const;

    /**
     * Sets whether the crop to contents option should be checked in the dialog
     * \param crop set to TRUE to check crop to contents
     * \see cropToContents()
     */
    void setCropToContents( bool crop );

    /**
     * Returns whether the crop to contents option is checked in the dialog.
     * \see setCropToContents()
     */
    bool cropToContents() const;

    /**
     * Sets whether the generate world file option should be checked.
     * \see generateWorldFile()
     */
    void setGenerateWorldFile( bool generate );

    /**
     * Returns whether the generate world file option is checked in the dialog.
     * \see setGenerateWorldFile()
     */
    bool generateWorldFile() const;

    /**
     * Sets whether antialiasing should be used in the export.
     * \see antialiasing()
     */
    void setAntialiasing( bool antialias );

    /**
     * Returns whether antialiasing should be used in the export.
     * \see setAntialiasing()
     */
    bool antialiasing() const;

    /**
     * Fetches the current crop to contents margin values, in pixels.
     * \param topMargin destination for top margin
     * \param rightMargin destination for right margin
     * \param bottomMargin destination for bottom margin
     * \param leftMargin destination for left margin
     */
    void getCropMargins( int &topMargin, int &rightMargin, int &bottomMargin, int &leftMargin ) const;

    /**
     * Sets the current crop to contents margin values, in pixels.
     * \param topMargin top margin
     * \param rightMargin right margin
     * \param bottomMargin bottom margin
     * \param leftMargin left margin
     */
    void setCropMargins( int topMargin, int rightMargin, int bottomMargin, int leftMargin );

    //! Sets whether to open the pdf after exporting it
    void setOpenAfterExporting( bool enabled );
    //! Returns whether the pdf should be opened after exporting it
    bool openAfterExporting() const;


    //! Sets the image quality (for JPEG)
    void setQuality( int quality );
    //! Returns the image quality
    int quality() const;

  private slots:

    void mWidthSpinBox_valueChanged( int value );
    void mHeightSpinBox_valueChanged( int value );
    void mResolutionSpinBox_valueChanged( int value );
    void clipToContentsToggled( bool state );
    void showHelp();

  private:
    bool shouldShowQuality() const;
    QSizeF mImageSize;
    QString mFileExtension;
};

#endif // QGSLAYOUTIMAGEEXPORTOPTIONSDIALOG_H
