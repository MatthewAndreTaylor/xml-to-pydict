/**
 * Copyright (c) 2023 Matthew Andre Taylor
 */
#include <Python.h>
#include <string>
#include <vector>
#include <stdio.h>

typedef enum {
  PRIMITIVE,
  CONTAINER_OPEN,
  CONTAINER_CLOSE,
  TEXT,
  COMMENT
} NodeType;

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

#define PARSE_SUCCESS 1
#define PARSE_FAILURE 0

static int parseContainerClose(XMLNode *node, const char *xmlContent) {
  node->type = CONTAINER_CLOSE;
  i++;
  if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' ||
      xmlContent[i] == ':') {
    node->elementName.push_back(xmlContent[i]);
    i++;
  } else {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return PARSE_FAILURE;
  }

  while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
    if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' ||
        xmlContent[i] == ':' || xmlContent[i] == '-' || xmlContent[i] == '.') {
      node->elementName.push_back(xmlContent[i]);
    } else if (std::isspace(xmlContent[i])) {
      if (node->elementName.empty()) {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                     i);
        return PARSE_FAILURE;
      }
    } else {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return PARSE_FAILURE;
    }
    i++;
  }
  i++;
  return PARSE_SUCCESS;
}

static int parseContainerOpen(XMLNode *node, const char *xmlContent) {
  node->type = CONTAINER_OPEN;

  if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' ||
      xmlContent[i] == ':') {
    node->elementName.push_back(xmlContent[i]);
    i++;
  } else {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return PARSE_FAILURE;
  }

  bool hasAttr = false;

  // Parse name
  while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
    if (xmlContent[i] == '/' && xmlContent[i + 1] == '>') {
      node->type = PRIMITIVE;
      i += 2;
      return PARSE_SUCCESS;
    }

    if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' ||
        xmlContent[i] == ':' || xmlContent[i] == '-' || xmlContent[i] == '.') {
      node->elementName.push_back(xmlContent[i]);
      i++;
    } else if (std::isspace(xmlContent[i])) {
      if (node->elementName.empty()) {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                     i);
        return PARSE_FAILURE;
      }
      hasAttr = true;
      break;
    } else {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return PARSE_FAILURE;
    }
  }

  // 0: space, 1: start, 2: name, 3: equals, 4: quote, 5: value
  char state = 0;

  if (hasAttr) {
    std::string key;
    std::string val;
    char quoteType = 0;

    while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
      switch (state) {
      case 0:
        if (xmlContent[i] == '/' && xmlContent[i + 1] == '>') {
          node->type = PRIMITIVE;
          i += 2;
          return PARSE_SUCCESS;
        }
        if (std::isspace(xmlContent[i])) {
          i++;
          state = 1;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                       i);
          return PARSE_FAILURE;
        }
        break;
      case 1:
        if (xmlContent[i] == '/' && xmlContent[i + 1] == '>') {
          node->type = PRIMITIVE;
          i += 2;
          return PARSE_SUCCESS;
        }
        if (std::isspace(xmlContent[i])) {
          i++;
        } else if (std::isalpha(xmlContent[i]) || xmlContent[i] == '_' ||
                   xmlContent[i] == ':') {
          state = 2;
          key.push_back(xmlContent[i]);
          i++;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                       i);
          return PARSE_FAILURE;
        }
        break;
      case 2:
        if (xmlContent[i] == '=') {
          state = 4;
        } else if (std::isalnum(xmlContent[i]) || xmlContent[i] == '_' ||
                   xmlContent[i] == ':' || xmlContent[i] == '-' ||
                   xmlContent[i] == '.') {
          key.push_back(xmlContent[i]);
        } else if (std::isspace(xmlContent[i])) {
          state = 3;
        } else {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                       i);
          return PARSE_FAILURE;
        }
        i++;
        break;
      case 3:
        if (xmlContent[i] == '=') {
          state = 4;
        } else if (!std::isspace(xmlContent[i])) {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                       i);
          return PARSE_FAILURE;
        }
        i++;
        break;
      case 4:
        if (xmlContent[i] == '\'' || xmlContent[i] == '\"') {
          state = 5;
          quoteType = xmlContent[i];
        } else if (!std::isspace(xmlContent[i])) {
          PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                       i);
          return PARSE_FAILURE;
        }
        i++;
        break;
      default:
        if (xmlContent[i] == quoteType) {
          state = 0;
          node->attr.push_back({key, val});
          key.clear();
          val.clear();
        } else {
          val.push_back(xmlContent[i]);
        }
        i++;
        break;
      }
    }
  }
  if (state > 1) {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return PARSE_FAILURE;
  }
  i++;
  return PARSE_SUCCESS;
}

