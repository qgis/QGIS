/***************************************************************************
    qgisexpressionbuilderdialog.h - A genric expression string builder dialog.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDERDIALOG_H
#define QGSEXPRESSIONBUILDERDIALOG_H

#include <QDialog>
#include "qgsdistancearea.h"
#include "ui_qgsexpressionbuilderdialogbase.h"

/** A generic dialog for building expression strings
  * @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
  */
class GUI_EXPORT QgsExpressionBuilderDialog : public QDialog, private Ui::QgsExpressionBuilderDialogBase
{
  public:
    QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText = QString(), QWidget* parent = NULL );

    /** The builder widget that is used by the dialog */
    QgsExpressionBuilderWidget* expressionBuilder();

    void setExpressionText( const QString& text );

    QString expressionText();

    /** Sets geometry calculator used in distance/area calculations.
      * @note added in version 2.0
      */
    void setGeomCalculator( const QgsDistanceArea & da );

  protected:
    /**
     * Is called when the dialog get accepted or rejected
     * Used to save geometry
     *
     * @param r result value (unused)
     */
    virtual void done( int r );
};

#endif
