#
# Copyright 2018 Patrick Dowling
#
# Author: Patrick Dowling (pld@gurkenkiste.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Classes to define tables/arrays that are compiled into .cc/.h files
# "Inspired" by pichenettes stmlib resources_compiler.py, and (possibly badly)
# re-engineered to learn python.

import os
import string
from collections import defaultdict

class ArrayResource(object):
  def __init__(self, name, c_type, values, formatter, values_per_line=8):
    self._name = name
    self._c_type = c_type
    self._values = values
    self._formatter = formatter
    self._values_per_line = values_per_line

  @property
  def name(self):
    return self._name

  @property
  def fullname(self):
    return self._fullname

  def generate(self, enum, alias):
    self._enum = enum
    self._alias = alias
    self._fullname = '%s_%s' % (alias, self._name)

  def compile(self, f, with_comment):
    if with_comment:
      f.write('// %s\n' % self._enum)
    f.write('static const %s %s[%d] = {' % (self._c_type, self._fullname, len(self._values)))
    num_values = len(self._values)
    if num_values > self._values_per_line:
      f.write('\n')
      for i in range(0, num_values, self._values_per_line):
        f.write('  ');
        f.write(', '.join(self._formatter(self._values[j]) for j in range(i, min(num_values, i + self._values_per_line))))
        f.write(',\n')
    else:
      f.write(' ')
      f.write(', '.join(self._formatter(value) for value in self._values))
      f.write(' ')
    f.write('};\n')

class ResourceTable(object):
  def __init__(self, resource):
    self._name = resource['name']
    self._prefix = resource['prefix']
    self._c_type = resource['c_type']
    self._entries = resource['entries']
    self._includes = resource['includes']
    self._use_aliases = resource['use_aliases']
    self._formatter = resource.get('formatter', lambda x: "%d" % x)

  @property
  def includes(self):
    return self._includes if self._includes else []

  def declare(self, f):
    pass

class ResourceTableEnumerated(ResourceTable):
  def __init__(self, resource):
    super(ResourceTableEnumerated, self).__init__(resource)

    self._enum_name = 'E' + ''.join(x.title() for x in self._name.split('_'))
    t = self._c_type + ' '
    if self._use_aliases:
      t = t + '*'
    self._declaration = "const %s%ss[%s_LAST]" % (t, self._name, self._prefix)
    self._enums = []
    self._aliases = []
    self._member_value_aliases = []

    for (key, value) in self._entries:
      self._enums.append('%s_%s' % (self._prefix, key.upper()))
      self._aliases.append('%s_%s' % (self._name, key.lower()))

    self._member_value_aliases_lut = defaultdict(list)
    for enum, alias, (key, value) in zip(self._enums, self._aliases, self._entries):
      self._generate_member_value_aliases(enum, alias, value)

  @property
  def declaration(self):
    return self._declaration

  @property
  def enum_name(self):
    return self._enum_name

  def declare(self, f):
    self.declare_enums(f)
    self.declare_externs(f)

  def _generate_member_value_aliases(self, enum, alias, value):
    if type(value) in (list, tuple):
      for i in value:
        self._generate_member_value_aliases(enum, alias, i)
    elif type(value) == ArrayResource:
      value.generate(enum, alias)
      self._member_value_aliases_lut[alias].append(value)
      self._member_value_aliases.append(value)
    else:
      pass

  def declare_externs(self, f):
    f.write('extern %s;\n\n' % self.declaration)

  def declare_enums(self, f):
    f.write('enum %s {\n' % self.enum_name);
    for enum in self._enums:
      f.write('  %s,\n' % enum)
    f.write('  %s_LAST\n' % self._prefix)
    f.write('}; // enum %s\n\n' % self.enum_name)

  def _format_value(self, value, alias=None):
    if type(value) in (list, tuple):
      return "{ %s }" % ', '.join(self._format_value(i, alias) for i in value)
    elif type(value) == ArrayResource:
      return '%s_%s' % (alias, value.name)
    else:
      return self._formatter(value)

  def _compile_value(self, value, f, alias):
    f.write("  %s, " % self._format_value(value, alias))

  def _compile_alias(self, value, f, alias):
    f.write('%s;' % self._format_value(value, alias));

  def compile(self, f):
    if self._use_aliases:
      for enum, alias, (key, value) in zip(self._enums, self._aliases, self._entries):
        f.write('// %s\n' % enum)
        for mva in self._member_value_aliases_lut[alias]:
          mva.compile(f, False)

        f.write('static const %s %s = ' % (self._c_type, alias))
        self._compile_alias(value, f, alias);
        f.write('\n')

      f.write('\n')
      f.write('%s = {\n' % self.declaration)
      for enum, alias in zip(self._enums, self._aliases):
        f.write('  &%s,\n' % alias)
      f.write('};\n\n')
    else:
      for mva in self._member_value_aliases:
        mva.compile(f, True)
      f.write('\n')

      f.write('%s = {\n' % self.declaration)
      for enum, alias, (key, value) in zip(self._enums, self._aliases, self._entries):
        f.write('  // %s\n' % enum)
        self._compile_value(value, f, alias)
        f.write('\n')
      f.write('};\n\n')

class ResourceLibrary(object):
  def __init__(self, resources, target, namespace, includes, header):
    self._tables = []
    self._namespace = namespace
    self._includes = includes
    if not self._includes:
      self._includes = []
    self._header = header

    self._header_guard = "%s_%s_H_" % (target.upper(), self._namespace.upper())

    for r in resources:
      table_type = r['type']
      table = table_type(r)
      self._tables.append(table)
      self._includes.extend(table.includes)

  @property
  def includes(self):
    return self._includes

  @property
  def header_guard(self):
    return self._header_guard

  @property
  def header(self):
    return self._header

  @property
  def namespace(self):
    return self._namespace

  def _open_namespace(self, f):
    f.write('namespace %s {\n\n' % self.namespace);

  def _close_namespace(self, f):
    f.write('} // namespace %s\n' % self.namespace);

  def generate_h(self, root_path):
    f = open(root_path + '.h', 'w')
    if self.header: f.write(self.header + '\n')
    f.write('#ifndef %s\n' % self.header_guard)
    f.write('#define %s\n' % self.header_guard)
    f.write('\n')

    f.write("\n".join(self.includes))
    f.write('\n\n')

    self._open_namespace(f)
    for table in self._tables:
      table.declare(f)
    self._close_namespace(f)

    f.write('\n#endif // %s\n' % self.header_guard)
    f.close()

  def generate_cc(self, root_path):
    f = open(root_path + '.cc', 'w')
    if self.header: f.write(self.header + '\n')
    f.write('#include "%s.h"\n\n' % self.namespace)
    self._open_namespace(f)
    for table in self._tables:
      table.compile(f)
    self._close_namespace(f)

# Acutal processor function
def Process(library, basename):
  library.generate_h(basename)
  library.generate_cc(basename)
