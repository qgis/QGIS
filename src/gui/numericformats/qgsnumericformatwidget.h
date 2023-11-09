/***************************************************************************
    qgsnumericformatwidget.h
    ------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNUMERICFORMATWIDGET_H
#define QGSNUMERICFORMATWIDGET_H

#include "qgis_sip.h"
#include "qgsnumericformat.h"
#include "qgspanelwidget.h"
#include <memory>
#include <QDialog>

class QgsFractionNumericFormat;

/**
 * \ingroup gui
 * \class QgsNumericFormatWidget
 * \brief Base class for widgets which allow control over the properties of QgsNumericFormat subclasses
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsNumericFormatWidget : public QgsPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNumericFormatWidget.
     */
    QgsNumericFormatWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsPanelWidget( parent )
    {}

    /**
     * Sets the \a format to show in the widget. Ownership is not transferred.
     * \see format()
     */
    virtual void setFormat( QgsNumericFormat *format ) = 0;

    /**
     * Returns the format defined by the current settings in the widget.
     *
     * Ownership of the returned object is transferred to the caller
     *
     * \see setFormat()
     */
    virtual QgsNumericFormat *format() = 0 SIP_TRANSFERBACK;

  signals:

    /**
     * Emitted whenever the configuration of the numeric format is changed.
     */
    void changed();

};


#include "ui_qgsbasicnumericformatwidgetbase.h"

class QgsBasicNumericFormat;

/**
 * \ingroup gui
 * \class QgsBasicNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsBasicNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsBasicNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsBasicNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsBasicNumericFormatWidget, initially showing the specified \a format.
     */
    QgsBasicNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsBasicNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsBasicNumericFormat > mFormat;
    bool mBlockSignals = false;

};

#include "ui_qgsbearingnumericformatwidgetbase.h"

class QgsBearingNumericFormat;

/**
 * \ingroup gui
 * \class QgsBearingNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsBearingNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsBearingNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsBearingNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsBearingNumericFormatWidget, initially showing the specified \a format.
     */
    QgsBearingNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsBearingNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsBearingNumericFormat > mFormat;
    bool mBlockSignals = false;

};


/**
 * \ingroup gui
 * \class QgsBearingNumericFormatDialog
 * \brief A dialog which allow control over the properties of a QgsBearingNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsBearingNumericFormatDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsBearingNumericFormatDialog, initially showing the specified \a format.
     */
    QgsBearingNumericFormatDialog( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the format defined by the current settings in the dialog.
     *
     * Ownership of the returned object is transferred to the caller
     */
    QgsBearingNumericFormat *format() SIP_FACTORY;

  private:

    QgsBearingNumericFormatWidget *mWidget = nullptr;
};


#include "ui_qgsgeographiccoordinatenumericformatwidgetbase.h"

class QgsGeographicCoordinateNumericFormat;

/**
 * \ingroup gui
 * \class QgsGeographicCoordinateNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsGeographicCoordinateNumericFormat.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsGeographicCoordinateNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsGeographicCoordinateNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeographicCoordinateNumericFormatWidget, initially showing the specified \a format.
     */
    QgsGeographicCoordinateNumericFormatWidget( const QgsNumericFormat *format, bool hidePrecisionControl = false, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsGeographicCoordinateNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsGeographicCoordinateNumericFormat > mFormat;
    bool mBlockSignals = false;

};


/**
 * \ingroup gui
 * \class QgsGeographicCoordinateNumericFormatDialog
 * \brief A dialog which allow control over the properties of a QgsGeographicCoordinateNumericFormat.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsGeographicCoordinateNumericFormatDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeographicCoordinateNumericFormatDialog, initially showing the specified \a format.
     */
    QgsGeographicCoordinateNumericFormatDialog( const QgsNumericFormat *format, bool hidePrecisionControl = false, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the format defined by the current settings in the dialog.
     *
     * Ownership of the returned object is transferred to the caller
     */
    QgsGeographicCoordinateNumericFormat *format() SIP_FACTORY;

  private:

    QgsGeographicCoordinateNumericFormatWidget *mWidget = nullptr;
};



#include "ui_qgscurrencynumericformatwidgetbase.h"

class QgsCurrencyNumericFormat;

/**
 * \ingroup gui
 * \class QgsCurrencyNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsCurrencyNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsCurrencyNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsCurrencyNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCurrencyNumericFormatWidget, initially showing the specified \a format.
     */
    QgsCurrencyNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsCurrencyNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsCurrencyNumericFormat > mFormat;
    bool mBlockSignals = false;

};


#include "ui_qgspercentagenumericformatwidgetbase.h"

class QgsPercentageNumericFormat;

/**
 * \ingroup gui
 * \class QgsPercentageNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsPercentageNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsPercentageNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsPercentageNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPercentageNumericFormatWidget, initially showing the specified \a format.
     */
    QgsPercentageNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsPercentageNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsPercentageNumericFormat > mFormat;
    bool mBlockSignals = false;

};



#include "ui_qgsscientificnumericformatwidgetbase.h"

class QgsScientificNumericFormat;

/**
 * \ingroup gui
 * \class QgsScientificNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsScientificNumericFormat.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsScientificNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsScientificNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsScientificNumericFormatWidget, initially showing the specified \a format.
     */
    QgsScientificNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsScientificNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsScientificNumericFormat > mFormat;
    bool mBlockSignals = false;

};


#include "ui_qgsfractionnumericformatwidgetbase.h"

/**
 * \ingroup gui
 * \class QgsFractionNumericFormatWidget
 * \brief A widget which allow control over the properties of a QgsFractionNumericFormat.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFractionNumericFormatWidget : public QgsNumericFormatWidget, private Ui::QgsFractionNumericFormatWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFractionNumericFormatWidget, initially showing the specified \a format.
     */
    QgsFractionNumericFormatWidget( const QgsNumericFormat *format, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsFractionNumericFormatWidget() override;

    void setFormat( QgsNumericFormat *format ) override;

    QgsNumericFormat *format() override SIP_FACTORY;

  private:
    std::unique_ptr< QgsFractionNumericFormat > mFormat;
    bool mBlockSignals = false;

};
#endif // QGSNUMERICFORMATWIDGET_H
