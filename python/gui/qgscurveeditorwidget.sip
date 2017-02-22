class QgsCurveEditorWidget : QWidget
{
%TypeHeaderCode
#include <qgscurveeditorwidget.h>
%End

  public:

    QgsCurveEditorWidget( QWidget* parent /TransferThis/ = 0, const QgsCurveTransform& curve = QgsCurveTransform() );
    ~QgsCurveEditorWidget();

    QgsCurveTransform curve() const;

    void setCurve( const QgsCurveTransform& curve );
    void setHistogramSource( const QgsVectorLayer* layer, const QString& expression );
    double minHistogramValueRange() const;
    double maxHistogramValueRange() const;

  public slots:
    void setMinHistogramValueRange( double minValueRange );
    void setMaxHistogramValueRange( double maxValueRange );

  signals:

    void changed();

  protected:

    virtual void keyPressEvent( QKeyEvent *event );

};
