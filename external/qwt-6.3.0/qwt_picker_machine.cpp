/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_picker_machine.h"
#include "qwt_event_pattern.h"

#include <qevent.h>

//! Constructor
QwtPickerMachine::QwtPickerMachine( SelectionType type )
    : m_selectionType( type )
    , m_state( 0 )
{
}

//! Destructor
QwtPickerMachine::~QwtPickerMachine()
{
}

//! Return the selection type
QwtPickerMachine::SelectionType QwtPickerMachine::selectionType() const
{
    return m_selectionType;
}

//! Return the current state
int QwtPickerMachine::state() const
{
    return m_state;
}

//! Change the current state
void QwtPickerMachine::setState( int state )
{
    m_state = state;
}

//! Set the current state to 0.
void QwtPickerMachine::reset()
{
    setState( 0 );
}

//! Constructor
QwtPickerTrackerMachine::QwtPickerTrackerMachine():
    QwtPickerMachine( NoSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerTrackerMachine::transition(
    const QwtEventPattern&, const QEvent* e )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( e->type() )
    {
        case QEvent::Enter:
        case QEvent::MouseMove:
        {
            if ( state() == 0 )
            {
                cmdList += Begin;
                cmdList += Append;
                setState( 1 );
            }
            else
            {
                cmdList += Move;
            }
            break;
        }
        case QEvent::Leave:
        {
            cmdList += Remove;
            cmdList += End;
            setState( 0 );
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerClickPointMachine::QwtPickerClickPointMachine():
    QwtPickerMachine( PointSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerClickPointMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                cmdList += Begin;
                cmdList += Append;
                cmdList += End;
            }
            break;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent* keyEvent = static_cast< const QKeyEvent* > ( event );
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1, keyEvent ) )
            {
                if ( !keyEvent->isAutoRepeat() )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += End;
                }
            }
            break;
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerDragPointMachine::QwtPickerDragPointMachine():
    QwtPickerMachine( PointSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerDragPointMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    setState( 1 );
                }
            }
            break;
        }
        case QEvent::MouseMove:
        case QEvent::Wheel:
        {
            if ( state() != 0 )
                cmdList += Move;
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            if ( state() != 0 )
            {
                cmdList += End;
                setState( 0 );
            }
            break;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent* keyEvent = static_cast< const QKeyEvent* > ( event );
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1, keyEvent ) )
            {
                if ( !keyEvent->isAutoRepeat() )
                {
                    if ( state() == 0 )
                    {
                        cmdList += Begin;
                        cmdList += Append;
                        setState( 1 );
                    }
                    else
                    {
                        cmdList += End;
                        setState( 0 );
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerClickRectMachine::QwtPickerClickRectMachine():
    QwtPickerMachine( RectSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerClickRectMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                switch ( state() )
                {
                    case 0:
                    {
                        cmdList += Begin;
                        cmdList += Append;
                        setState( 1 );
                        break;
                    }
                    case 1:
                    {
                        // Uh, strange we missed the MouseButtonRelease
                        break;
                    }
                    default:
                    {
                        cmdList += End;
                        setState( 0 );
                    }
                }
            }
            break;
        }
        case QEvent::MouseMove:
        case QEvent::Wheel:
        {
            if ( state() != 0 )
                cmdList += Move;
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 1 )
                {
                    cmdList += Append;
                    setState( 2 );
                }
            }
            break;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent* keyEvent = static_cast< const QKeyEvent* > ( event );
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1, keyEvent ) )
            {
                if ( !keyEvent->isAutoRepeat() )
                {
                    if ( state() == 0 )
                    {
                        cmdList += Begin;
                        cmdList += Append;
                        setState( 1 );
                    }
                    else
                    {
                        if ( state() == 1 )
                        {
                            cmdList += Append;
                            setState( 2 );
                        }
                        else if ( state() == 2 )
                        {
                            cmdList += End;
                            setState( 0 );
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerDragRectMachine::QwtPickerDragRectMachine():
    QwtPickerMachine( RectSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerDragRectMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += Append;
                    setState( 2 );
                }
            }
            break;
        }
        case QEvent::MouseMove:
        case QEvent::Wheel:
        {
            if ( state() != 0 )
                cmdList += Move;
            break;
        }
        case QEvent::MouseButtonRelease:
        {
            if ( state() == 2 )
            {
                cmdList += End;
                setState( 0 );
            }
            break;
        }
        case QEvent::KeyPress:
        {
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1,
                static_cast< const QKeyEvent* > ( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += Append;
                    setState( 2 );
                }
                else
                {
                    cmdList += End;
                    setState( 0 );
                }
            }
            break;
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerPolygonMachine::QwtPickerPolygonMachine():
    QwtPickerMachine( PolygonSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerPolygonMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch ( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += Append;
                    setState( 1 );
                }
                else
                {
                    cmdList += Append;
                }
            }
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect2,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 1 )
                {
                    cmdList += End;
                    setState( 0 );
                }
            }
            break;
        }
        case QEvent::MouseMove:
        case QEvent::Wheel:
        {
            if ( state() != 0 )
                cmdList += Move;
            break;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent* keyEvent = static_cast< const QKeyEvent* > ( event );
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1, keyEvent ) )
            {
                if ( !keyEvent->isAutoRepeat() )
                {
                    if ( state() == 0 )
                    {
                        cmdList += Begin;
                        cmdList += Append;
                        cmdList += Append;
                        setState( 1 );
                    }
                    else
                    {
                        cmdList += Append;
                    }
                }
            }
            else if ( eventPattern.keyMatch( QwtEventPattern::KeySelect2, keyEvent ) )
            {
                if ( !keyEvent->isAutoRepeat() )
                {
                    if ( state() == 1 )
                    {
                        cmdList += End;
                        setState( 0 );
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return cmdList;
}

//! Constructor
QwtPickerDragLineMachine::QwtPickerDragLineMachine():
    QwtPickerMachine( PolygonSelection )
{
}

//! Transition
QList< QwtPickerMachine::Command > QwtPickerDragLineMachine::transition(
    const QwtEventPattern& eventPattern, const QEvent* event )
{
    QList< QwtPickerMachine::Command > cmdList;

    switch( event->type() )
    {
        case QEvent::MouseButtonPress:
        {
            if ( eventPattern.mouseMatch( QwtEventPattern::MouseSelect1,
                static_cast< const QMouseEvent* >( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += Append;
                    setState( 1 );
                }
            }
            break;
        }
        case QEvent::KeyPress:
        {
            if ( eventPattern.keyMatch( QwtEventPattern::KeySelect1,
                static_cast< const QKeyEvent* > ( event ) ) )
            {
                if ( state() == 0 )
                {
                    cmdList += Begin;
                    cmdList += Append;
                    cmdList += Append;
                    setState( 1 );
                }
                else
                {
                    cmdList += End;
                    setState( 0 );
                }
            }
            break;
        }
        case QEvent::MouseMove:
        case QEvent::Wheel:
        {
            if ( state() != 0 )
                cmdList += Move;

            break;
        }
        case QEvent::MouseButtonRelease:
        {
            if ( state() != 0 )
            {
                cmdList += End;
                setState( 0 );
            }
        }
        default:
            break;
    }

    return cmdList;
}
