class QgsQueryBuilder : QDialog
{
%TypeHeaderCode
#include <qgsquerybuilder.h>
%End

  public:
    /** This constructor is used when the query builder is called from the
     * vector layer properties dialog
     * @param layer existing vector layer
     * @param parent Parent widget
     * @param fl dialog flags
     */
    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent /TransferThis/ = 0,
                     Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    ~QgsQueryBuilder();

    void showEvent( QShowEvent *event );

  public slots:
    void accept();
    void reject();
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
    void setSql( QString sqlStatement );
    void on_lstFields_clicked( const QModelIndex &index );
    void on_lstFields_doubleClicked( const QModelIndex &index );
    void on_lstValues_doubleClicked( const QModelIndex &index );
    void on_btnLessEqual_clicked();
    void on_btnGreaterEqual_clicked();
    void on_btnNotEqual_clicked();
    void on_btnAnd_clicked();
    void on_btnNot_clicked();
    void on_btnOr_clicked();

    void on_buttonBox_helpRequested();

    /** Test the constructed sql statement to see if the database likes it.
     * The number of rows that would be returned is displayed in a message box.
     * The test uses a "select count(*) from ..." query to test the SQL
     * statement.
     */
    void test();
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

    void setDatasourceDescription( QString uri );
};
