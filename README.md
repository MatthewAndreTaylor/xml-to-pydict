# xmlparse 📑

[![Tests](https://github.com/MatthewAndreTaylor/xml/actions/workflows/tests.yml/badge.svg)](https://github.com/MatthewAndreTaylor/xml/actions)
[![PyPI versions](https://img.shields.io/badge/python-3.7%2B-blue)](https://github.com/MatthewAndreTaylor/xml-to-pydict)

## Requirements

- `python 3.7+`

## Installation

To install xmlpydict_parser, using pip:

```bash
pip install xmlpydict_parser
```

## Quickstart

```py
>>> from xmlpydict_parser import parse
>>> parse("<package><xmlpydict_parser language='python'/></package>")
{'package': {'xmlpydict_parser': {'@language': 'python'}}}
>>> parse("<person name='Matthew'>Hello!</person>")
{'person': {'@name': 'Matthew', '#text': 'Hello!'}}
```
