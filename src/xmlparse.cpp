/**
 * Copyright (c) 2023 Matthew Andre Taylor
 */
#include <Python.h>
#include <string>
#include <vector>

typedef enum { PRIMITIVE, CONTAINER_OPEN, CONTAINER_CLOSE, TEXT, COMMENT } NodeType;

typedef struct {
  std::string key;
  std::string value;
} Pair;

typedef struct {
  NodeType type;
  std::string elementName;
  std::vector<Pair> attr;
} XMLNode;

size_t i;

static void parseContainerClose(XMLNode* node, const char *xmlContent) {
  node->type = CONTAINER_CLOSE;
  i++;
  if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':') {
    node->elementName.push_back(xmlContent[i]);
    i++;
  } else {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return;
  }

  while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
    if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':' || xmlContent[i] == '-' || xmlContent[i] == '.') {
      node->elementName.push_back(xmlContent[i]);
    } else if (std::isspace(xmlContent[i])){
      if (node->elementName.empty()) {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
        return;
      }
    } else {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return;
    }
    i++;
  }
  i++;
  return;
}

static void parseContainerOpen(XMLNode* node, const char *xmlContent) {
  node->type = CONTAINER_OPEN;

  if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':') {
    node->elementName.push_back(xmlContent[i]);
    i++;
  } else {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return;
  }

  bool hasAttr = false;

  // Parse name
  while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
    if (xmlContent[i] == '/' && xmlContent[i+1] == '>') {
      node->type = PRIMITIVE;
      i+=2;
      return;
    }

    if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':' || xmlContent[i] == '-' || xmlContent[i] == '.') {
      node->elementName.push_back(xmlContent[i]);
      i++;
    } else if (std::isspace(xmlContent[i])){
      if (node->elementName.empty()) {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
        return;
      }
      hasAttr = true;
      break;
    } else {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return;
    }
  }

  char state = 0; // 0: space, 1: start, 2: name, 3: equals, 4: quote, 5: value

  if (hasAttr) {
    std::string key;
    std::string val;
    char quoteType = 0;

    while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
      if (state == 0) {
        if (xmlContent[i] == '/' && xmlContent[i+1] == '>') {
          node->type = PRIMITIVE;
          i++;
          break;
        }
        if (std::isspace(xmlContent[i])) {
          i++;
          state=1;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
          return;
        }
      }
      else if (state == 1) {
        if (xmlContent[i] == '/' && xmlContent[i+1] == '>') {
          node->type = PRIMITIVE;
          i++;
          break;
        }

        if (std::isspace(xmlContent[i])) {
          i++;
        } else if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':') {
          state = 2;
          key.push_back(xmlContent[i]);
          i++;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
          return;
        }
      } else if (state == 2) {
        if (xmlContent[i] == '=') {
          state = 4;
        } else if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' || xmlContent[i] == ':' || xmlContent[i] == '-' || xmlContent[i] == '.') {
          key.push_back(xmlContent[i]);
        } else if (std::isspace(xmlContent[i])) {
          state = 3;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
          return;
        }
        i++;
      } else if (state == 3) {
        if (xmlContent[i] == '=') {
          state = 4;
        } else if (!std::isspace(xmlContent[i])) {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
          return;
        }
        i++;
      } else if (state == 4) {
        if (xmlContent[i] == '\'' || xmlContent[i] == '\"') {
          state = 5;
          quoteType = xmlContent[i];
        } else if (!std::isspace(xmlContent[i])) {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
          return;
        }
        i++;
      } else {
        if (xmlContent[i] == quoteType) {
          state = 0;
          node->attr.push_back({key, val});
          key.clear();
          val.clear();
        } else {
          val.push_back(xmlContent[i]);
        }
        i++;
      }
    }
  }
  if (state > 1) {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return;
  }
  i++;
  return;
}

static void parseComment(XMLNode* node, const char *xmlContent) {
  node->type = COMMENT;
  i++;
  if (xmlContent[i] != '-' || xmlContent[i+1] != '-') {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return;
  }
  i+=2;

  while (xmlContent[i] != '\0' || xmlContent[i+1] != '\0') {
    if (xmlContent[i] == '-' && xmlContent[i + 1] == '-' && xmlContent[i + 2] == '>') {
      // Found the end of the comment
      if (xmlContent[i-1] == '-') {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i-1);
        return;
      }
      i += 3;
      return;
    }
    i++;
  }
  PyErr_SetString(PyExc_Exception, "unclosed token");
  return;
}

