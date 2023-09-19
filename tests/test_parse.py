import pytest
from xmlpydict import parse
import json


def test_simple():
    assert parse("") == {}
    assert parse("<p/>") == {"p": {}}
    assert parse("<p></p>") == {"p": {}}
    assert parse('<p width="10"></p>') == {"p": {"@width": "10"}}
    assert parse('<p width="10">Hello World</p>') == {
        "p": {"@width": "10", "#text": "Hello World"}
    }
    assert parse('<p width="10" height="20"></p>') == {
        "p": {"@width": "10", "@height": "20"}
    }
    assert parse('<p width="10" height="20"/>') == {
        "p": {"@width": "10", "@height": "20"}
    }
    assert parse('<p width = "10"  height = "20"/>') == {
        "p": {"@width": "10", "@height": "20"}
    }
    assert parse("<p>Hey <b>bold</b>There</p>") == {
        "p": {"#text": "HeyThere", "b": {"#text": "bold"}}
    }

    assert (
        parse(
            """<svg width="200" height="100" xmlns="http://www.w3.org/2000/svg">
  <defs>
    <linearGradient id="gradient" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" style="stop-color:rgb(255,255,0);stop-opacity:1" />
      <stop offset="100%" style="stop-color:rgb(255,0,0);stop-opacity:1" />
    </linearGradient>
  </defs>
  <rect width="200" height="100" fill="url(#gradient)" />
</svg>"""
        )
        == {
            "svg": {
                "@height": "100",
                "@width": "200",
                "@xmlns": "http://www.w3.org/2000/svg",
                "defs": {
                    "linearGradient": {
                        "@id": "gradient",
                        "@x1": "0%",
                        "@x2": "100%",
                        "@y1": "0%",
                        "@y2": "0%",
                        "stop": [
                            {
                                "@offset": "0%",
                                "@style": "stop-color:rgb(255,255,0);stop-opacity:1",
                            },
                            {
                                "@offset": "100%",
                                "@style": "stop-color:rgb(255,0,0);stop-opacity:1",
                            },
                        ],
                    }
                },
                "rect": {"@fill": "url(#gradient)", "@height": "100", "@width": "200"},
            }
        }
    )


def test_cdata():
    assert parse("<content><![CDATA[<p>This is a paragraph</p>]]></content>") == {
        "content": {"#text": "<p>This is a paragraph</p>"}
    }


def test_nested():
    assert parse("<book><p/></book> ") == {"book": {"p": {}}}
    assert parse("<book><p></p></book>") == {"book": {"p": {}}}
    assert parse("<book><p></p></book><card/>") == {"book": {"p": {}}, "card": {}}
    assert parse("<pizza></pizza><book><p></p></book><card/>") == {
        "pizza": {},
        "book": {"p": {}},
        "card": {},
    }


def test_list():
    xml_items = """<items>
  <item id="1"></item>
  <item id="2"></item>
  <item id="3"></item>
</items>"""
    assert parse(xml_items) == {
        "items": {"item": [{"@id": "1"}, {"@id": "2"}, {"@id": "3"}]}
    }


def test_comment():
    assert parse("<!-- simple comment -->") == {}
    comment = """<world>
  <!-- $comment+++@python -->
  <lake>Content</lake>
</world>"""
    assert parse(comment) == {"world": {"lake": {"#text": "Content"}}}


