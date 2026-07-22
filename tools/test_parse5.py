import sys
import os

sys.path.insert(0, os.path.dirname(__file__))
import pack_map

for map_name in pack_map.MAP_REGISTRY.keys():
    pack_map.pack_map(map_name)

import xml.etree.ElementTree as ET
game_tree = ET.parse('python-reference/data/game.xml')
for script in game_tree.findall('.//DialogScript'):
    label = script.attrib.get('label')
    if label:
        if label not in pack_map.ACTION_REGISTRY:
            pack_map.ACTION_REGISTRY[label] = pack_map.NEXT_ACTION_ID
            pack_map.NEXT_ACTION_ID += 1
        pack_map.ACTION_DATA[label] = script

import pack_items
pack_map.NEXT_ACTION_ID, ITEM_MAP = pack_items.pack_items(game_tree, pack_map.ACTION_REGISTRY, pack_map.ACTION_DATA, pack_map.NEXT_ACTION_ID)

# Reverse lookup
id_to_label = {v: k for k, v in pack_map.ACTION_REGISTRY.items()}
for action_id in [134, 138, 135, 136, 137, 139, 140]:
    if action_id in id_to_label:
        print(f"Action {action_id}: {id_to_label[action_id]}")
