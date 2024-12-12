/***************************************************************************
                       qgshttpheaderswidget.h
  This class implements simple UI for http header.

                              -------------------
          begin                : 2021-09-09
          copyright            : (C) 2021 B. De Mezzo
          email                : benoit dot de dot mezzo at oslandia dot com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHTTPHEADERWIDGET_H
#define QGSHTTPHEADERWIDGET_H

#include <QWidget>
#include "ui_qgshttpheaderwidget.h"
#include "qgshttpheaders.h"


/**
 * \ingroup gui
 * \class QgsHttpHeaderWidget
 * \brief Display referer http header field and collapsible table of key/value pairs
 *
 * \since QGIS 3.24
 */
class GUI_EXPORT QgsHttpHeaderWidget : public QWidget, private Ui::QgsHttpHeaderWidget
{
    Q_OBJECT

  public:
    /**
     * Default constructor
     * \param parent parent widget
     */
    explicit QgsHttpHeaderWidget( QWidget *parent = nullptr );
    ~QgsHttpHeaderWidget();

    /**
     * \return build a new \a QgsHttpHeaders according to data in the UI
     */
    QgsHttpHeaders httpHeaders() const;

    /**
     * Sets the widget from the headers
     * \since QGIS 3.30
     */
    void setHeaders( const QgsHttpHeaders &headers );

    /**
     * \brief fill the inner header map from the settings defined at \a key
     * \see QgsHttpHeaders::setFromSettings()
     * \param settings
     * \param key
     * \deprecated QGIS 3.30. Use setFromHeaders() instead.
     */
    Q_DECL_DEPRECATED void setFromSettings( const QgsSettings &settings, const QString &key ) SIP_DEPRECATED;

    /**
     * \brief update the \a settings with the http headers present in the inner map.
     * \see QgsHttpHeaders::updateSettings()
     * \param settings
     * \param key
     * \deprecated QGIS 3.30. Use a variant map for settinds directly.
     */
    Q_DECL_DEPRECATED void updateSettings( QgsSettings &settings, const QString &key ) const SIP_DEPRECATED;

  private slots:

    /**
     * add a new key/value http header pair in the table
     */
    void addQueryPair();

    /**
     * remove a key/value http header pair from the table
     */
    void removeQueryPair();

  private:
    void addQueryPairRow( const QString &key, const QString &val );
};

#endif // QGSHTTPHEADERWIDGET_H
