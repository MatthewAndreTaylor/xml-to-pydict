from enum import Enum


class NodeType(Enum):
    PRIMITIVE = 0
    CONTAINER_OPEN = 1
    CONTAINER_CLOSE = 2
    TEXT = 3
    COMMENT = 4


def splitNodes(xmlContent: str) -> list:
    nodes: list = []
    i = 0
    key = "@"
    val = ""
    xmlContent += " "

    while i < len(xmlContent):
        node = [None, "", {}]

        if xmlContent[i] == '<':
            if xmlContent[i + 1] == '/':
                node[0] = NodeType.CONTAINER_CLOSE
                i = xmlContent.find('>', i + 1)
            elif xmlContent[i + 1] == '!':
                node[0] = NodeType.COMMENT
                i = xmlContent.find('>', i + 1)
            else:
                node[0] = NodeType.CONTAINER_OPEN
                i += 1

            hasAttr = False
            inquotes = False
            while i < len(xmlContent) and xmlContent[i] != '>':
                isSpace = xmlContent[i].isspace()
                if xmlContent[i] == '/' and xmlContent[i + 1] == '>':
                    node[0] = NodeType.PRIMITIVE
                elif not hasAttr and isSpace:
                    hasAttr = True
                else:
                    if hasAttr:
                        if xmlContent[i] == '\'' or xmlContent[i] == '\"':
                            inquotes = not inquotes
                            if not inquotes and key != "" and val != "":
                                node[2][key] = val
                                key = "@"
                                val = ""
                        elif inquotes:
                            val += xmlContent[i]
                        elif xmlContent[i] != '=' and not isSpace:
                            key += xmlContent[i]
                    else:
                        node[1] += xmlContent[i]
                i += 1
            i += 1
            nodes.append(node)
        else:
            node[0] = NodeType.TEXT
            isSpace = False
            j = xmlContent.find('<', i + 1)
            if j < 0:
                return nodes
            node[1] = xmlContent[i:j].strip()
            i = j
            if len(node[1]) > 0:
                nodes.append(node)

    return nodes


def parse(xmlContent: str):
    nodes = splitNodes(xmlContent)
    curr_dict = {}
    containerStack = [curr_dict]
    for node in nodes:
        if node[0] == NodeType.TEXT:
            curr_dict["#text"] = node[1]
        elif node[0] == NodeType.PRIMITIVE or node[
            0] == NodeType.CONTAINER_OPEN:
            d = node[2]
            item = curr_dict.get(node[1])
            if item is None:
                curr_dict[node[1]] = node[2]
            else:
                if isinstance(item, list):
                    item.append(d)
                else:
                    curr_dict[node[1]] = [item, d]

            if node[0] == NodeType.CONTAINER_OPEN:
                curr_dict = d
                containerStack.append(d)
        elif node[0] == NodeType.CONTAINER_CLOSE:
            containerStack.pop()
            curr_dict = containerStack[-1]

    return containerStack.pop()
