class QgsCategorizedSymbolRendererV2Widget : QgsRendererV2Widget
{
%TypeHeaderCode
#include <qgscategorizedsymbolrendererv2widget.h>
%End

  public:
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer ) /Factory/;

    QgsCategorizedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsCategorizedSymbolRendererV2Widget();

    virtual QgsFeatureRendererV2* renderer();

  public slots:
    void changeCategorizedSymbol();
    void categoryColumnChanged();
    void categoriesDoubleClicked( const QModelIndex & idx );
    void addCategories();
    void deleteCategory();
    void deleteAllCategories();
    void changeCurrentValue( QStandardItem * item );

    void rotationFieldChanged( QString fldName );
    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

  protected:

    void updateUiFromRenderer();

    void updateCategorizedSymbolIcon();

    //! populate categories view
    void populateCategories();

    //! populate column combo
    void populateColumns();

    void addCategory( const QgsRendererCategoryV2& cat );

    //! return row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! return key for the currently selected category
    QVariant currentCategory();

    void changeCategorySymbol();

    QList<QgsSymbolV2*> selectedSymbols();
    void refreshSymbolView();

  protected slots:
    void addCategory();
};
