class QgsSmartGroupCondition : QWidget
{
%TypeHeaderCode
#include <qgssmartgroupeditordialog.h>
%End

  public:
    QgsSmartGroupCondition( int id,  QWidget *parent /TransferThis/ = NULL );

    //! returns the constraint key
    QString constraint();

    //! returns the parameter
    QString parameter();

    //! sets the given constraint
    void setConstraint( const QString& constraint );

    //! sets the given param
    void setParameter( const QString& param );

    //! sets the remove button hidden state to 'hide'
    void hideRemoveButton( bool hide );

  public slots:
    void destruct();

  signals:
    void removed( int );
};

class QgsSmartGroupEditorDialog : QDialog
{
%TypeHeaderCode
#include <qgssmartgroupeditordialog.h>
%End

  public:
    QgsSmartGroupEditorDialog( QgsStyleV2* style, QWidget* parent /TransferThis/ = NULL );
    ~QgsSmartGroupEditorDialog();

    //! returns the value from mNameLineEdit
    QString smartgroupName();

    //! returns the condition map
    //! @note not available in python bindings
    // QgsSmartConditionMap conditionMap();

    //! returns the AND/OR condition
    QString conditionOperator();

    //! sets up the GUI for the given conditionmap
    //! @note not available in python bindings
    // void setConditionMap( const QgsSmartConditionMap& );

    //! sets the operator AND/OR
    void setOperator( const QString& );

    //! sets the smart group Name
    void setSmartgroupName( const QString& );

  public slots:

    //! function to create a new ConditionBox and update UI
    void addCondition();

    //! slot to remove the condition with id int
    void removeCondition( int );

    void on_buttonBox_accepted();
};
