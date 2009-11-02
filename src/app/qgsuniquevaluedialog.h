/***************************************************************************
                         qgsuniquevaluedialog.h  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSUNIQUEVALUEDIALOG_H
#define QGSUNIQUEVALUEDIALOG_H

#include "ui_qgsuniquevaluedialogbase.h"
#include "qgssinglesymboldialog.h"
#include <map>

class QgsVectorLayer;


class QgsUniqueValueDialog: public QDialog, private Ui::QgsUniqueValueDialogBase
{
    Q_OBJECT
  public:
    QgsUniqueValueDialog( QgsVectorLayer* vl );
    ~QgsUniqueValueDialog();

  public slots:
    void apply();
    void itemChanged( QListWidgetItem *item );
    void randomizeColors();
    void resetColors();

  protected:
    /**Pointer to the associated vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Set to store the already entered values*/
    QMap<QString, QgsSymbol*> mValues;
    QgsSingleSymbolDialog sydialog;

    // Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );

  protected slots:
    /**Set new attribut for classification*/
    void changeClassificationAttribute();
    /**update single symbol dialog after selection changed*/
    void selectionChanged();
    /**add a new classes to the classification*/
    void addClass( QString value = QString::null );
    /**Removes the selected classes from the classification*/
    void deleteSelectedClasses();
    /**Writes changes in the single symbol dialog to the corresponding QgsSymbol*/
    void applySymbologyChanges();

  private:
    /** Update the list widget item icon with a preview for the symbol.
     * @param QgsSymbol * - symbol holding the style info.
     * @param QListWidgetItem * - item to get its icon updated.
     */
    void updateEntryIcon( QgsSymbol * thepSymbol, QListWidgetItem * thepItem );
    QColor randomColor();
    void setSymbolColor( QgsSymbol *symbol, QColor thecolor );

    QString mOldClassificationAttribute;
};

#endif
