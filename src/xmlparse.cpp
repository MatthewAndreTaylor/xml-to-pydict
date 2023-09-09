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

std::vector<XMLNode> splitNodes(const char *xmlContent) {
  std::vector<XMLNode> nodes;
  
  size_t i = 0;
  while (xmlContent[i] != '\0') {
    XMLNode node;
    if (xmlContent[i] == '<') {
      if (xmlContent[i + 1] == '/') {
        node.type = CONTAINER_CLOSE;
        while (xmlContent[i] != '>') {
          i++;
        }
      } else if (xmlContent[i + 1] == '!') {
        node.type = COMMENT;
        while (xmlContent[i] != '>') {
          i++;
        }
      } else {
        node.type = CONTAINER_OPEN;
        i++;
      }

      bool hasAttr = false;

      std::string key = "";
      std::string val = "";
      bool inquotes = false;

      // Extract element
      node.elementName = "";
      while (xmlContent[i] != '\0' && xmlContent[i] != '>') {
        bool isSpace = std::isspace(xmlContent[i]);
        if (xmlContent[i] == '/' && xmlContent[i+1] == '>') {
          node.type = PRIMITIVE;
        } else if (!hasAttr && isSpace) {
          hasAttr = true;
        } else{
          if (hasAttr) {
            if (xmlContent[i] == '\'' || xmlContent[i] == '\"') {
              inquotes = !inquotes;
              if(!key.empty() && !val.empty() && !inquotes) {
                node.attr.push_back({key, val});
                key.clear();
                val.clear();
              }
            } else if (inquotes) {
              val.push_back(xmlContent[i]);
            } else if (xmlContent[i] != '=' && !isSpace){
              key.push_back(xmlContent[i]);
            }
          } else {
            node.elementName.push_back(xmlContent[i]);
          }
        }
        i++;
      }
      i++;

      nodes.push_back(node);
    } else {
      // Text node
      node.type = TEXT;
      bool isSpace = false;

      while (xmlContent[i] != '\0' && xmlContent[i] != '<') {
        if (isSpace || !std::isspace(xmlContent[i])) {
          node.elementName.push_back(xmlContent[i]);
          isSpace = true;
        }
        i++;
      }
      while (!node.elementName.empty() && std::isspace(node.elementName.back())) {
          node.elementName.pop_back();
      }
      if (!node.elementName.empty()) {
        nodes.push_back(node);
      }
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
  PyObject *dict = PyDict_New();

  PyObject *currDict = dict;
  std::vector<PyObject *> containerStack;
  containerStack.push_back(dict);

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
          PyObject *children = PyList_New(0);
          PyList_Append(children, item);
          PyList_Append(children, d);
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
      }
    } else if (node.type == CONTAINER_CLOSE) {
      containerStack.pop_back();
      currDict = containerStack.back();
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
    PyModuleDef_HEAD_INIT, "xmlpydict", NULL, -1, XMLParserMethods};

PyMODINIT_FUNC PyInit_xmlpydict() {
  return PyModule_Create(&xmlparsermodule);
}