def test_files():
    svg_data = parse(
        """<svg xmlns="http://www.w3.org/2000/svg" width="400" height="400">
  <rect x="50" y="50" width="100" height="50" fill="blue" />
  <circle cx="200" cy="100" r="50" fill="red" />
  <ellipse cx="350" cy="75" rx="50" ry="25" fill="green" />
  <line x1="50" y1="200" x2="150" y2="300" stroke="orange" />
  <polyline points="200,200 250,250 300,200 350,250" fill="none" stroke="purple" />
  <polygon points="350,200 400,250 400,150" fill="yellow" />
  <path d="M50,350 L100,350 Q125,375 150,350 T200,350" fill="none" stroke="black"/>

  <rect x="10" y="10" height="100" width="100"
        style="stroke:#ff0000; fill: #0000ff"/>
        <path d="M50,350 L100,350 Q125,375 150,350 T200,350" fill="none" stroke="black"/><polygon points="350,200 400,250 400,150" fill="yellow" />


  <circle cx="200" cy="100" r="50" fill="red"></circle>

  <polygon points="350,200 400,250 400,150" fill="yellow" />
</svg>"""
    )

    json_data = json.loads(
        """{
    "svg": {
        "@xmlns": "http://www.w3.org/2000/svg",
        "@width": "400",
        "@height": "400",
        "rect": [
            {
                "@x": "50",
                "@y": "50",
                "@width": "100",
                "@height": "50",
                "@fill": "blue"
            },
            {
                "@x": "10",
                "@y": "10",
                "@height": "100",
                "@width": "100",
                "@style": "stroke:#ff0000; fill: #0000ff"
            }
        ],
        "circle": [
            {
                "@cx": "200",
                "@cy": "100",
                "@r": "50",
                "@fill": "red"
            },
            {
                "@cx": "200",
                "@cy": "100",
                "@r": "50",
                "@fill": "red"
            }
        ],
        "ellipse": {
            "@cx": "350",
            "@cy": "75",
            "@rx": "50",
            "@ry": "25",
            "@fill": "green"
        },
        "line": {
            "@x1": "50",
            "@y1": "200",
            "@x2": "150",
            "@y2": "300",
            "@stroke": "orange"
        },
        "polyline": {
            "@points": "200,200 250,250 300,200 350,250",
            "@fill": "none",
            "@stroke": "purple"
        },
        "polygon": [
            {
                "@points": "350,200 400,250 400,150",
                "@fill": "yellow"
            },
            {
                "@points": "350,200 400,250 400,150",
                "@fill": "yellow"
            },
            {
                "@points": "350,200 400,250 400,150",
                "@fill": "yellow"
            }
        ],
        "path": [
            {
                "@d": "M50,350 L100,350 Q125,375 150,350 T200,350",
                "@fill": "none",
                "@stroke": "black"
            },
            {
                "@d": "M50,350 L100,350 Q125,375 150,350 T200,350",
                "@fill": "none",
                "@stroke": "black"
            }
        ]
    }
}"""
    )

    assert svg_data == json_data

    svg_data = parse(
        """<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200">
  <rect x="10" y="10" width="180" height="180" fill="lightgray" />
  
  <g transform="translate(100,100)">
    <circle cx="0" cy="0" r="80" fill="blue" />
    
    <g transform="translate(-40,-40)">
      <rect x="0" y="0" width="80" height="80" fill="red" />
      <text x="40" y="60" text-anchor="middle" fill="white">Nested</text>
    </g>
  </g>
</svg>"""
    )

    json_data = json.loads(
        """{
    "svg": {
        "@xmlns": "http://www.w3.org/2000/svg",
        "@width": "200",
        "@height": "200",
        "rect": {
            "@x": "10",
            "@y": "10",
            "@width": "180",
            "@height": "180",
            "@fill": "lightgray"
        },
        "g": {
            "@transform": "translate(100,100)",
            "circle": {
                "@cx": "0",
                "@cy": "0",
                "@r": "80",
                "@fill": "blue"
            },
            "g": {
                "@transform": "translate(-40,-40)",
                "rect": {
                    "@x": "0",
                    "@y": "0",
                    "@width": "80",
                    "@height": "80",
                    "@fill": "red"
                },
                "text": {
                    "@x": "40",
                    "@y": "60",
                    "@text-anchor": "middle",
                    "@fill": "white",
                    "#text": "Nested"
                }
            }
        }
    }
}"""
    )

    assert svg_data == json_data


def test_exception():
    xml_strings = [
        "< p/>",
        "<p>",
        "<p/ >",
        "<p height'10'/>",
        "<p height='10'width='5'/>",
        "<p width='5/>",
        "<p width=5'/>",
        "</p>",
        "<pwidth='5'/>",
    ]
    for xml_str in xml_strings:
        with pytest.raises(Exception):
            parse(xml_str)


def test_prefix():
    assert parse("<p></p>", attr_prefix="$") == {"p": {}}
    assert parse('<p width="10"></p>', attr_prefix="$") == {"p": {"$width": "10"}}
    assert parse('<p width="10" height="5"></p>', attr_prefix="$") == {
        "p": {"$width": "10", "$height": "5"}
    }
