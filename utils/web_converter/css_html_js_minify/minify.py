#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: juancarlospaco
# GitHub Repo: https://github.com/juancarlospaco/css-html-js-minify

"""CSS-HTML-JS-Minify.

Minifier for the Web.
"""


import atexit
import os
import sys
import gzip
import logging as log

from argparse import ArgumentParser
from datetime import datetime
from functools import partial
from hashlib import sha1
from multiprocessing import Pool, cpu_count
from subprocess import getoutput
from time import sleep

from .css_minifier import css_minify
from .html_minifier import html_minify
from .js_minifier import js_minify

from anglerfish import (check_encoding, check_folder, make_logger,
                        make_post_exec_msg, set_process_name,
                        set_single_instance, walk2list, beep,
                        set_terminal_title)


__version__ = '2.5.0'
__license__ = 'GPLv3+ LGPLv3+'
__author__ = 'Juan Carlos'
__email__ = 'juancarlospaco@gmail.com'
__url__ = 'https://github.com/juancarlospaco/css-html-js-minify'
__source__ = ('https://raw.githubusercontent.com/juancarlospaco/'
              'css-html-js-minify/master/css-html-js-minify.py')


start_time = datetime.now()


##############################################################################


def process_multiple_files(file_path, watch=False, wrap=False, timestamp=False,
                           comments=False, sort=False, overwrite=False,
                           zipy=False, prefix='', add_hash=False):
    """Process multiple CSS, JS, HTML files with multiprocessing."""
    log.debug("Process {} is Compressing {}.".format(os.getpid(), file_path))
    if watch:
        previous = int(os.stat(file_path).st_mtime)
        log.info("Process {} is Watching {}.".format(os.getpid(), file_path))
        while True:
            actual = int(os.stat(file_path).st_mtime)
            if previous == actual:
                sleep(60)
            else:
                previous = actual
                log.debug("Modification detected on {0}.".format(file_path))
                check_folder(os.path.dirname(file_path))
                if file_path.endswith(".css"):
                    process_single_css_file(
                        file_path, wrap=wrap, timestamp=timestamp,
                        comments=comments, sort=sort, overwrite=overwrite,
                        zipy=zipy, prefix=prefix, add_hash=add_hash)
                elif file_path.endswith(".js"):
                    process_single_js_file(
                        file_path, timestamp=timestamp,
                        overwrite=overwrite, zipy=zipy)
                else:
                    process_single_html_file(
                        file_path, comments=comments,
                        overwrite=overwrite, prefix=prefix, add_hash=add_hash)
    else:
        if file_path.endswith(".css"):
            process_single_css_file(
                file_path, wrap=wrap, timestamp=timestamp,
                comments=comments, sort=sort, overwrite=overwrite, zipy=zipy,
                prefix=prefix, add_hash=add_hash)
        elif file_path.endswith(".js"):
            process_single_js_file(
                file_path, timestamp=timestamp,
                overwrite=overwrite, zipy=zipy)
        else:
            process_single_html_file(
                file_path, comments=comments,
                overwrite=overwrite, prefix=prefix, add_hash=add_hash)


def prefixer_extensioner(file_path, old, new,
                         file_content=None, prefix='', add_hash=False):
    """Take a file path and safely preppend a prefix and change extension.

    This is needed because filepath.replace('.foo', '.bar') sometimes may
    replace '/folder.foo/file.foo' into '/folder.bar/file.bar' wrong!.
    >>> prefixer_extensioner('/tmp/test.js', '.js', '.min.js')
    '/tmp/test.min.js'
    """
    log.debug("Prepending '{}' Prefix to {}.".format(new.upper(), file_path))
    extension = os.path.splitext(file_path)[1].lower().replace(old, new)
    filenames = os.path.splitext(os.path.basename(file_path))[0]
    filenames = prefix + filenames if prefix else filenames
    if add_hash and file_content:  # http://stackoverflow.com/a/25568916
        filenames += "-" + sha1(file_content.encode("utf-8")).hexdigest()[:11]
        log.debug("Appending SHA1 HEX-Digest Hash to '{}'.".format(file_path))
    dir_names = os.path.dirname(file_path)
    file_path = os.path.join(dir_names, filenames + extension)
    return file_path


