/***************************************************************************
    qgscolorschemelist.h
    --------------------
    Date                 : August 2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORSCHEMELIST_H
#define QGSCOLORSCHEMELIST_H

#include "qgscolorscheme.h"
#include <QTreeView>
#include <QAbstractItemModel>
#include <QItemDelegate>
#include <QFile>

class QMimeData;

/** \ingroup gui
 * \class QgsColorSwatchDelegate
 * A delegate for showing a color swatch in a list
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class GUI_EXPORT QgsColorSwatchDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

  public:
    QgsColorSwatchDelegate( QWidget *parent = nullptr );
    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QSize sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index ) override;

  private:
    QWidget* mParent;

    /** Generates a checkboard pattern for transparent color backgrounds
     * @returns checkboard pixmap
     */
    const QPixmap &transparentBackground() const;
};


/** \ingroup gui
 * \class QgsColorSchemeModel
 * A model for colors in a color scheme
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class GUI_EXPORT QgsColorSchemeModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    /** Constructor
     * @param scheme color scheme for list
     * @param context context string for color scheme
     * @param baseColor base color for color scheme
     * @param parent parent object
     */
    explicit QgsColorSchemeModel( QgsColorScheme* scheme, const QString &context = QString(), const QColor &baseColor = QColor(), QObject* parent = nullptr );

    ~QgsColorSchemeModel();

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex & index ) const override;
    bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() ) override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    /** Returns a list of colors shown in the widget
     * @returns colors shown in the widget
     */
    QgsNamedColorList colors() const { return mColors; }

    /** Sets the color scheme to show in the widget
     * @param scheme color scheme
     * @param context context for color scheme
     * @param baseColor base color for color scheme
     */
    void setScheme( QgsColorScheme* scheme, const QString &context = QString(), const QColor &baseColor = QColor() );

    /** Get the current color scheme context for the model
     * @returns context string which is passed to scheme for color generation
     * @see baseColor
     */
    QString context() const { return mContext; }

    /** Get the base color for the color scheme used by the model
     * @returns base color which is passed to scheme for color generation
     * @see context
     */
    QColor baseColor() const { return mBaseColor; }

    /** Add a color to the list
     * @param color color to add
     * @param label label for color
     */
    void addColor( const QColor &color, const QString &label = QString() );

    /** Returns whether the color scheme model has been modified
     * @returns true if colors have been modified
     */
    bool isDirty() const { return mIsDirty; }

  private:

    enum Columns
    {
      ColorSwatch = 0,
      ColorLabel
    };

    QgsNamedColorList mColors;
    QgsColorScheme* mScheme;
    QString mContext;
    QColor mBaseColor;
    bool mIsDirty;
};

/** \ingroup gui
 * \class QgsColorSchemeList
 * An editable list of color swatches, taken from an associated QgsColorScheme.
 * @see QgsColorSchemeList
 * @note introduced in QGIS 2.5
 */
class GUI_EXPORT QgsColorSchemeList: public QTreeView
{
    Q_OBJECT

  public:

    /** Construct a new color swatch grid.
     * @param parent parent widget
     * @param scheme QgsColorScheme for colors to show in the list
     * @param context context string provided to color scheme
     * @param baseColor base color for color scheme
     */
    QgsColorSchemeList( QWidget *parent = nullptr, QgsColorScheme* scheme = nullptr, const QString &context = QString(), const QColor &baseColor = QColor() );

    virtual ~QgsColorSchemeList();

    /** Saves the current colors shown in the list back to a color scheme, if supported
     * by the color scheme.
     * @note this method is only effective if the color scheme is editable
     */
    bool saveColorsToScheme();

    /** Import colors from a GPL palette file to the list
     * @param file file to import
     * @see exportColorsToGpl
     */
    bool importColorsFromGpl( QFile &file );

    /** Export colors to a GPL palette file from the list
     * @param file destination file
     * @see importColorsFromGpl
     */
    bool exportColorsToGpl( QFile &file );

    /** Returns whether the color scheme list has been modified
     * @returns true if colors have been modified
     */
    bool isDirty() const;

  public slots:

    /** Sets the color scheme to show in the list
     * @param scheme QgsColorScheme for colors to show in the list
     * @param context context string provided to color scheme
     * @param baseColor base color for color scheme
     */
    void setScheme( QgsColorScheme* scheme, const QString &context = QString(), const QColor &baseColor = QColor() );

    /** Removes any selected colors from the list
     */
    void removeSelection();

    /** Adds a color to the list
     * @param color color to add
     * @param label optional label for color
     */
    void addColor( const QColor &color, const QString &label = QString() );

    /** Pastes colors from clipboard to the list
     * @see copyColors
     */
    void pasteColors();

    /** Copies colors from the list to the clipboard
     * @see pasteColors
     */
    void copyColors();

  signals:

    /** Emitted when a color is selected from the list
     * @param color color selected
     */
    void colorSelected( const QColor &color );

  protected:

    void keyPressEvent( QKeyEvent* event ) override;

    void mousePressEvent( QMouseEvent* event ) override;

    void mouseReleaseEvent( QMouseEvent* event ) override;

  private:
    QgsColorScheme* mScheme;
    QgsColorSchemeModel* mModel;
    QgsColorSwatchDelegate* mSwatchDelegate;

    QPoint mDragStartPosition;

};

#endif
