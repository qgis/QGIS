/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssmartgroupeditordialog.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsSmartGroupCondition : QWidget
{

%TypeHeaderCode
#include "qgssmartgroupeditordialog.h"
%End
  public:
    QgsSmartGroupCondition( int id,  QWidget *parent /TransferThis/ = 0 );

    QString constraint();
%Docstring
returns the constraint key
%End

    QString parameter();
%Docstring
returns the parameter
%End

    void setConstraint( const QString &constraint );
%Docstring
sets the given constraint
%End

    void setParameter( const QString &param );
%Docstring
sets the given param
%End

    void hideRemoveButton( bool hide );
%Docstring
sets the remove button hidden state to 'hide'
%End

  public slots:
    void destruct();

  signals:
    void removed( int );

  protected:

};





class QgsSmartGroupEditorDialog : QDialog
{

%TypeHeaderCode
#include "qgssmartgroupeditordialog.h"
%End
  public:
    QgsSmartGroupEditorDialog( QgsStyle *style, QWidget *parent /TransferThis/ = 0 );

    QString smartgroupName();
%Docstring
returns the value from mNameLineEdit
%End


    QString conditionOperator();
%Docstring
returns the AND/OR condition
%End


    void setOperator( const QString & );
%Docstring
sets the operator AND/OR
%End

    void setSmartgroupName( const QString & );
%Docstring
sets the smart group Name
%End

  public slots:

    void addCondition();
%Docstring
function to create a new ConditionBox and update UI
%End

    void removeCondition( int );
%Docstring
slot to remove the condition with id int
%End

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/symbology/qgssmartgroupeditordialog.h                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
