/***************************************************************************
                              qgsslider.h
                             -------------------
    begin                : July 2013
    copyright            : (C) 2013 by Daniel Vaz
    email                : danielvaz at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSlider>
#include "qgis_sip.h"
#include <QVariant>
#include "qgis_gui.h"

class QPaintEvent;

/**
 * \ingroup gui
 * \class QgsSlider
 */
class GUI_EXPORT QgsSlider : public QSlider
{
    Q_OBJECT
  public:

    //! Constructor for QgsSlider
    QgsSlider( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! Constructor for QgsSlider
    QgsSlider( Qt::Orientation orientation, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setMinimum( const QVariant &min );
    void setMaximum( const QVariant &max );
    void setSingleStep( const QVariant &step );
    void setValue( const QVariant &value );
    QVariant variantValue() const;

  signals:
    void valueChanged( const QVariant & );

  private slots:
    void onValueChanged( int );

  protected:
    void paintEvent( QPaintEvent *event ) override;

  private:
    void update();

    QVariant mMin, mMax, mStep, mValue;
};
