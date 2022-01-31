/***************************************************************************
    qgsdoublespinbox.h
     --------------------------------------
    Date                 : 09.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDOUBLESPINBOX_H
#define QGSDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsSpinBoxLineEdit;


#ifdef SIP_RUN
% ModuleHeaderCode
// fix to allow compilation with sip that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsdoublespinbox.h>
% End
#endif


/**
 * \ingroup gui
 * \brief The QgsSpinBox is a spin box with a clear button that will set the value to the defined clear value.
 * The clear value can be either the minimum or the maiximum value of the spin box or a custom value.
 * This value can then be handled by a special value text.
 */
class GUI_EXPORT QgsDoubleSpinBox : public QDoubleSpinBox
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsDoubleSpinBox *>( sipCpp ) )
      sipType = sipType_QgsDoubleSpinBox;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
    Q_PROPERTY( bool showClearButton READ showClearButton WRITE setShowClearButton )
    Q_PROPERTY( bool clearValue READ clearValue WRITE setClearValue )
    Q_PROPERTY( bool expressionsEnabled READ expressionsEnabled WRITE setExpressionsEnabled )

  public:

    //! Behavior when widget is cleared.
    enum ClearValueMode
    {
      MinimumValue, //!< Reset value to minimum()
      MaximumValue, //!< Reset value to maximum()
      CustomValue, //!< Reset value to custom value (see setClearValue() )
    };

    /**
     * Constructor for QgsDoubleSpinBox.
     * \param parent parent widget
     */
    explicit QgsDoubleSpinBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether the widget will show a clear button. The clear button
     * allows users to reset the widget to a default or empty state.
     * \param showClearButton set to TRUE to show the clear button, or FALSE to hide it
     * \see showClearButton()
     */
    void setShowClearButton( bool showClearButton );

    /**
     * Returns whether the widget is showing a clear button.
     * \see setShowClearButton()
     */
    bool showClearButton() const {return mShowClearButton;}

    /**
     * Sets if the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * \param enabled set to TRUE to allow expression entry
     * \since QGIS 2.7
     */
    void setExpressionsEnabled( bool enabled );

    /**
     * Returns whether the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * \returns TRUE if spin box allows expression entry
     * \since QGIS 2.7
     */
    bool expressionsEnabled() const {return mExpressionsEnabled;}

    //! Sets the current value to the value defined by the clear value.
    void clear() override;

    /**
     * Defines the clear value as a custom value and will automatically set the clear value mode to CustomValue.
     * \param customValue defines the numerical value used as the clear value
     * \param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     * \see clearValue()
     */
    void setClearValue( double customValue, const QString &clearValueText = QString() );

    /**
     * Defines if the clear value should be the minimum or maximum values of the widget or a custom value.
     * \param mode mode to user for clear value
     * \param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValueMode( ClearValueMode mode, const QString &clearValueText = QString() );

    /**
     * Returns the value used when clear() is called.
     * \see setClearValue()
     */
    double clearValue() const;

    /**
     * Set alignment in the embedded line edit widget
     * \param alignment
     */
    void setLineEditAlignment( Qt::Alignment alignment );

    /**
     * Set the special-value text to be \a txt
     * If set, the spin box will display this text instead of a numeric value whenever the current value
     * is equal to minimum(). Typical use is to indicate that this choice has a special (default) meaning.
     */
    void setSpecialValueText( const QString &txt );

    double valueFromText( const QString &text ) const override;
    QValidator::State validate( QString &input, int &pos ) const override;
    void paintEvent( QPaintEvent *e ) override;

  protected:
    void changeEvent( QEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    // This is required because private implementation of
    // QAbstractSpinBoxPrivate may trigger a second
    // undesired event from the auto-repeat mouse timer
    void timerEvent( QTimerEvent *event ) override;

  private slots:
    void changed( double value );

  private:
    int frameWidth() const;
    bool shouldShowClearForValue( double value ) const;

    QgsSpinBoxLineEdit *mLineEdit = nullptr;

    bool mShowClearButton = true;
    ClearValueMode mClearValueMode = MinimumValue;
    double mCustomClearValue = 0.0;

    bool mExpressionsEnabled = true;

    QString stripped( const QString &originalText ) const;

    friend class TestQgsRangeWidgetWrapper;
};

#endif // QGSDOUBLESPINBOX_H
