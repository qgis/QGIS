/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssearchquerybuilder.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSearchQueryBuilder : QDialog
{
%Docstring
 Query Builder for search strings
%End

%TypeHeaderCode
#include "qgssearchquerybuilder.h"
%End
  public:
    QgsSearchQueryBuilder( QgsVectorLayer *layer,
                           QWidget *parent /TransferThis/ = 0,
                           Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor - takes pointer to vector layer as a parameter
%End

    QString searchString();
%Docstring
returns newly created search string
 :rtype: str
%End

    void setSearchString( const QString &searchString );
%Docstring
change search string shown in text field
%End

  public slots:
    void on_btnEqual_clicked();
    void on_btnOk_clicked();
    void on_btnLessThan_clicked();
    void on_btnGreaterThan_clicked();
    void on_btnLike_clicked();
    void on_btnILike_clicked();
    void on_btnPct_clicked();
    void on_btnIn_clicked();
    void on_btnNotIn_clicked();

    void on_lstFields_doubleClicked( const QModelIndex &index );
    void on_lstValues_doubleClicked( const QModelIndex &index );
    void on_btnLessEqual_clicked();
    void on_btnGreaterEqual_clicked();
    void on_btnNotEqual_clicked();
    void on_btnAnd_clicked();
    void on_btnNot_clicked();
    void on_btnOr_clicked();
    void on_btnClear_clicked();

    void on_btnTest_clicked();
%Docstring
 Test the constructed search string to see if it's correct.
 The number of rows that would be returned is displayed in a message box.
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

    void saveQuery();
    void loadQuery();

};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssearchquerybuilder.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
