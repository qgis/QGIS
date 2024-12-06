/***************************************************************************
    qgscheckboxsearchwidgetwrapper.h
     -------------------------------
    Date                 : 2016-05-23
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCHECKBOXSEARCHWIDGETWRAPPER_H
#define QGSCHECKBOXSEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgis_sip.h"

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include "qgis_gui.h"
class QCheckBox;

class QgsCheckboxWidgetFactory;

/**
 * \ingroup gui
 * \class QgsCheckboxSearchWidgetWrapper
 * \brief Wraps a checkbox edit widget for searching.
 */

class GUI_EXPORT QgsCheckboxSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsCheckboxSearchWidgetWrapper.
     * \param vl associated vector layer
     * \param fieldIdx index of associated field
     * \param parent parent widget
     */
    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a variant representing the current state of the widget.
     * \note this will not be a boolean TRUE or FALSE value, it will instead
     * be the values configured to represent checked and unchecked states in
     * the editor widget configuration.
     */
    QVariant value() const;

    bool applyDirectly() override;
    QString expression() const override;
    bool valid() const override;
    QgsSearchWidgetWrapper::FilterFlags supportedFlags() const override;
    QgsSearchWidgetWrapper::FilterFlags defaultFlags() const override;
    QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const override;

  public slots:

    void clearWidget() override;
    void setEnabled( bool enabled ) override;

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;

  protected slots:
    void setExpression( const QString &expression ) override;

  private slots:
    void stateChanged( int state );

  private:
    QCheckBox *mCheckBox = nullptr;

    friend class QgsCheckboxWidgetFactory;
};

#endif // QGSCHECKBOXSEARCHWIDGETWRAPPER_H
