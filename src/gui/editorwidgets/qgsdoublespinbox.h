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

#ifndef QGSDOUBLESPPINBOX_H
#define QGSDOUBLESPPINBOX_H

#include <QDoubleSpinBox>
#include <QToolButton>

/**
 * @brief The QgsSpinBox is a spin box with a clear button that will set the value to the defined clear value.
 * The clear value can be either the minimum or the maiximum value of the spin box or a custom value.
 * This value can then be handled by a special value text.
 */
class GUI_EXPORT QgsDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY( bool showClearButton READ showClearButton WRITE setShowClearButton )
    Q_PROPERTY( bool expressionsEnabled READ expressionsEnabled WRITE setExpressionsEnabled )

  public:
    enum ClearValueMode
    {
      MinimumValue,
      MaximumValue,
      CustomValue
    };

    explicit QgsDoubleSpinBox( QWidget *parent = 0 );

    //! determines if the widget will show a clear button
    //! @note the clear button will set the widget to its minimum value
    void setShowClearButton( const bool showClearButton );
    bool showClearButton() const {return mShowClearButton;}

    /**Sets if the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * @param enabled set to true to allow expression entry
     * @note added in QGIS 2.7
     */
    void setExpressionsEnabled( const bool enabled );
    /**Returns whether the widget will allow entry of simple expressions, which are
     * evaluated and then discarded.
     * @returns true if spin box allows expression entry
     * @note added in QGIS 2.7
     */
    bool expressionsEnabled() const {return mExpressionsEnabled;}

    //! Set the current value to the value defined by the clear value.
    virtual void clear() override;

    /**
     * @brief setClearValue defines the clear value as a custom value and will automatically set the clear value mode to CustomValue
     * @param customValue defines the numerical value used as the clear value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValue( double customValue, QString clearValueText = QString() );
    /**
     * @brief setClearValueMode defines if the clear value should be the minimum or maximum values of the widget or a custom value
     * @param mode mode to user for clear value
     * @param clearValueText is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
     */
    void setClearValueMode( ClearValueMode mode, QString clearValueText = QString() );

    //! returns the value used when clear() is called.
    double clearValue() const;

    virtual double valueFromText( const QString & text ) const override;
    virtual QValidator::State validate( QString & input, int & pos ) const override;

  protected:
    virtual void resizeEvent( QResizeEvent* event ) override;
    virtual void changeEvent( QEvent* event ) override;
    virtual void paintEvent( QPaintEvent* event ) override;

  private slots:
    void changed( const double &value );

  private:
    int frameWidth() const;
    bool shouldShowClearForValue( const double value ) const;

    bool mShowClearButton;
    ClearValueMode mClearValueMode;
    double mCustomClearValue;

    bool mExpressionsEnabled;

    QToolButton* mClearButton;
    QString stripped( const QString &originalText ) const;
};

#endif // QGSDOUBLESPPINBOX_H