static int parseComment(XMLNode *node, const char *xmlContent) {
  node->type = COMMENT;
  i++;
  if (xmlContent[i] != '-' || xmlContent[i + 1] != '-') {
    PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
    return PARSE_FAILURE;
  }
  i += 2;

  while (xmlContent[i] != '\0' || xmlContent[i + 1] != '\0') {
    if (xmlContent[i] == '-' && xmlContent[i + 1] == '-' &&
        xmlContent[i + 2] == '>') {
      // Found the end of the comment
      if (xmlContent[i - 1] == '-') {
        PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)",
                     i - 1);
        return PARSE_FAILURE;
      }
      i += 3;
      return PARSE_SUCCESS;
    }
    i++;
  }
  PyErr_SetString(PyExc_Exception, "unclosed token");
  return PARSE_FAILURE;
}

static int parseCData(XMLNode *node, const char *xmlContent) {
  node->type = TEXT;
  i+=2;
  std::string cdata = "CDATA[";
  size_t j = 0;
  while (xmlContent[i] != '\0') {
    if (j >= cdata.size()) {
      break;
    }
    if (cdata[j] != xmlContent[i]) {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return PARSE_FAILURE;
    }
    i++;
    j++;
  }
  while (xmlContent[i] != '\0' || xmlContent[i + 1] != '\0') {
    if (xmlContent[i] == ']' && xmlContent[i + 1] == ']' &&
        xmlContent[i + 2] == '>') {
      i += 3;
      return PARSE_SUCCESS;
    }
    node->elementName.push_back(xmlContent[i]);
    i++;
  }
  PyErr_SetString(PyExc_Exception, "unclosed token");
  return PARSE_FAILURE;
}

