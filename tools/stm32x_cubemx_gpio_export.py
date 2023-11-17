#!/usr/local/bin/python3
#
# First attempt to parser the Stm32CubeMX .ioc/.csv files and generate a
# set of GPIO constructs that automatically initialize themselves, as well
# as the required GPIO ports.
# Extensions might include
# - exclude/ignore patterns
# - generate typedefed/using pins instead of member variables
#   Note: These will need initializers in the GPIO constructor

import os
import re
import csv
import argparse
import datetime
from operator import itemgetter
import xml.etree.ElementTree as ET

HEADER = """// Copyright %4d Patrick Dowling
//
// Author: Patrick Dowling (pld@gurkenkiste.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// AUTOMATICALLY GENERATED FILE, DO NOT EDIT
// clang-format off
"""

STM32X_PULLUP = {
    'GPIO_PULLUP': 'stm32x::GPIO_PUPD::PULLUP',
    'GPIO_PULLDOWN': 'stm32x::GPIO_PUPD::PULLDOWN',
    'GPIO_NOPULL': 'stm32x::GPIO_PUPD::NONE'
}

STM32X_SPEED = {
    'GPIO_SPEED_FREQ_LOW': 'stm32x::GPIO_SPEED::LOW',
    'GPIO_SPEED_FREQ_MEDIUM': 'stm32x::GPIO_SPEED::MEDIUM',
    'GPIO_SPEED_FREQ_HIGH': 'stm32x::GPIO_SPEED::FAST',
    'GPIO_SPEED_FREQ_VERY_HIGH': 'stm32x::GPIO_SPEED::FASTEST',
}

STM32X_OTYPE = {
    'GPIO_MODE_OUTPUT_PP': 'stm32x::GPIO_OTYPE::PP',
    'GPIO_MODE_OUTPUT_OD': 'stm32x::GPIO_OTYPE::OD'
}

GPIO_FORMATTERS = {
    'GPIO_IN': 'stm32x::GPIOx<stm32x::GPIO_PORT_%(port)s>::GPIO_IN<%(pin)d, %(pullup)s> %(label)s',
    'GPIO_OUT': 'stm32x::GPIOx<stm32x::GPIO_PORT_%(port)s>::GPIO_OUT<%(pin)d, %(speed)s, %(otype)s, %(pullup)s> %(label)s',
    'GPIO_AN': 'stm32x::GPIOx<stm32x::GPIO_PORT_%(port)s>::GPIO_OUT<%(pin)d> %(label)s',
    'GPIO_AF': 'stm32x::GPIOx<stm32x::GPIO_PORT_%(port)s>::GPIO_AF<%(pin)d, %(speed)s, %(otype)s, %(pullup)s, %(af)s> %(label)s'
}

GPIO_TYPEDEF_FORMATTERS = {
    'GPIO_IN': 'using %(label)s = stm32x::GPIO_IN<stm32x::GPIO_PORT_%(port)s, %(pin)d, %(pullup)s, false>',
    'GPIO_OUT': 'using %(label)s = stm32x::GPIO_OUT<stm32x::GPIO_PORT_%(port)s, %(pin)d, %(speed)s, %(otype)s, %(pullup)s, false>',
    'GPIO_AN': 'using %(label)s = stm32x::GPIO_AN<stm32x::GPIO_PORT_%(port)s, %(pin)d, false>',
    'GPIO_AF': 'using %(label)s = stm32x::GPIO_AF<stm32x::GPIO_PORT_%(port)s, %(pin)d, %(speed)s, %(otype)s, %(pullup)s, %(af)s, false>'
}
# Return a tuple of (port, pin_number)


def parse_pin_name(name):
    m = re.match(r'P(?P<port>[A-Z])(?P<pin>\d+)', name)
    port, pin = m.group(1, 2)
    return (port, int(pin))


def natural_key(string_):
    """See http://www.codinghorror.com/blog/archives/001018.html"""
    return [int(s) if s.isdigit() else s for s in re.split(r'(\d+)', string_)]


