/***************************************************************************
    qgscolorwidgets.h - color selection widgets
    ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORWIDGETS_H
#define QGSCOLORWIDGETS_H

#include <QWidgetAction>
#include <QWidget>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QColor;
class QSpinBox;
class QLineEdit;
class QToolButton;

/**
 * \ingroup gui
 * \class QgsColorWidget
 * \brief A base class for interactive color widgets. Widgets can either allow setting a single component of
 * a color (e.g., the red or green components), or an entire color. The QgsColorWidget also keeps track of
 * any explicitly set hue for the color, so that this information is not lost when the widget is
 * set to a color with an ambiguous hue (e.g., black or white shades).
 */

class GUI_EXPORT QgsColorWidget : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Specifies the color component which the widget alters
     */
    enum ColorComponent
    {
      Multiple = 0, //!< Widget alters multiple color components
      Red, //!< Red component of color
      Green, //!< Green component of color
      Blue, //!< Blue component of color
      Hue, //!< Hue component of color (based on HSV model)
      Saturation, //!< Saturation component of color (based on HSV model)
      Value, //!< Value component of color (based on HSV model)
      Alpha, //!< Alpha component (opacity) of color
      Cyan, //!< Cyan component (based on CMYK model) of color
      Magenta, //!< Magenta component (based on CMYK model) of color
      Yellow, //!< Yellow component (based on CMYK model) of color
      Black //!< Black component (based on CMYK model) of color
    };

    /**
     * Construct a new color widget.
     * \param parent parent QWidget for the widget
     * \param component color component the widget alters
     */
    QgsColorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, ColorComponent component = Multiple );

    /**
     * Returns the current color for the widget
     * \returns current widget color
     * \see setColor
     */
    QColor color() const;

    /**
     * Returns the color component which the widget controls
     * \returns color component for widget
     * \see setComponent
     */
    ColorComponent component() const { return mComponent; }

    /**
     * Returns the current value of the widget's color component
     * \returns value of color component, or -1 if widget has multiple components or an invalid color
     * set
     * \see setComponentValue
     * \see component
     */
    int componentValue() const;

    /**
     * Create an icon for dragging colors
     * \param color for icon
     */
    static QPixmap createDragIcon( const QColor &color );

  public slots:

    /**
     * Sets the color for the widget
     * \param color widget color
     * \param emitSignals set to TRUE to emit the colorChanged signal after setting color
     * \see color
     */
    virtual void setColor( const QColor &color, bool emitSignals = false );

    /**
     * Sets the color component which the widget controls
     * \param component color component for widget
     * \see component
     */
    virtual void setComponent( QgsColorWidget::ColorComponent component );

    /**
     * Alters the widget's color by setting the value for the widget's color component
     * \param value value for widget's color component. This value is automatically
     * clipped to the range of valid values for the color component.
     * \see componentValue
     * \see setComponent
     * \note this method has no effect if the widget is set to the QgsColorWidget::Multiple
     * component
     */
    virtual void setComponentValue( int value );

  signals:

    /**
     * Emitted when the widget's color changes
     * \param color new widget color
     */
    void colorChanged( const QColor &color );

    /**
     * Emitted when mouse hovers over widget.
     */
    void hovered();

  protected:

    QColor mCurrentColor;

    ColorComponent mComponent;

    /**
     * QColor wipes the hue information when it is ambiguous (e.g., for saturation = 0). So
     * the hue is stored in mExplicit hue to keep it around, as it is useful when modifying colors
     */
    int mExplicitHue = 0;

    /**
     * Returns the range of valid values for the color widget's component
     * \returns maximum value allowed for color component, or -1 if widget has multiple components
     */
    int componentRange() const;

    /**
     * Returns the range of valid values a color component
     * \returns maximum value allowed for color component
     */
    static int componentRange( ColorComponent component );

    /**
     * Returns the value of a component of the widget's current color. This method correctly
     * handles hue values when the color has an ambiguous hue (e.g., black or white shades)
     * \param component color component to return
     * \returns value of color component, or -1 if widget has an invalid color set
     * \see hue
     */
    int componentValue( ColorComponent component ) const;

    /**
     * Returns the hue for the widget. This may differ from the hue for the QColor returned by color(),
     * as QColor returns a hue of -1 if the color's hue is ambiguous (e.g., if the saturation is zero).
     * \returns explicitly set hue for widget
     */
    int hue() const;

    /**
     * Alters a color by modifying the value of a specific color component
     * \param color color to alter
     * \param component color component to alter
     * \param newValue new value of color component. Values are automatically clipped to a
     * valid range for the color component.
     */
    static void alterColor( QColor &color, QgsColorWidget::ColorComponent component, int newValue );

    /**
     * Returns color widget type of color, either RGB, HSV, CMYK, or Invalid if this component value is Multiple or Alpha
     */
    QColor::Spec colorSpec() const;

    /**
     * Returns \a component type of color, either RGB, HSV, CMYK, or Invalid if \a component value is Multiple or Alpha
     */
    static QColor::Spec colorSpec( QgsColorWidget::ColorComponent component );

    /**
     * Generates a checkboard pattern pixmap for use as a background to transparent colors
     * \returns checkerboard pixmap
     */
    static const QPixmap &transparentBackground();

    //Reimplemented to accept dragged colors
    void dragEnterEvent( QDragEnterEvent *e ) override;

    //Reimplemented to accept dropped colors
    void dropEvent( QDropEvent *e ) override;

    void mouseMoveEvent( QMouseEvent *e ) override;
    void mousePressEvent( QMouseEvent *e ) override;
    void mouseReleaseEvent( QMouseEvent *e ) override;

    friend class TestQgsCompoundColorWidget;
};


