def parse(xml_content: str) -> dict:
    i = 0
    key = "@"
    val = ""
    xml_content += " "

    curr_dict = {}
    container_stack = [curr_dict]

    while i < len(xml_content):
        element_name = ""

        if xml_content[i] == "<":
            if xml_content[i + 1] == "/":
                container_stack.pop()
                curr_dict = container_stack[-1]
                i = xml_content.find(">", i + 1)
            elif xml_content[i + 1] == "!":
                i = xml_content.find(">", i + 1)
            else:
                i += 1
                has_attr = False
                in_quotes = False
                is_container = True
                d = {}
                while i < len(xml_content) and xml_content[i] != ">":
                    is_space = xml_content[i].isspace()
                    if xml_content[i] == "/" and xml_content[i + 1] == ">":
                        is_container = False
                    elif not has_attr and is_space:
                        has_attr = True
                    else:
                        if has_attr:
                            if xml_content[i] == "'" or xml_content[i] == '"':
                                in_quotes = not in_quotes
                                if not in_quotes and key != "" and val != "":
                                    d[key] = val
                                    key = "@"
                                    val = ""
                            elif in_quotes:
                                val += xml_content[i]
                            elif xml_content[i] != "=" and not is_space:
                                key += xml_content[i]
                        else:
                            element_name += xml_content[i]
                    i += 1
                item = curr_dict.get(element_name)
                if item is None:
                    curr_dict[element_name] = d
                else:
                    if isinstance(item, list):
                        item.append(d)
                    else:
                        curr_dict[element_name] = [item, d]
                if is_container:
                    curr_dict = d
                    container_stack.append(d)
            i += 1
        else:
            j = xml_content.find("<", i + 1)
            if j < 0:
                return container_stack.pop()
            element_name = xml_content[i:j].strip()
            i = j
            if len(element_name) > 0:
                curr_dict["#text"] = element_name

    return container_stack.pop()
