def parse(xmlContent: str) -> dict:
    i = 0
    key = "@"
    val = ""
    xmlContent += " "
    
    curr_dict = {}
    containerStack = [curr_dict]

    while i < len(xmlContent):
        element_name = ""

        if xmlContent[i] == '<':
            if xmlContent[i + 1] == '/':
                containerStack.pop()
                curr_dict = containerStack[-1]
                i = xmlContent.find('>', i + 1)
            elif xmlContent[i + 1] == '!':
                i = xmlContent.find('>', i + 1)
            else:
                i += 1
                hasAttr = False
                inquotes = False
                isContainer = True
                d = {}
                while i < len(xmlContent) and xmlContent[i] != '>':
                    isSpace = xmlContent[i].isspace()
                    if xmlContent[i] == '/' and xmlContent[i + 1] == '>':
                        isContainer = False
                    elif not hasAttr and isSpace:
                        hasAttr = True
                    else:
                        if hasAttr:
                            if xmlContent[i] == '\'' or xmlContent[i] == '\"':
                                inquotes = not inquotes
                                if not inquotes and key != "" and val != "":
                                    d[key] = val
                                    key = "@"
                                    val = ""
                            elif inquotes:
                                val += xmlContent[i]
                            elif xmlContent[i] != '=' and not isSpace:
                                key += xmlContent[i]
                        else:
                            element_name += xmlContent[i]
                    i += 1
                item = curr_dict.get(element_name)
                if item is None:
                    curr_dict[element_name] = d
                else:
                    if isinstance(item, list):
                        item.append(d)
                    else:
                        curr_dict[element_name] = [item, d]
                if isContainer:
                    curr_dict = d
                    containerStack.append(d)
            i += 1
        else:
            j = xmlContent.find('<', i + 1)
            if j < 0:
                return containerStack.pop()
            element_name = xmlContent[i:j].strip()
            i = j
            if len(element_name) > 0:
                curr_dict["#text"] = element_name

    return containerStack.pop()