static void parseText(XMLNode* node, const char *xmlContent) {
  node->type = TEXT;
  bool isSpace = false;

  while (xmlContent[i] != '\0' && xmlContent[i] != '<') {
    if (xmlContent[i] == '&') {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return;
    }
    if (isSpace || !std::isspace(xmlContent[i])) {
      node->elementName.push_back(xmlContent[i]);
      isSpace = true;
    }
    i++;
  }
  while (std::isspace(node->elementName.back())) {
    node->elementName.pop_back();
  }
}

static std::vector<XMLNode> splitNodes(const char *xmlContent) {
  std::vector<XMLNode> nodes;
  i = 0;
  
  while (xmlContent[i] != '\0') {
    XMLNode node;
    if (xmlContent[i] == '<') {
      i++;
      if (xmlContent[i] == '/') {
        parseContainerClose(&node, xmlContent);
      } else if (xmlContent[i] == '!') {
        parseComment(&node, xmlContent);
      } else {
        parseContainerOpen(&node, xmlContent);
      }
    } else {
      parseText(&node, xmlContent);
    }
    if (!node.elementName.empty()) {
      nodes.push_back(node);
    }
  }

  return nodes;
}

static PyObject *createDict(const std::vector<Pair> &attributes) {
  PyObject *dict = PyDict_New();
  for (const Pair &attr : attributes) {
    const std::string &key = "@" + attr.key;
    PyObject *val = PyUnicode_FromString(attr.value.c_str());
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

  std::vector<XMLNode> nodes = splitNodes(xmlContent);
  if (PyErr_Occurred() != NULL) {
    return NULL;
  }
  PyObject *currDict = PyDict_New();
  std::vector<std::string> containerStackNames;
  std::vector<PyObject *> containerStack;
  containerStack.push_back(currDict);
  containerStackNames.push_back("");

  bool isList = false;

  for (const XMLNode &node : nodes) {
    PyObject *childKey = PyUnicode_FromString(node.elementName.c_str());

    if (node.type == TEXT) {
      PyDict_SetItemString(currDict, "#text", childKey);
    } else if (node.type == CONTAINER_OPEN || node.type == PRIMITIVE) {
      PyObject *d = createDict(node.attr);

      PyObject *item = PyDict_GetItem(currDict, childKey);
      if (item != NULL) {
        // Check if it is a List or dict
        if (isList && PyList_Check(item)) {
          PyList_Append(item, d);
        } else {
          PyObject *children = PyList_New(2);
          PyList_SetItem(children, 0, item); 
          PyList_SetItem(children, 1, d); 
          PyDict_SetItem(currDict, childKey, children);
          isList = true;
        }
      } else {
        PyDict_SetItem(currDict, childKey, d);
        isList = false;
      }

      if (node.type == CONTAINER_OPEN) {
        currDict = d;
        containerStack.push_back(d);
        containerStackNames.push_back(node.elementName);
      }
    } else if (node.type == CONTAINER_CLOSE) {
      if (containerStackNames.back() != node.elementName) {
        PyErr_Format(PyExc_Exception, "tag mismatch ('%U' does not match the last start tag)", childKey);
      }
      containerStackNames.pop_back();
      containerStack.pop_back();
      currDict = containerStack.back();
    }

    Py_DECREF(childKey);
  }

  if (containerStack.size() > 1) {
    PyErr_Format(PyExc_Exception, "not well formed (%d unclosed tags)", containerStack.size()-1);
    return NULL;
  }

  PyObject *result = containerStack.front();
  Py_INCREF(result);
  return result;
}

static PyMethodDef XMLParserMethods[] = {
    {"parse", xml_parse, METH_VARARGS, xml_parse_doc}, {NULL, NULL, 0, NULL}};

static struct PyModuleDef xmlparsermodule = {
    PyModuleDef_HEAD_INIT, "xmlpydict", NULL, -1, XMLParserMethods};

PyMODINIT_FUNC PyInit_xmlpydict() {
  return PyModule_Create(&xmlparsermodule);
}