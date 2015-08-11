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

    /** Replaces category symbols with the symbols from a style that have a matching
     * name.
     * @param style style containing symbols to match with
     * @return number of symbols matched
     * @see matchToSymbolsFromLibrary
     * @see matchToSymbolsFromXml
     * @note added in QGIS 2.9
     */
    int matchToSymbols( QgsStyleV2* style );

  public slots:
    void changeCategorizedSymbol();
    void categoryColumnChanged( QString field );
    void categoriesDoubleClicked( const QModelIndex & idx );
    void addCategory();
    void addCategories();
    void applyColorRamp();
    void deleteCategories();
    void deleteAllCategories();

    void sizeScaleFieldChanged( QString fldName );
    void scaleMethodChanged( QgsSymbolV2::ScaleMethod scaleMethod );

    void showSymbolLevels();

    void rowsMoved();

    /** Replaces category symbols with the symbols from the users' symbol library that have a
     * matching name.
     * @see matchToSymbolsFromXml
     * @see matchToSymbols
     * @note added in QGIS 2.9
     */
    void matchToSymbolsFromLibrary();

    /** Prompts for selection of an xml file, then replaces category symbols with the symbols
     * from the XML file with a matching name.
     * @see matchToSymbolsFromLibrary
     * @see matchToSymbols
     * @note added in QGIS 2.9
     */
    void matchToSymbolsFromXml();

  protected:

    void updateUiFromRenderer();

    void updateCategorizedSymbolIcon();

    // Called by virtual refreshSymbolView()
    void populateCategories();

    //! return row index for the currently selected category (-1 if on no selection)
    int currentCategoryRow();

    //! return a list of indexes for the categories unders selection
    QList<int> selectedCategories();

    //! change the selected symbols alone for the change button, if there is a selection
    void changeSelectedSymbols();

    void changeCategorySymbol();

    QgsVectorColorRampV2* getColorRamp();

    QList<QgsSymbolV2*> selectedSymbols();
    QgsCategoryList selectedCategoryList();
    void refreshSymbolView();
    void keyPressEvent( QKeyEvent* event );
};
