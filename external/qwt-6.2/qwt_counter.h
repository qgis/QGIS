/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_COUNTER_H
#define QWT_COUNTER_H

#include "qwt_global.h"
#include <qwidget.h>

/*!
   \brief The Counter Widget

   A Counter consists of a label displaying a number and
   one ore more (up to three) push buttons on each side
   of the label which can be used to increment or decrement
   the counter's value.

   A counter has a range from a minimum value to a maximum value
   and a step size. When the wrapping property is set
   the counter is circular.

   The number of steps by which a button increments or decrements the value
   can be specified using setIncSteps(). The number of buttons can be
   changed with setNumButtons().

   Example:
   \code
 #include <qwt_counter.h>

   QwtCounter *counter = new QwtCounter(parent);

   counter->setRange(0.0, 100.0);                  // From 0.0 to 100
   counter->setSingleStep( 1.0 );                  // Step size 1.0
   counter->setNumButtons(2);                      // Two buttons each side
   counter->setIncSteps(QwtCounter::Button1, 1);   // Button 1 increments 1 step
   counter->setIncSteps(QwtCounter::Button2, 20);  // Button 2 increments 20 steps

   connect(counter, SIGNAL(valueChanged(double)), myClass, SLOT(newValue(double)));
   \endcode
 */

class QWT_EXPORT QwtCounter : public QWidget
{
    Q_OBJECT

    Q_PROPERTY( double value READ value WRITE setValue NOTIFY valueChanged USER true )
    Q_PROPERTY( double minimum READ minimum WRITE setMinimum )
    Q_PROPERTY( double maximum READ maximum WRITE setMaximum )
    Q_PROPERTY( double singleStep READ singleStep WRITE setSingleStep )

    Q_PROPERTY( int numButtons READ numButtons WRITE setNumButtons )
    Q_PROPERTY( int stepButton1 READ stepButton1 WRITE setStepButton1 )
    Q_PROPERTY( int stepButton2 READ stepButton2 WRITE setStepButton2 )
    Q_PROPERTY( int stepButton3 READ stepButton3 WRITE setStepButton3 )

    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool wrapping READ wrapping WRITE setWrapping )

  public:
    //! Button index
    enum Button
    {
        //! Button intended for minor steps
        Button1,

        //! Button intended for medium steps
        Button2,

        //! Button intended for large steps
        Button3,

        //! Number of buttons
        ButtonCnt
    };

    explicit QwtCounter( QWidget* parent = NULL );
    virtual ~QwtCounter();

    void setValid( bool );
    bool isValid() const;

    void setWrapping( bool );
    bool wrapping() const;

    bool isReadOnly() const;
    void setReadOnly( bool );

    void setNumButtons( int );
    int numButtons() const;

    void setIncSteps( QwtCounter::Button, int numSteps );
    int incSteps( QwtCounter::Button ) const;

    virtual QSize sizeHint() const QWT_OVERRIDE;

    double singleStep() const;
    void setSingleStep( double stepSize );

    void setRange( double min, double max );

    double minimum() const;
    void setMinimum( double );

    double maximum() const;
    void setMaximum( double );

    void setStepButton1( int nSteps );
    int stepButton1() const;

    void setStepButton2( int nSteps );
    int stepButton2() const;

    void setStepButton3( int nSteps );
    int stepButton3() const;

    double value() const;

  public Q_SLOTS:
    void setValue( double );


  Q_SIGNALS:
    /*!
        This signal is emitted when a button has been released
        \param value The new value
     */
    void buttonReleased ( double value );

    /*!
        This signal is emitted when the counter's value has changed
        \param value The new value
     */
    void valueChanged ( double value );

  protected:
    virtual bool event( QEvent* ) QWT_OVERRIDE;
    virtual void wheelEvent( QWheelEvent* ) QWT_OVERRIDE;
    virtual void keyPressEvent( QKeyEvent* ) QWT_OVERRIDE;

  private Q_SLOTS:
    void btnReleased();
    void btnClicked();
    void textChanged();

  private:
    void incrementValue( int numSteps );
    void initCounter();
    void updateButtons();
    void showNumber( double );

    class PrivateData;
    PrivateData* m_data;
};

#endif
