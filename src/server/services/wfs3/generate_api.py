
import json
import os

j = json.load(open(os.path.join(os.path.dirname(__file__), 'openapi.json')))

from IPython import embed
embed(using=False)
