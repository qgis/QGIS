class QgsProjectionSelectionTreeWidget : QWidget
{
%TypeHeaderCode
#include <qgsprojectionselectiontreewidget.h>
%End

  public:
    QgsProjectionSelectionTreeWidget( QWidget* parent /TransferThis/ );

    ~QgsProjectionSelectionTreeWidget();

    QgsCoordinateReferenceSystem crs() const;

  public slots:

    void setCrs( const QgsCoordinateReferenceSystem& crs );
    void setOgcWmsCrsFilter( const QSet<QString>& crsFilter );
    void pushProjectionToFront();

  signals:

    void crsSelected();
    void initialized();
    void projectionDoubleClicked();

  protected:
    void showEvent( QShowEvent * event );

    void resizeEvent( QResizeEvent * event );
};

