# xmlpydict 📑

[![XML Tests](https://github.com/MatthewAndreTaylor/xml-to-pydict/actions/workflows/tests.yml/badge.svg)](https://github.com/MatthewAndreTaylor/xml-to-pydict/actions/workflows/tests.yml)
[![PyPI versions](https://img.shields.io/badge/python-3.7%2B-blue)](https://github.com/MatthewAndreTaylor/xml-to-pydict)
[![PyPI](https://img.shields.io/pypi/v/xmlpydict.svg)](https://pypi.org/project/xmlpydict/)

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

## Goals

Create a consistent parsing strategy between xml and python dictionaries.
xmlpydict takes a more laid pack approack to enforcing the syntax of xml.

## Features

xmlpydict allows for multiple root elements.
The root object is treated as the python object.

### xmlpydict supports the following 

[CDataSection](https://www.w3.org/TR/xml/#sec-cdata-sect):  CDATA Sections are stored as {'#text': CData}.

[Comments](https://www.w3.org/TR/xml/#sec-comments):  Comments are tokenized for corectness, but have no effect in what is returned.

[Element Tags](https://www.w3.org/TR/xml/#sec-starttags):  Allows for duplicate attributes, however only the latest defined will be taken. 

[Characters](https://www.w3.org/TR/xml/#charsets):  Similar to CDATA text is stored as {'#text': Char} , however this text is stripped.

### dict.get(key[, default]) will not cause exceptions

```py
# Empty tags are containers
>>> from xmlpydict import parse
>>> parse("<a></a>")
{'a': {}}
>>> parse("<a/>")
{'a': {}}
>>> parse("<a/>").get('href')
None
>>> parse("")
{}
```

### Attribute prefixing

```py
# Change prefix from default "@" with keyword argument attr_prefix
>>> from xmlpydict import parse
>>> parse('<p width="10" height="5"></p>', attr_prefix="$")
{"p": {"$width": "10", "$height": "5"}}
```


### Exceptions

```py
# Grammar and structure of the xml_content is checked while parsing
>>> from xmlpydict import parse
>>> parse("<a></ a>")
Exception: not well formed (violation at pos=5)
```


### Unsupported

Prolog / Enforcing Document Type Definition and Element Type Declarations

Entity Referencing

Namespaces
