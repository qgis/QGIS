/***************************************************************************
    qgsrangewidgetwrapper.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRANGEWIDGETWRAPPER_H
#define QGSRANGEWIDGETWRAPPER_H

#include "qgis_gui.h"
#include "qgsdial.h"
#include "qgsdoublespinbox.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsslider.h"
#include "qgsspinbox.h"

#include <QDoubleSpinBox>
#include <QSpinBox>

SIP_NO_FILE

class QAbstractSlider;
class QSlider;
class QDial;
class QgsSlider;
class QgsDial;
class TestQgsRangeWidgetWrapper;

/**
 * \ingroup gui
 * \brief Extension of QgsSpinBox to allow values outside the valid range and return them as long as there has been no interaction with the widget.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsRangeWidgetSpinBox : public QgsSpinBox
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRangeWidgetSpinBox.
     * \param parent parent widget
     */
    explicit QgsRangeWidgetSpinBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QString textFromValue( int val ) const override;
    void stepBy( int steps ) override;

    /**
     * Returns the tolerant value as long as there has been no interaction with the widget.
     */
    int value();

  public slots:
    /**
     * Sets \a val as both the tolerance value and the base class value.
     * If the value is below the minimum value, a valid value is set in the base class (to prevent it from being recognized as an invalid value).
     */
    void setValuesTolerant( int val );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;

  protected slots:
    /**
     * Sets mTolerantValue and the value of the base class without checking for minimum value.
     */
    void setValues( int val );

  private:
    int mTolerantValue = 0;
    bool mTouched = false;

    friend class TestQgsRangeWidgetWrapper;
};

/**
 * \ingroup gui
 * \brief Extension of QgsDoubleSpinBox to allow values outside the valid range and return them as long as there has been no interaction with the widget.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsRangeWidgetDoubleSpinBox : public QgsDoubleSpinBox
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRangeWidgetDoubleSpinBox.
     * \param parent parent widget
     */
    explicit QgsRangeWidgetDoubleSpinBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QString textFromValue( double val ) const override;
    void stepBy( int steps ) override;

    /**
     * Returns the tolerant value as long as there has been no interaction with the widget.
     */
    double value();

  public slots:
    /**
     * Sets \a val as both the tolerance value and the base class value.
     * If the value is below the minimum value, a valid value is set in the base class (to prevent it from being recognized as an invalid value).
     */
    void setValuesTolerant( double val );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;

  protected slots:
    /**
     * Sets mTolerantValue and the value of the base class without checking for minimum value.
     */
    void setValues( double val );

  private:
    double mTolerantValue = 0.0;
    bool mTouched = false;

    friend class TestQgsRangeWidgetWrapper;
};

/**
 * \ingroup gui
 * \brief Extension of QgsSlider to allow values outside the valid range and return them as long as there has been no interaction with the widget.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsRangeWidgetSlider : public QgsSlider
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRangeWidgetSlider.
     * \param parent parent widget
     */
    QgsRangeWidgetSlider( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Constructor for QgsRangeWidgetSlider
    QgsRangeWidgetSlider( Qt::Orientation orientation, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QVariant variantValue() const override;

  public slots:
    /**
     * Sets \a val as both the tolerance value and the base class value.
     * If the value is below the minimum value, a valid value is set in the base class (to prevent it from being recognized as an invalid value).
     */
    void setValuesTolerant( const QVariant &val );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;

  protected slots:
    /**
     * Sets mTolerantValue and the value of the base class without checking for minimum value.
     */
    void setValues( const QVariant &val );

  private:
    QVariant mTolerantValue;
    bool mTouched = false;

    friend class TestQgsRangeWidgetWrapper;
};

/**
 * \ingroup gui
 * \brief Extension of QgsDial to allow values outside the valid range and return them as long as there has been no interaction with the widget.
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsRangeWidgetDial : public QgsDial
{
    Q_OBJECT
  public:
    //! Constructor for QgsRangeWidgetDial
    explicit QgsRangeWidgetDial( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QVariant variantValue() const override;

  public slots:
    /**
     * Sets \a val as both the tolerance value and the base class value.
     * If the value is below the minimum value, a valid value is set in the base class (to prevent it from being recognized as an invalid value).
     */
    void setValuesTolerant( const QVariant &val );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void focusInEvent( QFocusEvent *event ) override;
    void focusOutEvent( QFocusEvent *event ) override;

  protected slots:
    /**
     * Sets mTolerantValue and the value of the base class without checking for minimum value.
     */
    void setValues( const QVariant &val );

  private:
    QVariant mTolerantValue;
    bool mTouched = false;

    friend class TestQgsRangeWidgetWrapper;
};

/**
 * \ingroup gui
 * \brief Wraps a range widget.
 *
 * Options:
 * <ul>
 * <li><b>Style</b> <i>Can be "Dial" or "Slider"</i></li>
 * <li><b>Min</b> <i>The minimal allowed value</i></li>
 * <li><b>Max</b> <i>The maximal allowed value</i></li>
 * <li><b>Step</b> <i>The step size when incrementing/decrementing the value</i></li>
 * </ul>
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsRangeWidgetWrapper : public QgsEditorWidgetWrapper
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsRangeWidgetWrapper.
     *
     * The \a layer and \a fieldIdx arguments specify the vector layer field associated with the wrapper.
     *
     * The \a editor argument indicates the editor widget to use with the wrapper. This can be NULLPTR if a
     * new widget should be autogenerated.
     *
     * A \a parent widget for this widget wrapper and the created widget can also be specified.
     */
    explicit QgsRangeWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent = nullptr );

    /**
     * Returns the default field precision to use for a \a field.
     *
     * This precision will be used by the widget if the user has not manually set a precision.
     *
     * \since QGIS 4.0
     */
    static int defaultFieldPrecision( const QgsField &field );

    // QgsEditorWidgetWrapper interface
  public:
    QVariant value() const override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

  public slots:
    void setEnabled( bool enabled ) override;

  private slots:
    // NOTE - cannot be named "valueChanged", otherwise implicit conversion to QVariant results in
    // infinite recursion
    void valueChangedVariant( const QVariant & );

  private:
    void updateValues( const QVariant &value, const QVariantList & = QVariantList() ) override;

    QSpinBox *mIntSpinBox = nullptr;
    QgsRangeWidgetSpinBox *mQgsRangeWidgetIntSpinBox = nullptr;
    QDoubleSpinBox *mDoubleSpinBox = nullptr;
    QgsRangeWidgetDoubleSpinBox *mQgsRangeWidgetDoubleSpinBox = nullptr;
    QSlider *mSlider = nullptr;
    QDial *mDial = nullptr;
    QgsSlider *mQgsSlider = nullptr;
    QgsDial *mQgsDial = nullptr;
    QgsRangeWidgetSlider *mQgsRangeWidgetSlider = nullptr;
    QgsRangeWidgetDial *mQgsRangeWidgetDial = nullptr;

    friend class TestQgsRangeWidgetWrapper;
};

#endif // QGSRANGEWIDGETWRAPPER_H
