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

#include "qgis_sip.h"
#include "qgspanelwidget.h"
#include "ui_qgscompoundcolorwidget.h"
#include "qgis_gui.h"

class QgsScreenHelper;

/**
 * \ingroup gui
 * \class QgsCompoundColorWidget
 * \brief A custom QGIS widget for selecting a color, including options for selecting colors via
 * hue wheel, color swatches, and a color sampler.
 * \since QGIS 2.16
 */

class GUI_EXPORT QgsCompoundColorWidget : public QgsPanelWidget, private Ui::QgsCompoundColorWidgetBase
{

    Q_OBJECT

  public:

    //! Widget layout
    enum Layout
    {
      LayoutDefault = 0, //!< Use the default (rectangular) layout
      LayoutVertical, //!< Use a narrower, vertically stacked layout
    };

    /**
     * Constructor for QgsCompoundColorWidget
     * \param parent parent widget
     * \param color initial color for dialog
     * \param layout widget layout to use
     */
    QgsCompoundColorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QColor &color = QColor(), Layout layout = LayoutDefault );

    ~QgsCompoundColorWidget() override;

    /**
     * Returns the current color for the dialog
     * \returns dialog color
     */
    QColor color() const;

    /**
     * Sets whether opacity modification (transparency) is permitted
     * for the color dialog. Defaults to TRUE.
     * \param allowOpacity set to FALSE to disable opacity modification
     * \since QGIS 3.0
     */
    void setAllowOpacity( bool allowOpacity );

    /**
     * Sets whether the widget's color has been "discarded" and the selected color should not
     * be stored in the recent color list.
     * \param discarded set to TRUE to avoid adding color to recent color list on widget destruction.
     * \since QGIS 3.0
     */
    void setDiscarded( bool discarded ) { mDiscarded = discarded; }

    /**
     * Triggers a user prompt for importing a new color scheme from an existing GPL file.
     *
     * The \a parent argument must be set to a valid parent widget for the dialog prompts.
     *
     *
     * \see createNewUserPalette()
     * \see removeUserPalette()
     * \since QGIS 3.2
     */
    static QgsUserColorScheme *importUserPaletteFromFile( QWidget *parent );

    /**
     * Triggers a user prompt for creating a new user color scheme.
     *
     * The \a parent argument must be set to a valid parent widget for the dialog prompts.
     *
     *
     * \see importUserPaletteFromFile()
     * \see removeUserPalette()
     * \since QGIS 3.2
     */
    static QgsUserColorScheme *createNewUserPalette( QWidget *parent );

    /**
     * Triggers a user prompt for removing an existing user color \a scheme.
     *
     * The \a parent argument must be set to a valid parent widget for the dialog prompts.
     *
     *
     * \see importUserPaletteFromFile()
     * \see createNewUserPalette()
     * \since QGIS 3.2
     */
    static bool removeUserPalette( QgsUserColorScheme *scheme, QWidget *parent );

  signals:

    /**
     * Emitted when the dialog's color changes
     * \param color current color
     */
    void currentColorChanged( const QColor &color );

  public slots:

    /**
     * Sets the current color for the dialog
     * \param color desired color
     */
    void setColor( const QColor &color );

    /**
     * Sets the color to show in an optional "previous color" section
     * \param color previous color
     */
    void setPreviousColor( const QColor &color );

  protected:

    void hideEvent( QHideEvent *e ) override;

    void mousePressEvent( QMouseEvent *e ) override;

    void mouseMoveEvent( QMouseEvent *e ) override;

    void mouseReleaseEvent( QMouseEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

  private slots:

    void mHueRadio_toggled( bool checked );
    void mSaturationRadio_toggled( bool checked );
    void mValueRadio_toggled( bool checked );
    void mRedRadio_toggled( bool checked );
    void mGreenRadio_toggled( bool checked );
    void mBlueRadio_toggled( bool checked );

    void mAddColorToSchemeButton_clicked();

    void importPalette();
    void removePalette();
    void newPalette();

    void schemeIndexChanged( int index );
    void listSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

    void mAddCustomColorButton_clicked();

    void mSampleButton_clicked();
    void mTabWidget_currentChanged( int index );

  private slots:

    void mActionShowInButtons_toggled( bool state );

  private:

    static QScreen *findScreenAt( QPoint pos );

    QgsScreenHelper *mScreenHelper = nullptr;

    bool mAllowAlpha = true;

    int mLastCustomColorIndex = 0;

    bool mPickingColor = false;

    bool mDiscarded = false;

    /**
     * Saves all widget settings
     */
    void saveSettings();

    /**
     * Ends a color picking operation
     * \param eventPos global position of pixel to sample color from
     * \param takeSample set to TRUE to actually sample the color, FALSE to just cancel
     * the color picking operation
     */
    void stopPicking( QPoint eventPos, bool takeSample = true );

    /**
     * Returns the average color from the pixels in an image
     * \param image image to sample
     * \returns average color from image
     */
    QColor averageColor( const QImage &image ) const;

    /**
     * Samples a color from the desktop
     * \param point position of color to sample
     * \returns average color from sampled position
     */
    QColor sampleColor( QPoint point ) const;

    /**
     * Repopulates the scheme combo box with current color schemes
     */
    void refreshSchemeComboBox();

    /**
     * Returns the path to the user's palette folder
     */
    static QString gplFilePath();

    //! Updates the state of actions for the current selected scheme
    void updateActionsForCurrentScheme();
};

#endif // QGSCOMPOUNDCOLORWIDGET_H
