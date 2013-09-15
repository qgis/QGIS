/***************************************************************************
                             qgsfieldchoosercombo.cpp
                             -------------------------
    begin                : September 2013
    copyright            : (C) 2013 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDCHOOSERCOMBO_H
#define QGSFIELDCHOOSERCOMBO_H

#include "qgsfieldchooserwidget.h"

/**
 * @brief implements a field chooser widget for combo boxes.
 * @see QgsLayerChooserWidget
 */
class QgsFieldChooserCombo : public QgsFieldChooserWidget
{
    Q_OBJECT

  public:
    /** constructor */
    QgsFieldChooserCombo( QgsLayerChooserWidget* layerChooser, QObject *parent = 0 );
    QgsFieldChooserCombo( QgsVectorLayer* vl, QObject* parent );

  protected slots:
    void currentIndexChanged( int idx );

  private:
    QComboBox* mWidget;

    // QgsFieldChooserWidget interface
  public:
    virtual bool initWidget( QWidget* widget );
    virtual int getFieldIndex();
    virtual QString getFieldName();
  protected:
    virtual void clearWidget();
    virtual void addField( QString fieldAlias, QString fieldName, DisplayStatus display );
    virtual void unselect();
  public slots:
    virtual void setField( QString fieldName );
};

#endif // QGSFIELDCHOOSERCOMBO_H
