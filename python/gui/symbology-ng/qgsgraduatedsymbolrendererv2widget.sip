class QgsGraduatedSymbolRendererV2Widget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgsgraduatedsymbolrendererv2widget.h>
%End
  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

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
    /**Removes currently selected classes */
    void deleteClasses();
    /**Removes all classes from the classification*/
    void deleteAllClasses();

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

    void rowsMoved();

  protected:
    void updateUiFromRenderer();

    void updateGraduatedSymbolIcon();

    //! return a list of indexes for the classes under selection
    QList<int> selectedClasses();

    //! populate column combos in categorized and graduated page
    void populateColumns();

    void changeRangeSymbol( int rangeIdx );
    void changeRange( int rangeIdx );

    void changeSelectedSymbols();

    QList<QgsSymbolV2*> selectedSymbols();
    QgsSymbolV2* findSymbolForRange( double lowerBound, double upperBound, const QgsRangeList& ranges ) const;
    void refreshSymbolView();
};
