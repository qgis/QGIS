/***************************************************************************
    qgssmartgroupeditordialog.h
    ---------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSMARTGROUPCONDITION_H
#define QGSSMARTGROUPCONDITION_H

#include "ui_qgssmartgroupconditionwidget.h"

class GUI_EXPORT QgsSmartGroupCondition : public QWidget, private Ui::QgsSmartGroupConditionWidget
{
    Q_OBJECT

  public:
    QgsSmartGroupCondition( int id,  QWidget *parent = NULL );

    //! returns the constraint key
    QString constraint();

    //! returns the parameter
    QString parameter();

    //! sets the given constraint
    void setConstraint( QString constraint );

    //! sets the given param
    void setParameter( QString param );

    //! sets the remove button hidden state to 'hide'
    void hideRemoveButton( bool hide );

  public slots:
    void destruct();

  signals:
    void removed( int );

  protected:
    int mConditionId;

};

#endif //QGSSMARTGROUPCONDITION


#ifndef QGSSMARTGROUPEDITORDIALOG_H
#define QGSSMARTGROUPEDITORDIALOG_H

#include <QDialog>

#include "ui_qgssmartgroupeditordialogbase.h"
//#include "qgscontexthelp.h"

#include "qgsstylev2.h" //for QgsSmartConditionMap

class GUI_EXPORT QgsSmartGroupEditorDialog : public QDialog, private Ui::QgsSmartGroupEditorDialogBase
{
    Q_OBJECT

  public:
    QgsSmartGroupEditorDialog( QgsStyleV2* style, QWidget* parent = NULL );
    ~QgsSmartGroupEditorDialog();

    //! returns the value from mNameLineEdit
    QString smartgroupName();

    //! returns the condition map
    QgsSmartConditionMap conditionMap();

    //! returns the AND/OR condition
    QString conditionOperator();

    //! sets up the GUI for the given conditionmap
    void setConditionMap( QgsSmartConditionMap );

    //! sets the operator AND/OR
    void setOperator( QString );

    //! sets the smart group Name
    void setSmartgroupName( QString );

  public slots:

    //! function to create a new ConditionBox and update UI
    void addCondition();

    //! slot to remove the condition with id int
    void removeCondition( int );

    void on_buttonBox_accepted();

  protected:
    QgsStyleV2* mStyle;
    // layout of the mConditionsBox
    QGridLayout *mLayout;
    // counter for the number of conditions
    int mCondCount;
    // map tracking the condition widget and the ids
    QMap<int,QgsSmartGroupCondition*> mConditionMap;
};

#endif // QGSSMARTGROUPEDITORDIALOG_H

