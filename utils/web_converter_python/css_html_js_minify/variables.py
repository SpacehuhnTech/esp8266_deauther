#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Created by: juancarlospaco
# GitHub Repo: https://github.com/juancarlospaco/css-html-js-minify

"""Variables for CSS processing for CSS-HTML-JS-Minify."""


# 'Color Name String': (R, G, B)
EXTENDED_NAMED_COLORS = {
    'azure': (240, 255, 255),
    'beige': (245, 245, 220),
    'bisque': (255, 228, 196),
    'blanchedalmond': (255, 235, 205),
    'brown': (165, 42, 42),
    'burlywood': (222, 184, 135),
    'chartreuse': (127, 255, 0),
    'chocolate': (210, 105, 30),
    'coral': (255, 127, 80),
    'cornsilk': (255, 248, 220),
    'crimson': (220, 20, 60),
    'cyan': (0, 255, 255),
    'darkcyan': (0, 139, 139),
    'darkgoldenrod': (184, 134, 11),
    'darkgray': (169, 169, 169),
    'darkgreen': (0, 100, 0),
    'darkgrey': (169, 169, 169),
    'darkkhaki': (189, 183, 107),
    'darkmagenta': (139, 0, 139),
    'darkolivegreen': (85, 107, 47),
    'darkorange': (255, 140, 0),
    'darkorchid': (153, 50, 204),
    'darkred': (139, 0, 0),
    'darksalmon': (233, 150, 122),
    'darkseagreen': (143, 188, 143),
    'darkslategray': (47, 79, 79),
    'darkslategrey': (47, 79, 79),
    'darkturquoise': (0, 206, 209),
    'darkviolet': (148, 0, 211),
    'deeppink': (255, 20, 147),
    'dimgray': (105, 105, 105),
    'dimgrey': (105, 105, 105),
    'firebrick': (178, 34, 34),
    'forestgreen': (34, 139, 34),
    'gainsboro': (220, 220, 220),
    'gold': (255, 215, 0),
    'goldenrod': (218, 165, 32),
    'gray': (128, 128, 128),
    'green': (0, 128, 0),
    'grey': (128, 128, 128),
    'honeydew': (240, 255, 240),
    'hotpink': (255, 105, 180),
    'indianred': (205, 92, 92),
    'indigo': (75, 0, 130),
    'ivory': (255, 255, 240),
    'khaki': (240, 230, 140),
    'lavender': (230, 230, 250),
    'lavenderblush': (255, 240, 245),
    'lawngreen': (124, 252, 0),
    'lemonchiffon': (255, 250, 205),
    'lightcoral': (240, 128, 128),
    'lightcyan': (224, 255, 255),
    'lightgray': (211, 211, 211),
    'lightgreen': (144, 238, 144),
    'lightgrey': (211, 211, 211),
    'lightpink': (255, 182, 193),
    'lightsalmon': (255, 160, 122),
    'lightseagreen': (32, 178, 170),
    'lightslategray': (119, 136, 153),
    'lightslategrey': (119, 136, 153),
    'lime': (0, 255, 0),
    'limegreen': (50, 205, 50),
    'linen': (250, 240, 230),
    'magenta': (255, 0, 255),
    'maroon': (128, 0, 0),
    'mediumorchid': (186, 85, 211),
    'mediumpurple': (147, 112, 219),
    'mediumseagreen': (60, 179, 113),
    'mediumspringgreen': (0, 250, 154),
    'mediumturquoise': (72, 209, 204),
    'mediumvioletred': (199, 21, 133),
    'mintcream': (245, 255, 250),
    'mistyrose': (255, 228, 225),
    'moccasin': (255, 228, 181),
    'navy': (0, 0, 128),
    'oldlace': (253, 245, 230),
    'olive': (128, 128, 0),
    'olivedrab': (107, 142, 35),
    'orange': (255, 165, 0),
    'orangered': (255, 69, 0),
    'orchid': (218, 112, 214),
    'palegoldenrod': (238, 232, 170),
    'palegreen': (152, 251, 152),
    'paleturquoise': (175, 238, 238),
    'palevioletred': (219, 112, 147),
    'papayawhip': (255, 239, 213),
    'peachpuff': (255, 218, 185),
    'peru': (205, 133, 63),
    'pink': (255, 192, 203),
    'plum': (221, 160, 221),
    'purple': (128, 0, 128),
    'rosybrown': (188, 143, 143),
    'saddlebrown': (139, 69, 19),
    'salmon': (250, 128, 114),
    'sandybrown': (244, 164, 96),
    'seagreen': (46, 139, 87),
    'seashell': (255, 245, 238),
    'sienna': (160, 82, 45),
    'silver': (192, 192, 192),
    'slategray': (112, 128, 144),
    'slategrey': (112, 128, 144),
    'snow': (255, 250, 250),
    'springgreen': (0, 255, 127),
    'teal': (0, 128, 128),
    'thistle': (216, 191, 216),
    'tomato': (255, 99, 71),
    'turquoise': (64, 224, 208),
    'violet': (238, 130, 238),
    'wheat': (245, 222, 179)
}


