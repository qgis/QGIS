/***************************************************************************
  rulesDialog.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
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
#include <QMenu>


#include "ui_rulesDialog.h"
#include "topolTest.h"

class QgisInterface;
class QgsProject;

class rulesDialog : public QDialog, private Ui::rulesDialog
{
    Q_OBJECT

  public:
    /*
     * Constructor
     * \param layerList List of layer IDs
     * \param testMap maps test names to test routines
     * \param qgisIface pointer to a QgisInterface instance
     * \param parent parent widget
     */
    rulesDialog( const QMap<QString, TopologyRule> &testMap, QgisInterface *qgisIface, QWidget *parent );

    /*
     * Returns pointer to the test table
     */
    QTableWidget *rulesTable() { return mRulesTable; }
    /*
     * Returns pointer to the test combobox
     */
    QComboBox *rulesBox() { return mRuleBox; }

    /*
     * Initialize Rules UI with layers and rules
     */
    void initGui();

  public slots:

    /*
     * Deletes all rules from rules dialog
     */
    void clearRules();

  private:
    QMap<QString, TopologyRule> mTestConfMap;
    QList<QString> mLayerIds;
    QgisInterface *mQgisIface = nullptr;
    QMenu *mContextMenu = nullptr;


    /*
     * Reads a test from the project
     * \param index test index
     * \param project pointer to QgsProject
     */
    void readTest( int index, QgsProject *project );
    /*
     * Sets the horizontal header for tet table
     */
    void setHorizontalHeaderItems();

  private slots:
    /*
     * Shows or hides controls according to test settings
     * \param testName name of the test
     */
    void showControls( const QString &testName );
    /*
     * Adds test to the table
     */
    void addRule();
    /*
     * Deletes selected test from the table
     */
    void deleteTests();
    /*
     * Reads tests from the project
     */
    void projectRead();
    /*
     * Updates Rule combobox to match first layer
     * \param layerId layer ID
     */
    void updateRuleItems( const QString &layerName );
    //! Open the associated help
    void showHelp();
};

#endif
