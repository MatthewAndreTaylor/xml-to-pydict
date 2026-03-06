cdef object _MISSING = object()

cdef dict _update_children(dict target, str key, object value):
    cdef object existing
    if target is None:
        target = {}

    existing = target.get(key, _MISSING)

    if existing is _MISSING:
        target[key] = value
    elif type(existing) is list:
        existing.append(value)
    else:
        target[key] = [existing, value]

    return target


cdef class _PyDictHandler:
    """
    Handler class for parsing XML content into a Python dictionary using the expat parser.
    """
    
    cdef public object item
    cdef list _data
    cdef list item_stack
    cdef list data_stack
    cdef str attr_prefix
    cdef str cdata_key

    def __cinit__(
        self,
        attr_prefix: str = "@",
        cdata_key: str = "#text",
    ):
        self.attr_prefix = attr_prefix
        self.cdata_key = cdata_key
        self.item = None
        self._data = []
        self.item_stack = []
        self.data_stack = []

    cpdef void characters(self, str data):
        self._data.append(data)

    cpdef void startElement(self, str name, dict attrs):
        self.item_stack.append(self.item)
        self.data_stack.append(self._data)
        self._data = []
        self.item = (
            None if not attrs else {self.attr_prefix + k: v for k, v in attrs.items()}
        )
                
    cpdef void endElement(self, str name):
        if self.data_stack:
            py_data = "".join(self._data).strip() or None
            temp_item = self.item

            self.item = self.item_stack.pop()
            self._data = self.data_stack.pop()

            if temp_item is not None:
                if py_data:
                    temp_item[self.cdata_key] = py_data
                self.item = _update_children(self.item, name, temp_item)
            else:
                self.item = _update_children(self.item, name, py_data)