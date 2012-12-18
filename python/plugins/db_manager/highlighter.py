# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on
- Python Syntax Highlighting Example by Carson J. Q. Farmer (GPLv2 license)
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

class SqlHighlighter(QSyntaxHighlighter):

	COLOR_KEYWORD = QColor(0x00,0x00,0xE6)
	COLOR_FUNCTION = QColor(0xCE,0x7B,0x00)
	COLOR_COMMENT = QColor(0x96,0x96,0x96)
	COLOR_CONSTANT = Qt.magenta
	COLOR_IDENTIFIER = QColor(0x00,0x99,0x00)
	COLOR_PARAMETER = QColor(0x25,0x9D,0x9D)

	def __init__(self, editor, db=None):
		QSyntaxHighlighter.__init__(self, editor)
		self.editor = editor
		self.rules = []
		self.styles = {}

		# keyword
		format = QTextCharFormat()
		format.setForeground( QBrush(self.COLOR_KEYWORD, Qt.SolidPattern) )
		#format.setFontWeight( QFont.Bold )
		self.styles['keyword'] = format

		# function and delimiter
		format = QTextCharFormat()
		format.setForeground( QBrush(self.COLOR_FUNCTION, Qt.SolidPattern) )
		self.styles['function'] = format
		self.styles['delimiter'] = format

		# identifier
		format = QTextCharFormat()
		format.setForeground( QBrush(self.COLOR_IDENTIFIER, Qt.SolidPattern) )
		self.styles['identifier'] = format

		# comment
		format = QTextCharFormat()
		format.setForeground( QBrush(self.COLOR_COMMENT, Qt.SolidPattern) )
		self.styles['comment'] = format

		# constant (numbers, strings)
		format = QTextCharFormat()
		format.setForeground( QBrush(self.COLOR_CONSTANT, Qt.SolidPattern) )
		self.styles['constant'] = format

		if db:
			self.load(db)

	def highlightBlock(self, text):
		index = 0
		rule_sel = None
		rule_index = -1

		while index < text.length():
			# search for the rule that matches starting from the less index
			rule_sel = None
			rule_index = -1
			rule_length = 0
			for rule in self.rules:
				regex = rule.regex()
				pos = regex.indexIn(text, index)
				if pos >= 0:
					if rule_sel == None or rule_index > pos:
						rule_sel = rule
						rule_index = pos
						rule_length = regex.cap(0).length()

			if rule_sel == None:	# no rule matches
				break

			# apply the rule found before
			self.setFormat(rule_index, rule_length, self.styles[rule_sel.type()])
			index = rule_index + rule_length

		self.setCurrentBlockState( 0 )

		# handle with multiline comments
		index = 0
		if self.previousBlockState() != 1:
			index = self.multiLineCommentStart.indexIn(text, index)
		while index >= 0:
			# if the last applied rule is a single-line comment,
			# then avoid multiline comments that start after it
			if rule_sel != None and rule_sel.type() == 'comment' and index >= rule_index:
				break

			pos = self.multiLineCommentEnd.indexIn(text, index)
			comment_length = 0
			if pos < 0:
				self.setCurrentBlockState(1)
				comment_length = text.length() - index;
			else:
				comment_length = pos - index + self.multiLineCommentEnd.cap(0).length()
			self.setFormat(index, comment_length, self.styles['comment'])
			index = self.multiLineCommentStart.indexIn(text, index + comment_length)


	def load(self, db=None):
		self.rules = []

		rules = None

		if db:
			rules = db.connector.getSqlDictionary()
		if not rules:
			# use the generic sql dictionary
			from .sql_dictionary import getSqlDictionary
			rules = getSqlDictionary()

		for name in self.styles.keys():
			if not name in rules:
				continue
			for value in rules[name]:
				regex = QRegExp( u"\\b%s\\b" % QRegExp.escape(value), Qt.CaseInsensitive )
				rule = HighlightingRule(name, regex)
				self.rules.append( rule )

		# delimiter
		regex = QRegExp( "[\)\(]" )
		rule = HighlightingRule('delimiter', regex)
		self.rules.append( rule )

		# identifier
		regex = QRegExp( r'"[^"\\]*(\\.[^"\\]*)*"' )
		regex.setMinimal( True )
		rule = HighlightingRule('identifier', regex)
		self.rules.append( rule )

		# constant (numbers, strings)
		# string
		regex = QRegExp( r"'[^'\\]*(\\.[^'\\]*)*'" )
		regex.setMinimal( True )
		rule = HighlightingRule('constant', regex)
		self.rules.append( rule )
		# number
		regex = QRegExp( r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b' )
		regex.setMinimal( True )
		rule = HighlightingRule('constant', regex)
		self.rules.append( rule )

		# single-line comment
		regex = QRegExp( "--.*$" )
		rule = HighlightingRule('comment', regex)
		self.rules.append( rule )

		# multi-line comment
		self.multiLineCommentStart = QRegExp( "/\\*" )
		self.multiLineCommentEnd = QRegExp( "\\*/" )


class HighlightingRule:
	def __init__(self, typ, regex):
		self._type = typ
		self._regex = regex

	def type(self):
		return self._type

	def regex(self):
		return QRegExp(self._regex)

