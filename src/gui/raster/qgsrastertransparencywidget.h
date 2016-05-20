#ifndef QGSRASTERTRANSPARENCYWIDGET_H
#define QGSRASTERTRANSPARENCYWIDGET_H

#include <QWidget>

#include "ui_qgsrastertransparencywidget.h"


class QgsRasterLayer;
class QgsRasterRenderer;
class QgsMapCanvas;
class QgsMapToolEmitPoint;
class QgsPoint;


class GUI_EXPORT QgsRasterTransparencyWidget : public QWidget, private Ui::QgsRasterTransparencyWidget
{
    Q_OBJECT
  public:
    QgsRasterTransparencyWidget( QgsRasterLayer* layer, QgsMapCanvas *canvas, QWidget *parent = 0 );
    ~QgsRasterTransparencyWidget();

  signals:
    void widgetChanged();

  public slots:
    void syncToLayer();

    void apply();

  private slots:

    void pixelSelected( const QgsPoint& canvasPoint );

    /** Transparency cell changed */
    void transparencyCellTextEdited( const QString & text );

    /** \brief slot executed when the transparency level changes. */
    void sliderTransparency_valueChanged( int theValue );

    /** \brief slot executed when user presses "Add Values From Display" button on the transparency page */
    void on_pbnAddValuesFromDisplay_clicked();

  private:
    /** \brief  A constant that signals property not used */
    const QString TRSTRING_NOT_SET;

    /** \brief Clear the current transparency table and populate the table with the correct types for current drawing mode and data type*/
    void populateTransparencyTable( QgsRasterRenderer* renderer );

    void setupTransparencyTable( int nBands );

    void setTransparencyCell( int row, int column, double value );

    void adjustTransparencyCellWidth( int row, int column );

    void setTransparencyToEdited( int row );

    double transparencyCellValue( int row, int column );

    QgsRasterLayer* mRasterLayer;

    QgsMapCanvas* mMapCanvas;

    QgsMapToolEmitPoint* mPixelSelectorTool;

    QVector<bool> mTransparencyToEdited;
};
#endif // QGSRASTERTRANSPARENCYWIDGET_H
