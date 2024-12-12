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

#ifndef QGSSMARTGROUPEDITORDIALOG_H
#define QGSSMARTGROUPEDITORDIALOG_H

#include "ui_qgssmartgroupconditionwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsSmartGroupCondition
 */
class GUI_EXPORT QgsSmartGroupCondition : public QWidget, private Ui::QgsSmartGroupConditionWidget
{
    Q_OBJECT

  public:
    QgsSmartGroupCondition( int id, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! returns the constraint key
    QString constraint();

    //! returns the parameter
    QString parameter();

    //! sets the given constraint
    void setConstraint( const QString &constraint );

    //! sets the given param
    void setParameter( const QString &param );

    //! sets the remove button hidden state to 'hide'
    void hideRemoveButton( bool hide );

  public slots:
    void destruct();

  signals:

    /**
     * Emitted when the group with the specified \a id is removed.
     */
    void removed( int id );

  protected:
    int mConditionId;
};


#include <QDialog>

#include "ui_qgssmartgroupeditordialogbase.h"

#include "qgsstyle.h" //for QgsSmartConditionMap

/**
 * \ingroup gui
 * \class QgsSmartGroupEditorDialog
 */
class GUI_EXPORT QgsSmartGroupEditorDialog : public QDialog, private Ui::QgsSmartGroupEditorDialogBase
{
    Q_OBJECT

  public:
    QgsSmartGroupEditorDialog( QgsStyle *style, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    //! returns the value from mNameLineEdit
    QString smartgroupName();

    /**
     * returns the condition map
     * \note not available in Python bindings
     */
    QgsSmartConditionMap conditionMap() SIP_SKIP;

    //! returns the AND/OR condition
    QString conditionOperator();

    /**
     * sets up the GUI for the given conditionmap
     * \note not available in Python bindings
     */
    void setConditionMap( const QgsSmartConditionMap & ) SIP_SKIP;

    //! sets the operator AND/OR
    void setOperator( const QString & );

    //! sets the smart group Name
    void setSmartgroupName( const QString & );

  public slots:

    //! function to create a new ConditionBox and update UI
    void addCondition();

    //! slot to remove the condition with id int
    void removeCondition( int );

  protected:
    QgsStyle *mStyle = nullptr;
    // layout of the mConditionsBox
    QGridLayout *mLayout = nullptr;
    // counter for the number of conditions
    int mCondCount;
    // map tracking the condition widget and the ids
    QMap<int, QgsSmartGroupCondition *> mConditionMap;

  private slots:

    void buttonBox_accepted();
};

#endif // QGSSMARTGROUPEDITORDIALOG_H
