/***************************************************************************
    qgsvaluemapsearchwidgetwrapper.h
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

#ifndef QGSVALUEMAPSEARCHWIDGETWRAPPER_H
#define QGSVALUEMAPSEARCHWIDGETWRAPPER_H

#include "qgssearchwidgetwrapper.h"
#include "qgis.h"
#include <QComboBox>
#include "qgis_gui.h"

/**
 * \ingroup gui
 * Wraps a value map search widget. This widget will offer a combobox with values from another layer
 * referenced by a foreign key (a constraint may be set but is not required on data level).
 * It will be used as a search widget and produces expression to look for in the layer.
 */

class GUI_EXPORT QgsValueMapSearchWidgetWrapper : public QgsSearchWidgetWrapper
{
    Q_OBJECT
  public:

    //! Constructor for QgsValueMapSearchWidgetWrapper
    explicit QgsValueMapSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS = nullptr );
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
    void setExpression( const QString &exp ) override;

  private slots:
    void comboBoxIndexChanged( int idx );

  private:
    QComboBox *mComboBox = nullptr;
};

#endif // QGSVALUEMAPSEARCHWIDGETWRAPPER_H
