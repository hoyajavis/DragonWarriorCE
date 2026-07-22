import xml.etree.ElementTree as ET
import os

MAP_REGISTRY = {
    'tantegel_lvl1': ('PYDW036', 'legacy/tantegel_lvl1.dat', 'map_tantegel_lvl1_legacy.xml')
}

GAME_XML_MAPS = {}

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

def pack_map(internal_name):
    appvar_name, dat_file, xml_file = MAP_REGISTRY[internal_name]
    xml_path = f'python-reference/data/maps/{xml_file}'
    
    interactables = []
    
    maps = []
    if os.path.exists(xml_path):
        tree = ET.parse(xml_path)
        root = tree.getroot()
        resolve_includes(root, os.path.dirname(xml_path))
        maps = root.findall('.//Map')
        if not maps and root.tag == 'Map':
            maps = [root]
            
    OBJ_TYPES = {
        'none': 0, 'king': 1, 'princess': 2, 'guard': 3, 'sage': 4,
        'merchant': 5, 'warrior': 6, 'boy': 7, 'girl': 8, 'trumpeter': 9,
        'chest': 10, 'door': 11, 'stairs_up': 12, 'stairs_down': 13,
    }
    DIR_MAPPING = {'NORTH': 0, 'SOUTH': 1, 'EAST': 2, 'WEST': 3}
    
    for m in maps:
        if m.get('name') == internal_name:
            locations = {}
            for loc in m.findall('.//MapLocation'):
                name = loc.get('name')
                if name:
                    locations[name] = (int(loc.get('x', '0')), int(loc.get('y', '0')))
                    
            for npc in m.findall('.//NonPlayerCharacter'):
                obj_type = OBJ_TYPES.get(npc.get('type', '').lower(), 0)
                dir_val = DIR_MAPPING.get(npc.get('dir', 'SOUTH').upper(), 1)
                if 'x' in npc.attrib and 'y' in npc.attrib:
                    x = int(npc.get('x'))
                    y = int(npc.get('y'))
                else:
                    loc_name = npc.get('location')
                    x, y = locations.get(loc_name, (0, 0))
                interactables.append((x, y, obj_type))
                
            for dec in m.findall('.//MapDecoration'):
                obj_type = OBJ_TYPES.get(dec.get('type', '').lower(), 0)
                if 'x' in dec.attrib and 'y' in dec.attrib:
                    x = int(dec.get('x'))
                    y = int(dec.get('y'))
                else:
                    loc_name = dec.get('location')
                    x, y = locations.get(loc_name, (0, 0))
                interactables.append((x, y, obj_type))

    print(f"Packed {len(interactables)} interactables")

pack_map('tantegel_lvl1')
