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
    void btnEqual_clicked();
    void on_btnOk_clicked();
    void btnLessThan_clicked();
    void btnGreaterThan_clicked();
    void btnLike_clicked();
    void btnILike_clicked();
    void btnPct_clicked();
    void btnIn_clicked();
    void btnNotIn_clicked();

    void lstFields_doubleClicked( const QModelIndex &index );
    void lstValues_doubleClicked( const QModelIndex &index );
    void btnLessEqual_clicked();
    void btnGreaterEqual_clicked();
    void btnNotEqual_clicked();
    void btnAnd_clicked();
    void btnNot_clicked();
    void btnOr_clicked();
    void btnClear_clicked();

    void btnTest_clicked();
%Docstring
 Test the constructed search string to see if it's correct.
 The number of rows that would be returned is displayed in a message box.
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
