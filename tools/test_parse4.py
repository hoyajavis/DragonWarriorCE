import sys
import os
import struct
import xml.etree.ElementTree as ET

sys.path.insert(0, os.path.dirname(__file__))
import pack_map

def run_test():
    internal_name = 'tantegel_lvl0'
    appvar_name, dat_file, xml_file = pack_map.MAP_REGISTRY[internal_name]
    xml_path = f'python-reference/data/maps/{xml_file}'
    
    maps = []
    if os.path.exists(xml_path):
        tree = ET.parse(xml_path)
        root = tree.getroot()
        pack_map.resolve_includes(root, os.path.dirname(xml_path))
        maps = root.findall('.//Map')
        if not maps and root.tag == 'Map':
            maps = [root]

    interactables = []
    for m in maps:
        if m.get('name') == internal_name:
            locations = {}
            for loc in m.findall('.//MapLocation'):
                name = loc.get('name')
                if name:
                    locations[name] = (int(loc.get('x', '0')), int(loc.get('y', '0')))
                    
            for npc in m.findall('.//NonPlayerCharacter'):
                obj_type = npc.get('type', '')
                if 'x' in npc.attrib and 'y' in npc.attrib:
                    x = int(npc.get('x'))
                    y = int(npc.get('y'))
                else:
                    loc_name = npc.get('location')
                    x, y = locations.get(loc_name, (0, 0))
                    
                action_id = pack_map.get_action_id(npc, internal_name)
                print(f"NPC: type={obj_type}, x={x}, y={y}, loc={npc.get('location')}, action_id={action_id}")
                interactables.append(1)

    print(f"Total: {len(interactables)}")

run_test()