class Pin:
    def __init__(self, owner, name):
        self.owner = owner
        self.name = name
        self.port, self.pin = parse_pin_name(self.name)
        self.label = None
        self.signal = None
        self.mode = None
        self.modeex = None
        self.otype = None
        self.speed = None
        self.pullup = None
        self.af = None

    def portname(self):
        return "P{}".format(self.port)

    def update(self, key, value):
        if key in ['Locked', 'GPIOParameters']:
            pass
        elif 'GPIO_Label' == key:
            self.label = value
        elif 'Signal' == key:
            self.signal = value
        elif 'GPIO_Mode' == key:
            self.mode = value
        elif 'Mode' == key:
            self.modeex = value
        elif 'GPIO_Speed' == key:
            self.speed = value
        elif 'GPIO_PuPd' == key:
            self.pullup = value
        elif 'GPIO_ModeDefaultOutputPP' == key:
            self.otype = value
        else:
            print("Skipping property {key}={value}".format(**locals()))

    def __repr__(self):
        return "{name} label={label} signal={signal} mode={mode} pullup={pullup}".format(**self.__dict__)

    def ignored(self):
        return re.findall(r'SYS|RCC', self.signal)

    def validate(self):
        if self.signal in ['GPIO_Input', 'GPIO_Output'] and not self.label:
            return False

        if self.is_af():
            self.af = self.owner.gpio_db_find_pin_af(self)
            if not self.af:
                return False

        if self.pullup is None:
            self.pullup = 'GPIO_NOPULL'
        if self.speed is None:
            self.speed = 'GPIO_SPEED_FREQ_MEDIUM'
        if self.otype is None:
            self.otype = 'GPIO_MODE_OUTPUT_PP'
        return True

    def is_input(self):
        return 'GPIO_Input' == self.signal or 'GPIO_MODE_INPUT' == self.modeex

    def is_output(self):
        return 'GPIO_Output' == self.signal

    def is_analog(self):
        return re.search(r'AIN|ADC|DAC|COMP{1,2}', self.signal)

    def is_af(self):
        return not self.is_input() and not self.is_output() and not self.is_analog()

    def build_gpio(self, numeric):
        if self.label:
            label = self.label
        else:
            label = self.signal

        af = ''
        if self.is_input():
            gpio_type = 'GPIO_IN'
        elif self.is_output():
            gpio_type = 'GPIO_OUT'
        elif self.is_analog():
            gpio_type = 'GPIO_AN'
        else:
            gpio_type = 'GPIO_AF'
            # NOTE This isn't ideal, there are multiple ways the AF are defined in
            # the StdPeriph headers depending on model, e.g.
            # F0/F3 -> GPIO_AF_<number>
            # F4 -> GPIO_AF_<function> (with some GPIO_AF<n>_<function>)
            # So the compromise seems to be to use the integer value and annotate
            # with the function as a comment
            if numeric:
                m = re.findall(r'AF(?P<af>\d)', self.af)
                af = "{}/*{}*/".format(m[0], self.af)
            else:
                af = self.af

        gpio = {'label': label,
                'type': gpio_type,
                'port': self.port,
                'pin': self.pin,
                'pullup': STM32X_PULLUP[self.pullup],
                'otype': STM32X_OTYPE[self.otype],
                'speed': STM32X_SPEED[self.speed],
                'af': af}
        return gpio


