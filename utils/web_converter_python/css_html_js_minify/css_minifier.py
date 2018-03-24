#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: juancarlospaco
# GitHub Repo: https://github.com/juancarlospaco/css-html-js-minify

"""CSS Minifier functions for CSS-HTML-JS-Minify."""


import re
import itertools

import logging as log

from .variables import EXTENDED_NAMED_COLORS, CSS_PROPS_TEXT


__all__ = ['css_minify', 'condense_semicolons']


def _compile_props(props_text, grouped=False):
    """Take a list of props and prepare them."""
    props, prefixes = [], "-webkit-,-khtml-,-epub-,-moz-,-ms-,-o-,".split(",")
    for propline in props_text.strip().lower().splitlines():
        props += [pre + pro for pro in propline.split(" ") for pre in prefixes]
    props = filter(lambda line: not line.startswith('#'), props)
    if not grouped:
        props = list(filter(None, props))
        return props, [0]*len(props)
    final_props, groups, g_id = [], [], 0
    for prop in props:
        if prop.strip():
            final_props.append(prop)
            groups.append(g_id)
        else:
            g_id += 1
    return final_props, groups


def _prioritify(line_of_css, css_props_text_as_list):
    """Return args priority, priority is integer and smaller means higher."""
    sorted_css_properties, groups_by_alphabetic_order = css_props_text_as_list
    priority_integer, group_integer = 9999, 0
    for css_property in sorted_css_properties:
        if css_property.lower() == line_of_css.split(":")[0].lower().strip():
            priority_integer = sorted_css_properties.index(css_property)
            group_integer = groups_by_alphabetic_order[priority_integer]
            break
    return priority_integer, group_integer


def _props_grouper(props, pgs):
    """Return groups for properties."""
    if not props:
        return props
    # props = sorted([
        # _ if _.strip().endswith(";")
        # and not _.strip().endswith("*/") and not _.strip().endswith("/*")
        # else _.rstrip() + ";\n" for _ in props])
    props_pg = zip(map(lambda prop: _prioritify(prop, pgs), props), props)
    props_pg = sorted(props_pg, key=lambda item: item[0][1])
    props_by_groups = map(
        lambda item: list(item[1]),
        itertools.groupby(props_pg, key=lambda item: item[0][1]))
    props_by_groups = map(lambda item: sorted(
        item, key=lambda item: item[0][0]), props_by_groups)
    props = []
    for group in props_by_groups:
        group = map(lambda item: item[1], group)
        props += group
        props += ['\n']
    props.pop()
    return props


def sort_properties(css_unsorted_string):
    """CSS Property Sorter Function.

    This function will read buffer argument, split it to a list by lines,
    sort it by defined rule, and return sorted buffer if it's CSS property.
    This function depends on '_prioritify' function.
    """
    log.debug("Alphabetically Sorting all CSS / SCSS Properties.")
    css_pgs = _compile_props(CSS_PROPS_TEXT, grouped=False)  # Do Not Group.
    pattern = re.compile(r'(.*?{\r?\n?)(.*?)(}.*?)|(.*)',
                         re.DOTALL + re.MULTILINE)
    matched_patterns = pattern.findall(css_unsorted_string)
    sorted_patterns, sorted_buffer = [], css_unsorted_string
    re_prop = re.compile(r'((?:.*?)(?:;)(?:.*?\n)|(?:.*))',
                         re.DOTALL + re.MULTILINE)
    if len(matched_patterns) != 0:
        for matched_groups in matched_patterns:
            sorted_patterns += matched_groups[0].splitlines(True)
            props = map(lambda line: line.lstrip('\n'),
                        re_prop.findall(matched_groups[1]))
            props = list(filter(lambda line: line.strip('\n '), props))
            props = _props_grouper(props, css_pgs)
            sorted_patterns += props
            sorted_patterns += matched_groups[2].splitlines(True)
            sorted_patterns += matched_groups[3].splitlines(True)
        sorted_buffer = ''.join(sorted_patterns)
    return sorted_buffer


def remove_comments(css):
    """Remove all CSS comment blocks."""
    log.debug("Removing all Comments.")
    iemac, preserve = False, False
    comment_start = css.find("/*")
    while comment_start >= 0:  # Preserve comments that look like `/*!...*/`.
        # Slicing is used to make sure we dont get an IndexError.
        preserve = css[comment_start + 2:comment_start + 3] == "!"
        comment_end = css.find("*/", comment_start + 2)
        if comment_end < 0:
            if not preserve:
                css = css[:comment_start]
                break
        elif comment_end >= (comment_start + 2):
            if css[comment_end - 1] == "\\":
                # This is an IE Mac-specific comment; leave this one and the
                # following one alone.
                comment_start = comment_end + 2
                iemac = True
            elif iemac:
                comment_start = comment_end + 2
                iemac = False
            elif not preserve:
                css = css[:comment_start] + css[comment_end + 2:]
            else:
                comment_start = comment_end + 2
        comment_start = css.find("/*", comment_start)
    return css


