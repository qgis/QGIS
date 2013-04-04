/***************************************************************************
  rulesDialog.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RULESDIALOG_H_
#define RULESDIALOG_H_

#include <QDialog>

#include <qgsvectorlayer.h>

#include "ui_rulesDialog.h"
#include "topolTest.h"

class QgisInterface;
class QgsMapLayerRegistry;

class rulesDialog : public QDialog, public Ui::rulesDialog
{
    Q_OBJECT

  public:
    /*
     * Constructor
     * @param layerList List of layer IDs
     * @param testMap maps test names to test routines
     * @param theQgisIface pointer to a QgisInterface instance
     * @param parent parent widget
     */
    rulesDialog( QList<QString> layerList, QMap<QString, TopologyRule> testMap, QgisInterface* theQgisIface, QWidget *parent );
    ~rulesDialog();
    /*
     * Returns pointer to the test table
     */
    QTableWidget* testTable() { return mTestTable; }
    /*
     * Returns pointer to the test combobox
     */
    QComboBox* testBox() { return mTestBox; }

  private:
    QMap<QString, TopologyRule> mTestConfMap;
    QList<QString> mLayerIds;
    QgisInterface* mQgisIface;

    /*
     * Reads a test from the project
     * @param index test index
     * @param layerRegistry pointer to a QgsMapLayerRegistry instance
     */
    void readTest( int index, QgsMapLayerRegistry* layerRegistry );
    /*
     * Sets the horizontal header for tet table
     */
    void setHorizontalHeaderItems();

  private slots:
    /*
     * Shows or hides controls according to test settings
     * @param testName name of the test
     */
    void showControls( const QString& testName );
    /*
     * Adds test to the table
     */
    void addTest();
    /*
     * Deletes test from the table
     */
    void deleteTest();
    /*
     * Reads tests from the project
     */
    void projectRead();
    /*
     * Adds layer to layer comboboxes
     * @param layer layer pointer
     */
    void addLayer( QgsMapLayer* layer );
    /*
     * Deletes layer to layer comboboxes
     * @param layerId layer ID
     */
    void removeLayer( QString layerId );


    /*
     * Updates Rule combobox to mach first layer
     * @param layerId layer ID
     */
    void updateRuleItems( const QString& layerName );

};

#endif
