//    Copyright (C) 2021-2022 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#include "pdficontheme.h"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

#include "pdfdbgheap.h"

namespace pdf
{

void PDFIconTheme::registerAction(QAction* action, QString iconFileName)
{
    m_actionInfos.emplace_back(action, std::move(iconFileName));
}

QString PDFIconTheme::getDirectory() const
{
    return m_directory;
}

void PDFIconTheme::setDirectory(const QString& directory)
{
    m_directory = directory;
}

QString PDFIconTheme::getPrefix() const
{
    return m_prefix;
}

void PDFIconTheme::setPrefix(const QString& prefix)
{
    m_prefix = prefix;
}

void PDFIconTheme::prepareTheme()
{
    QString directory = getThemeDirectory();
    QDir dir(directory);

    if (!dir.exists())
    {
        dir.mkpath(directory);

        QStringList infoList;

        for (const ActionInfo& actionInfo : m_actionInfos)
        {
            QString fileName = formatFileName(actionInfo, directory);
            if (!QFile::exists(fileName))
            {
                QFile::copy(actionInfo.fileName, fileName);
                QFile::setPermissions(fileName, QFile::Permissions(0xFFFF));
            }

            infoList << QString("%1;%2").arg(actionInfo.action->text(), fileName);
        }

        if (!infoList.isEmpty())
        {
            QFile file(directory + "/icons.txt");
            if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
            {
                {
                    QTextStream stream(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                    stream.setEncoding(QStringConverter::Utf8);
#else
                    stream.setCodec("UTF-8");
#endif
                    stream.setGenerateByteOrderMark(true);
                    for (const QString& text : infoList)
                    {
                        stream << text << Qt::endl;
                    }
                }
                file.close();
            }
        }
    }
}

void PDFIconTheme::loadTheme()
{
    QString directory = getThemeDirectory();
    QDir dir(directory);

    if (dir.exists())
    {
        for (const ActionInfo& actionInfo : m_actionInfos)
        {
            QString fileName = formatFileName(actionInfo, directory);
            if (QFile::exists(fileName))
            {
                actionInfo.action->setIcon(QIcon(fileName));
            }
        }
    }
}

QString PDFIconTheme::getThemeDirectory() const
{
    return QCoreApplication::applicationDirPath() + "/" + m_directory;
}

QString PDFIconTheme::formatFileName(const ActionInfo& info, const QString& themeDirectory) const
{
    QString fileName = info.fileName;
    fileName.remove(m_prefix);
    return QString("%1/%2").arg(themeDirectory, fileName);
}

}   // namespace pdf