def process_single_css_file(css_file_path, wrap=False, timestamp=False,
                            comments=False, sort=False, overwrite=False,
                            zipy=False, prefix='', add_hash=False,
                            output_path=None):
    """Process a single CSS file."""
    log.info("Processing CSS file: {0}.".format(css_file_path))
    with open(css_file_path, encoding="utf-8") as css_file:
        original_css = css_file.read()
    log.debug("INPUT: Reading CSS file {}.".format(css_file_path))
    minified_css = css_minify(original_css, wrap=wrap,
                              comments=comments, sort=sort)
    if timestamp:
        taim = "/* {0} */ ".format(datetime.now().isoformat()[:-7].lower())
        minified_css = taim + minified_css
    if output_path is None:
        min_css_file_path = prefixer_extensioner(
            css_file_path, ".css", ".css" if overwrite else ".min.css",
            original_css, prefix=prefix, add_hash=add_hash)
        if zipy:
            gz_file_path = prefixer_extensioner(
                css_file_path, ".css",
                ".css.gz" if overwrite else ".min.css.gz", original_css,
                prefix=prefix, add_hash=add_hash)
            log.debug("OUTPUT: Writing ZIP CSS {}.".format(gz_file_path))
    else:
        min_css_file_path = gz_file_path = output_path
    if not zipy or output_path is None:
        # if specific output path is requested,write write only one output file
        with open(min_css_file_path, "w", encoding="utf-8") as output_file:
            output_file.write(minified_css)
    if zipy:
        with gzip.open(gz_file_path, "wt", encoding="utf-8") as output_gz:
            output_gz.write(minified_css)
    log.debug("OUTPUT: Writing CSS Minified {0}.".format(min_css_file_path))
    return min_css_file_path


def process_single_html_file(html_file_path, comments=False, overwrite=False,
                             prefix='', add_hash=False, output_path=None):
    """Process a single HTML file."""
    log.info("Processing HTML file: {0}.".format(html_file_path))
    with open(html_file_path, encoding="utf-8") as html_file:
        minified_html = html_minify(html_file.read(), comments=comments)
    log.debug("INPUT: Reading HTML file {0}.".format(html_file_path))
    if output_path is None:
        html_file_path = prefixer_extensioner(
            html_file_path, ".html" if overwrite else ".htm", ".html",
            prefix=prefix, add_hash=add_hash)
    else:
        html_file_path = output_path
    with open(html_file_path, "w", encoding="utf-8") as output_file:
        output_file.write(minified_html)
    log.debug("OUTPUT: Writing HTML Minified {0}.".format(html_file_path))
    return html_file_path


def process_single_js_file(js_file_path, timestamp=False, overwrite=False,
                           zipy=False, output_path=None):
    """Process a single JS file."""
    log.info("Processing JS file: {0}.".format(js_file_path))
    with open(js_file_path, encoding="utf-8") as js_file:
        original_js = js_file.read()
    log.debug("INPUT: Reading JS file {0}.".format(js_file_path))
    minified_js = js_minify(original_js)
    if timestamp:
        taim = "/* {} */ ".format(datetime.now().isoformat()[:-7].lower())
        minified_js = taim + minified_js
    if output_path is None:
        min_js_file_path = prefixer_extensioner(
            js_file_path, ".js", ".js" if overwrite else ".min.js",
            original_js)
        if zipy:
            gz_file_path = prefixer_extensioner(
                js_file_path, ".js", ".js.gz" if overwrite else ".min.js.gz",
                original_js)
            log.debug("OUTPUT: Writing ZIP JS {}.".format(gz_file_path))
    else:
        min_js_file_path = gz_file_path = output_path
    if not zipy or output_path is None:
        # if specific output path is requested,write write only one output file
        with open(min_js_file_path, "w", encoding="utf-8") as output_file:
            output_file.write(minified_js)
    if zipy:
        with gzip.open(gz_file_path, "wt", encoding="utf-8") as output_gz:
            output_gz.write(minified_js)
    log.debug("OUTPUT: Writing JS Minified {0}.".format(min_js_file_path))
    return min_js_file_path


