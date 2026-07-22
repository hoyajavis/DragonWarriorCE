import os
import sys
import xml.etree.ElementTree as ET
import subprocess
import struct
import copy
import pack_text
import pack_items

def resolve_includes(elem, base_dir):
    # ElementTree doesn't easily let you find parents, so we iterate and check children
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
import pack_text

# Pre-parse game.xml to find ALL inline Map definitions.
# Many maps (charlock, garin's grave, caves, shrines) are defined inline
# in game.xml rather than in standalone XML files.
GAME_XML_MAPS = {}  # Populated by _load_game_xml_maps()

def _load_game_xml_maps():
    """Parse game.xml and cache all <Map> elements by name."""
    game_xml_path = 'python-reference/data/game.xml'
    if not os.path.exists(game_xml_path):
        return
    try:
        tree = ET.parse(game_xml_path)
        root = tree.getroot()
        # Resolve xi:includes so we get the standalone map files too
        resolve_includes(root, os.path.dirname(game_xml_path))
        for m in root.findall('.//Map'):
            name = m.get('name')
            if name:
                GAME_XML_MAPS[name] = m
    except Exception as e:
        print(f"Warning: Failed to pre-parse game.xml: {e}")

_load_game_xml_maps()

# Registry linking internal map names to their TI-84 AppVar names and source files
MAP_REGISTRY = {
    'overworld': ('PYDW001', 'legacy/alefgard.dat', 'map_alefgard_legacy.xml'),
    'brecconary': ('PYDW002', 'legacy/brecconary.dat', 'map_brecconary_legacy.xml'),
    'brecconary_overlay': ('PYDW003', 'legacy/brecconary_overlay.dat', 'map_brecconary_overlay_legacy.xml'),
    'cantlin': ('PYDW005', 'legacy/cantlin.dat', 'map_cantlin_legacy.xml'),
    'cantlin_overlay': ('PYDW006', 'legacy/cantlin_overlay.dat', 'map_cantlin_overlay_legacy.xml'),
    'charlock_lvl1': ('PYDW007', 'legacy/charlock_lvl1.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl2': ('PYDW008', 'legacy/charlock_lvl2.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl3': ('PYDW009', 'legacy/charlock_lvl3.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl4': ('PYDW010', 'legacy/charlock_lvl4.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl5': ('PYDW011', 'legacy/charlock_lvl5.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl6': ('PYDW012', 'legacy/charlock_lvl6.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl7a': ('PYDW013', 'legacy/charlock_lvl7a.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl7b': ('PYDW014', 'legacy/charlock_lvl7b.dat', 'map_charlock_legacy.xml'),
    'charlock_lvl8': ('PYDW015', 'legacy/charlock_lvl8.dat', 'map_charlock_legacy.xml'),
    'city': ('PYDW016', 'legacy/city.dat', 'map_city_legacy.xml'),
    'erdricks_tomb_lvl1': ('PYDW017', 'legacy/erdricks_tomb_lvl1.dat', 'map_erdricks_tomb_legacy.xml'),
    'erdricks_tomb_lvl2': ('PYDW018', 'legacy/erdricks_tomb_lvl2.dat', 'map_erdricks_tomb_legacy.xml'),
    'garinham': ('PYDW019', 'legacy/garinham.dat', 'map_garinham_legacy.xml'),
    'garinham_overlay': ('PYDW020', 'legacy/garinham_overlay.dat', 'map_garinham_overlay_legacy.xml'),
    'garins_grave_lvl1': ('PYDW021', 'legacy/garins_grave_lvl1.dat', 'map_garins_grave_legacy.xml'),
    'garins_grave_lvl2': ('PYDW022', 'legacy/garins_grave_lvl2.dat', 'map_garins_grave_legacy.xml'),
    'garins_grave_lvl3': ('PYDW023', 'legacy/garins_grave_lvl3.dat', 'map_garins_grave_legacy.xml'),
    'garins_grave_lvl4': ('PYDW024', 'legacy/garins_grave_lvl4.dat', 'map_garins_grave_legacy.xml'),
    'hauksness': ('PYDW025', 'legacy/hauksness.dat', 'map_hauksness_legacy.xml'),
    'kol': ('PYDW026', 'legacy/kol.dat', 'map_kol_legacy.xml'),
    'mountain_cave_lvl1': ('PYDW027', 'legacy/mountain_cave_lvl1.dat', 'map_mountain_cave_legacy.xml'),
    'mountain_cave_lvl2': ('PYDW028', 'legacy/mountain_cave_lvl2.dat', 'map_mountain_cave_legacy.xml'),
    'northern_shrine': ('PYDW029', 'legacy/northern_shrine.dat', 'map_northern_shrine_legacy.xml'),
    'rimuldar': ('PYDW031', 'legacy/rimuldar.dat', 'map_rimuldar_legacy.xml'),
    'rimuldar_overlay': ('PYDW032', 'legacy/rimuldar_overlay.dat', 'map_rimuldar_overlay_legacy.xml'),
    'southern_shrine': ('PYDW033', 'legacy/southern_shrine.dat', 'map_southern_shrine_legacy.xml'),
    'swamp_cave': ('PYDW034', 'legacy/swamp_cave.dat', 'map_swamp_cave_legacy.xml'),
    'tantegel_lvl0': ('PYDW035', 'legacy/tantegel_lvl0.dat', 'map_tantegel_lvl0_legacy.xml'),
    'tantegel_lvl1': ('PYDW036', 'legacy/tantegel_lvl1.dat', 'map_tantegel_lvl1_legacy.xml'),
    'tantegel_lvl2': ('PYDW037', 'legacy/tantegel_lvl2.dat', 'map_tantegel_lvl2_legacy.xml'),
    'town': ('PYDW038', 'legacy/town.dat', 'map_town_legacy.xml'),
}

# TILE_ values must match src/map.h
TILE_MAPPING = {
    '_': 0, # TILE_PLAIN
    'w': 1, # TILE_WATER
    'f': 2, # TILE_FOREST
    'M': 3, # TILE_MOUNTAIN
    'm': 4, # TILE_HILL
    '^': 4, 
    '*': 5, # TILE_SWAMP
    's': 5,
    '-': 6, # TILE_DESERT
    'S': 7, # TILE_STONE
    'b': 8, # TILE_BRIDGE
    ' ': 9, # TILE_PATH
    'n': 10, # TILE_COUNTER
    'i': 11, # TILE_INN
    'd': 12, # TILE_DARKNESS
    'c': 0,  # Chests are entities, floor is plain
    'R': 14, # TILE_ROOF
    'a': 15, # TILE_ARMOR
    'B': 16, # TILE_BARRIER
    '.': 0,  # TILE_PLAIN for transparent overlays
    'o': 0,  # Doors are entities or ignored for now
    't': 0,  # Stairs are entities or ignored for now
}

ACTION_REGISTRY = {'none': 0}
ACTION_DATA = {} # Maps action_id -> xml element
NEXT_ACTION_ID = 1

def get_action_id(element, map_name):
    global NEXT_ACTION_ID
    
    label = None
    if 'progressMarker' in element.attrib:
        label = element.attrib['progressMarker']
    elif element.tag == 'MapDecoration' and element.get('type', '').lower() in ['chest', 'door']:
        loc_str = element.get('location', f"{element.get('x', '0')}_{element.get('y', '0')}")
        label = f"DECO_{map_name}_{loc_str.replace(' ', '_').upper()}"
    else:
        dialog_goto = element.find('.//DialogGoTo')
        if dialog_goto is not None and 'label' in dialog_goto.attrib:
            label = dialog_goto.attrib['label']
            
    if label is None:
        if element.find('.//Dialog') is not None or element.find('.//DialogOptions') is not None or element.find('.//DialogVendorBuyOptions') is not None:
            if 'location' in element.attrib:
                label = f"DIALOG_{map_name}_{element.attrib['location'].replace(' ', '_').upper()}"
            elif 'name' in element.attrib:
                label = f"DIALOG_{map_name}_{element.attrib['name'].replace(' ', '_').upper()}"
            else:
                label = f"DIALOG_{map_name}_NPC_{NEXT_ACTION_ID}"
                
    if label is None:
        return 0
        
    if label not in ACTION_REGISTRY:
        ACTION_REGISTRY[label] = NEXT_ACTION_ID
        ACTION_DATA[label] = element
        NEXT_ACTION_ID += 1
        
    return ACTION_REGISTRY[label]

def get_spawn_coords(target_internal_name, source_internal_name):
    if target_internal_name not in MAP_REGISTRY:
        return 0, 0
    
    _, _, target_xml = MAP_REGISTRY[target_internal_name]
    target_xml_path = f'python-reference/data/maps/{target_xml}'
    
    # Build list of candidate Map elements to search
    maps = []
    if os.path.exists(target_xml_path):
        try:
            tree = ET.parse(target_xml_path)
            root = tree.getroot()
            maps = root.findall('.//Map')
            if not maps and root.tag == 'Map':
                maps = [root]
        except Exception as e:
            print(f"Failed to parse {target_xml_path} for spawn coords: {e}")
    
    # Fallback: check pre-parsed game.xml inline maps
    if not maps and target_internal_name in GAME_XML_MAPS:
        maps = [GAME_XML_MAPS[target_internal_name]]
    
    for m in maps:
        if m.get('name') == target_internal_name:
            for trans in m.findall('.//PointTransition'):
                if trans.get('toMap') == source_internal_name:
                    return int(trans.get('x', '0')), int(trans.get('y', '0'))
            for trans in m.findall('.//LeavingTransition'):
                if trans.get('toMap') == source_internal_name:
                    return int(trans.get('x', '0')), int(trans.get('y', '0'))
        
    return 0, 0

def pack_map(internal_name):
    if internal_name not in MAP_REGISTRY:
        print(f"Warning: {internal_name} not in registry, skipping.")
        return
        
    appvar_name, dat_file, xml_file = MAP_REGISTRY[internal_name]
    dat_path = f'python-reference/data/maps/{dat_file}'
    xml_path = f'python-reference/data/maps/{xml_file}'
    
    # 1. Parse the visual matrix
    try:
        with open(dat_path, 'r') as f:
            lines = [line.strip('\n') for line in f.readlines()]
    except Exception as e:
        print(f"Failed to read {dat_path}: {e}")
        return
        
    height = len(lines)
    width = len(lines[0]) if height > 0 else 0
    
    is_outside = True
    mapping = TILE_MAPPING
    
    tile_data = bytearray()
    for row in lines:
        for char in row:
            tile_byte = mapping.get(char, 0)
            tile_data.append(tile_byte)
            
    # 2. Parse the metadata (Transitions)
    transitions = []
    interactables = []
    
    # Try standalone XML first, then fall back to game.xml inline maps
    maps = []
    if os.path.exists(xml_path):
        try:
            tree = ET.parse(xml_path)
            root = tree.getroot()
            resolve_includes(root, os.path.dirname(xml_path))
        except Exception as e:
            print(f"Warning: Failed to parse {xml_file}: {e}")
            return
            
        maps = root.findall('.//Map')
        if not maps and root.tag == 'Map':
            maps = [root]
    
    # Fallback: use pre-parsed game.xml inline map definition
    if not maps and internal_name in GAME_XML_MAPS:
        maps = [GAME_XML_MAPS[internal_name]]
        
    global_monster_set = 255
    monster_zones = []
            
    if maps:
        for m in maps:
            if m.get('name') == internal_name:
                is_outside = m.get('isOutside', 'yes') == 'yes'
                light_diameter_str = m.get('lightDiameter', 'unlimited')
                light_diameter = 255 if light_diameter_str == 'unlimited' else int(light_diameter_str)
                for trans in m.findall('.//PointTransition'):
                    tx = int(trans.get('x', '0'))
                    ty = int(trans.get('y', '0'))
                    to_map = trans.get('toMap', '')
                    
                    target_appvar = MAP_REGISTRY.get(to_map, ("UNKNOWN",))[0]
                    target_bytes = target_appvar.encode('ascii')[:7].ljust(8, b'\0')
                    
                    sx, sy = get_spawn_coords(to_map, internal_name)
                    
                    transitions.append(struct.pack('<BB8sBB', tx, ty, target_bytes, sx, sy))
                        
                for trans in m.findall('.//LeavingTransition'):
                    tx = 255
                    ty = 255
                    to_map = trans.get('toMap', '')
                    
                    target_appvar = MAP_REGISTRY.get(to_map, ("UNKNOWN",))[0]
                    target_bytes = target_appvar.encode('ascii')[:7].ljust(8, b'\0')
                    
                    sx, sy = get_spawn_coords(to_map, internal_name)
                    
                    transitions.append(struct.pack('<BB8sBB', tx, ty, target_bytes, sx, sy))
                        
                        
            # 2.4 Parse Encounter Data
            for m in maps:
                if m.get('name') == internal_name:
                    if 'monsterSet' in m.attrib:
                        global_monster_set = int(m.attrib['monsterSet'])
                    break
                    
            for m in maps:
                if m.get('name') == internal_name:
                    for mz in m.findall('.//MonsterZones/MonsterZone'):
                        zx = int(mz.get('x', '0'))
                        zy = int(mz.get('y', '0'))
                        zw = int(mz.get('w', '0'))
                        zh = int(mz.get('h', '0'))
                        zset = int(mz.get('set', '255'))
                        monster_zones.append(struct.pack('<BBBBB', zx, zy, zw, zh, zset))
                    break
            # 2.5 Parse MapInteractables (MapDecorations and NPCs)
            interactables = []
            OBJ_TYPES = {
                'none': 0, 'king': 1, 'princess': 2, 'guard': 3, 'sage': 4,
                'merchant': 5, 'warrior': 6, 'boy': 7, 'girl': 8, 'trumpeter': 9,
                'chest': 10, 'door': 11, 'stairs_up': 12, 'stairs_down': 13,
                'upstairs': 12, 'downstairs': 13,
                # Overworld decorations → matches map_h ObjectType enum values 14–22
                'castle_stone_a': 14, 'castle_stone_a.png': 14,
                'castle_stone_tall_a': 15,
                'town_stone_med_a': 16,
                'town_stone_med_b': 17,
                'town_wood_med_a': 18,
                'town_wood_med_b': 19,
                'cave': 20,
                'shrine_stone_a': 21,
                'shrine_stone_b': 22,
            }
            DIR_MAPPING = {'NORTH': 0, 'SOUTH': 1, 'EAST': 2, 'WEST': 3}
            
            for m in maps:
                if m.get('name') == internal_name:
                    # Gather MapLocations first
                    locations = {}
                    for loc in m.findall('.//MapLocation'):
                        name = loc.get('name')
                        if name:
                            locations[name] = (int(loc.get('x', '0')), int(loc.get('y', '0')))
                            
                    # Parse NPCs
                    for npc in m.findall('.//NonPlayerCharacter'):
                        obj_type = OBJ_TYPES.get(npc.get('type', '').lower(), 0)
                        dir_val = DIR_MAPPING.get(npc.get('dir', 'SOUTH').upper(), 1)
                        
                        if 'x' in npc.attrib and 'y' in npc.attrib:
                            x = int(npc.get('x'))
                            y = int(npc.get('y'))
                        else:
                            loc_name = npc.get('location')
                            x, y = locations.get(loc_name, (0, 0))
                            
                        action_id = get_action_id(npc, internal_name)
                            
                        interactables.append(struct.pack('<BBBBH', x, y, obj_type, dir_val, action_id))
                        
                    # Parse Decorations
                    for dec in m.findall('.//MapDecoration'):
                        obj_type = OBJ_TYPES.get(dec.get('type', '').lower(), 0)
                        dir_val = 1 # Default South
                        
                        if 'x' in dec.attrib and 'y' in dec.attrib:
                            x = int(dec.get('x'))
                            y = int(dec.get('y'))
                        else:
                            loc_name = dec.get('location')
                            x, y = locations.get(loc_name, (0, 0))
                            
                        action_id = get_action_id(dec, internal_name)
                            
                        interactables.append(struct.pack('<BBBBH', x, y, obj_type, dir_val, action_id))
                        
                    # Parse stairs/overworld decorations from PointTransitions
                    for trans in m.findall('.//PointTransition'):
                        dec_type = trans.get('decoration')
                        if dec_type:
                            obj_type = OBJ_TYPES.get(dec_type.lower(), 0)
                            if obj_type != 0:
                                # Resolve x/y — some use location= name
                                if 'x' in trans.attrib and 'y' in trans.attrib:
                                    x = int(trans.get('x'))
                                    y = int(trans.get('y'))
                                else:
                                    loc_name = trans.get('location', '')
                                    x, y = locations.get(loc_name, (0, 0))
                                if x != 0 or y != 0:
                                    interactables.append(struct.pack('<BBBBH', x, y, obj_type, 1, 0))

                    for trans in m.findall('.//LeavingTransition'):
                        dec_type = trans.get('decoration')
                        if dec_type:
                            obj_type = OBJ_TYPES.get(dec_type.lower(), 0)
                            if obj_type != 0:
                                x = int(trans.get('x', '0'))
                                y = int(trans.get('y', '0'))
                                interactables.append(struct.pack('<BBBBH', x, y, obj_type, 1, 0))
                            
                    # Enforce the hard cap of 64 interactables per map
                    if len(interactables) > 64:
                        print(f"Warning: {internal_name} has {len(interactables)} interactables, truncating to 64.")
                        interactables = interactables[:64]
                        

            
    # 3. Construct binary payload
    payload = bytearray()
    payload.append(width)
    payload.append(height)
    payload.append(1 if is_outside else 0)
    
    # We need to compute light_diameter if it hasn't been defined, which happens if `maps` was empty
    if 'light_diameter' not in locals():
        light_diameter = 255
    payload.append(light_diameter)
    
    payload.extend(tile_data)
    payload.append(len(transitions))
    for t in transitions:
        payload.extend(t)
        
    payload.append(len(interactables))
    for i in interactables:
        payload.extend(i)
        
    # Append encounter data
    payload.extend(struct.pack('<B', global_monster_set))
    payload.extend(struct.pack('<B', len(monster_zones)))
    for mz_bytes in monster_zones:
        payload.extend(mz_bytes)
        
    # Write raw binary
    os.makedirs('obj', exist_ok=True)
    os.makedirs('bin', exist_ok=True)
    raw_bin_path = f'obj/{appvar_name}.bin'
    with open(raw_bin_path, 'wb') as f:
        f.write(payload)
        
    # 4. Use convbin to convert to .8xv
    out_8xv_path = f'bin/{appvar_name}.8xv'
    print(f"Packing {internal_name} -> {out_8xv_path} ({width}x{height}, {len(transitions)} transitions)")
    
    cedev_path = os.environ.get('CEDEV', 'CEdev')
    if not os.path.exists(cedev_path) and os.path.exists('C:/CEdev'):
        cedev_path = 'C:/CEdev'
        
    convbin_exe = os.path.abspath(os.path.join(cedev_path, 'bin', 'convbin.exe'))
    
    cmd = [convbin_exe, '-j', 'bin', '-k', '8xv', '-r', '-i', raw_bin_path, '-o', out_8xv_path, '-n', appvar_name]
    try:
        subprocess.run(cmd, check=True, capture_output=True)
    except subprocess.CalledProcessError as e:
        print(f"convbin failed: {e.stderr.decode()}")
        print(f"Ensure the CE C Toolchain is installed and 'convbin' is in your PATH.")

if __name__ == '__main__':
    # Add convbin to path if CEDEV is defined
    if 'CEDEV' in os.environ:
        os.environ['PATH'] = os.environ['CEDEV'] + '/bin;' + os.environ.get('PATH', '')
        
    print("Packing maps...")
    for map_name in MAP_REGISTRY.keys():
        pack_map(map_name)
        
    print("Parsing game.xml for global DialogScripts...")
    game_tree = ET.parse('python-reference/data/game.xml')
    for script in game_tree.findall('.//DialogScript'):
        label = script.attrib.get('label')
        if label:
            if label not in ACTION_REGISTRY:
                ACTION_REGISTRY[label] = NEXT_ACTION_ID
                NEXT_ACTION_ID += 1
            ACTION_DATA[label] = script
            
    # Pre-pass for START_ENCOUNTER victory dialogs
    for action in game_tree.findall('.//DialogAction[@type="START_ENCOUNTER"]'):
        victory = action.find('VictoryDialog')
        if victory is not None:
            label = f"VICTORY_ENCOUNTER_{NEXT_ACTION_ID}"
            victory.attrib['__internal_label'] = label
            ACTION_REGISTRY[label] = NEXT_ACTION_ID
            ACTION_DATA[label] = victory
            NEXT_ACTION_ID += 1
            
    # Build MONSTER_MAP
    MONSTER_MAP = {}
    monster_id = 0
    # The first monster is SLIME in monster_data.c, wait, we must match the order in pack_monsters.py!
    # pack_monsters.py finds all <Monster> with "strength" attribute in game.xml and assigns IDs sequentially starting at 0.
    for monster_node in game_tree.findall(".//Monster"):
        if "strength" not in monster_node.attrib:
            continue
        m_name = monster_node.attrib.get("name")
        MONSTER_MAP[m_name] = monster_id
        monster_id += 1
            
    # Parse Items and their use-actions
    NEXT_ACTION_ID, ITEM_MAP = pack_items.pack_items(game_tree, ACTION_REGISTRY, ACTION_DATA, NEXT_ACTION_ID)
        
    pack_text.pack_text(ACTION_DATA, ACTION_REGISTRY, ITEM_MAP, MONSTER_MAP)
    print("Generating src/action_ids.h...")
    with open('src/action_ids.h', 'w') as f:
        f.write("#ifndef ACTION_IDS_H\n")
        f.write("#define ACTION_IDS_H\n\n")
        f.write("// Auto-generated by tools/pack_map.py\n\n")
        for label, action_id in ACTION_REGISTRY.items():
            if label == 'none': continue
            safe_label = label.upper().replace(' ', '_').replace('-', '_')
            f.write(f"#define ACTION_{safe_label} {action_id}\n")
        f.write("\n#endif // ACTION_IDS_H\n")
    print("Done!\n")


