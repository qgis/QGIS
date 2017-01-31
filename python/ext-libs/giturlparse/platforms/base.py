# Imports
import re

class BasePlatform(object):
    FORMATS = {
        'ssh': r"%(_user)s@%(host)s:%(repo)s.git",
        'http': r"http://%(host)s/%(repo)s.git",
        'https': r"http://%(host)s/%(repo)s.git",
        'git': r"git://%(host)s/%(repo)s.git"
    }

    PATTERNS = {
        'ssh': r"(?P<_user>.+)s@(?P<domain>.+)s:(?P<repo>.+)s.git",
        'http': r"http://(?P<domain>.+)s/(?P<repo>.+)s.git",
        'https': r"http://(?P<domain>.+)s/(?P<repo>.+)s.git",
        'git': r"git://(?P<domain>.+)s/(?P<repo>.+)s.git"
    }

    # None means it matches all domains
    DOMAINS = None
    DEFAULTS = {}

    def __init__(self):
        # Precompile PATTERNS
        self.COMPILED_PATTERNS = dict(
            (proto, re.compile(regex))
            for proto, regex in self.PATTERNS.items()
        )

        # Supported protocols
        self.PROTOCOLS = self.PATTERNS.keys()
