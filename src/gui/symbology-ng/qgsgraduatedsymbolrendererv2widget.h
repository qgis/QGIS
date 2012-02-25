#ifndef QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H
#define QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H

#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsrendererv2widget.h"
#include <QStandardItem>

#include "ui_qgsgraduatedsymbolrendererv2widget.h"

class GUI_EXPORT QgsGraduatedSymbolRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsGraduatedSymbolRendererV2Widget
{
    Q_OBJECT

  public:

    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    QgsGraduatedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsGraduatedSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:
    void changeGraduatedSymbol();
    void graduatedColumnChanged();
    void classifyGraduated();
    void reapplyColorRamp();
    void rangesDoubleClicked( const QModelIndex & idx );
    void rangesClicked( const QModelIndex & idx );
    void changeCurrentValue( QStandardItem * item );

    /**Adds a class manually to the classification*/
    void addClass();
    /**Removes a class from the classification*/
    void deleteCurrentClass();

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );

    void showSymbolLevels();

  protected:
    void updateUiFromRenderer();

    void updateGraduatedSymbolIcon();

    //! populate column combos in categorized and graduated page
    void populateColumns();

    void populateColorRamps();

    //! populate ranges of graduated symbol renderer
    void populateRanges();

    void changeRangeSymbol( int rangeIdx );
    void changeRange( int rangeIdx );

    QList<QgsSymbolV2*> selectedSymbols();
    QgsSymbolV2* findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList& ranges ) const;
    void refreshSymbolView();


  protected:
    QgsGraduatedSymbolRendererV2* mRenderer;

    QgsSymbolV2* mGraduatedSymbol;

    int mRowSelected;

    QgsRendererV2DataDefinedMenus* mDataDefinedMenus;

};


#endif // QGSGRADUATEDSYMBOLRENDERERV2WIDGET_H
