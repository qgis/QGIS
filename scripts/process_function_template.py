import sys
import os
import json
import glob

sys.path.append(
    os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        '../python/ext-libs'))

from six import string_types

cpp = open(sys.argv[1], "w")
cpp.write(
    "#include \"qgsexpression.h\"\n"
    "\n"
    "QHash<QString, QgsExpression::Help> QgsExpression::gFunctionHelpTexts;\n"
    "\n"
    "void QgsExpression::initFunctionHelp()\n"
    "{\n"
    "  if( !gFunctionHelpTexts.isEmpty() )\n"
    "    return;"
)


def quote(v):
    if isinstance(v, dict):
        for k in v:
            v[k] = quote(v[k])
        return v
    elif isinstance(v, list):
        return map(quote, v)

    elif isinstance(v, string_types):
        return v.replace('"', '\\"').replace('\n', '\\n')

    elif isinstance(v, bool):
        return v

    else:
        raise BaseException("unexpected type " + repr(v))

for f in sorted(glob.glob('resources/function_help/json/*')):
    with open(f) as function_file:
        try:
            json_params = json.load(function_file)
        except:
            print(f)
            raise

    json_params = quote(json_params)

    for field in ['name', 'type']:
        if not field in json_params:
            raise BaseException("%s: %s missing" % (f, field))

    if not json_params['type'] in ['function', 'operator', 'value', 'expression', 'group']:
        raise BaseException("%s: invalid type %s " % (f, json_params['type']))

    if not 'variants' in json_params:
        # convert single variant shortcut to a expanded variant
        v = {}
        for i in json_params:
            v[i] = json_params[i]

        v['variant'] = json_params['name']
        v['variant_description'] = json_params['description']
        json_params['variants'] = [v]

    name = "\"{0}\"".format(json_params['name'])

    if json_params['type'] == 'operator':
        for v in json_params['variants']:
            if not 'arguments' in v:
                raise BaseException("%s: arguments expected for operator")
            if len(list(v['arguments'])) < 1 or len(list(v['arguments'])) > 2:
                raise BaseException("%s: 1 or 2 arguments expected for operator")

    cpp.write("\n\n  gFunctionHelpTexts.insert( {0},\n    Help( {0}, tr( \"{1}\" ), tr( \"{2}\" ),\n      QList<HelpVariant>()".format(
        name, json_params['type'], json_params['description'])
    )

    for v in json_params['variants']:
        cpp.write(
            "\n        << HelpVariant( tr( \"{0}\" ), tr( \"{1}\" ),\n          QList<HelpArg>()".format(v['variant'], v['variant_description']))

        if 'arguments' in v:
            for a in v['arguments']:
                cpp.write("\n              << HelpArg( \"{0}\", tr( \"{1}\" ), {2}, {3} )".format(
                    a['arg'],
                    a.get('description', ''),
                    "true" if a.get('descOnly', False) else "false",
                    "true" if a.get('syntaxOnly', False) else "false",
                    "true" if a.get('optional', False) else "false",
                    a.get('default', ''))
                )

        cpp.write(",\n          /* variableLenArguments */ {0}".format(
            "true" if v.get('variableLenArguments', False) else "false"))
        cpp.write(",\n          QList<HelpExample>()")

        if 'examples' in v:
            for e in v['examples']:
                cpp.write("\n            << HelpExample( tr( \"{0}\" ), tr( \"{1}\" ), tr( \"{2}\") )".format(
                    e['expression'],
                    e['returns'],
                    e.get('note', ''))
                )

        if 'notes' in v:
            cpp.write(",\n      tr( \"{0}\" )".format(v['notes']))

        cpp.write("\n       )")

    cpp.write("\n      )")
    cpp.write("\n    );")

for f in sorted(glob.glob('resources/function_help/text/*')):
    n = os.path.basename(f)

    with open(f) as content:
        cpp.write("\n\n  gFunctionHelpTexts.insert( \"{0}\",\n    Help( tr( \"{0}\" ), tr( \"group\" ), tr( \"{1}\" ), QList<HelpVariant>() ) );\n".format(
            n, content.read().replace("\\", "&#92;").replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n')))

cpp.write("\n}\n")
cpp.close()
