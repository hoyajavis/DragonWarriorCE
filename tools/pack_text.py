import struct
import os
import subprocess

OP_END = 0x00
OP_TEXT = 0x01
OP_JUMP_IF_FLAG = 0x02
OP_JUMP_IF_NOT_FLAG = 0x03
OP_SET_FLAG = 0x04
OP_CLEAR_FLAG = 0x05
OP_GIVE_ITEM = 0x06
OP_REMOVE_ITEM = 0x07
OP_CHECK_ITEM = 0x08
OP_PLAY_SOUND = 0x09
OP_GOTO_ACTION = 0x0A
OP_PROMPT_YES_NO = 0x0B
OP_GIVE_GOLD = 0x0C
OP_JUMP = 0x0D
OP_SHOW_MENU = 0x0E
OP_VENDOR_BUY = 0x0F
OP_VENDOR_SELL = 0x10
OP_TAKE_GOLD = 0x11
OP_CHECK_GOLD = 0x12
OP_HEALTH_RESTORE = 0x13
OP_ASSERT_FACING_LOCKED = 0x14
OP_OPEN_LOCKED = 0x15
OP_GOTO_COORDINATES = 0x16
OP_REPEL_MONSTERS = 0x17
OP_ASSERT_OUTSIDE = 0x18
OP_ASSERT_NOT_COMBAT = 0x19
OP_SET_LIGHT = 0x1A
OP_PLAY_MUSIC = 0x1B
OP_CAST_SPELL = 0x1C
OP_START_ENCOUNTER = 0x1D
OP_MAGIC_RESTORE = 0x1E
OP_TRIGGER_RANDOM_ENCOUNTER = 0x1F
OP_IS_AT_COORDINATES = 0x20
OP_VISUAL_EFFECT = 0x21

def stable_hash(s):
    """Stable DJB2 hash - same result on every Python run (no PYTHONHASHSEED)."""
    h = 5381
    for c in s:
        h = ((h << 5) + h) + ord(c)
        h &= 0xFFFFFFFF
    return h % 512

def extract_flag_id(flag_name):
    return stable_hash(flag_name) if flag_name else 0

