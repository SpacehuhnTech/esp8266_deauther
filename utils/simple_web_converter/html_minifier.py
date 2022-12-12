#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: juancarlospaco
# GitHub Repo: https://github.com/juancarlospaco/css-html-js-minify

"""HTML Minifier functions for CSS-HTML-JS-Minify."""


import re

import logging as log


__all__ = ['html_minify']


def condense_html_whitespace(html):
    """Condense HTML, but be safe first if it have textareas or pre tags.

    >>> condense_html_whitespace('<i>  <b>    <a> test </a>    </b> </i><br>')
    '<i><b><a> test </a></b></i><br>'
    """  # first space between tags, then empty new lines and in-between.
    log.debug("Removing unnecessary HTML White Spaces and Empty New Lines.")
    tagsStack = []
    split = re.split('(<\\s*pre.*>|<\\s*/\\s*pre\\s*>|<\\s*textarea.*>|<\\s*/\\s*textarea\\s*>)', html, flags=re.IGNORECASE)
    for i in range(0, len(split)):
    	#if we are on a tag
        if (i + 1) % 2 == 0:
            tag = rawtag(split[i])
            if tag.startswith('/'):
                if not tagsStack or '/' + tagsStack.pop() != tag:
                    raise Exception("Some tag is not closed properly")
            else:
                tagsStack.append(tag)
            continue

		#else check if we are outside any nested <pre>/<textarea> tag
        if not tagsStack:
            temp = re.sub(r'>\s+<', '> <', split[i])
            split[i] = re.sub(r'\s{2,}|[\r\n]', ' ', temp)
    return ''.join(split)


def rawtag(str):
    if re.match('<\\s*pre.*>', str, flags=re.IGNORECASE):
        return 'pre'
    if re.match('<\\s*textarea.*>', str, flags=re.IGNORECASE):
        return 'txt'
    if re.match('<\\s*/\\s*pre\\s*>', str, flags=re.IGNORECASE):
        return '/pre'
    if re.match('<\\s*/\\s*textarea\\s*>', str, flags=re.IGNORECASE):
        return '/txt'

def condense_style(html):
    """Condense style html tags.

    >>> condense_style('<style type="text/css">*{border:0}</style><p>a b c')
    '<style>*{border:0}</style><p>a b c'
    """  # May look silly but Emmet does this and is wrong.
    log.debug("Condensing HTML Style CSS tags.")
    return html.replace('<style type="text/css">', '<style>').replace(
        "<style type='text/css'>", '<style>').replace(
            "<style type=text/css>", '<style>')


def condense_script(html):
    """Condense script html tags.

    >>> condense_script('<script type="text/javascript"> </script><p>a b c')
    '<script> </script><p>a b c'
    """  # May look silly but Emmet does this and is wrong.
    log.debug("Condensing HTML Script JS tags.")
    return html.replace('<script type="text/javascript">', '<script>').replace(
        "<style type='text/javascript'>", '<script>').replace(
            "<style type=text/javascript>", '<script>')


def clean_unneeded_html_tags(html):
    """Clean unneeded optional html tags.

    >>> clean_unneeded_html_tags('a<body></img></td>b</th></tr></hr></br>c')
    'abc'
    """
    log.debug("Removing unnecessary optional HTML tags.")
    for tag_to_remove in ("""</area> </base> <body> </body> </br> </col>
        </colgroup> </dd> </dt> <head> </head> </hr> <html> </html> </img>
        </input> </li> </link> </meta> </option> </param> <tbody> </tbody>
        </td> </tfoot> </th> </thead> </tr> </basefont> </isindex> </param>
            """.split()):
            html = html.replace(tag_to_remove, '')
    return html  # May look silly but Emmet does this and is wrong.


def remove_html_comments(html):
    """Remove all HTML comments, Keep all for Grunt, Grymt and IE.

    >>> _="<!-- build:dev -->a<!-- endbuild -->b<!--[if IE 7]>c<![endif]--> "
    >>> _+= "<!-- kill me please -->keep" ; remove_html_comments(_)
    '<!-- build:dev -->a<!-- endbuild -->b<!--[if IE 7]>c<![endif]--> keep'
    """  # Grunt uses comments to as build arguments, bad practice but still.
    log.debug("""Removing all unnecessary HTML comments; Keep all containing:
    'build:', 'endbuild', '<!--[if]>', '<![endif]-->' for Grunt/Grymt, IE.""")
    return re.compile('<!-- [^(build|endbuild)].*? -->', re.I).sub('', html)


def unquote_html_attributes(html):
    """Remove all HTML quotes on attibutes if possible.

    >>> unquote_html_attributes('<img   width="9" height="5" data-foo="0"  >')
    '<img width=9 height=5 data-foo=0 >'
    """  # data-foo=0> might cause errors on IE, we leave 1 space data-foo=0 >
    log.debug("Removing unnecessary Quotes on attributes of HTML tags.")
    # cache all regular expressions on variables before we enter the for loop.
    any_tag = re.compile(r"<\w.*?>", re.I | re.MULTILINE | re.DOTALL)
    space = re.compile(r' \s+|\s +', re.MULTILINE)
    space1 = re.compile(r'\w\s+\w', re.MULTILINE)
    space2 = re.compile(r'"\s+>', re.MULTILINE)
    space3 = re.compile(r"'\s+>", re.MULTILINE)
    space4 = re.compile('"\s\s+\w+="|\'\s\s+\w+=\'|"\s\s+\w+=|\'\s\s+\w+=',
                        re.MULTILINE)
    space6 = re.compile(r"\d\s+>", re.MULTILINE)
    quotes_in_tag = re.compile('([a-zA-Z]+)="([a-zA-Z0-9-_\.]+)"')
    # iterate on a for loop cleaning stuff up on the html markup.
    for tag in iter(any_tag.findall(html)):
        # exceptions of comments and closing tags
        if tag.startswith('<!') or tag.find('</') > -1:
            continue
        original = tag
        # remove white space inside the tag itself
        tag = space2.sub('" >', tag)  # preserve 1 white space is safer
        tag = space3.sub("' >", tag)
        for each in space1.findall(tag) + space6.findall(tag):
            tag = tag.replace(each, space.sub(' ', each))
        for each in space4.findall(tag):
            tag = tag.replace(each, each[0] + ' ' + each[1:].lstrip())
        # remove quotes on some attributes
        tag = quotes_in_tag.sub(r'\1=\2 ', tag)  # See Bug #28
        if original != tag:  # has the tag been improved ?
            html = html.replace(original, tag)
    return html.strip()


def html_minify(html, comments=False):
    """Minify HTML main function.

    >>> html_minify(' <p  width="9" height="5"  > <!-- a --> b </p> c <br> ')
    '<p width=9 height=5 > b c <br>'
    """
    log.info("Compressing HTML...")
    html = remove_html_comments(html) if not comments else html
    html = condense_style(html)
    html = condense_script(html)
    html = clean_unneeded_html_tags(html)
    html = condense_html_whitespace(html)
    html = unquote_html_attributes(html)
    log.info("Finished compressing HTML !.")
    return html.strip()
