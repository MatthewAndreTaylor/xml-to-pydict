# xmlpydict 📑

[![XML Tests](https://github.com/MatthewAndreTaylor/xml-to-pydict/actions/workflows/tests.yml/badge.svg)](https://github.com/MatthewAndreTaylor/xml-to-pydict/actions/workflows/tests.yml)
[![PyPI versions](https://img.shields.io/badge/python-3.7%2B-blue)](https://github.com/MatthewAndreTaylor/xml-to-pydict)

## Requirements

- `python 3.7+`

## Installation

To install xmlpydict, using pip:

```bash
pip install xmlpydict
```

## Quickstart

```py
>>> from xmlpydict import parse
>>> parse("<package><xmlpydict language='python'/></package>")
{'package': {'xmlpydict': {'@language': 'python'}}}
>>> parse("<person name='Matthew'>Hello!</person>")
{'person': {'@name': 'Matthew', '#text': 'Hello!'}}
```