static int parseText(XMLNode *node, const char *xmlContent) {
  node->type = TEXT;
  bool isSpace = false;

  while (xmlContent[i] != '\0' && xmlContent[i] != '<') {
    if (xmlContent[i] == '&' || xmlContent[i] == '>' || xmlContent[i] == '\"' || xmlContent[i] == '\'') {
      PyErr_Format(PyExc_Exception, "not well formed (violation at pos=%d)", i);
      return PARSE_FAILURE;
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
  return PARSE_SUCCESS;
}

static int parseProlog(const char *xmlContent) {
  const char* startTag = "<?xml";
  int j = 0;
  for (j = 0; j < 5 && xmlContent[j] != '\0'; j++) {
      if (xmlContent[j] != startTag[j]) {
          return PARSE_SUCCESS;
      }
  }
  if (xmlContent[j] == '\0') {
    return PARSE_SUCCESS;
  }
  i = j;
  while (xmlContent[i] != '\0') {
    if (xmlContent[i] == '?' && xmlContent[i+1] == '>'){
      i+=2;
      return PARSE_SUCCESS;
    }
    i++;
  }
  PyErr_SetString(PyExc_Exception, "unclosed token");
  return PARSE_FAILURE;
}

static std::vector<XMLNode> splitNodes(const char *xmlContent) {
  std::vector<XMLNode> nodes;
  i = 0;
  int parsing_state = PARSE_SUCCESS;

  parsing_state = parseProlog(xmlContent);
  if (parsing_state == PARSE_FAILURE) {
    return nodes;
  }

  while (xmlContent[i] != '\0') {
    XMLNode node;
    if (xmlContent[i] == '<') {
      i++;
      if (xmlContent[i] == '/') {
        parsing_state = parseContainerClose(&node, xmlContent);
      } else if (xmlContent[i] == '!') {
        if (xmlContent[i+1] == '[') {
          parsing_state = parseCData(&node, xmlContent);
        } else {
          parsing_state = parseComment(&node, xmlContent);
        }
      } else {
        parsing_state = parseContainerOpen(&node, xmlContent);
      }
    } else {
      parsing_state = parseText(&node, xmlContent);
    }
    if (parsing_state == PARSE_FAILURE) {
      return nodes;
    }
    if (!node.elementName.empty()) {
      nodes.push_back(node);
    }
  }

  return nodes;
}

static PyObject *createDict(const std::vector<Pair> &attributes, char* attributePrefix) {
  PyObject *dict = PyDict_New();
  for (const Pair &attr : attributes) {
    const std::string &key = attributePrefix + attr.key;
    PyObject *val = PyUnicode_FromString(attr.value.c_str());
    PyDict_SetItemString(dict, key.c_str(), val);
  }

  return dict;
}

PyDoc_STRVAR(xml_parse_doc, "parse(xml_content: str, attr_prefix=\"@\") -> dict:\n"
                            "...\n\n"
                            "Parse XML content into a dictionary.\n\n"
                            "Args:\n\t"
                            "xml_content (str): xml document to be parsed.\n"
                            "Returns:\n\t"
                            "dict: Dictionary of the xml dom.\n");
static PyObject *xml_parse(PyObject *self, PyObject *args, PyObject *kwargs) {
  const char *xmlContent;
  char* attributePrefix = "@";
  char* cdata_key = "#text";

  static char *kwlist[] = {"xml_content", "attr_prefix", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|s", kwlist, &xmlContent, &attributePrefix)) {
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
      PyObject *item = PyDict_GetItemString(currDict, cdata_key);
      if (item != NULL) {
        PyDict_SetItemString(currDict, cdata_key, PyUnicode_Concat(item, childKey));
      } else {
        PyDict_SetItemString(currDict, cdata_key, childKey);
      }
    } else if (node.type == CONTAINER_OPEN || node.type == PRIMITIVE) {
      PyObject *d = createDict(node.attr, attributePrefix);

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
      if (containerStackNames.size() <= 0 || containerStackNames.back() != node.elementName) {
        PyErr_Format(PyExc_Exception,
                     "tag mismatch ('%U' does not match the last start tag)",
                     childKey);
        return NULL;
      }
      containerStackNames.pop_back();
      containerStack.pop_back();
      currDict = containerStack.back();

      // If there are no child attributes do not nest CDATA
      PyObject *d = PyDict_GetItem(currDict, childKey);
      if (PyList_Check(d)) {
        Py_ssize_t len = PyList_Size(d);
        for (Py_ssize_t i = 0; i < len; ++i) {
          PyObject* dict = PyList_GetItem(d, i);
          if (PyDict_Check(dict)) {
            if (PyDict_Size(dict) == 1) {
              PyObject *item = PyDict_GetItemString(dict, cdata_key);
              if (item != NULL) {
                PyList_SetItem(d, i, item);
              }
            }
          }
        }
      }
      else {
        if (PyDict_Size(d) == 1) {
          PyObject *item = PyDict_GetItemString(d, cdata_key);
          if (item != NULL) {
            PyDict_SetItem(currDict, childKey, item);
          }
        }
      }
    }

    Py_DECREF(childKey);
  }

  if (containerStack.size() > 1) {
    PyErr_Format(PyExc_Exception, "not well formed (%d unclosed tags)",
                 containerStack.size() - 1);
    return NULL;
  }

  PyObject *result = containerStack.front();
  Py_INCREF(result);
  return result;
}

static PyMethodDef XMLParserMethods[] = {
    {"parse", (PyCFunction)xml_parse, METH_VARARGS | METH_KEYWORDS, xml_parse_doc},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef xmlparsermodule = {PyModuleDef_HEAD_INIT, "xmlpydict",
                                             NULL, -1, XMLParserMethods};

PyMODINIT_FUNC PyInit_xmlpydict() { return PyModule_Create(&xmlparsermodule); }
