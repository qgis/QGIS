/***************************************************************************
    qgsdefaultsearchwidgetwrapper.h
     --------------------------------------
    Date                 : 21.5.2015
    Copyright            : (C) 2015 Karolina Alexiou
    Email                : carolinegr at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEFAULTSEARCHWIDGETWRAPPER_H
#define QGSDEFAULTSEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgis_sip.h"
#include "qgsfilterlineedit.h"

#include <QCheckBox>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * Wraps a search widget. Default form is just a QgsLineFilterEdit
 */

class GUI_EXPORT QgsDefaultSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT
  public:

    //! Constructor for QgsDefaultSearchWidgetWrapper
    explicit QgsDefaultSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    // QgsSearchWidgetWrapper interface
  public:
    QString expression() const override;
    bool applyDirectly() override;
    QgsSearchWidgetWrapper::FilterFlags supportedFlags() const override;
    QgsSearchWidgetWrapper::FilterFlags defaultFlags() const override;
    QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const override;

  public slots:

    void clearWidget() override;

    void setEnabled( bool enabled ) override;

  protected slots:
    void setExpression( const QString &exp ) override;

  private slots:
    void setCaseString( int caseSensitiveCheckState );
    void filterChanged();
    void textChanged( const QString &text );

  protected:
    QWidget *createWidget( QWidget *parent ) override;
    void initWidget( QWidget *editor ) override;
    bool valid() const override;

    /**
     * Returns a pointer to the line edit part of the widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QgsFilterLineEdit *lineEdit();

    /**
     * Returns a pointer to the case sensitivity checkbox in the widget.
     * \note this method is in place for unit testing only, and is not considered
     * stable API
     */
    QCheckBox *caseSensitiveCheckBox();

  private:
    QgsFilterLineEdit *mLineEdit = nullptr;
    QCheckBox *mCheckbox = nullptr;
    QWidget *mContainer = nullptr;
    QString mCaseString;
};

#endif // QGSDEFAULTSEARCHWIDGETWRAPPER_H
