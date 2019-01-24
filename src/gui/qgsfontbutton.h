/***************************************************************************
     qgsfontbutton.h
     ---------------
    Date                 : May 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFONTBUTTON_H
#define QGSFONTBUTTON_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgstextrenderer.h"

#include <QToolButton>

class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsFontButton
 * A button for customizing QgsTextFormat settings.
 *
 * The button will open a detailed text format settings dialog when clicked. An attached drop-down
 * menu allows for copying and pasting text styles, picking colors for the text, and for dropping
 * colors from other color widgets.
 *
 * The button can be used in two different modes(). The default behavior is to include
 * all settings used for configuring QgsTextFormat/QgsTextRenderer classes. A cut down
 * mode (without settings for color) is also available when the resultant font is
 * used only in a QFont object.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsFontButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY( Mode mode READ mode WRITE setMode )
    Q_PROPERTY( QString dialogTitle READ dialogTitle WRITE setDialogTitle )
    Q_PROPERTY( QFont currentFont READ currentFont WRITE setCurrentFont NOTIFY changed )
    Q_PROPERTY( QgsTextFormat textFormat READ textFormat WRITE setTextFormat NOTIFY changed )

  public:

    //! Available button modes.
    enum Mode
    {
      ModeTextRenderer,  //!< Configure font settings for use with QgsTextRenderer
      ModeQFont, //!< Configure font settings for use with QFont objects
    };

    Q_ENUM( Mode )

    /**
     * Construct a new font button.
     * Use \a parent to attach a parent QWidget to the dialog.
     * Use \a dialogTitle string to define the title to show in the text settings dialog.
     */
    QgsFontButton( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &dialogTitle = QString() );

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    /**
     * Returns the current button mode.
     * \see setMode()
     */
    QgsFontButton::Mode mode() const;

    /**
     * Sets the current button \a mode. This can be used to toggle between
     * the full capabilities of the button (for configuring QgsTextFormat/QgsTextRenderer objects)
     * and a cut-back version for configuring QFont object properties (i.e. with
     * no color settings or the other advanced options QgsTextFormat allows).
     * \see mode()
     */
    void setMode( Mode mode );

    /**
     * Sets the \a title for the text settings dialog window.
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the text settings dialog window.
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     */
    QgsMapCanvas *mapCanvas() const;

    /**
     * Sets a map \a canvas to associate with the widget. This allows the
     * widget to fetch current settings from the map canvas, such as current scale.
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the current text formatting set by the widget.
     * This is only used when mode() is ModeTextRenderer.
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const { return mFormat; }

    /**
     * Returns the current QFont set by the widget.
     * This is only used when mode() is ModeQFont.
     * \see setCurrentFont()
     */
    QFont currentFont() const;


  public slots:

    /**
     * Sets the current text \a format to show in the widget.
     * This is only used when mode() is ModeTextRenderer.
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Sets the current text \a font to show in the widget.
     * This is only used when mode() is ModeQFont.
     * \see currentFont()
     */
    void setCurrentFont( const QFont &font );

    /**
     * Sets the current \a color for the text. Will emit a changed signal if the color is different
     * to the previous text color.
     * This is only used when mode() is ModeTextRenderer.
     */
    void setColor( const QColor &color );

    /**
     * Copies the current text format to the clipboard.
     * \see pasteFormat()
     */
    void copyFormat();

    /**
     * Pastes a format from the clipboard. If clipboard does not contain a valid
     * format then no change is applied.
     * \see copyFormat()
     */
    void pasteFormat();

    /**
     * Copies the current text color to the clipboard.
     * This is only used when mode() is ModeTextRenderer.
     * \see pasteColor()
     */
    void copyColor();

    /**
     * Pastes a color from the clipboard to the text format. If clipboard does not contain a valid
     * color or string representation of a color, then no change is applied.
     * This is only used when mode() is ModeTextRenderer.
     * \see copyColor()
     */
    void pasteColor();

  signals:

    /**
     * Emitted when the widget's text format settings are changed.
     */
    void changed();

  protected:

    bool event( QEvent *e ) override;
    void changeEvent( QEvent *e ) override;
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent *event ) override;

    // Reimplemented to detect right mouse button clicks on the color button and allow dragging colors
    void mousePressEvent( QMouseEvent *e ) override;
    // Reimplemented to allow dragging fonts from button
    void mouseMoveEvent( QMouseEvent *e ) override;

    // Reimplemented to accept dragged colors
    void dragEnterEvent( QDragEnterEvent *e ) override;

    // Reimplemented to reset button appearance after drag leave
    void dragLeaveEvent( QDragLeaveEvent *e ) override;

    // Reimplemented to accept dropped colors
    void dropEvent( QDropEvent *e ) override;

    void wheelEvent( QWheelEvent *event ) override;

  private slots:

    void showSettingsDialog();

    /**
     * Creates the drop-down menu entries
     */
    void prepareMenu();

    void addRecentColor( const QColor &color );

  private:

    QSize mSizeHint;

    Mode mMode = ModeTextRenderer;

    QString mDialogTitle;
    QgsTextFormat mFormat;
    QFont mFont;

    QgsMapCanvas *mMapCanvas = nullptr;

    QPoint mDragStartPosition;

    QMenu *mMenu = nullptr;

    QSize mIconSize;

    /**
     * Attempts to parse \a mimeData as a text format.
     * \returns true if mime data could be intrepreted as a format
     * \param mimeData mime data
     * \param resultFormat destination for text format
     * \see colorFromMimeData
     */
    bool formatFromMimeData( const QMimeData *mimeData, QgsTextFormat &resultFormat ) const;


    /**
     * Attempts to parse \a mimeData as a QFont.
     * \returns true if mime data could be intrepreted as a QFont
     * \param mimeData mime data
     * \param resultFont destination for font
     * \see formatFromMimeData
     */
    bool fontFromMimeData( const QMimeData *mimeData, QFont &resultFont ) const;

    /**
     * Attempts to parse mimeData as a color, either via the mime data's color data or by
     * parsing a textual representation of a color.
     * \returns true if mime data could be intrepreted as a color
     * \param mimeData mime data
     * \param resultColor QColor to store evaluated color
     * \param hasAlpha will be set to true if mime data also included an alpha component
     * \see formatFromMimeData
     */
    bool colorFromMimeData( const QMimeData *mimeData, QColor &resultColor, bool &hasAlpha );

    /**
     * Create a \a color icon for display in the drop-down menu.
     */
    QPixmap createColorIcon( const QColor &color ) const;

    /**
     * Creates a drag icon showing the current font style.
     */
    QPixmap createDragIcon( QSize size = QSize( 50, 50 ), const QgsTextFormat *tempFormat = nullptr, const QFont *tempFont = nullptr ) const;

    /**
     * Regenerates the text preview. If \a color is specified, a temporary color preview
     * is shown instead.
     */
    void updatePreview( const QColor &color = QColor(), QgsTextFormat *tempFormat = nullptr, QFont *tempFont = nullptr );
};

#endif // QGSFONTBUTTON_H
