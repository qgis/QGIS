class QgsSearchQueryBuilder : QDialog
{
%TypeHeaderCode
#include <qgssearchquerybuilder.h>
%End

  public:
    //! Constructor - takes pointer to vector layer as a parameter
    QgsSearchQueryBuilder( QgsVectorLayer* layer, QWidget *parent /TransferThis/ = 0,
                           Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    ~QgsSearchQueryBuilder();

    //! returns newly created search string
    QString searchString();

    //! change search string shown in text field
    void setSearchString( QString searchString );

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

    /*! Test the constructed search string to see if it's correct.
     * The number of rows that would be returned is displayed in a message box.
     */
    void on_btnTest_clicked();

    /*!
     * Get all distinct values for the field. Values are inserted
     * into the value list box
     */
    void on_btnGetAllValues_clicked();

    /*!
     * Get sample distinct values for the selected field. The sample size is
     * limited to an arbitrary value (currently set to 25). The values
     * are inserted into the values list box.
     */
    void on_btnSampleValues_clicked();

    void on_buttonBox_helpRequested();

    void saveQuery();
    void loadQuery();

};
