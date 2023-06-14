import pytest
from xmlpydict_parser import parse
import json


def test_simple():
    assert parse("<p/>") == {"p": {}}
    assert parse("<p></p>") == {"p": {}}
    assert parse('<p width="10"></p>') == {"p": {"@width": "10"}}
    assert parse('<p width="10">Hello World</p>') == {
        "p": {"@width": "10", "#text": "Hello World"}
    }


def test_nested():
    assert parse("<book><p/></book>") == {"book": {"p": {}}}
    assert parse("<book><p></p></book>") == {"book": {"p": {}}}


def test_list():
    xml_items = """<items>
  <item id="1"></item>
  <item id="2"></item>
  <item id="3"></item>
</items>"""
    assert parse(xml_items) == {
        "items": {"item": [{"@id": "1"}, {"@id": "2"}, {"@id": "3"}]}
    }
    xml_items = """<items>
      <item id="1"/>
      <item id="2"/>
      <item id="3"/>
    </items>"""
    assert parse(xml_items) == {
        "items": {"item": [{"@id": "1"}, {"@id": "2"}, {"@id": "3"}]}
    }
    xml_items = """<items>
          <item id="1"/>
          <item id="2"></item>
          <item id="3"/>
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
