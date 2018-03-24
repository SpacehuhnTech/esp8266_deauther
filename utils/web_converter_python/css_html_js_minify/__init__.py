#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: juancarlospaco
# GitHub Repo: https://github.com/juancarlospaco/css-html-js-minify


"""CSS-HTML-JS-Minify.

Minifier for the Web.
"""


from .minify import (process_single_html_file, process_single_js_file,
                     process_single_css_file, html_minify, js_minify,
                     css_minify)


__version__ = '2.5.0'
__license__ = 'GPLv3+ LGPLv3+'
__author__ = 'Juan Carlos'
__email__ = 'juancarlospaco@gmail.com'
__url__ = 'https://github.com/juancarlospaco/css-html-js-minify'
__source__ = ('https://raw.githubusercontent.com/juancarlospaco/'
              'css-html-js-minify/master/css-html-js-minify.py')


__all__ = ['__version__', '__license__', '__author__',
           '__email__', '__url__', '__source__',
           'process_single_html_file', 'process_single_js_file',
           'process_single_css_file', 'html_minify', 'js_minify',
           'css_minify', 'minify']