def compile_dialog_node(node, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables, depth=0, visited_gotos=None):
    if variables is None: variables = {}
    if visited_gotos is None: visited_gotos = set()
    if depth > 20:
        return bytearray()  # Prevent runaway recursion
    bytecode = bytearray()
    
    if node.tag == 'DialogVariable':
        var_name = node.attrib.get('name')
        var_value = node.attrib.get('value')
        if var_name and var_value:
            if var_value == "ITEM_LIST":
                items = []
                for item_node in node.findall('Item'):
                    items.append(item_node.attrib.get('name'))
                variables[var_name] = items
            else:
                variables[var_name] = var_value
            
    elif node.tag == 'Dialog':
        text = node.text
        if text:
            # Replace variables
            for k, v in variables.items():
                text = text.replace(k, str(v))
                
            # Clean up text (newlines to spaces, multiple spaces to single)
            text = " ".join(text.split())
            text = text.replace('"', '')
            # Enforce 255 char limit
            if len(text) > 255:
                text = text[:255]
            bytecode.append(OP_TEXT)
            bytecode.extend(text.encode('ascii', errors='ignore'))
            bytecode.append(0) # Null terminator
            
    elif node.tag == 'DialogAssert':
        assert_type = node.attrib.get('type')
        child_bytecode = bytearray()
        for child in node:
            child_bytecode.extend(compile_dialog_node(child, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables.copy(), depth+1, visited_gotos))
            
        if assert_type == 'HAS_ITEM' or assert_type == 'LACKS_ITEM':
            is_lacks = assert_type == 'LACKS_ITEM'
            flag_name = node.attrib.get('name')
            if flag_name == 'gp':
                count = node.attrib.get('count', '0')
                for k, v in variables.items():
                    count = count.replace(k, str(v))
                if '-' in count: count = count.split('-')[0]
                try:
                    amount = int(count)
                except ValueError:
                    amount = 0
                bytecode.append(OP_CHECK_GOLD)
                bytecode.extend(struct.pack('<HH', amount, len(child_bytecode) + 1))
                bytecode.extend(child_bytecode)
                bytecode.append(OP_END)
            else:
                item_id = ITEM_MAP.get(flag_name, 0)
                if item_id > 0:
                    bytecode.append(OP_CHECK_ITEM)
                    bytecode.extend(struct.pack('<BBH', item_id, 0 if is_lacks else 1, len(child_bytecode) + 1))
                    bytecode.extend(child_bytecode)
                    bytecode.append(OP_END)
                else:
                    flag_id = stable_hash(flag_name) if flag_name else 0
                    bytecode.append(OP_JUMP_IF_FLAG if is_lacks else OP_JUMP_IF_NOT_FLAG)
                    bytecode.extend(struct.pack('<HH', flag_id, len(child_bytecode) + 1))
                    bytecode.extend(child_bytecode)
                    bytecode.append(OP_END)
        elif assert_type == 'IS_FACING_LOCKED_ITEM':
            bytecode.append(OP_ASSERT_FACING_LOCKED)
            bytecode.extend(struct.pack('<H', len(child_bytecode) + 1))
            bytecode.extend(child_bytecode)
            bytecode.append(OP_END)
        elif assert_type == 'IS_OUTSIDE':
            bytecode.append(OP_ASSERT_OUTSIDE)
            bytecode.extend(struct.pack('<H', len(child_bytecode) + 1))
            bytecode.extend(child_bytecode)
            bytecode.append(OP_END)
        elif assert_type == 'IS_NOT_IN_COMBAT':
            bytecode.append(OP_ASSERT_NOT_COMBAT)
            bytecode.extend(struct.pack('<H', len(child_bytecode) + 1))
            bytecode.extend(child_bytecode)
            bytecode.append(OP_END)
        elif assert_type == 'IS_DARK':
            # Not implementing darkness overhead for now, but we'll consume it
            pass
        elif assert_type == 'IS_AT_COORDINATES':
            bytecode.append(OP_IS_AT_COORDINATES)
            # Support Rainbow Bridge specifically
            x, y = 66, 50
            if node.attrib.get('location') == "Southern Shrine" or node.attrib.get('name') == "Southern Shrine":
                x, y = 14, 110 # Example override
            bytecode.extend(struct.pack('<BBH', x, y, len(child_bytecode) + 1))
            bytecode.extend(child_bytecode)
            bytecode.append(OP_END)
            
            
    elif node.tag == 'DialogAction':
        action_type = node.attrib.get('type')
        item_name = node.attrib.get('name')
        count = node.attrib.get('count')
        
        if count:
            for k, v in variables.items():
                count = count.replace(k, str(v))
        if action_type == 'GAIN_ITEM':
            if item_name == 'gp':
                bytecode.append(0x0C) # OP_GIVE_GOLD
                try:
                    gold_amount = int(count) if count else 0
                except ValueError:
                    gold_amount = 0
                bytecode.extend(struct.pack('<H', gold_amount))
                return bytecode
            
            bytecode.append(OP_GIVE_ITEM)
            
            # Map item name to enum ID (1: Herb, 2: Key, 3: Torch for now, or flags)
            item_id = ITEM_MAP.get(item_name, 0)
            if item_id == 0 and item_name not in ['gp']:
                # If it's a progress marker
                item_id = stable_hash(item_name) if item_name else 0
                bytecode[-1] = OP_SET_FLAG
                bytecode.extend(struct.pack('<H', item_id))
                return bytecode
            bytecode.append(item_id)
            
        elif action_type == 'LOSE_ITEM':
            item_id = ITEM_MAP.get(item_name, 0)
            if item_id > 0:
                bytecode.append(OP_REMOVE_ITEM)
                bytecode.append(item_id)
            elif item_name == 'gp':
                count = node.attrib.get('count', '0')
                for k, v in variables.items():
                    count = count.replace(k, str(v))
                if '-' in count: count = count.split('-')[0]
                try:
                    amount = int(count)
                except ValueError:
                    amount = 0
                bytecode.append(OP_TAKE_GOLD)
                bytecode.extend(struct.pack('<H', amount))
            else:
                bytecode.append(OP_CLEAR_FLAG)
                flag_id = stable_hash(item_name) if item_name else 0
                bytecode.extend(struct.pack('<H', flag_id))
        elif action_type == 'HEALTH_RESTORE':
            if count == 'unlimited':
                min_val, max_val = 255, 255
            else:
                try:
                    min_val, max_val = map(int, count.split('-')) if '-' in count else (int(count), int(count))
                except ValueError:
                    min_val, max_val = 0, 0
            bytecode.append(OP_HEALTH_RESTORE)
            bytecode.extend(struct.pack('<BB', min_val, max_val))
        elif action_type == 'MAGIC_RESTORE':
            bytecode.append(OP_MAGIC_RESTORE)
        elif action_type == 'OPEN_LOCKED_ITEM':
            bytecode.append(OP_OPEN_LOCKED)
        elif action_type == 'GOTO_COORDINATES':
            # E.g. map="overworld" location="Return Spell Location Point" dir="SOUTH"
            # For simplicity, we just trigger the opcode, and let C handle warp to Tantegel
            bytecode.append(OP_GOTO_COORDINATES)
        elif action_type == 'REPEL_MONSTERS':
            decay = int(node.attrib.get('decay', '128'))
            bytecode.append(OP_REPEL_MONSTERS)
            bytecode.extend(struct.pack('<B', decay))
        elif action_type == 'SET_LIGHT_DIAMETER':
            count = int(node.attrib.get('count', '3'))
            decay_str = node.attrib.get('decay', '0')
            decay = 0 if decay_str == 'unlimited' else int(decay_str)
            bytecode.append(OP_SET_LIGHT)
            bytecode.extend(struct.pack('<BB', count, decay))
        elif action_type == 'PLAY_MUSIC':
            bytecode.append(OP_PLAY_MUSIC)
        elif action_type == 'CAST_SPELL':
            bytecode.append(OP_CAST_SPELL)
        elif action_type == 'VISUAL_EFFECT':
            bytecode.append(OP_VISUAL_EFFECT)
            effect_name = node.attrib.get('name', '')
            effect_id = 0
            if effect_name == 'flickering': effect_id = 1
            elif effect_name == 'hideDialog': effect_id = 2
            bytecode.append(effect_id)
        elif action_type == 'START_ENCOUNTER':
            monster_name = node.attrib.get('name')
            if monster_name and '|' in monster_name:
                bytecode.append(OP_TRIGGER_RANDOM_ENCOUNTER)
                bytecode.append(OP_END)
            else:
                monster_id = MONSTER_MAP.get(monster_name, 0)
                
                approach = node.find('ApproachDialog')
                if approach is not None:
                    for child in approach:
                        bytecode.extend(compile_dialog_node(child, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables, depth+1, visited_gotos))
                        
                bytecode.append(OP_START_ENCOUNTER)
                bytecode.append(monster_id)
                
                victory = node.find('VictoryDialog')
                victory_id = 0
                if victory is not None:
                    label = victory.attrib.get('__internal_label')
                    if label and label in action_registry:
                        victory_id = action_registry[label]
                bytecode.extend(struct.pack('<H', victory_id))
                bytecode.append(OP_END) # Terminate VM and yield to combat
                
    elif node.tag == 'DialogGoTo':
        label = node.attrib.get('label')
        if label in action_data and label not in visited_gotos:
            # Inline it, but track visited labels to prevent cycles
            new_visited = visited_gotos | {label}
            inline_node = action_data[label]
            if inline_node is not None:
                for child in inline_node:
                    if child.tag in ['Dialog', 'DialogAssert', 'DialogAction', 'DialogOptions', 'DialogGoTo']:
                        bytecode.extend(compile_dialog_node(child, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables.copy(), depth+1, new_visited))
        elif label in action_registry:
            bytecode.append(OP_GOTO_ACTION)
            bytecode.extend(struct.pack('<H', action_registry[label]))
            
    elif node.tag == 'DialogOptions':
        options = node.findall('DialogOption')
        num_options = len(options)
        
        if num_options > 0:
            bytecode.append(OP_SHOW_MENU)
            bytecode.append(num_options)
            
            # Emit strings
            for opt in options:
                text = opt.get('name', 'Option')
                if len(text) > 31: text = text[:31]
                bytecode.extend(text.encode('ascii', errors='ignore'))
                bytecode.append(0)
                
            # Placeholders for jump offsets relative to END of this instruction block
            jump_offset_indices = []
            for _ in options:
                jump_offset_indices.append(len(bytecode))
                bytecode.extend(struct.pack('<H', 0))
                
            # Now compile each option's bytecode
            option_bytecodes = []
            for opt in options:
                opt_bc = bytearray()
                for child in opt:
                    opt_bc.extend(compile_dialog_node(child, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables.copy(), depth+1, visited_gotos))
                option_bytecodes.append(opt_bc)
                
            # Stitch them together with OP_JUMP
            base_idx = len(bytecode)
            current_offset = 0
            
            jump_indices = []
            
            for i in range(num_options):
                struct.pack_into('<H', bytecode, jump_offset_indices[i], current_offset)
                
                bc = option_bytecodes[i]
                bytecode.extend(bc)
                current_offset += len(bc)
                
                if i < num_options - 1:
                    jump_indices.append(len(bytecode))
                    bytecode.append(OP_JUMP)
                    bytecode.extend(struct.pack('<H', 0)) # Placeholder
                    current_offset += 3
                    
            # Backpatch the OP_JUMPs to the end
            # The jump offset is relative to the byte AFTER the OP_JUMP instruction
            end_idx = len(bytecode)
            for idx in jump_indices:
                jump_val = end_idx - (idx + 3)
                struct.pack_into('<H', bytecode, idx + 1, jump_val)

    elif node.tag == 'DialogVendorBuyOptions':
        bytecode.append(OP_VENDOR_BUY)
        var_name = node.attrib.get('values')
        items = variables.get(var_name, [])
        bytecode.append(len(items))
        for item_name in items:
            item_id = ITEM_MAP.get(item_name, 0)
            bytecode.append(item_id)
            
    elif node.tag == 'DialogVendorSellOptions':
        bytecode.append(OP_VENDOR_SELL)
        # Sell doesn't take specific items, it dynamically lists the player's inventory
        # The XML provides 'itemTypes' to restrict what can be sold (e.g. tools vs weapons)
        # We can just emit the opcode and let C handle reading the inventory
        item_types = node.attrib.get('itemTypes')
        type_val = 0
        if item_types == "['Tool']":
            type_val = 1
        elif item_types == "['Weapon', 'Armor', 'Shield']":
            type_val = 2
        bytecode.append(type_val)
        
    return bytecode

