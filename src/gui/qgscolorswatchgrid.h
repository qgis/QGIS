/***************************************************************************
    qgscolorswatchgrid.h
    ------------------
    Date                 : July 2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORSWATCHGRID_H
#define QGSCOLORSWATCHGRID_H

#include "qgscolorscheme.h"
#include <QWidget>
#include <QWidgetAction>
#include "qgis_gui.h"
#include "qgis.h"

/**
 * \ingroup gui
 * \class QgsColorSwatchGrid
 * A grid of color swatches, which allows for user selection. Colors are taken from an
 * associated QgsColorScheme.
 * \see QgsColorGridAction
 * \since QGIS 2.5
 */
class GUI_EXPORT QgsColorSwatchGrid : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Construct a new color swatch grid.
     * \param scheme QgsColorScheme for colors to show in grid
     * \param context context string provided to color scheme
     * \param parent parent widget
     */
    QgsColorSwatchGrid( QgsColorScheme *scheme, const QString &context = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //Reimplemented to set fixed size on widget
    QSize minimumSizeHint() const override;

    //Reimplemented to set fixed size on widget
    QSize sizeHint() const override;

    /**
     * Gets the current context for the grid
     * \returns context string which is passed to scheme for color generation
     * \see setContext
     */
    QString context() const { return mContext; }

    /**
     * Sets the current context for the grid
     * \param context string which is passed to scheme for color generation
     * \see context
     */
    void setContext( const QString &context );

    /**
     * Gets the base color for the widget
     * \returns base color which is passed to scheme for color generation
     * \see setBaseColor
     */
    QColor baseColor() const { return mBaseColor; }

    /**
     * Sets the base color for the widget
     * \param baseColor base color to pass to scheme for color generation
     * \see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**
     * Gets the list of colors shown in the grid
     * \returns list of colors currently shown in the grid
     */
    QgsNamedColorList *colors() { return &mColors; }

  public slots:

    /**
     * Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**
     * Emitted when a color has been selected from the widget
     * \param color selected color
     */
    void colorChanged( const QColor &color );

    /**
     * Emitted when mouse hovers over widget
     */
    void hovered();

  protected:

    //reimplemented QWidget events
    void paintEvent( QPaintEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;

  private:
    QgsColorScheme *mScheme = nullptr;
    QString mContext;
    QgsNamedColorList mColors;
    QColor mBaseColor;

    bool mDrawBoxDepressed;
    int mCurrentHoverBox;

    bool mFocused;
    int mCurrentFocusBox;

    int mWidth;
    //! Label rect height
    int mLabelHeight = 0;
    //! Spacing between label box and text
    int mLabelMargin = 0;

    //! Width/height of color swatches
    int mSwatchSize = 0;
    //! Swatch outline size
    int mSwatchOutlineSize = 0;

    //! Margins between edges of grid widget and swatches
    int mSwatchMargin = 0;

    //! Horizontal/vertical gap between swatches
    int mSwatchSpacing = 0;

    bool mPressedOnWidget;

    /**
     * Calculate height of widget based on number of colors
     * \returns required height of widget in pixels
     */
    int calculateHeight() const;

    /**
     * Draws widget
     * \param painter destination painter
     */
    void draw( QPainter &painter );

    /**
     * Calculate swatch corresponding to a position within the widget
     * \param position position
     * \returns swatch number (starting at 0), or -1 if position is outside a swatch
     */
    int swatchForPosition( QPoint position ) const;

    /**
     * Updates the widget's tooltip for a given color index
     * \param colorIdx color index to use for calculating tooltip
     */
    void updateTooltip( int colorIdx );

    /**
     * Generates a checkboard pattern for transparent color backgrounds
     * \returns checkboard pixmap
     */
    QPixmap transparentBackground();
};


/**
 * \ingroup gui
 * \class QgsColorSwatchGridAction
 * A color swatch grid which can be embedded into a menu.
 * \see QgsColorSwatchGrid
 * \since QGIS 2.5
 */

class GUI_EXPORT QgsColorSwatchGridAction: public QWidgetAction
{
    Q_OBJECT

  public:

    /**
     * Construct a new color swatch grid action.
     * \param scheme QgsColorScheme for colors to show in grid
     * \param menu parent menu
     * \param context context string provided to color scheme
     * \param parent parent widget
     */
    QgsColorSwatchGridAction( QgsColorScheme *scheme, QMenu *menu = nullptr, const QString &context = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the base color for the color grid
     * \param baseColor base color to pass to scheme for color generation
     * \see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**
     * Gets the base color for the color grid
     * \returns base color which is passed to scheme for color generation
     * \see setBaseColor
     */
    QColor baseColor() const;

    /**
     * Gets the current context for the color grid
     * \returns context string which is passed to scheme for color generation
     * \see setContext
     */
    QString context() const;

    /**
     * Sets the current context for the color grid
     * \param context string which is passed to scheme for color generation
     * \see context
     */
    void setContext( const QString &context );

    /**
     * Sets whether the parent menu should be dismissed and closed when a color is selected
     * from the action's color widget.
     * \param dismiss set to true (default) to immediately close the menu when a color is selected
     * from the widget. If set to false, the colorChanged signal will be emitted but the menu will
     * stay open.
     * \see dismissOnColorSelection()
     * \since QGIS 2.14
     */
    void setDismissOnColorSelection( bool dismiss ) { mDismissOnColorSelection = dismiss; }

    /**
     * Returns whether the parent menu will be dismissed after a color is selected from the
     * action's color widget.
     * \see setDismissOnColorSelection
     * \since QGIS 2.14
     */
    bool dismissOnColorSelection() const { return mDismissOnColorSelection; }

  public slots:

    /**
     * Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**
     * Emitted when a color has been selected from the widget
     * \param color selected color
     */
    void colorChanged( const QColor &color );

  private:
    QMenu *mMenu = nullptr;
    QgsColorSwatchGrid *mColorSwatchGrid = nullptr;

    //used to suppress recursion with hover events
    bool mSuppressRecurse;
    bool mDismissOnColorSelection;

  private slots:

    /**
     * Emits color changed signal and closes parent menu
     */
    void setColor( const QColor &color );

    /**
     * Handles setting the active action for the menu when cursor hovers over color grid
     */
    void onHover();
};

#endif