class PinoutParser:
    def __init__(self, numeric=False, verbose=False):
        self.numeric = numeric
        self.verbose = verbose
        self.mcu_family = ""
        self.mcu_name = ""
        self.mcu_package = ""
        self.ioc_pin_attributes = {}
        self.ioc_file = ""
        self.csv_file = ""

    def read_ioc(self, filename):
        print("Reading IOC definitions from '%s'..." % filename)
        self.ioc_file = filename
        ioc_pin_attributes = {}
        with open(filename, "r") as lines:
            for line in lines:
                line = line.strip()
                if not line or line[0] == '#':
                    continue

                m = re.match(r'^(?P<pin_name>P[A-Z]\d+.*)\.(?P<key>\w*)=(?P<value>.*)', line)
                if m:
                    pin_name, key, value = itemgetter(*['pin_name', 'key', 'value'])(m.groupdict())
                    if pin_name not in ioc_pin_attributes:
                        ioc_pin_attributes[pin_name] = Pin(self, pin_name)
                    pin = ioc_pin_attributes[pin_name]
                    pin.update(key, value)
                    continue
                m = re.match(r'Mcu\.Family=(?P<family>\w+)', line)
                if m:
                    self.mcu_family = m.groupdict()['family']
                    continue
                m = re.match(r'Mcu\.Package=(?P<package>\w+)', line)
                if m:
                    self.mcu_package = m.groupdict()['package']
                    continue
                m = re.match(r'Mcu\.Name=(?P<name>.*)', line)
                if m:
                    self.mcu_name = m.groupdict()['name']
                    continue

        self.ioc_pin_attributes = ioc_pin_attributes
        print("%02d pins found" % len(self.ioc_pin_attributes))

    # Reading the CSV file is optional, but it seems to provide nicer names for
    # some of the pins (DAC1_OUT1 vs. COMP_DAC11_group)
    def read_csv(self, filename):
        print("Reading CSV definitions from '%s'..." % filename)
        self.csv_file = filename
        with open(filename, 'r') as csvfile:
            reader = csv.reader(csvfile, delimiter=',', quotechar='"')
            next(reader, None)
            for row in reader:
                pin, name, pintype, signal, label = row
                if "I/O" == pintype and name in self.ioc_pin_attributes:
                    pin = self.ioc_pin_attributes[name]
                    if signal != pin.signal:
                        if self.verbose:
                            print("{:>4}: Update signal name to {} (was {})".format(name, signal, pin.signal))
                        pin.update('Signal', signal)

    def parse_xml_ns(self, path):
        if self.verbose:
            print(path)
        root = ET.parse(path).getroot()
        ns = re.match(r'\{.*\}', root.tag)
        return root, ns.group(0) if ns else ''

    def read_gpio_db(self, cubemx):
        mcu_db_path = os.path.join(cubemx, "db/mcu", self.mcu_name + ".xml")
        print("Reading MCU DB for '%s'..." % self.mcu_name)
        root, ns = self.parse_xml_ns(mcu_db_path)
        gpio_ip = root.findall(".//%s%s" % (ns, "IP[@Name='GPIO']"))
        if len(gpio_ip) < 1:
            raise RuntimeError("ERROR: Unable to find GPIO IP in MCU XML file...")

        version = gpio_ip[0].attrib['Version']
        gpio_mode_db_path = os.path.join(cubemx, "db/mcu/IP/", ("GPIO-%s_Modes.xml" % version))
        print("Reading GPIO modes version '%s'..." % version)
        self.gpio_root, self.gpio_ns = self.parse_xml_ns(gpio_mode_db_path)

    def gpio_db_find_pin_af(self, pin):
        xpath = ".//{0}GPIO_Pin[@Name='{1}']".format(self.gpio_ns, pin.name)
        pins = self.gpio_root.findall(xpath)
        if len(pins) < 1:
            raise RuntimeError("No XML root found for %s, xpath=%s" % (pin.name, xpath))
        pin_root = pins[0]

        xpath = ".//{0}PinSignal[@Name='{1}']//{0}PossibleValue".format(self.gpio_ns, pin.signal)
        af = pin_root.findall(xpath)
        if len(af) < 1:
            raise RuntimeError("No AF found for %s on %s, xpath=%s" % (pin.name, pin.signal, xpath))
        return af[0].text

    def export_h(self, basename, ns, typedef, clocks):
        h = basename + '.h'
        print("Exporting header file {}".format(h))
        self.warnings = []
        self.unused = []
        self.exported = 0

        pin_declarations = []
        used_ports = set()
        for pin_name in sorted(self.ioc_pin_attributes, key=natural_key):
            pin = self.ioc_pin_attributes[pin_name]
            if pin.ignored():
                print("%s ignored (%s)" % (pin.name, pin.signal))
                continue
            if not pin.validate():
                self.warnings.append(pin_name)
                continue

            if self.verbose:
                print(pin)
            else:
                print("{0:<12}\t{1}\t{2}".format(pin.name, pin.signal, pin.label))
            gpio = pin.build_gpio(self.numeric)
            if typedef:
                pin_declarations.append(GPIO_TYPEDEF_FORMATTERS[gpio['type']] % gpio)
            else:
                pin_declarations.append(GPIO_FORMATTERS[gpio['type']] % gpio)
            self.exported += 1
            used_ports.add(pin.port)

        header_guard = "%s_GPIO_H_" % ns.upper()
        f = open(h, 'w')
        f.write(HEADER % datetime.datetime.now().year)
        f.write("// IOC source: {}\n".format(self.ioc_file))
        if self.csv_file:
            f.write("// CSV source: {}\n".format(self.csv_file))
        f.write("// " + ("-" * 80) + "\n")
        f.write("#ifndef {}\n".format(header_guard))
        f.write("#define {}\n".format(header_guard))
        f.write("#include \"stm32x/stm32x_core.h\"\n\n")
        if ns:
            f.write("namespace {} {{\n".format(ns))
        f.write("class GPIO {\n")
        if clocks == 'auto':
            f.write("  struct GPIO_PORT_INIT {\n")
            f.write("    GPIO_PORT_INIT() {\n")
            for port in sorted(used_ports):
                f.write("      stm32x::GPIOx<stm32x::GPIO_PORT_{0}>::EnableClock(true);\n".format(port))
            f.write("    }\n")
            f.write("  };\n")
            f.write("  GPIO_PORT_INIT port_initializer_;\n\n")
        f.write("public:\n")
        f.write("  GPIO() = default;\n")
        f.write("  DISALLOW_COPY_AND_ASSIGN(GPIO);\n")
        for pin in pin_declarations:
            f.write("  {};\n".format(pin))

        if clocks == 'manual':
                f.write("\n")
                f.write("  static void EnableClocks(bool enable = true) {\n")
                for port in sorted(used_ports):
                    f.write("    stm32x::GPIOx<stm32x::GPIO_PORT_{0}>::EnableClock(enable);\n".format(port))
                f.write("  }\n")

        f.write("}; // class GPIO\n\n")
        f.write("extern GPIO gpio;\n")
        if ns:
            f.write("}} // namespace {}\n".format(ns))
        f.write("#endif // {}\n".format(header_guard))

        if self.verbose:
            print(" %2d exported" % self.exported)
        if len(self.unused):
            print(" %2d unused  : %s" % (len(self.unused), ", ".join(self.unused)))
        if len(self.warnings):
            print(" %2d warnings: %s" % (len(self.warnings), ", ".join(self.warnings)))

    def export_cc(self, basename, ns):
        cc = basename + '.cc'
        print("Exporting cc file (may be empty){}".format(cc))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('files', nargs='+', help='IOC file to process')
    parser.add_argument('-v', '--verbose', dest='verbose', action='store_true', help='Extra spam')
    parser.add_argument('--namespace', help='Namespace for generated C++ files')
    parser.add_argument('-n', '--numeric', action='store_true', help='Translate AF names to numeric')
    parser.add_argument('-m', '--cubemx', required=True, help='Root path to CubeMX files')
    parser.add_argument('-o', '--output', required=True, help='Output path (.h .cc will be replaced/appended)')
    parser.add_argument('-t', '--typedef', action='store_true', help='Export typedefs instead of members')
    parser.add_argument('--clocks', default='auto', choices=['auto', 'manual'], help='Determine whether GPIO clocks are enabled in constructor or manually')
    args = parser.parse_args()

    ioc_file = None
    csv_file = None

    for f in args.files:
        ext = os.path.splitext(f)[1]
        if '.ioc' == ext:
            ioc_file = f
        elif '.csv' == ext:
            csv_file = f

    if not ioc_file:
        print("error: No IOC file specified")
        exit(-1)

    basename = os.path.splitext(args.output)[0]

    parser = PinoutParser(numeric=args.numeric, verbose=args.verbose)
    parser.read_ioc(ioc_file)
    if csv_file:
        parser.read_csv(csv_file)
    parser.read_gpio_db(args.cubemx)

    if args.verbose:
        print('-' * 80)
        print("PACKAGE: %s" % parser.mcu_package)
        print("FAMILY : %s (%s)" % (parser.mcu_family, parser.mcu_name))
        print('-' * 80)

    parser.export_h(basename, args.namespace, args.typedef, args.clocks)
#  parser.export_cc(basename, args.numeric, args.namespace)
