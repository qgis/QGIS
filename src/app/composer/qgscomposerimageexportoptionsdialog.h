/***************************************************************************
                         qgscomposerimageexportoptionsdialog.h
                         -------------------------------------
    begin                : September 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#ifndef QGSCOMPOSERIMAGEEXPORTOPTIONSDIALOG_H
#define QGSCOMPOSERIMAGEEXPORTOPTIONSDIALOG_H

#include <QDialog>
#include "ui_qgscomposerimageexportoptions.h"
#include "qgscomposertablev2.h"


/** A dialog for customising the properties of an exported image file.
 * /note added in QGIS 2.12
*/
class QgsComposerImageExportOptionsDialog: public QDialog, private Ui::QgsComposerImageExportOptionsDialog
{
    Q_OBJECT

  public:

    /** Constructor for QgsComposerImageExportOptionsDialog
     * @param parent parent widget
     * @param flags window flags
     */
    QgsComposerImageExportOptionsDialog( QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr );

    ~QgsComposerImageExportOptionsDialog();

    /** Sets the initial resolution displayed in the dialog.
     * @param resolution default resolution in DPI
     * @see resolution()
     */
    void setResolution( int resolution );

    /** Returns the selected resolution from the dialog.
     * @returns image resolution in DPI
     * @see setResolution()
     */
    int resolution() const;

    /** Sets the target image size. This is used to calculate the default size in pixels
     * and also for determining the image's width to height ratio.
     * @param size image size
     */
    void setImageSize( QSizeF size );

    /** Returns the user-set image width in pixels.
     * @see imageHeight
     */
    int imageWidth() const;

    /** Returns the user-set image height in pixels.
     * @see imageWidth
     */
    int imageHeight() const;

    /** Sets whether the crop to contents option should be checked in the dialog
     * @param crop set to true to check crop to contents
     * @see cropToContents()
     */
    void setCropToContents( bool crop );

    /** Returns whether the crop to contents option is checked in the dialog.
     * @see setCropToContents()
     */
    bool cropToContents() const;

    /** Fetches the current crop to contents margin values, in pixels.
     * @param topMargin destination for top margin
     * @param rightMargin destination for right margin
     * @param bottomMargin destination for bottom margin
     * @param leftMargin destination for left margin
     */
    void getCropMargins( int& topMargin, int& rightMargin, int& bottomMargin, int& leftMargin ) const;

    /** Sets the current crop to contents margin values, in pixels.
     * @param topMargin top margin
     * @param rightMargin right margin
     * @param bottomMargin bottom margin
     * @param leftMargin left margin
     */
    void setCropMargins( int topMargin, int rightMargin, int bottomMargin, int leftMargin );

  private slots:

    void on_mWidthSpinBox_valueChanged( int value );
    void on_mHeightSpinBox_valueChanged( int value );
    void on_mResolutionSpinBox_valueChanged( int value );
    void clipToContentsToggled( bool state );

  private:

    QSizeF mImageSize;


};

#endif // QGSCOMPOSERIMAGEEXPORTOPTIONSDIALOG_H
