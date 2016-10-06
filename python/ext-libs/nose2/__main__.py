"""Main entry point"""

import sys
if sys.argv[0].endswith("__main__.py"):
    sys.argv[0] = "nose2"

__unittest = True


if __name__ == '__main__':
    from nose2 import discover
    discover()