/**
 * \ingroup gui
 * \class QgsColorWidgetAction
 * \brief An action containing a color widget, which can be embedded into a menu.
 * \see QgsColorWidget
 */

class GUI_EXPORT QgsColorWidgetAction: public QWidgetAction
{
    Q_OBJECT

  public:

    /**
     * Construct a new color widget action.
     * \param colorWidget QgsColorWidget to show in action
     * \param menu parent menu
     * \param parent parent widget
     */
    QgsColorWidgetAction( QgsColorWidget *colorWidget, QMenu *menu = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the color widget contained in the widget action.
     */
    QgsColorWidget *colorWidget() { return mColorWidget; }

    /**
     * Sets whether the parent menu should be dismissed and closed when a color is selected
     * from the action's color widget.
     * \param dismiss set to TRUE (default) to immediately close the menu when a color is selected
     * from the widget. If set to FALSE, the colorChanged signal will be emitted but the menu will
     * stay open.
     * \see dismissOnColorSelection()
     */
    void setDismissOnColorSelection( bool dismiss ) { mDismissOnColorSelection = dismiss; }

    /**
     * Returns whether the parent menu will be dismissed after a color is selected from the
     * action's color widget.
     * \see setDismissOnColorSelection
     */
    bool dismissOnColorSelection() const { return mDismissOnColorSelection; }

  signals:

    /**
     * Emitted when a color has been selected from the widget
     * \param color selected color
     */
    void colorChanged( const QColor &color );

  private:
    QMenu *mMenu = nullptr;
    QgsColorWidget *mColorWidget = nullptr;

    //used to suppress recursion with hover events
    bool mSuppressRecurse;

    bool mDismissOnColorSelection;

  private slots:

    /**
     * Handles setting the active action for the menu when cursor hovers over color widget
     */
    void onHover();

    /**
     * Emits color changed signal and closes parent menu
     */
    void setColor( const QColor &color );
};



/**
 * \ingroup gui
 * \class QgsColorWheel
 * \brief A color wheel widget. This widget consists of an outer ring which allows for hue selection, and an
 * inner rotating triangle which allows for saturation and value selection.
 */

class GUI_EXPORT QgsColorWheel : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Constructs a new color wheel widget.
     * \param parent parent QWidget for the widget
     */
    QgsColorWheel( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsColorWheel() override;

    QSize sizeHint() const override;
    void paintEvent( QPaintEvent *event ) override;

  public slots:

    void setColor( const QColor &color, bool emitSignals = false ) override;

  protected:

    void resizeEvent( QResizeEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

  private:

    enum ControlPart
    {
      None,
      Wheel,
      Triangle
    };

    bool mIsDragging = false;

    /*Margin between outer ring and edge of widget*/
    int mMargin = 4;

    /*Thickness of hue ring in pixels*/
    int mWheelThickness = 18;

    /*Part of the wheel where the mouse was originally depressed*/
    ControlPart mClickedPart = QgsColorWheel::None;

    /*Cached image of hue wheel*/
    QImage mWheelImage;

    /*Cached image of inner triangle*/
    QImage mTriangleImage;

    /*Resuable, temporary image for drawing widget*/
    QImage mWidgetImage;

    /*Whether the color wheel image requires redrawing*/
    bool mWheelDirty = true;

    /*Whether the inner triangle image requires redrawing*/
    bool mTriangleDirty = true;

    /*Conical gradient brush used for drawing hue wheel*/
    QBrush mWheelBrush;

    /**
     * Creates cache images for specified widget size
     * \param size widget size for images
     */
    void createImages( QSizeF size );

    //! Creates the hue wheel image
    void createWheel();

    //! Creates the inner triangle image
    void createTriangle();

    /**
     * Sets the widget color based on a point in the widget
     * \param pos position for color
     */
    void setColorFromPos( QPointF pos );

};


/**
 * \ingroup gui
 * \class QgsColorBox
 * \brief A color box widget. This widget consists of a two dimensional rectangle filled with color
 * variations, where a different color component varies along both the horizontal and vertical
 * axis.
 */

class GUI_EXPORT QgsColorBox : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Construct a new color box widget.
     * \param parent parent QWidget for the widget
     * \param component constant color component for the widget. The color components
     * which vary along the horizontal and vertical axis are automatically assigned
     * based on this constant color component.
     */
    QgsColorBox( QWidget *parent SIP_TRANSFERTHIS = nullptr, ColorComponent component = Value );

    ~QgsColorBox() override;

    QSize sizeHint() const override;
    void paintEvent( QPaintEvent *event ) override;

    void setComponent( ColorComponent component ) override;

  public slots:
    void setColor( const QColor &color, bool emitSignals = false ) override;

  protected:

    void resizeEvent( QResizeEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

  private:

    bool mIsDragging = false;

    /*Margin between outer ring and edge of widget*/
    int mMargin = 2;

    /*Cached image for color box*/
    QImage *mBoxImage = nullptr;

    /*Whether the cached image requires redrawing*/
    bool mDirty = true;

    /**
     * Creates the color box background cached image
     */
    void createBox();

    /**
     * Returns the range of permissible values along the x axis
     * \returns maximum color component value for x axis
     */
    int valueRangeX() const;

    /**
     * Returns the range of permissible values along the y axis
     * \returns maximum color component value for y axis
     */
    int valueRangeY() const;

    /**
     * Returns the color component which varies along the y axis
     */
    QgsColorWidget::ColorComponent yComponent() const;

    /**
     * Returns the value of the color component which varies along the y axis
     */
    int yComponentValue() const;

    /**
     * Returns the color component which varies along the x axis
     */
    QgsColorWidget::ColorComponent xComponent() const;

    /**
     * Returns the value of the color component which varies along the x axis
     */
    int xComponentValue() const;

    /**
     * Updates the widget's color based on a point within the widget
     * \param point point within the widget
     */
    void setColorFromPoint( QPoint point );

};


/**
 * \ingroup gui
 * \class QgsColorRampWidget
 * \brief A color ramp widget. This widget consists of an interactive box filled with a color which varies along
 * its length by a single color component (e.g., varying saturation from 0 to 100%).
 */

class GUI_EXPORT QgsColorRampWidget : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Specifies the orientation of a color ramp
     */
    enum Orientation
    {
      Horizontal = 0, //!< Horizontal ramp
      Vertical //!< Vertical ramp
    };

    /**
     * Construct a new color ramp widget.
     * \param parent parent QWidget for the widget
     * \param component color component which varies along the ramp
     * \param orientation orientation for widget
     */
    QgsColorRampWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                        ColorComponent component = QgsColorWidget::Red,
                        Orientation orientation = QgsColorRampWidget::Horizontal );

