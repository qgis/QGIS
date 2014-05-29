##[Example scripts]=group
##Batch string replace via regex dictionary=name
##input=string John has a blue car.
##ignore_case=boolean True
##verbose=boolean False
##replaceDict=string {'John': 'Mary', 'blue': 'red', 'car': 'bike'}
##output=output string

import ast
import re

if not input: input = ''
if not replaceDict: replaceDict = '{}'

if verbose:
    progress.setText('INPUT = \n%s\n' % input)
    progress.setText('REPLACE DICT = \n%s\n' % replaceDict)

reOption = re.MULTILINE
if ignore_case:
    reOption = reOption|re.IGNORECASE

# Set output string
output = input

# Get replaceDict string
# and convert it to dict
d = ast.literal_eval(replaceDict)

# Only replace strings if d is a dict
if d and isinstance(d, dict):
    for k, v in d.items():
        # Replace search string by value
        r = re.compile(k, reOption)
        output = r.sub(v, output)
else:
    progress.setText('ERROR - Replace dict does not represent a dictionary. String not changed!' )

if verbose:
    progress.setText('OUTPUT = \n%s\n' % output)
