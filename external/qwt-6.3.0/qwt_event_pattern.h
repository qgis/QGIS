/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_EVENT_PATTERN
#define QWT_EVENT_PATTERN

#include "qwt_global.h"

#include <qnamespace.h>
#include <qvector.h>

class QMouseEvent;
class QKeyEvent;

/*!
   \brief A collection of event patterns

   QwtEventPattern introduces an level of indirection for mouse and
   keyboard inputs. Those are represented by symbolic names, so
   the application code can be configured by individual mappings.

   \sa QwtPicker, QwtPickerMachine, QwtPlotZoomer
 */
class QWT_EXPORT QwtEventPattern
{
  public:
    /*!
       \brief Symbolic mouse input codes

       QwtEventPattern implements 3 different settings for
       mice with 1, 2, or 3 buttons that can be activated
       using initMousePattern(). The default setting is for
       3 button mice.

       Individual settings can be configured using setMousePattern().

       \sa initMousePattern(), setMousePattern(), setKeyPattern()
     */
    enum MousePatternCode
    {
        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton
           - Qt::LeftButton
           - Qt::LeftButton
         */
        MouseSelect1,

        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton + Qt::ControlModifier
           - Qt::RightButton
           - Qt::RightButton
         */
        MouseSelect2,

        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton + Qt::AltModifier
           - Qt::LeftButton + Qt::AltModifier
           - Qt::MidButton
         */
        MouseSelect3,

        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton + Qt::ShiftModifier
           - Qt::LeftButton + Qt::ShiftModifier
           - Qt::LeftButton + Qt::ShiftModifier
         */
        MouseSelect4,

        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton + Qt::ControlButton | Qt::ShiftModifier
           - Qt::RightButton + Qt::ShiftModifier
           - Qt::RightButton + Qt::ShiftModifier
         */
        MouseSelect5,

        /*!
           The default setting for 1, 2 and 3 button mice is:

           - Qt::LeftButton + Qt::AltModifier + Qt::ShiftModifier
           - Qt::LeftButton + Qt::AltModifier | Qt::ShiftModifier
           - Qt::MidButton + Qt::ShiftModifier
         */
        MouseSelect6,

        //! Number of mouse patterns
        MousePatternCount
    };

    /*!
       \brief Symbolic keyboard input codes

       Individual settings can be configured using setKeyPattern()

       \sa setKeyPattern(), setMousePattern()
     */
    enum KeyPatternCode
    {
        //! Qt::Key_Return
        KeySelect1,

        //! Qt::Key_Space
        KeySelect2,

        //! Qt::Key_Escape
        KeyAbort,

        //! Qt::Key_Left
        KeyLeft,

        //! Qt::Key_Right
        KeyRight,

        //! Qt::Key_Up
        KeyUp,

        //! Qt::Key_Down
        KeyDown,

        //! Qt::Key_Plus
        KeyRedo,

        //! Qt::Key_Minus
        KeyUndo,

        //! Qt::Key_Escape
        KeyHome,

        //! Number of key patterns
        KeyPatternCount
    };

    //! A pattern for mouse events
    class MousePattern
    {
      public:
        //! Constructor
        MousePattern( Qt::MouseButton btn = Qt::NoButton,
            Qt::KeyboardModifiers modifierCodes = Qt::NoModifier ):
            button( btn ),
            modifiers( modifierCodes )
        {
        }

        //! Button
        Qt::MouseButton button;

        //! Keyboard modifier
        Qt::KeyboardModifiers modifiers;
    };

    //! A pattern for key events
    class KeyPattern
    {
      public:
        //! Constructor
        KeyPattern( int keyCode = Qt::Key_unknown,
            Qt::KeyboardModifiers modifierCodes = Qt::NoModifier ):
            key( keyCode ),
            modifiers( modifierCodes )
        {
        }

        //! Key code
        int key;

        //! Modifiers
        Qt::KeyboardModifiers modifiers;
    };

    QwtEventPattern();
    virtual ~QwtEventPattern();

    void initMousePattern( int numButtons );
    void initKeyPattern();

    void setMousePattern( MousePatternCode, Qt::MouseButton button,
        Qt::KeyboardModifiers = Qt::NoModifier );

    void setKeyPattern( KeyPatternCode, int key,
        Qt::KeyboardModifiers modifiers = Qt::NoModifier );

    void setMousePattern( const QVector< MousePattern >& );
    void setKeyPattern( const QVector< KeyPattern >& );

    const QVector< MousePattern >& mousePattern() const;
    const QVector< KeyPattern >& keyPattern() const;

    QVector< MousePattern >& mousePattern();
    QVector< KeyPattern >& keyPattern();

    bool mouseMatch( MousePatternCode, const QMouseEvent* ) const;
    bool keyMatch( KeyPatternCode, const QKeyEvent* ) const;

  protected:
    virtual bool mouseMatch( const MousePattern&, const QMouseEvent* ) const;
    virtual bool keyMatch( const KeyPattern&, const QKeyEvent* ) const;

  private:

#if defined( _MSC_VER )
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    QVector< MousePattern > m_mousePattern;
    QVector< KeyPattern > m_keyPattern;
#if defined( _MSC_VER )
#pragma warning(pop)
#endif
};

//! Compare operator
inline bool operator==( QwtEventPattern::MousePattern b1,
    QwtEventPattern::MousePattern b2 )
{
    return b1.button == b2.button && b1.modifiers == b2.modifiers;
}

//! Compare operator
inline bool operator==( QwtEventPattern::KeyPattern b1,
    QwtEventPattern::KeyPattern b2 )
{
    return b1.key == b2.key && b1.modifiers == b2.modifiers;
}

#endif
