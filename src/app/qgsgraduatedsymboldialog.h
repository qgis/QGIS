/***************************************************************************
                         qgsgraduatedsymboldialog.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRADUATEDSYMBOLDIALOG_H
#define QGSGRADUATEDSYMBOLDIALOG_H

#include "ui_qgsgraduatedsymboldialogbase.h"
#include "qgssinglesymboldialog.h"
#include <map>

class QgsVectorLayer;


class QgsGraduatedSymbolDialog: public QDialog, private Ui::QgsGraduatedSymbolDialogBase
{
    Q_OBJECT
  public:
    /**Enumeration describing the automatic settings of values*/
    enum mode {EMPTY, EQUAL_INTERVAL, QUANTILES};
    QgsGraduatedSymbolDialog( QgsVectorLayer* layer );
    ~QgsGraduatedSymbolDialog();
  public slots:
    void apply();
  protected slots:
    /**Changes only the number of classes*/
    void adjustNumberOfClasses();
    /**Sets a new classification field and a new classification mode*/
    void adjustClassification();
    /**Changes the display of the single symbol dialog*/
    void changeCurrentValue();
    /**Writes changes in the single symbol dialog to the corresponding QgsRangeRenderItem*/
    void applySymbologyChanges();
    /**Shows a dialog to modify lower and upper values*/
    void modifyClass( QListWidgetItem* item );
  protected:
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Stores the names and numbers of the fields with numeric values*/
    std::map<QString, int> mFieldMap;
    /**Stores the classes*/
    std::map<QString, QgsSymbol*> mEntries;
    /**Dialog which shows the settings of the activated class*/
    QgsSingleSymbolDialog sydialog;
    int mClassificationField;

    /**Calculates quantiles from mVectorLayer.
     @return 0 in case of success*/
    int quantilesFromVectorLayer( std::list<double>& result, int attributeIndex, int numQuantiles ) const;
    /**A function that calculates the values of the quantiles
    @param result the list where the function inserts the result values
    @param values a _sorted_ vector of variable values
    @param numQuantiles the number of quantiles, e.g. 4 calculates the quantiles for 25%, 50%, 75%, 100%
     @return 0 in case of success*/
    int calculateQuantiles( std::list<double>& result, const std::vector<double>& values, int numQuantiles ) const;
    /**Gets the color value along a specified ramp**/
    QColor getColorFromRamp( QString ramp, int step, int totalSteps );

    // Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

  protected slots:
    /**Removes a class from the classification*/
    void deleteCurrentClass();

  private:
    /** Update the list widget item icon with a preview for the symbol.
     * @param QgsSymbol * - symbol holding the style info.
     * @param QListWidgetItem * - item to get its icon updated.
     */
    void updateEntryIcon( QgsSymbol * thepSymbol, QListWidgetItem * thepItem );
    /**Default constructor is privat to not use is*/
    QgsGraduatedSymbolDialog();
};

#endif
