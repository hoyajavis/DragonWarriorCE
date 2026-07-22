import xml.etree.ElementTree as ET
import os

def resolve_includes(elem, base_dir):
    for parent in elem.iter():
        for child in list(parent):
            if child.tag == '{http://www.w3.org/2001/XInclude}include':
                href = child.get('href')
                if href:
                    inc_path = os.path.join(base_dir, href)
                    if os.path.exists(inc_path):
                        inc_tree = ET.parse(inc_path)
                        inc_root = inc_tree.getroot()
                        resolve_includes(inc_root, os.path.dirname(inc_path))
                        
                        idx = list(parent).index(child)
                        parent.remove(child)
                        for i, new_child in enumerate(inc_root):
                            parent.insert(idx + i, new_child)

xml_path = 'python-reference/data/maps/map_tantegel_lvl1_legacy.xml'
tree = ET.parse(xml_path)
root = tree.getroot()
resolve_includes(root, os.path.dirname(xml_path))

maps = root.findall('.//Map')
if not maps and root.tag == 'Map':
    maps = [root]

for m in maps:
    if m.get('name') == 'tantegel_lvl1':
        npcs = m.findall('.//NonPlayerCharacter')
        print(f"Found {len(npcs)} NPCs!")
        decs = m.findall('.//MapDecoration')
        print(f"Found {len(decs)} Decorations!")
