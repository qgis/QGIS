import sys
import os
import json

ext_libs_path=os.path.join(os.path.dirname(os.path.realpath(__file__)),'../python/ext-libs')
sys.path.append(ext_libs_path)
import jinja2

with open(sys.argv[1]) as function_file:
  json_params = json.load(function_file)

if not 'variants' in json_params:
  #convert single variant shortcut to a expanded variant
  v = {}
  for i in json_params:
     v[i] = json_params[i]
  json_params['variants'] = [v]

template = jinja2.Template('''
<h3>{{f.function}}</h3>
<div class="description">
<p>{{f.description}}</p>
</div>
{% for v in f.variants %}
{% if f.variants|length > 1 %}
<h3>{{v.variant}}</h3>
<div class="description">
<p>{{v.variant_description}}</p>
</div>
{% endif %}
<h4>Syntax</h4>
<div class="syntax">
<code><span class="functionname">{{f.function}}</span>{% if not f.function[0] =='$' %}({% for a in v.arguments if not a.descOnly %}<span class="argument">{{ a.arg }}</span>{% if not loop.last or v.variableLenArguments%}, {% endif %}{% endfor %}{% if v.variableLenArguments %}...{% endif %}){% endif %}</code>
</div>

{% if v.arguments %}<h4>Arguments</h4>
<div class="arguments">
<table>
{% for a in v.arguments if not a.syntaxOnly %}<tr><td class="argument">{{ a.arg }}</td><td>{{ a.description }}</td></tr>
{% endfor %}
</table>
</div>
{% endif %}

{% if v.examples %}
<h4>Examples</h4>
<div class="examples">
<ul>
{% for e in v.examples %}<li><code>{{e.expression}}</code> &rarr; <code>{{e.returns}}</code>{% if e.note %} ({{ e.note }}){% endif %}</li>
{% endfor %}
</ul>
</div>
{% endif %}

{% if v.notes %}<h4>Notes</h4>
<div class="notes">
<p>{{v.notes}}</p></div>
{% endif %}

{% endfor %}

''')
print template.render(f=json_params)



