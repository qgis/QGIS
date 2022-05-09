/***************************************************************************
     qgssymbolbutton.h
     -----------------
    Date                 : July 2017
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
#ifndef QGSSYMBOLBUTTON_H
#define QGSSYMBOLBUTTON_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgis.h"

#include <QToolButton>
#include <QPointer>
#include <memory>

class QgsMapCanvas;
class QgsVectorLayer;
class QgsExpressionContextGenerator;
class QgsPanelWidget;
class QgsMessageBar;
class QMimeData;
class QgsSymbol;

/**
 * \ingroup gui
 * \class QgsSymbolButton
 * \brief A button for creating and modifying QgsSymbol settings.
 *
 * The button shows a preview icon for the current symbol, and will open a detailed symbol editor dialog (or
 * panel widget) when clicked.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSymbolButton : public QToolButton
{
    Q_OBJECT

    Q_PROPERTY( QString dialogTitle READ dialogTitle WRITE setDialogTitle )

  public:

    /**
     * Construct a new symbol button.
     * Use \a dialogTitle string to define the title to show in the symbol settings dialog.
     */
    QgsSymbolButton( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &dialogTitle = QString() );
    ~QgsSymbolButton();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    /**
     * Sets the symbol \a type which the button requires.
     * If the type differs from the current symbol type, the symbol will be reset
     * to a default symbol style of the new type.
     * \see symbolType()
     */
    void setSymbolType( Qgis::SymbolType type );

    /**
     * Returns the symbol type which the button requires.
     * \see setSymbolType()
     */
    Qgis::SymbolType symbolType() const { return mType; }

    /**
     * Sets the \a title for the symbol settings dialog window.
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    /**
     * Returns the title for the symbol settings dialog window.
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

    /**
    * Returns the current symbol defined by the button.
    * \see setSymbol()
    * \see changed()
    */
    QgsSymbol *symbol();

    /**
    * Returns a clone of the current symbol (as the specified template type) defined by the button.
    * \see setSymbol()
    * \see changed()
    * \note Not available in Python bindings.
    */
    template <class SymbolType> SymbolType *clonedSymbol() SIP_SKIP
    {
      QgsSymbol *tmpSymbol = mSymbol.get();
      SymbolType *symbolCastToType = dynamic_cast<SymbolType *>( tmpSymbol );

      if ( symbolCastToType )
      {
        return symbolCastToType->clone();
      }
      else
      {
        //could not cast
        return nullptr;
      }
    }

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
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     * \since QGIS 3.6
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget.
     * \see setMessageBar()
     * \since QGIS 3.6
     */
    QgsMessageBar *messageBar() const;

    /**
     * Returns the layer associated with the widget.
     * \see setLayer()
     */
    QgsVectorLayer *layer() const;

    /**
     * Sets a \a layer to associate with the widget. This allows the
     * widget to setup layer related settings within the symbol settings dialog,
     * such as correctly populating data defined override buttons.
     * \see layer()
     */
    void setLayer( QgsVectorLayer *layer );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the button when required.
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

  public slots:

    /**
     * Sets the \a symbol for the button. Ownership of \a symbol is transferred to the
     * button.
     * \see symbol()
     * \see changed()
     */
    void setSymbol( QgsSymbol *symbol SIP_TRANSFER );

    /**
     * Sets the current \a color for the symbol. Will emit a changed() signal if the color is different
     * to the previous symbol color.
     */
    void setColor( const QColor &color );

    /**
     * Copies the current symbol to the clipboard.
     * \see pasteSymbol()
     */
    void copySymbol();

    /**
     * Pastes a symbol from the clipboard. If clipboard does not contain a valid
     * symbol then no change is applied.
     * \see copySymbol()
     */
    void pasteSymbol();

    /**
     * Copies the current symbol color to the clipboard.
     * \see pasteColor()
     */
    void copyColor();

    /**
     * Pastes a color from the clipboard to the symbol. If clipboard does not contain a valid
     * color or string representation of a color, then no change is applied.
     * \see copyColor()
     */
    void pasteColor();

    /**
     * Sets whether a set to null (clear) option is shown in the button's drop-down menu.
     * \param showNull set to TRUE to show a null option
     * \see showNull()
     * \see isNull()
     * \since QGIS 3.26
     */
    void setShowNull( bool showNull );

    /**
     * Returns whether the set to null (clear) option is shown in the button's drop-down menu.
     * \see setShowNull()
     * \see isNull()
     * \since QGIS 3.26
     */
    bool showNull() const;

    /**
     * Returns TRUE if the current symbol is null.
     * \see setShowNull()
     * \see showNull()
     * \since QGIS 3.26
     */
    bool isNull() const;

    /**
     * Sets symbol to to null.
     * \see setShowNull()
     * \see showNull()
     * \since QGIS 3.26
     */
    void setToNull();

  signals:

    /**
     * Emitted when the symbol's settings are changed.
     * \see symbol()
     * \see setSymbol()
     */
    void changed();

  protected:

    void changeEvent( QEvent *e ) override;
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent *event ) override;

    // Reimplemented to detect right mouse button clicks on the color button and allow dragging colors
    void mousePressEvent( QMouseEvent *e ) override;
    // Reimplemented to allow dragging colors/symbols from button
    void mouseMoveEvent( QMouseEvent *e ) override;
    void mouseReleaseEvent( QMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    // Reimplemented to accept dragged colors
    void dragEnterEvent( QDragEnterEvent *e ) override;

    // Reimplemented to reset button appearance after drag leave
    void dragLeaveEvent( QDragLeaveEvent *e ) override;

    // Reimplemented to accept dropped colors
    void dropEvent( QDropEvent *e ) override;

    void wheelEvent( QWheelEvent *event ) override;

  private slots:

    void showSettingsDialog();
    void updateSymbolFromWidget();
    void cleanUpSymbolSelector( QgsPanelWidget *container );

    /**
     * Creates the drop-down menu entries
     */
    void prepareMenu();

    void addRecentColor( const QColor &color );

    /**
     * Activates the color picker tool, which allows for sampling a color from anywhere on the screen
     */
    void activatePicker();

  private:

    QSize mSizeHint;

    QString mDialogTitle;

    Qgis::SymbolType mType = Qgis::SymbolType::Fill;

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    QPoint mDragStartPosition;

    QMenu *mMenu = nullptr;

    QPointer< QgsVectorLayer > mLayer;

    QSize mIconSize;

    std::unique_ptr< QgsSymbol > mSymbol;

    QgsExpressionContextGenerator *mExpressionContextGenerator = nullptr;

    bool mPickingColor = false;

    bool mShowNull = false;

    /**
     * Regenerates the text preview. If \a color is specified, a temporary color preview
     * is shown instead.
     */
    void updatePreview( const QColor &color = QColor(), QgsSymbol *tempSymbol = nullptr );

    /**
     * Attempts to parse mimeData as a color, either via the mime data's color data or by
     * parsing a textual representation of a color.
     * \returns TRUE if mime data could be intrepreted as a color
     * \param mimeData mime data
     * \param resultColor QColor to store evaluated color
     * \param hasAlpha will be set to TRUE if mime data also included an alpha component
     * \see formatFromMimeData
     */
    bool colorFromMimeData( const QMimeData *mimeData, QColor &resultColor, bool &hasAlpha );

    /**
     * Create a \a color icon for display in the drop-down menu.
     */
    QPixmap createColorIcon( const QColor &color ) const;

    /**
     * Ends a color picking operation
     * \param eventPos global position of pixel to sample color from
     * \param samplingColor set to TRUE to actually sample the color, FALSE to just cancel
     * the color picking operation
     */
    void stopPicking( QPoint eventPos, bool samplingColor = true );

    void showColorDialog();

};

#endif // QGSSYMBOLBUTTON_H
