/***************************************************************************
    qgsvaluemapconfigdlg.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUEMAPCONFIGDLG_H
#define QGSVALUEMAPCONFIGDLG_H

#include "ui_qgsvaluemapconfigdlgbase.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

class QComboBox;

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsValueMapConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsValueMapConfigDlg : public QgsEditorConfigWidget, private Ui::QgsValueMapWidget
{
    Q_OBJECT

  public:
    explicit QgsValueMapConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent );
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;

    /**
     * Updates the displayed table with the values from \a map.
     * If \a insertNull is set to TRUE, it will also insert a NULL value.
     *
     * \note In most cases the overload that accepts a list is preferred as it
     * keeps the order of the values.
     */
    void updateMap( const QMap<QString, QVariant> &map, bool insertNull );

    /**
     * Updates the displayed table with the values from \a list, the order of the values
     * is preserved.
     * If \a insertNull is set to TRUE, it will also insert a NULL value.
     *
     * \since QGIS 3.12
     */
    void updateMap( const QList<QPair<QString, QVariant>> &list, bool insertNull );

    /**
     * Updates the displayed table with the values from a CSV file.
     * \param filePath the absolute file path of the CSV file.
     * \since QGIS 3.24
     */
    void loadMapFromCSV( const QString &filePath );

    /**
     * Populates a \a comboBox with the appropriate entries based on a value map \a configuration.
     *
     * If \a skipNull is TRUE, then NULL entries will not be added.
     *
     */
    static void populateComboBox( QComboBox *comboBox, const QVariantMap &configuration, bool skipNull );

    bool eventFilter( QObject *watched, QEvent *event ) override;

  private:
    void setRow( int row, const QString &value, const QString &description );

    /**
     * Validates a value against the maximum allowed field length and trims it is necessary.
     * \param value
     * \return the validated field value trimmed if necessary
     */
    QString checkValueLength( const QString &value );


  private slots:
    void copySelectionToClipboard();
    void vCellChanged( int row, int column );
    void addNullButtonPushed();
    void removeSelectedButtonPushed();
    void loadFromLayerButtonPushed();
    void loadFromCSVButtonPushed();
};

#endif // QGSVALUEMAPCONFIGDLG_H
