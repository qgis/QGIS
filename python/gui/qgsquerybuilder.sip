/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsquerybuilder.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsQueryBuilder : QDialog
{
%Docstring
 Query Builder for layers.

 The query builder allows interactive creation of a SQL for limiting the
 features displayed in a vector layer.  The fields in the table are
 displayed and sample values (or all values) can be viewed to aid in
 constructing the query. A test function returns the number of features that
 will be returned.
%End

%TypeHeaderCode
#include "qgsquerybuilder.h"
%End
  public:

    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent /TransferThis/ = 0,
                     Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
 This constructor is used when the query builder is called from the
 vector layer properties dialog
 \param layer existing vector layer
 \param parent Parent widget
 \param fl dialog flags
%End

    ~QgsQueryBuilder();

    virtual void showEvent( QShowEvent *event );


  public slots:
    virtual void accept();

    virtual void reject();

    void clear();
    void on_btnEqual_clicked();
    void on_btnLessThan_clicked();
    void on_btnGreaterThan_clicked();
    void on_btnPct_clicked();
    void on_btnIn_clicked();
    void on_btnNotIn_clicked();
    void on_btnLike_clicked();
    void on_btnILike_clicked();
    QString sql();
%Docstring
 :rtype: str
%End
    void setSql( const QString &sqlStatement );
    void on_lstFields_clicked( const QModelIndex &index );
    void on_lstFields_doubleClicked( const QModelIndex &index );
    void on_lstValues_doubleClicked( const QModelIndex &index );
    void on_btnLessEqual_clicked();
    void on_btnGreaterEqual_clicked();
    void on_btnNotEqual_clicked();
    void on_btnAnd_clicked();
    void on_btnNot_clicked();
    void on_btnOr_clicked();

    void test();
%Docstring
 Test the constructed sql statement to see if the vector layer data provider likes it.
 The number of rows that would be returned is displayed in a message box.
 The test uses a "select count(*) from ..." query to test the SQL
 statement.
%End

    void on_btnGetAllValues_clicked();
%Docstring
 Get all distinct values for the field. Values are inserted
 into the value list box
%End

    void on_btnSampleValues_clicked();
%Docstring
 Get sample distinct values for the selected field. The sample size is
 limited to an arbitrary value (currently set to 25). The values
 are inserted into the values list box.
%End

    void setDatasourceDescription( const QString &uri );

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsquerybuilder.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
