// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfdocumenttextfloweditormodel.h"
#include "pdfdocumenttextflow.h"

#include <QColor>
#include <QBrush>

#include "pdfdbgheap.h"

namespace pdf
{

PDFDocumentTextFlowEditorModel::PDFDocumentTextFlowEditorModel(QObject* parent) :
    BaseClass(parent),
    m_editor(nullptr)
{

}

PDFDocumentTextFlowEditorModel::~PDFDocumentTextFlowEditorModel()
{

}

QVariant PDFDocumentTextFlowEditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
    {
        return BaseClass::headerData(section, orientation, role);
    }

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
            case ColumnPageNo:
                return tr("Page No.");

            case ColumnType:
                return tr("Type");

            case ColumnState:
                return tr("State");

            case ColumnText:
                return tr("Text");

            default:
                Q_ASSERT(false);
                break;
        }
    }

    return BaseClass::headerData(section, orientation, role);
}

int PDFDocumentTextFlowEditorModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    if (m_editor)
    {
        return int(m_editor->getItemCount());
    }

    return 0;
}

int PDFDocumentTextFlowEditorModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return ColumnLast;
}

QVariant PDFDocumentTextFlowEditorModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_editor)
    {
        return QVariant();
    }

    if (role == Qt::BackgroundRole)
    {
        if (m_editor->isSelected(index.row()))
        {
            return QBrush(QColor(255, 255, 200));
        }
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
            case ColumnPageNo:
            {
                PDFInteger pageIndex = m_editor->getPageIndex(index.row());
                if (pageIndex >= 0)
                {
                    return QString::number(pageIndex + 1);
                }

                return QVariant();
            }

            case ColumnType:
            {
                if (m_editor->isItemTypeTitle(index.row()))
                {
                    return tr("Title");
                }
                if (m_editor->isItemTypeLanguage(index.row()))
                {
                    return tr("Language");
                }
                if (m_editor->isItemTypeSpecial(index.row()))
                {
                    return tr("Special");
                }

                return tr("Text");
            }

            case ColumnState:
            {
                const bool isModified = m_editor->isModified(index.row());
                const bool isRemoved = m_editor->isRemoved(index.row());

                if (isRemoved)
                {
                    return tr("Removed");
                }

                if (isModified)
                {
                    return tr("Modified");
                }

                return tr("Active");
            }

            case ColumnText:
                return m_editor->getText(index.row());

            default:
                Q_ASSERT(false);
                break;
        }
    }

    return QVariant();
}

bool PDFDocumentTextFlowEditorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole && index.column() == ColumnText)
    {
        m_editor->setText(value.toString(), index.row());
        return true;
    }

    return false;
}

Qt::DropActions PDFDocumentTextFlowEditorModel::supportedDropActions() const
{
    return Qt::DropAction();
}

Qt::DropActions PDFDocumentTextFlowEditorModel::supportedDragActions() const
{
    return Qt::DropActions();
}

Qt::ItemFlags PDFDocumentTextFlowEditorModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = BaseClass::flags(index);

    if (index.column() == ColumnText)
    {
        flags.setFlag(Qt::ItemIsEditable);
    }

    return flags;
}

PDFDocumentTextFlowEditor* PDFDocumentTextFlowEditorModel::getEditor() const
{
    return m_editor;
}

void PDFDocumentTextFlowEditorModel::setEditor(PDFDocumentTextFlowEditor* editor)
{
    if (m_editor != editor)
    {
        beginResetModel();
        m_editor = editor;
        endResetModel();
    }
}

void PDFDocumentTextFlowEditorModel::beginFlowChange()
{
    beginResetModel();
}

void PDFDocumentTextFlowEditorModel::endFlowChange()
{
    endResetModel();
}

void PDFDocumentTextFlowEditorModel::clear()
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    beginFlowChange();
    m_editor->clear();
    endFlowChange();
}

void PDFDocumentTextFlowEditorModel::setSelectionActivated(bool activate)
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->setSelectionActive(activate);
    m_editor->deselect();
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::selectByRectangle(QRectF rectangle)
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->selectByRectangle(rectangle);
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::selectByContainedText(QString text)
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->selectByContainedText(text);
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::selectByRegularExpression(const QRegularExpression& expression)
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->selectByRegularExpression(expression);
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::selectByPageIndices(const PDFClosedIntervalSet& indices)
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->selectByPageIndices(indices);
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::restoreOriginalTexts()
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->restoreOriginalTexts();
    m_editor->deselect();
    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

void PDFDocumentTextFlowEditorModel::moveSelectionUp()
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->moveSelectionUp();
    notifyDataChanged();
}

void PDFDocumentTextFlowEditorModel::moveSelectionDown()
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    m_editor->moveSelectionDown();
    notifyDataChanged();
}

void PDFDocumentTextFlowEditorModel::notifyDataChanged()
{
    if (!m_editor || m_editor->isEmpty())
    {
        return;
    }

    Q_EMIT dataChanged(index(0, 0), index(rowCount(QModelIndex()) - 1, ColumnLast));
}

}   // namespace pdf