def pack_text(action_data, action_registry, ITEM_MAP, MONSTER_MAP):
    print("Packing dialogue and actions into PYDWTXT.bin...")
    
    # We need to build the offset table.
    num_actions = max(action_registry.values()) if action_registry else 0
    
    # +1 because action_ids start at 1, so array size is num_actions + 1
    offset_table = [0] * (num_actions + 1)
    action_bytecodes = {}
    
    current_offset = 2 + ((num_actions + 1) * 2) # Header size: 2 bytes for count, then offsets
    
    # First pass: compile all actions
    for label, element in action_data.items():
        if label not in action_registry: continue
        action_id = action_registry[label]
        
        bytecode = bytearray()
        
        # If element is None, just OP_END
        if element is None:
            bytecode.append(OP_END)
        else:
            # Pre-collect DialogVariable bindings FIRST so they can be substituted
            # into any following Dialog text or count= attributes (e.g. [CHEST_GOLD])
            variables = {}
            for child in element:
                if child.tag == 'DialogVariable':
                    vname = child.attrib.get('name')
                    vval  = child.attrib.get('value')
                    if vname and vval:
                        variables[vname] = vval

            has_dialog = False
            for child in element:
                if child.tag in ['Dialog', 'DialogAssert', 'DialogAction', 'DialogOptions', 'DialogGoTo', 'DialogVendorBuyOptions', 'DialogVendorSellOptions']:
                    has_dialog = True
                    bytecode.extend(compile_dialog_node(child, action_registry, action_data, ITEM_MAP, MONSTER_MAP, variables.copy()))
            
            if not has_dialog:
                # If there are no dialog children, maybe the element itself IS a Dialog?
                if element.tag in ['Dialog', 'DialogAssert', 'DialogAction', 'DialogOptions', 'DialogGoTo']:
                    bytecode.extend(compile_dialog_node(element, action_registry, action_data, ITEM_MAP, MONSTER_MAP, {}))
                    
            bytecode.append(OP_END)
            
        action_bytecodes[action_id] = bytecode
        offset_table[action_id] = current_offset
        current_offset += len(bytecode)
        
    # Write the binary file
    out_path = os.path.join(os.path.dirname(__file__), '..', 'bin', 'PYDWTXT.bin')
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    
    with open(out_path, 'wb') as f:
        f.write(struct.pack('<H', num_actions + 1))
        for i in range(num_actions + 1):
            f.write(struct.pack('<H', offset_table[i]))
            
        for i in range(1, num_actions + 1):
            if i in action_bytecodes:
                f.write(action_bytecodes[i])
            else:
                f.write(bytes([OP_END]))
                
    print(f"Generated PYDWTXT.bin ({current_offset} bytes)")
    
    # Run convbin to make it an 8xv
    out_8xv = os.path.join(os.path.dirname(__file__), '..', 'bin', 'PYDWTXT.8xv')
    
    cedev_path = os.environ.get('CEDEV', 'CEdev')
    if not os.path.exists(cedev_path) and os.path.exists('C:/CEdev'):
        cedev_path = 'C:/CEdev'
        
    convbin_exe = os.path.abspath(os.path.join(cedev_path, 'bin', 'convbin.exe'))
    
    cmd = [convbin_exe, '-j', 'bin', '-k', '8xv', '-r', '-i', out_path, '-o', out_8xv, '-n', 'PYDWTXT']
    try:
        subprocess.run(cmd, check=True, capture_output=True)
        print("Packed into PYDWTXT.8xv")
    except subprocess.CalledProcessError as e:
        print(f"convbin failed: {e.stderr.decode()}")
