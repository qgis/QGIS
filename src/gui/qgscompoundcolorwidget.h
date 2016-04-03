/***************************************************************************
    qgscompoundcolorwidget.h
    ------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOUNDCOLORWIDGET_H
#define QGSCOMPOUNDCOLORWIDGET_H

#include "qgisgui.h"
#include "ui_qgscompoundcolorwidget.h"

/** \ingroup gui
 * \class QgsCompoundColorWidget
 * A custom QGIS widget for selecting a color, including options for selecting colors via
 * hue wheel, color swatches, and a color sampler.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsCompoundColorWidget : public QWidget, private Ui::QgsCompoundColorWidgetBase
{

    Q_OBJECT

  public:

    /** Constructor for QgsCompoundColorWidget
     * @param parent parent widget
     * @param color initial color for dialog
     */
    QgsCompoundColorWidget( QWidget *parent = nullptr, const QColor& color = QColor() );

    ~QgsCompoundColorWidget();

    /** Returns the current color for the dialog
     * @returns dialog color
     */
    QColor color() const;

    /** Sets whether alpha modification (transparency) is permitted
     * for the color dialog. Defaults to true.
     * @param allowAlpha set to false to disable alpha modification
     */
    void setAllowAlpha( const bool allowAlpha );

  signals:

    /** Emitted when the dialog's color changes
     * @param color current color
     */
    void currentColorChanged( const QColor &color );

  public slots:

    /** Sets the current color for the dialog
     * @param color desired color
     */
    void setColor( const QColor &color );

    /** Sets the color to show in an optional "previous color" section
     * @param color previous color
     */
    void setPreviousColor( const QColor& color );

  protected:

    void mousePressEvent( QMouseEvent* e ) override;

    void mouseMoveEvent( QMouseEvent *e ) override;

    void mouseReleaseEvent( QMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

  private slots:

    void on_mHueRadio_toggled( bool checked );
    void on_mSaturationRadio_toggled( bool checked );
    void on_mValueRadio_toggled( bool checked );
    void on_mRedRadio_toggled( bool checked );
    void on_mGreenRadio_toggled( bool checked );
    void on_mBlueRadio_toggled( bool checked );

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

    bool mAllowAlpha;

    int mLastCustomColorIndex;

    bool mPickingColor;

    /** Saves all widget settings
     */
    void saveSettings();

    /** Ends a color picking operation
     * @param eventPos global position of pixel to sample color from
     * @param takeSample set to true to actually sample the color, false to just cancel
     * the color picking operation
     */
    void stopPicking( QPoint eventPos, const bool takeSample = true );

    /** Returns the average color from the pixels in an image
     * @param image image to sample
     * @returns average color from image
     */
    QColor averageColor( const QImage &image ) const;

    /** Samples a color from the desktop
     * @param point position of color to sample
     * @returns average color from sampled position
     */
    QColor sampleColor( QPoint point ) const;

    /** Repopulates the scheme combo box with current color schemes
     */
    void refreshSchemeComboBox();

    /** Returns the path to the user's palette folder
     */
    QString gplFilePath();
};

#endif // QGSCOMPOUNDCOLORWIDGET_H