def remove_unnecessary_whitespace(css):
    """Remove unnecessary whitespace characters."""
    log.debug("Removing all unnecessary white spaces.")

    def pseudoclasscolon(css):
        """Prevent 'p :link' from becoming 'p:link'.

        Translates 'p :link' into 'p ___PSEUDOCLASSCOLON___link'.
        This is translated back again later.
        """
        regex = re.compile(r"(^|\})(([^\{\:])+\:)+([^\{]*\{)")
        match = regex.search(css)
        while match:
            css = ''.join([
                css[:match.start()],
                match.group().replace(":", "___PSEUDOCLASSCOLON___"),
                css[match.end():]])
            match = regex.search(css)
        return css

    css = pseudoclasscolon(css)
    # Remove spaces from before things.
    css = re.sub(r"\s+([!{};:>\(\)\],])", r"\1", css)
    # If there is a `@charset`, then only allow one, and move to beginning.
    css = re.sub(r"^(.*)(@charset \"[^\"]*\";)", r"\2\1", css)
    css = re.sub(r"^(\s*@charset [^;]+;\s*)+", r"\1", css)
    # Put the space back in for a few cases, such as `@media screen` and
    # `(-webkit-min-device-pixel-ratio:0)`.
    css = re.sub(r"\band\(", "and (", css)
    # Put the colons back.
    css = css.replace('___PSEUDOCLASSCOLON___', ':')
    # Remove spaces from after things.
    css = re.sub(r"([!{}:;>\(\[,])\s+", r"\1", css)
    return css


def remove_unnecessary_semicolons(css):
    """Remove unnecessary semicolons."""
    log.debug("Removing all unnecessary semicolons.")
    return re.sub(r";+\}", "}", css)


def remove_empty_rules(css):
    """Remove empty rules."""
    log.debug("Removing all unnecessary empty rules.")
    return re.sub(r"[^\}\{]+\{\}", "", css)


def normalize_rgb_colors_to_hex(css):
    """Convert `rgb(51,102,153)` to `#336699`."""
    log.debug("Converting all rgba to hexadecimal color values.")
    regex = re.compile(r"rgb\s*\(\s*([0-9,\s]+)\s*\)")
    match = regex.search(css)
    while match:
        colors = map(lambda s: s.strip(), match.group(1).split(","))
        hexcolor = '#%.2x%.2x%.2x' % tuple(map(int, colors))
        css = css.replace(match.group(), hexcolor)
        match = regex.search(css)
    return css


def condense_zero_units(css):
    """Replace `0(px, em, %, etc)` with `0`."""
    log.debug("Condensing all zeroes on values.")
    return re.sub(r"([\s:])(0)(px|em|%|in|q|ch|cm|mm|pc|pt|ex|rem|s|ms|"
                  r"deg|grad|rad|turn|vw|vh|vmin|vmax|fr)", r"\1\2", css)


def condense_multidimensional_zeros(css):
    """Replace `:0 0 0 0;`, `:0 0 0;` etc. with `:0;`."""
    log.debug("Condensing all multidimensional zeroes on values.")
    return css.replace(":0 0 0 0;", ":0;").replace(
        ":0 0 0;", ":0;").replace(":0 0;", ":0;").replace(
            "background-position:0;", "background-position:0 0;").replace(
                "transform-origin:0;", "transform-origin:0 0;")


def condense_floating_points(css):
    """Replace `0.6` with `.6` where possible."""
    log.debug("Condensing all floating point values.")
    return re.sub(r"(:|\s)0+\.(\d+)", r"\1.\2", css)


def condense_hex_colors(css):
    """Shorten colors from #AABBCC to #ABC where possible."""
    log.debug("Condensing all hexadecimal color values.")
    regex = re.compile(
        r"""([^\"'=\s])(\s*)#([0-9a-f])([0-9a-f])([0-9a-f])"""
        r"""([0-9a-f])([0-9a-f])([0-9a-f])""", re.I | re.S)
    match = regex.search(css)
    while match:
        first = match.group(3) + match.group(5) + match.group(7)
        second = match.group(4) + match.group(6) + match.group(8)
        if first.lower() == second.lower():
            css = css.replace(
                match.group(), match.group(1) + match.group(2) + '#' + first)
            match = regex.search(css, match.end() - 3)
        else:
            match = regex.search(css, match.end())
    return css


