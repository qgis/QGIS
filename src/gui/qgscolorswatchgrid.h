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

/** \ingroup gui
 * \class QgsColorSwatchGrid
 * A grid of color swatches, which allows for user selection. Colors are taken from an
 * associated QgsColorScheme.
 * @see QgsColorGridAction
 * @note introduced in QGIS 2.5
 */
class GUI_EXPORT QgsColorSwatchGrid : public QWidget
{
    Q_OBJECT

  public:

    /**Construct a new color swatch grid.
     * @param scheme QgsColorScheme for colors to show in grid
     * @param context context string provided to color scheme
     * @param parent parent widget
     */
    QgsColorSwatchGrid( QgsColorScheme* scheme, QString context = QString(), QWidget *parent = 0 );

    virtual ~QgsColorSwatchGrid();

    //Reimplemented to set fixed size on widget
    virtual QSize minimumSizeHint() const override;

    //Reimplemented to set fixed size on widget
    virtual QSize sizeHint() const override;

    /**Get the current context for the grid
     * @returns context string which is passed to scheme for color generation
     * @see setContext
     */
    QString context() const { return mContext; }

    /**Sets the current context for the grid
     * @param context string which is passed to scheme for color generation
     * @see context
     */
    void setContext( const QString &context );

    /**Get the base color for the widget
     * @returns base color which is passed to scheme for color generation
     * @see setBaseColor
     */
    QColor baseColor() const { return mBaseColor; }

    /**Sets the base color for the widget
     * @param baseColor base color to pass to scheme for color generation
     * @see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**Gets the list of colors shown in the grid
     * @returns list of colors currently shown in the grid
     */
    QgsNamedColorList *colors() { return &mColors; }

  public slots:

    /**Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**Emitted when a color has been selected from the widget
     * @param color selected color
     */
    void colorChanged( const QColor &color );

    /**Emitted when mouse hovers over widget
     */
    void hovered();

  protected:

    //reimplemented QWidget events
    void paintEvent( QPaintEvent * event ) override;
    void mouseMoveEvent( QMouseEvent * event ) override;
    void mousePressEvent( QMouseEvent * event ) override;
    void mouseReleaseEvent( QMouseEvent * event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void focusInEvent( QFocusEvent* event ) override;
    void focusOutEvent( QFocusEvent* event ) override;

  private:
    QgsColorScheme* mScheme;
    QString mContext;
    QgsNamedColorList mColors;
    QColor mBaseColor;

    bool mDrawBoxDepressed;
    int mCurrentHoverBox;

    bool mFocused;
    int mCurrentFocusBox;

    int mWidth;

    bool mPressedOnWidget;

    /**Calculate height of widget based on number of colors
     * @returns required height of widget in pixels
     */
    int calculateHeight() const;

    /**Draws widget
     * @param painter destination painter
     */
    void draw( QPainter &painter );

    /**Calculate swatch corresponding to a position within the widget
     * @param position position
     * @returns swatch number (starting at 0), or -1 if position is outside a swatch
     */
    int swatchForPosition( const QPoint &position ) const;

    /**Updates the widget's tooltip for a given color index
     * @param colorIdx color index to use for calculating tooltip
     */
    void updateTooltip( const int colorIdx );

    /**Generates a checkboard pattern for transparent color backgrounds
     * @returns checkboard pixmap
     */
    const QPixmap &transparentBackground();
};


/** \ingroup gui
 * \class QgsColorGridAction
 * A color swatch grid which can be embedded into a menu.
 * @see QgsColorSwatchGrid
 * @note introduced in QGIS 2.5
 */

class GUI_EXPORT QgsColorSwatchGridAction: public QWidgetAction
{
    Q_OBJECT

  public:

    /**Construct a new color swatch grid action.
     * @param scheme QgsColorScheme for colors to show in grid
     * @param menu parent menu
     * @param context context string provided to color scheme
     * @param parent parent widget
     */
    QgsColorSwatchGridAction( QgsColorScheme* scheme, QMenu* menu = 0, QString context = QString(), QWidget *parent = 0 );

    virtual ~QgsColorSwatchGridAction();

    /**Sets the base color for the color grid
     * @param baseColor base color to pass to scheme for color generation
     * @see baseColor
     */
    void setBaseColor( const QColor &baseColor );

    /**Get the base color for the color grid
     * @returns base color which is passed to scheme for color generation
     * @see setBaseColor
     */
    QColor baseColor() const;

    /**Get the current context for the color grid
     * @returns context string which is passed to scheme for color generation
     * @see setContext
     */
    QString context() const;

    /**Sets the current context for the color grid
     * @param context string which is passed to scheme for color generation
     * @see context
     */
    void setContext( const QString &context );

  public slots:

    /**Reload colors from scheme and redraws the widget
     */
    void refreshColors();

  signals:

    /**Emitted when a color has been selected from the widget
     * @param color selected color
     */
    void colorChanged( const QColor &color );

  private:
    QMenu* mMenu;
    QgsColorSwatchGrid* mColorSwatchGrid;

    //used to supress recursion with hover events
    bool mSuppressRecurse;

  private slots:

    /**Emits color changed signal and closes parent menu
     */
    void setColor( const QColor &color );

    /**Handles setting the active action for the menu when cursor hovers over color grid
     */
    void onHover();
};

#endif