# Do Not compact this string, new lines are used to Group up stuff.
CSS_PROPS_TEXT = '''

alignment-adjust alignment-baseline animation animation-delay
animation-direction animation-duration animation-iteration-count
animation-name animation-play-state animation-timing-function appearance
azimuth

backface-visibility background background-blend-mode background-attachment
background-clip background-color background-image background-origin
background-position background-position-block background-position-inline
background-position-x background-position-y background-repeat background-size
baseline-shift bikeshedding bookmark-label bookmark-level bookmark-state
bookmark-target border border-bottom border-bottom-color
border-bottom-left-radius border-bottom-parts border-bottom-right-radius
border-bottom-style border-bottom-width border-clip border-clip-top
border-clip-right border-clip-bottom border-clip-left border-collapse
border-color border-corner-shape border-image border-image-outset
border-image-repeat border-image-slice border-image-source border-image-width
border-left border-left-color border-left-style border-left-parts
border-left-width border-limit border-parts border-radius border-right
border-right-color border-right-style border-right-width border-right-parts
border-spacing border-style border-top border-top-color border-top-left-radius
border-top-parts border-top-right-radius border-top-style border-top-width
border-width bottom box-decoration-break box-shadow box-sizing

caption-side clear clip color column-count column-fill column-gap column-rule
column-rule-color column-rule-style column-rule-width column-span column-width
columns content counter-increment counter-reset corners corner-shape
cue cue-after cue-before cursor

direction display drop-initial-after-adjust drop-initial-after-align
drop-initial-before-adjust drop-initial-before-align drop-initial-size
drop-initial-value

elevation empty-cells

flex flex-basis flex-direction flex-flow flex-grow flex-shrink flex-wrap fit
fit-position float font font-family font-size font-size-adjust font-stretch
font-style font-variant font-weight

grid-columns grid-rows

justify-content

hanging-punctuation height hyphenate-character hyphenate-resource hyphens

icon image-orientation image-resolution inline-box-align

left letter-spacing line-height line-stacking line-stacking-ruby
line-stacking-shift line-stacking-strategy linear-gradient list-style
list-style-image list-style-position list-style-type

margin margin-bottom margin-left margin-right margin-top marquee-direction
marquee-loop marquee-speed marquee-style max-height max-width min-height
min-width

nav-index

opacity orphans outline outline-color outline-offset outline-style
outline-width overflow overflow-style overflow-x overflow-y

padding padding-bottom padding-left padding-right padding-top page
page-break-after page-break-before page-break-inside pause pause-after
pause-before perspective perspective-origin pitch pitch-range play-during
position presentation-level

quotes

resize rest rest-after rest-before richness right rotation rotation-point
ruby-align ruby-overhang ruby-position ruby-span

size speak speak-header speak-numeral speak-punctuation speech-rate src
stress string-set

table-layout target target-name target-new target-position text-align
text-align-last text-decoration text-emphasis text-indent text-justify
text-outline text-shadow text-transform text-wrap top transform
transform-origin transition transition-delay transition-duration
transition-property transition-timing-function

unicode-bidi unicode-range

vertical-align visibility voice-balance voice-duration voice-family
voice-pitch voice-range voice-rate voice-stress voice-volume volume

white-space widows width word-break word-spacing word-wrap

z-index

'''
