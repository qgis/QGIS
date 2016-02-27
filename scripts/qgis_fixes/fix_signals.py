"""Migrate signals from old style to new style
"""
# Author: Juergen E. Fischer

# .connect( sender, signal, receiver, slot )

# Local imports
from lib2to3 import fixer_base, pytree
from lib2to3.fixer_util import Call, Name, Attr, ArgList, Node, syms

import re


class FixSignals(fixer_base.BaseFix):
    PATTERN = """
  (
  power<
    any trailer< '.' method=('connect'|'disconnect') >
    trailer<
      '('
      arglist<
        sender=any ','
        power< 'SIGNAL' trailer< '(' signal=any ')' > > ','
        slot=any
      >
      ')'
    >
  >
  |
  power<
    emitter=any trailer< '.' 'emit' >
    trailer<
      '('
        args=arglist<
            power< 'SIGNAL' trailer< '(' signal=any ')' > >
            ( ',' any )*
        >
      ')'
    >
  >
  |
  power<
    emitter=any trailer< '.' 'emit' >
    trailer<
      '('
        args=power< 'SIGNAL' trailer< '(' signal=any ')' > >
      ')'
    >
  >
  )
"""

#    def match(self, node):
#        res = super(FixSignals, self).match( node )
#        r = repr(node)
#        if "emit" in r:
#            print "yes" if res else "no", ": ", r
#        return res

    def transform(self, node, results):
        signal = results.get("signal").value
        signal = re.sub('^["\']([^(]+)(?:\(.*\))?["\']$', '\\1', signal)

        if 'emitter' in results:
            emitter = results.get("emitter").clone()
            emitter.prefix = node.prefix
            args = results.get("args").clone()
            args.children = args.children[2:]
            if args.children:
                args.children[0].prefix = ''
            res = Node(syms.power, [emitter, Name('.'), Name(signal), Name('.'), Name('emit')] + [ArgList([args])])
        else:
            sender = results.get("sender").clone()
            method = results.get("method")
            if isinstance(method, list):
                method = method[0]
            method = method.clone()
            sender.prefix = node.prefix
            slot = results.get("slot").clone()
            slot.prefix = ""
            res = Node(syms.power, [sender, Name('.'), Name(signal), Name('.'), method] + [ArgList([slot])])
        return res
