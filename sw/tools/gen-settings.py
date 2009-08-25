#!/usr/bin/env python
# vim: ai ts=4 sts=4 et sw=4

import gentools

try:
    import wasp.settings as settings
    import wasp.xmlobject as xmlobject
except ImportError:
    import settings
    import xmlobject

import string
import optparse
import re
import os.path
import sys

if __name__ == "__main__":
    H = "SETTINGS_GENERATED_H"

    parser = optparse.OptionParser()
    parser.add_option("-s", "--settings",
                    default="settings.xml",
                    help="settings xml file", metavar="FILE")
    options, args = parser.parse_args()

    if not os.path.exists(options.settings):
        parser.error("could not find settings.xml")

    try:
        settings_path = os.path.abspath(options.settings)
        x = xmlobject.XMLFile(path=settings_path)

        settings = settings.Settings(x.root)

    except:
        import traceback
        parser.error("invalid xml\n%s" % traceback.format_exc())

    gentools.print_header(H, generatedfrom=settings_path)
    print '#include "std.h"\n'
    settings.print_typedefs()
    settings.print_defines()
    settings.print_values()
    gentools.print_footer(H)
