/***************************************************************************
    qgscolordialog.h - color selection dialog

    ---------------------
    begin                : March 19, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORDIALOG_H
#define QGSCOLORDIALOG_H

#include <QColorDialog>
#include "qgisgui.h"
#include "ui_qgscolordialog.h"

class QColor;

/** \ingroup gui
 * \class QgsColorDialog
 * A native operating system dialog for selecting a color
 */

class GUI_EXPORT QgsColorDialog : public QObject
{
    Q_OBJECT

  public:
    QgsColorDialog();
    ~QgsColorDialog();

    /** Return a color selection from a QColorDialog, with live updating of interim selections.
     * @param initialColor The initial color of the selection dialog.
     * @param updateObject The receiver object of the live updating.
     * @param updateSlot The receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent Parent widget. Usually 0 is best for native system color dialogs.
     * @param title The title of the QColorDialog.
     * @param options ColorDialogOptions passed to QColorDialog.
     * @return Selected color on accepted() or initialColor on rejected().
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = "",
                                QColorDialog::ColorDialogOptions options = 0 );
};


/** \ingroup gui
 * \class QgsColorDialogV2
 * A custom QGIS dialog for selecting a color. Has many improvements over the standard Qt color picker dialog, including
 * hue wheel supports, color swatches, and a color sampler.
 * \note Added in version 2.5
 */

class GUI_EXPORT QgsColorDialogV2 : public QDialog, private Ui::QgsColorDialogBase
{

    Q_OBJECT

  public:

    /**Create a new color picker dialog
     * @param parent parent widget
     * @param fl window flags
     * @param color initial color for dialog
     */
    QgsColorDialogV2( QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags,
                      const QColor& color = QColor() );

    ~QgsColorDialogV2();

    /**Returns the current color for the dialog
     * @returns dialog color
     */
    QColor color() const;

    /**Sets the title for the color dialog
     * @param title title for dialog box
     */
    void setTitle( const QString title );

    /**Sets whether alpha modification (transparency) is permitted
     * for the color dialog. Defaults to true.
     * @param allowAlpha set to false to disable alpha modification
     */
    void setAllowAlpha( const bool allowAlpha );

    /**Return a color selection from a color dialog, with live updating of interim selections.
     * @param initialColor the initial color of the selection dialog.
     * @param updateObject the receiver object of the live updating.
     * @param updateSlot the receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent parent widget
     * @param title the title of the dialog.
     * @param allowAlpha set to true to allow modification of color alpha value (transparency)
     * @return Selected color on accepted() or initialColor on rejected().
     * @see getColor
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = QString(),
                                const bool allowAlpha = true );

    /**Return a color selection from a color dialog.
     * @param initialColor the initial color of the selection dialog.
     * @param parent parent widget
     * @param title the title of the dialog.
     * @param allowAlpha set to true to allow modification of color alpha value (transparency)
     * @return Selected color on accepted() or initialColor on rejected().
     * @see getLiveColor
     */
    static QColor getColor( const QColor &initialColor, QWidget *parent, const QString &title = QString(),
                            const bool allowAlpha = false );

  signals:

    /**Emitted when the dialog's color changes
     * @param color current color
     */
    void currentColorChanged( const QColor &color );

  public slots:

    /**Sets the current color for the dialog
     * @param color desired color
     */
    void setColor( const QColor &color );

  protected:

    void closeEvent( QCloseEvent* e );

    void mousePressEvent( QMouseEvent* e );

    void mouseMoveEvent( QMouseEvent *e );

    void mouseReleaseEvent( QMouseEvent *e );

    void keyPressEvent( QKeyEvent *e );

  private slots:

    void on_mHueRadio_toggled( bool checked );
    void on_mSaturationRadio_toggled( bool checked );
    void on_mValueRadio_toggled( bool checked );
    void on_mRedRadio_toggled( bool checked );
    void on_mGreenRadio_toggled( bool checked );
    void on_mBlueRadio_toggled( bool checked );

    void on_mButtonBox_accepted();
    void on_mButtonBox_rejected();
    void on_mButtonBox_clicked( QAbstractButton * button );
    void on_mAddColorToSchemeButton_clicked();

    void exportColors();
    void importColors();
    void importPalette();
    void removePalette();
    void newPalette();

    void schemeIndexChanged( int index );
    void listSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    void on_mAddCustomColorButton_clicked();

    void on_mSampleButton_clicked();
    void on_mTabWidget_currentChanged( int index );

  private:

    QColor mPreviousColor;

    bool mAllowAlpha;

    int mLastCustomColorIndex;

    bool mPickingColor;

    /**Saves all dialog and widget settings
     */
    void saveSettings();

    /**Ends a color picking operation
     * @param eventPos global position of pixel to sample color from
     * @param takeSample set to true to actually sample the color, false to just cancel
     * the color picking operation
     */
    void stopPicking( const QPoint& eventPos, const bool takeSample = true );

    /**Returns the average color from the pixels in an image
     * @param image image to sample
     * @returns average color from image
     */
    QColor averageColor( const QImage &image ) const;

    /**Samples a color from the desktop
     * @param point position of color to sample
     * @returns average color from sampled position
     */
    QColor sampleColor( const QPoint &point ) const;

    /**Repopulates the scheme combo box with current color schemes
     */
    void refreshSchemeComboBox();

    /**Returns the path to the user's palette folder
     */
    QString gplFilePath();
};

#endif // #ifndef QGSCOLORDIALOG_H
