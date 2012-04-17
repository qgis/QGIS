# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on
- QTextEdit with autocompletion using pyqt by rowinggolfer (GPLv2 license)
see http://rowinggolfer.blogspot.com/2010/08/qtextedit-with-autocompletion-using.html
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from PyQt4.QtGui import *
from PyQt4.QtCore import *

class SqlCompleter(QCompleter):
	def __init__(self, editor, db=None):
		# get the wordlist
		dictionary = None
		if db:
			dictionary = db.connector.getSqlDictionary()
		if not dictionary:
			# use the generic sql dictionary
			from .sql_dictionary import getSqlDictionary
			dictionary = getSqlDictionary()

		wordlist = QStringList()
		for name, value in dictionary.iteritems():
			wordlist << value

		# setup the completer
		QCompleter.__init__(self, sorted(wordlist), editor)
		self.setModelSorting(QCompleter.CaseInsensitivelySortedModel)
		self.setWrapAround(False)

		if isinstance(editor, CompletionTextEdit):
			editor.setCompleter(self)


class CompletionTextEdit(QTextEdit):
	def __init__(self, *args, **kwargs):
		QTextEdit.__init__(self, *args, **kwargs)
		self.completer = None

	def setCompleter(self, completer):
		if self.completer:
			self.disconnect(self.completer, 0, self, 0)
		if not completer:
			return

		completer.setWidget(self)
		completer.setCompletionMode(QCompleter.PopupCompletion)
		completer.setCaseSensitivity(Qt.CaseInsensitive)
		self.completer = completer
		self.connect(self.completer, SIGNAL("activated(const QString&)"), self.insertCompletion)

	def insertCompletion(self, completion):
		tc = self.textCursor()
		extra = completion.length() - self.completer.completionPrefix().length()
		tc.movePosition(QTextCursor.Left)
		tc.movePosition(QTextCursor.EndOfWord)
		tc.insertText(completion.right(extra))
		self.setTextCursor(tc)

	def textUnderCursor(self):
		tc = self.textCursor()
		tc.select(QTextCursor.WordUnderCursor)
		return tc.selectedText()

	def focusInEvent(self, event):
		if self.completer:
			self.completer.setWidget(self)
		QTextEdit.focusInEvent(self, event)

	def keyPressEvent(self, event):
		if self.completer and self.completer.popup().isVisible():
			if event.key() in (Qt.Key_Enter, Qt.Key_Return, Qt.Key_Escape, Qt.Key_Tab, Qt.Key_Backtab):
				event.ignore()
				return

		# has ctrl-E or ctrl-space been pressed??
		isShortcut = event.modifiers() == Qt.ControlModifier and event.key() in (Qt.Key_E, Qt.Key_Space)
		if not self.completer or not isShortcut:
			QTextEdit.keyPressEvent(self, event)

		# ctrl or shift key on it's own??
		ctrlOrShift = event.modifiers() in (Qt.ControlModifier, Qt.ShiftModifier)
		if ctrlOrShift and event.text().isEmpty():
			# ctrl or shift key on it's own
			return

		eow = QString("~!@#$%^&*()+{}|:\"<>?,./;'[]\\-=") # end of word

		hasModifier = event.modifiers() != Qt.NoModifier and not ctrlOrShift

		completionPrefix = self.textUnderCursor()

		if not isShortcut and (hasModifier or event.text().isEmpty() or
				completionPrefix.length() < 3 or eow.contains(event.text().right(1))):
			self.completer.popup().hide()
			return

		if completionPrefix != self.completer.completionPrefix():
			self.completer.setCompletionPrefix(completionPrefix)
			popup = self.completer.popup()
			popup.setCurrentIndex(self.completer.completionModel().index(0,0))

		cr = self.cursorRect()
		cr.setWidth(self.completer.popup().sizeHintForColumn(0)
				+ self.completer.popup().verticalScrollBar().sizeHint().width())
		self.completer.complete(cr) # popup it up!

#if __name__ == "__main__":
#	app = QApplication([])
#	te = CompletionTextEdit()
#	SqlCompleter( te )
#	te.show()
#	app.exec_()