    QSize sizeHint() const override;
    void paintEvent( QPaintEvent *event ) override;

    /**
     * Sets the orientation for the color ramp
     * \param orientation new orientation for the ramp
     * \see orientation
     */
    void setOrientation( Orientation orientation );

    /**
     * Fetches the orientation for the color ramp
     * \returns orientation for the ramp
     * \see setOrientation
     */
    Orientation orientation() const { return mOrientation; }

    /**
     * Sets the margin between the edge of the widget and the ramp
     * \param margin margin around the ramp
     * \see interiorMargin
     */
    void setInteriorMargin( int margin );

    /**
     * Fetches the margin between the edge of the widget and the ramp
     * \returns margin around the ramp
     * \see setInteriorMargin
     */
    int interiorMargin() const { return mMargin; }

    /**
     * Sets whether the ramp should be drawn within a frame
     * \param showFrame set to TRUE to draw a frame around the ramp
     * \see showFrame
     */
    void setShowFrame( bool showFrame );

    /**
     * Fetches whether the ramp is drawn within a frame
     * \returns TRUE if a frame is drawn around the ramp
     * \see setShowFrame
     */
    bool showFrame() const { return mShowFrame; }

    /**
     * Sets the size for drawing the triangular markers on the ramp
     * \param markerSize marker size in pixels
     */
    void setMarkerSize( int markerSize );

