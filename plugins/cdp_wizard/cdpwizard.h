/***************************************************************************
  cdpwizard.h  -  description
  -------------------
begin                : Wed May 14 2003
copyright            : (C) 2003 by Tim Sutton
email                : t.sutton@reading.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef WIZARD_H
#define WIZARD_H

// include files for Qt
#include <qwidget.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qfiledialog.h>
// application specific includes
#include "filegroup.h"
#include "climatedataprocessor.h"
#include "dataprocessor.h"
#include "filereader.h"
#include "filewriter.h"

#ifdef WIN32
#include "cdpwizardbase.h"
#else
#include "cdpwizardbase.uic.h"
#endif

//other libs
#include <sstream>
#include <vector>
#include <string>



/**This is the inherited class from the QTDesigner CDPWizardBase.ui
 *@author Tim Sutton
 */

class CDPWizard : public CDPWizardBase
{
    Q_OBJECT
public:
    /** Default constructor */
    CDPWizard();

    /** Qt style constructor where the parent widget can be set */
    CDPWizard( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );

    /** Destructor */
    ~CDPWizard();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void saveDefaults();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void loadDefaults();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void accept();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void checkInputFilenames();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnMeanTemp_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnMinTemp_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnMaxTemp_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnDiurnalTemp_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnMeanPrecipitation_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnFrostDays_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnTotalSolarRad_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void pbtnOutputPath_clicked();

    /**
    * Reimplements method by the same name in parent class
    * @param theInt - the new value for first year to be calculated.
    * @return void
    */
    void spinFirstYearToCalc_valueChanged( int theInt);

    /**
    * Reimplements method by the same name in parent class
    * @param theInt - the new value for first year contained in the input file(s).
    * @return void
    */
    void spinFirstYearInFile_valueChanged( int theInt);

    /**
    * Reimplements method by the same name in parent class
    * @param theInt - the new value for last year to be processed.
    * @return void
    */
    void spinLastYearToCalc_valueChanged( int theInt);

    /**
    * Reimplements method by the same name in parent class
    * @return void
    */
    void lstVariablesToCalc_selectionChanged();

    /**
    * Reimplements method by the same name in parent class
    * @param theQString - new selected file type - will be converted to FileReader::FileTypeEnum
    * @return void
    * @see FileReader::FileTypeEnum
    */
    void cboFileType_textChanged( const QString & theQString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for mean temperature file
    * @return void
    */
    void leMeanTemp_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for min temperature file
    * @return void
    */
    void leMinTemp_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for max temperature file
    * @return void
    */
    void leMaxTemp_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for mean diurnal temperature file
    * @return void
    */
    void leDiurnalTemp_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for mean precipitation file
    * @return void
    */
    void leMeanPrecipitation_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for frost days file
    * @return void
    */
    void leFrostDays_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for solar radiation file
    * @return void
    */
    void leTotalSolarRadiation_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theFileNameString - new filename for windspeed file
    * @return void
    */
    void leWindSpeed_textChanged( const QString & theFileNameString);

    /**
    * Reimplements method by the same name in parent class
    * @param theOutputPath - path where output files will be stored
    * @return void
    */
    void leOutputPath_textChanged( const QString & theOutputPath);

    /**
    * Sets up the climateDataProcessor and calls its run method.
    * @return void - No return.
    */
    void run();

    /**
     * Reimplements method by the same name in parent class
     * @param theYearType - format for dates either AD or BP
     * @return void
     */
    void cbxYearType_highlighted( const QString & theYearType );

private:
    /**
    * Qt mechanism alias for constructor (called after moc generated ctor)
    * @todo - remove this if possible?
    * @return void
    */
    bool initialise();

    /** Private pointer to a FileReader object
    * @todo Check if needed / used, otherwise explain what its used for
    */
    FileReader* fileReader;

    /** Private pointer to a file group object */
    FileGroup* fileGroup;

    /** debug mode helper
    * @todo Get rid of this in favour of a #define e.g.QGISDEBUG
    */
    bool debugModeFlag;

    /** Private pointer to the climate data proccessor that does all the work
    * of producing aggregates. */
    ClimateDataProcessor *climateDataProcessor;

    /**
    * This is used to mark the time at start of processing so
    * we can keep track of elapsed time.
    */
    QTime startTime;

    /**This checks whether any of the file input list boxes
     *are full and if not disables the next button
     */

public slots: // Public slots

    /** This method overrides the virtual CDPWizardBase method (slot) of the same name.
    * @param myQString - Used to notify listeners when the filetype has changed
    * @return void
    */
    void cboFileType_activated( const QString &myQString );

    /** This method overrides the virtual CDPWizardBase method (slot) of the same name.
    * It is called when each page selected event (by pressing the next button)
    * which I am reimplementing to do some housekeeping between wizard pages.
    * @param thePageNameQString - name of the newly activated page
    * @return void
    */
    void formSelected(const QString &thePageNameQString);

    //
    // This next lot of slots are for linking up to the climatedataprocessor
    //

    /**
     * A slot for notices of how many years'
     * data will be calculated.
     *@param theNumberInt - The total number of years
     *@return void - No return
     */
    void numberOfYearsToCalc(int theNumberInt);
    /**
    * A slot for notices of how many variables
    * are going to be calculated for each years data.
    *@param theNumberInt - The total number of variables
    *@return void - No return
    */
    void numberOfVariablesToCalc(int theNumberInt);
    /**
    * A slot for notices of how many cells
    * will be passed through in each block.
    *@param theNumberInt - The total number of cells in any block
    *@return void - No return
    */
    void numberOfCellsToCalc(int theNumberInt);
    /**
    * A slot for notices of that we are about to
    * start processing data for a given year.
    *@param theNameQString - A String containing the year e.g. '1998' or '20000BP'
    *@return void - No return
    */
    void yearStart(QString theNameQString);
    /**
    * A slot for notices of that we have
    * completed processing the current year.
    *@return void - No return
    */
    void yearDone();
    /**
    * A slot for notices of that we are about to
    * start calculating a variable for one years data.
    *@param theNameQString - A String containing the variable name e.g.
    *                        'Precipitation over coolest month'
    *@return void - No return
    */
    void variableStart(QString theNameQString);
    /**
    * A slot for notices of that we
    * completed calculating the given variable.
    *@param QString theFileNameString - the filename that the variable layer 
    *was saved to.
    *@return void - No return
    */
    void variableDone(QString theFileNameString);
    /**
    * A slot for notices of that we
    * have completed calculating a given cell.
    *@param theResultFloat - The calculated value for a cell
    *@return void - No return
    */
    void cellDone(float theResultFloat);

};

#endif