def make_arguments_parser():
    """Build and return a command line agument parser."""
    parser = ArgumentParser(description=__doc__, epilog="""CSS-HTML-JS-Minify:
    Takes a file or folder full path string and process all CSS/HTML/JS found.
    If argument is not file/folder will fail. Check Updates works on Python3.
    Std-In to Std-Out is deprecated since it may fail with unicode characters.
    SHA1 HEX-Digest 11 Chars Hash on Filenames is used for Server Cache.
    CSS Properties are Alpha-Sorted, to help spot cloned ones, Selectors not.
    Watch works for whole folders, with minimum of ~60 Secs between runs.""")
    parser.add_argument('--version', action='version', version=__version__)
    parser.add_argument('fullpath', metavar='fullpath', type=str,
                        help='Full path to local file or folder.')
    parser.add_argument('--wrap', action='store_true',
                        help="Wrap output to ~80 chars per line, CSS only.")
    parser.add_argument('--prefix', type=str,
                        help="Prefix string to prepend on output filenames.")
    parser.add_argument('--timestamp', action='store_true',
                        help="Add a Time Stamp on all CSS/JS output files.")
    parser.add_argument('--quiet', action='store_true', help="Quiet, Silent.")
    parser.add_argument('--hash', action='store_true',
                        help="Add SHA1 HEX-Digest 11chars Hash to Filenames.")
    parser.add_argument('--zipy', action='store_true',
                        help="GZIP Minified files as '*.gz', CSS/JS only.")
    parser.add_argument('--sort', action='store_true',
                        help="Alphabetically Sort CSS Properties, CSS only.")
    parser.add_argument('--comments', action='store_true',
                        help="Keep comments, CSS/HTML only (Not Recommended)")
    parser.add_argument('--overwrite', action='store_true',
                        help="Force overwrite all in-place (Not Recommended)")
    parser.add_argument('--after', type=str,
                        help="Command to execute after run (Experimental).")
    parser.add_argument('--before', type=str,
                        help="Command to execute before run (Experimental).")
    parser.add_argument('--watch', action='store_true', help="Watch changes.")
    parser.add_argument('--multiple', action='store_true',
                        help="Allow Multiple instances (Not Recommended).")
    parser.add_argument('--beep', action='store_true',
                        help="Beep sound will be played when it ends at exit.")
    return parser.parse_args()


def prepare():
    """Prepare basic setup for main loop running."""
    global log
    log = make_logger("css-html-js-minify", emoji=True)  # Make a Logger Log.
    set_terminal_title("css-html-js-minify")
    check_encoding()  # AutoMagically Check Encodings/root
    set_process_name("css-html-js-minify")  # set Name
    set_single_instance("css-html-js-minify")  # set Single Instance
    return log


def main():
    """Main Loop."""
    args = make_arguments_parser()
    log.disable(log.CRITICAL) if args.quiet else log.debug("Max Logging ON")
    log.info(__doc__ + __version__)
    check_folder(os.path.dirname(args.fullpath))
    atexit.register(beep) if args.beep else log.debug("Beep sound at exit OFF")
    if os.path.isfile(args.fullpath) and args.fullpath.endswith(".css"):
        log.info("Target is a CSS File.")  # Work based on if argument is
        list_of_files = str(args.fullpath)  # file or folder, folder is slower.
        process_single_css_file(
            args.fullpath, wrap=args.wrap, timestamp=args.timestamp,
            comments=args.comments, sort=args.sort, overwrite=args.overwrite,
            zipy=args.zipy, prefix=args.prefix, add_hash=args.hash)
    elif os.path.isfile(args.fullpath) and args.fullpath.endswith(
            ".html" if args.overwrite else ".htm"):
        log.info("Target is HTML File.")
        list_of_files = str(args.fullpath)
        process_single_html_file(
            args.fullpath, comments=args.comments,
            overwrite=args.overwrite, prefix=args.prefix, add_hash=args.hash)
    elif os.path.isfile(args.fullpath) and args.fullpath.endswith(".js"):
        log.info("Target is a JS File.")
        list_of_files = str(args.fullpath)
        process_single_js_file(
            args.fullpath, timestamp=args.timestamp,
            overwrite=args.overwrite, zipy=args.zipy)
    elif os.path.isdir(args.fullpath):
        log.info("Target is a Folder with CSS, HTML, JS files !.")
        log.warning("Processing a whole Folder may take some time...")
        list_of_files = walk2list(
            args.fullpath,
            (".css", ".js", ".html" if args.overwrite else ".htm"),
            (".min.css", ".min.js", ".htm" if args.overwrite else ".html"))
        log.info('Total Maximum CPUs used: ~{0} Cores.'.format(cpu_count()))
        pool = Pool(cpu_count())  # Multiprocessing Async
        pool.map_async(partial(
                process_multiple_files, watch=args.watch,
                wrap=args.wrap, timestamp=args.timestamp,
                comments=args.comments, sort=args.sort,
                overwrite=args.overwrite, zipy=args.zipy,
                prefix=args.prefix, add_hash=args.hash),
            list_of_files)
        pool.close()
        pool.join()
    else:
        log.critical("File or folder not found,or cant be read,or I/O Error.")
        sys.exit(1)
    if args.after and getoutput:
        log.info(getoutput(str(args.after)))
    log.info('\n {0} \n Files Processed: {1}.'.format('-' * 80, list_of_files))
    log.info('Number of Files Processed: {0}.'.format(
        len(list_of_files) if isinstance(list_of_files, tuple) else 1))
    set_terminal_title()
    make_post_exec_msg(start_time)