  signals:

    /**
     * Emitted when the widget's color component value changes
     * \param value new value of color component
     */
    void valueChanged( int value );

  protected:

    void mouseMoveEvent( QMouseEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:

    bool mIsDragging = false;

    /*Orientation for ramp*/
    Orientation mOrientation;

    /*Margin around ramp*/
    int mMargin = 4;

    /*Whether to draw a frame around the ramp*/
    bool mShowFrame = false;

    /*Polygon for upper triangle marker*/
    QPolygonF mTopTriangle;

    /*Polygon for lower triangle marker*/
    QPolygonF mBottomTriangle;

    /**
     * Updates the widget's color based on a point within the widget
     * \param point point within the widget
     */
    void setColorFromPoint( QPointF point );

};


/**
 * \ingroup gui
 * \class QgsColorSliderWidget
 * \brief A composite horizontal color ramp widget and associated spinbox for manual value entry.
 */

class GUI_EXPORT QgsColorSliderWidget : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Construct a new color slider widget.
     * \param parent parent QWidget for the widget
     * \param component color component which is controlled by the slider
     */
    QgsColorSliderWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, ColorComponent component = QgsColorWidget::Red );

    void setComponent( ColorComponent component ) override;
    void setComponentValue( int value ) override;
    void setColor( const QColor &color, bool emitSignals = false ) override;

  private:

    /*Color ramp widget*/
    QgsColorRampWidget *mRampWidget = nullptr;

    /*Spin box widget*/
    QSpinBox *mSpinBox = nullptr;

    /**
     * Converts the real value of a color component to a friendly display value. For instance,
     * alpha values from 0-255 have little meaning to users, so we translate them to 0-100%
     * \param realValue actual value of the color component
     * \returns display value of color component
     * \see convertDisplayToReal
     */
    int convertRealToDisplay( int realValue ) const;

    /**
     * Converts the display value of a color component to a real value.
     * \param displayValue friendly display value of the color component
     * \returns real value of color component
     * \see convertRealToDisplay
     */
    int convertDisplayToReal( int displayValue ) const;

  private slots:

    /**
     * Called when the color for the ramp changes
     */
    void rampColorChanged( const QColor &color );

    /**
     * Called when the value of the spin box changes
     */
    void spinChanged( int value );

    /**
     * Called when the value for the ramp changes
     */
    void rampChanged( int value );

};