def condense_whitespace(css):
    """Condense multiple adjacent whitespace characters into one."""
    log.debug("Condensing all unnecessary white spaces.")
    return re.sub(r"\s+", " ", css)


def condense_semicolons(css):
    """Condense multiple adjacent semicolon characters into one."""
    log.debug("Condensing all unnecessary multiple adjacent semicolons.")
    return re.sub(r";;+", ";", css)


def wrap_css_lines(css, line_length=80):
    """Wrap the lines of the given CSS to an approximate length."""
    log.debug("Wrapping lines to ~{0} max line lenght.".format(line_length))
    lines, line_start = [], 0
    for i, char in enumerate(css):
        # Its safe to break after } characters.
        if char == '}' and (i - line_start >= line_length):
            lines.append(css[line_start:i + 1])
            line_start = i + 1
    if line_start < len(css):
        lines.append(css[line_start:])
    return '\n'.join(lines)


def condense_font_weight(css):
    """Condense multiple font weights into shorter integer equals."""
    log.debug("Condensing font weights on values.")
    return css.replace('font-weight:normal;', 'font-weight:400;').replace(
        'font-weight:bold;', 'font-weight:700;')


def condense_std_named_colors(css):
    """Condense named color values to shorter replacement using HEX."""
    log.debug("Condensing standard named color values.")
    for color_name, color_hexa in iter(tuple({
        ':aqua;': ':#0ff;', ':blue;': ':#00f;',
            ':fuchsia;': ':#f0f;', ':yellow;': ':#ff0;'}.items())):
        css = css.replace(color_name, color_hexa)
    return css


def condense_xtra_named_colors(css):
    """Condense named color values to shorter replacement using HEX."""
    log.debug("Condensing extended named color values.")
    for k, v in iter(tuple(EXTENDED_NAMED_COLORS.items())):
        same_color_but_rgb = 'rgb({0},{1},{2})'.format(v[0], v[1], v[2])
        if len(k) > len(same_color_but_rgb):
            css = css.replace(k, same_color_but_rgb)
    return css


def remove_url_quotes(css):
    """Fix for url() does not need quotes."""
    log.debug("Removing quotes from url.")
    return re.sub(r'url\((["\'])([^)]*)\1\)', r'url(\2)', css)


def condense_border_none(css):
    """Condense border:none; to border:0;."""
    log.debug("Condense borders values.")
    return css.replace("border:none;", "border:0;")


def add_encoding(css):
    """Add @charset 'UTF-8'; if missing."""
    log.debug("Adding encoding declaration if needed.")
    return "@charset utf-8;" + css if "@charset" not in css.lower() else css


def restore_needed_space(css):
    """Fix CSS for some specific cases where a white space is needed."""
    return css.replace("!important", " !important").replace(  # !important
        "@media(", "@media (").replace(  # media queries # jpeg > jpg
            "data:image/jpeg;base64,", "data:image/jpg;base64,").rstrip("\n;")


def unquote_selectors(css):
    """Fix CSS for some specific selectors where Quotes is not needed."""
    log.debug("Removing unnecessary Quotes on selectors of CSS classes.")
    return re.compile('([a-zA-Z]+)="([a-zA-Z0-9-_\.]+)"]').sub(r'\1=\2]', css)


def css_minify(css, wrap=False, comments=False, sort=False, noprefix=False):
    """Minify CSS main function."""
    log.info("Compressing CSS...")
    css = remove_comments(css) if not comments else css
    css = sort_properties(css) if sort else css
    css = unquote_selectors(css)
    css = condense_whitespace(css)
    css = remove_url_quotes(css)
    css = condense_xtra_named_colors(css)
    css = condense_font_weight(css)
    css = remove_unnecessary_whitespace(css)
    css = condense_std_named_colors(css)
    css = remove_unnecessary_semicolons(css)
    css = condense_zero_units(css)
    css = condense_multidimensional_zeros(css)
    css = condense_floating_points(css)
    css = normalize_rgb_colors_to_hex(css)
    css = condense_hex_colors(css)
    css = condense_border_none(css)
    css = wrap_css_lines(css, 80) if wrap else css
    css = condense_semicolons(css)
    css = add_encoding(css) if not noprefix else css
    css = restore_needed_space(css)
    log.info("Finished compressing CSS !.")
    return css.strip()
