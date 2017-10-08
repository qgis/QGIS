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
    void btnEqual_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnPct_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();
    void btnLike_clicked();
    void btnILike_clicked();
    QString sql();
%Docstring
 :rtype: str
%End
    void setSql( const QString &sqlStatement );
    void lstFields_clicked( const QModelIndex &index );
    void lstFields_doubleClicked( const QModelIndex &index );
    void lstValues_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnNot_clicked();
    void btnOr_clicked();

    void test();
%Docstring
 Test the constructed sql statement to see if the vector layer data provider likes it.
 The number of rows that would be returned is displayed in a message box.
 The test uses a "select count(*) from ..." query to test the SQL
 statement.
%End

    void btnGetAllValues_clicked();
%Docstring
 Get all distinct values for the field. Values are inserted
 into the value list box
%End

    void btnSampleValues_clicked();
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