/**
 * \ingroup gui
 * \class QgsColorTextWidget
 * \brief A line edit widget which displays colors as text and accepts string representations
 * of colors.
 */

class GUI_EXPORT QgsColorTextWidget : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Specifies the display format for a color
     */
    enum ColorTextFormat
    {
      HexRgb = 0, //!< \#RRGGBB in hexadecimal
      HexRgbA, //!< \#RRGGBBAA in hexadecimal, with alpha
      Rgb, //!< Rgb( r, g, b ) format
      Rgba //!< Rgba( r, g, b, a ) format, with alpha
    };
    Q_ENUM( ColorTextFormat )

    /**
     * Construct a new color line edit widget.
     * \param parent parent QWidget for the widget
     */
    QgsColorTextWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setColor( const QColor &color, bool emitSignals = false ) override;

    /**
     * Sets whether opacity modification (transparency) is permitted.
     * Defaults to TRUE.
     * \param allowOpacity set to FALSE to disable opacity modification
     */
    void setAllowOpacity( bool allowOpacity );

  protected:
    void resizeEvent( QResizeEvent *event ) override;

  private:

    QLineEdit *mLineEdit = nullptr;

    /*Drop-down menu button*/
    QToolButton *mMenuButton = nullptr;

    /*Display format for colors*/
    ColorTextFormat mFormat = QgsColorTextWidget::HexRgb;

    bool mAllowAlpha = true;

    /**
     * Updates the text based on the current color
     */
    void updateText();

  private slots:

    /**
     * Called when the user enters text into the widget
     */
    void textChanged();

    /**
     * Called when the drop-down arrow is clicked to show the format selection menu
     */
    void showMenu();
};


/**
 * \ingroup gui
 * \class QgsColorPreviewWidget
 * \brief A preview box which displays one or two colors as swatches.
 */

class GUI_EXPORT QgsColorPreviewWidget : public QgsColorWidget
{
    Q_OBJECT

  public:

    /**
     * Construct a new color preview widget.
     * \param parent parent QWidget for the widget
     */
    QgsColorPreviewWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void paintEvent( QPaintEvent *event ) override;
    QSize sizeHint() const override;

    /**
     * Returns the secondary color for the widget
     * \returns secondary widget color, or an invalid color if the widget
     * has no secondary color
     * \see color
     * \see setColor2
     */
    QColor color2() const { return mColor2; }

  public slots:

    /**
     * Sets the second color for the widget
     * \param color secondary widget color. Set to an invalid color to prevent
     * drawing of a secondary color
     * \see setColor
     * \see color2
     */
    virtual void setColor2( const QColor &color );

  protected:

    //reimplemented to allow dragging colors
    void mousePressEvent( QMouseEvent *e ) override;

    //reimplemented to click colors
    void mouseReleaseEvent( QMouseEvent *e ) override;

    //reimplemented to allow dragging colors
    void mouseMoveEvent( QMouseEvent *e ) override;

  private:

    /*Secondary color for widget*/
    QColor mColor2;

    QPoint mDragStartPosition;

    /*Draws a color preview within the specified rect.
     * \param color color to draw
     * \param rect rect to draw color in
     * \param painter destination painter
     */
    void drawColor( const QColor &color, QRect rect, QPainter &painter );
};

#endif // #ifndef QGSCOLORWIDGETS_H
