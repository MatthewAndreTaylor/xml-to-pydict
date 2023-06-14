/**
 * Copyright (c) 2023 Matthew Andre Taylor
 */
#include <Python.h>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

typedef enum { PRIMITVE, CONTAINER_OPEN, CONTAINER_CLOSE, TEXT } PrimitiveType;

typedef struct {
  std::string key;
  std::string value;
} Pair;

typedef struct Node {
  PrimitiveType type;
  std::string elementName;
  std::vector<Pair> attr;
} PathNode;

std::regex primitiveRegex(".*<([a-zA-Z_]+)", std::regex_constants::icase);
std::regex attrRegex(R"(([a-zA-Z][a-zA-Z0-9_-]*)=\"([^\"]*)\")",
                     std::regex_constants::icase);
std::regex selfclosingRegex(R"(\/>)", std::regex_constants::icase);
std::regex closingtagRegex(R"(</\w+>)", std::regex_constants::icase);
std::regex notCommentRegex("^(?!<!--)[\\s\\S]*?(?!-->)$");

PathNode ParsePrimitive(std::string &input) {
  std::smatch match;

  PathNode p;
  if (std::regex_search(input, match, primitiveRegex)) {
    std::string name = match.str(1);
    p.elementName = name;
    if (std::regex_search(input, selfclosingRegex)) {
      p.type = PRIMITVE;
    } else {
      p.type = CONTAINER_OPEN;
    }
  } else if (std::regex_search(input, closingtagRegex)) {
    p.type = CONTAINER_CLOSE;
    return p;
  } else {
    p.type = TEXT;
    p.elementName = input;
    return p;
  }
  std::string::size_type pos = (match.size() > 0) ? match[0].length() : 0;

  std::string attributes = input.substr(pos);

  std::sregex_iterator it(attributes.begin(), attributes.end(), attrRegex);
  std::sregex_iterator end;

  while (it != end) {
    std::smatch match = *it;
    std::string attrName = match.str(1);
    std::string attrValue = match.str(2);
    p.attr.push_back({std::move(attrName), std::move(attrValue)});
    ++it;
  }
  return p;
}

std::vector<std::string> splitNodes(const std::string &xmlContent) {
  std::vector<std::string> nodes;
  std::istringstream xmlStream(xmlContent);
  std::string line;
  std::string currentNode;
  std::string currentText;
  std::string text;

  while (std::getline(xmlStream, line)) {
    currentNode += line;

    // Check if the current node is complete (ends with ">") or a closing tag
    std::smatch match;
    std::regex regexPattern("<[^>]+>");
    while (std::regex_search(currentNode, match, regexPattern)) {
      std::string token = match.str();

      text.clear();

      // Extract the text between the nodes and push it to the nodes vector
      std::regex textRegex(">([^<]+)<");
      std::smatch textMatch;
      if (std::regex_search(currentNode, textMatch, textRegex)) {
        text = textMatch[1].str();
      }

      nodes.push_back(token);
      if (!text.empty()) {
        nodes.push_back(text);
      }
      currentNode = match.suffix().str();
    }
  }

  return nodes;
}

static PyObject *createDict(const std::vector<Pair> &attributes) {
  PyObject *dict = PyDict_New();
  for (size_t j = 0; j < attributes.size(); j++) {
    const std::string &key = "@" + attributes[j].key;
    const std::string &attrVal = attributes[j].value;
    PyObject *val = PyUnicode_FromString(attrVal.c_str());
    PyDict_SetItemString(dict, key.c_str(), val);
  }

  return dict;
}

PyDoc_STRVAR(xml_parse_doc, "Parse XML content into a dictionary.");
static PyObject *xml_parse(PyObject *self, PyObject *args) {
  const char *xmlContent;

  if (!PyArg_ParseTuple(args, "s", &xmlContent)) {
    return NULL;
  }

  std::string content(xmlContent);

  // Call the splitNodes and ParsePrimitive functions to process the XML
  std::vector<std::string> nodes = splitNodes(content);

  PyObject *dict = PyDict_New();

  PyObject *currDict = dict;
  std::vector<PyObject *> containerStack;
  containerStack.push_back(dict);

  for (size_t i = 0; i < nodes.size(); i++) {
    std::string &node = nodes[i];
    PathNode parsedNode = ParsePrimitive(node);

    PyObject *childKey = PyUnicode_FromString(parsedNode.elementName.c_str());

    if (parsedNode.type == TEXT) {
      if (std::regex_match(parsedNode.elementName, notCommentRegex)) {
        PyObject *item = PyDict_GetItemString(currDict, "#text");
        if (item != NULL) {
          PyUnicode_Concat(item, childKey);
        } else {
          PyDict_SetItemString(currDict, "#text", childKey);
        }
      }
    } else if (parsedNode.type == CONTAINER_OPEN) {
      PyObject *d = createDict(parsedNode.attr);

      PyObject *item = PyDict_GetItem(currDict, childKey);
      if (item != NULL) {
        // Check if it is a List or dict
        if (PyList_Check(item)) {
          PyList_Append(item, d);
        } else {
          PyObject *children = PyList_New(0);
          PyList_Append(children, item);
          PyList_Append(children, d);
          PyDict_SetItem(currDict, childKey, children);
        }
      } else {
        PyDict_SetItem(currDict, childKey, d);
      }

      currDict = d;
      containerStack.push_back(d);
    } else if (parsedNode.type == CONTAINER_CLOSE) {
      containerStack.pop_back();
      currDict = containerStack.back();
    } else {
      PyObject *dom = createDict(parsedNode.attr);

      PyObject *item = PyDict_GetItem(currDict, childKey);
      if (item != NULL) {
        // Check if it is a List or dict
        if (PyList_Check(item)) {
          PyList_Append(item, dom);
        } else {
          PyObject *children = PyList_New(0);
          PyList_Append(children, item);
          PyList_Append(children, dom);
          PyDict_SetItem(currDict, childKey, children);
        }
      } else {
        PyDict_SetItem(currDict, childKey, dom);
      }
    }

    Py_DECREF(childKey);
  }

  PyObject *result = containerStack.front();
  Py_INCREF(result);
  return result;
}

static PyMethodDef XMLParserMethods[] = {
    {"parse", xml_parse, METH_VARARGS, xml_parse_doc}, {NULL, NULL, 0, NULL}};

static struct PyModuleDef xmlparsermodule = {
    PyModuleDef_HEAD_INIT, "xmlpydict_parser", NULL, -1, XMLParserMethods};

PyMODINIT_FUNC PyInit_xmlpydict_parser() {
  return PyModule_Create(&xmlparsermodule);
}