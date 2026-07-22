import os
import xml.etree.ElementTree as ET

def pack_items(game_tree, ACTION_REGISTRY, ACTION_DATA, NEXT_ACTION_ID):
    print("Packing items...")
    
    item_types = ['Weapons', 'Armors', 'Shields', 'Tools']
    
    items = []
    
    # 0 = ITEM_NONE
    items.append({
        'id': 0,
        'name': 'None',
        'type': 'ITEM_TYPE_NONE',
        'attackBonus': 0,
        'defenseBonus': 0,
        'price': 0,
        'useActionId': '0',
        'isEquippable': False,
        'isKeyItem': False
    })
    
    item_id = 1
    item_name_to_id = {}
    
    for category in item_types:
        cat_node = game_tree.find(f'.//{category}')
        if cat_node is None:
            continue
            
        for child in cat_node:
            name = child.attrib.get('name', 'Unknown')
            attackBonus = int(child.attrib.get('attackBonus', 0))
            defenseBonus = int(child.attrib.get('defenseBonus', 0))
            price = int(child.attrib.get('gp', 0))
            
            # Determine type
            item_type_enum = 'ITEM_TYPE_TOOL'
            if category == 'Weapons': item_type_enum = 'ITEM_TYPE_WEAPON'
            elif category == 'Armors': item_type_enum = 'ITEM_TYPE_ARMOR'
            elif category == 'Shields': item_type_enum = 'ITEM_TYPE_SHIELD'
            
            # Check for use action
            useActionId = '0'
            has_dialog = False
            for dchild in child:
                if dchild.tag in ['Dialog', 'DialogAction', 'DialogAssert']:
                    has_dialog = True
                    break
                    
            if has_dialog:
                # We need to register an action for this item's use
                safe_name = name.upper().replace(' ', '_').replace("'", "").replace("-", "_")
                label = f"USE_ITEM_{safe_name}"
                if label not in ACTION_REGISTRY:
                    ACTION_REGISTRY[label] = NEXT_ACTION_ID
                    NEXT_ACTION_ID += 1
                ACTION_DATA[label] = child
                useActionId = f"ACTION_{label}"
            
            isEquippable = False
            if category in ['Weapons', 'Armors', 'Shields']:
                isEquippable = True
            elif category == 'Tools' and child.attrib.get('equippable') == 'yes':
                isEquippable = True
            isKeyItem = child.attrib.get('droppable', 'yes') == 'no'
            
            items.append({
                'id': item_id,
                'name': name,
                'type': item_type_enum,
                'attackBonus': attackBonus,
                'defenseBonus': defenseBonus,
                'price': price,
                'useActionId': useActionId,
                'isEquippable': isEquippable,
                'isKeyItem': isKeyItem
            })
            item_name_to_id[name] = item_id
            item_id += 1
            
    # Generate item_data.h
    print("Generating src/item_data.h...")
    with open('src/item_data.h', 'w') as f:
        f.write("#ifndef ITEM_DATA_H\n")
        f.write("#define ITEM_DATA_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stdbool.h>\n\n")
        
        f.write(f"#define NUM_ITEMS {len(items)}\n\n")
        
        f.write("typedef enum {\n")
        f.write("    ITEM_TYPE_NONE = 0,\n")
        f.write("    ITEM_TYPE_WEAPON,\n")
        f.write("    ITEM_TYPE_ARMOR,\n")
        f.write("    ITEM_TYPE_SHIELD,\n")
        f.write("    ITEM_TYPE_TOOL\n")
        f.write("} ItemCategory;\n\n")
        
        f.write("typedef enum {\n")
        for item in items:
            safe_name = item['name'].upper().replace(' ', '_').replace("'", "").replace("-", "_")
            if safe_name == 'NONE':
                f.write(f"    ITEM_NONE = 0,\n")
            else:
                f.write(f"    ITEM_{safe_name} = {item['id']},\n")
        f.write("} ItemEnum;\n\n")
        
        f.write("typedef struct {\n")
        f.write("    const char *name;\n")
        f.write("    ItemCategory type;\n")
        f.write("    uint8_t attackBonus;\n")
        f.write("    uint8_t defenseBonus;\n")
        f.write("    uint16_t price;\n")
        f.write("    uint16_t useActionId;\n")
        f.write("    bool isEquippable;\n")
        f.write("    bool isKeyItem;\n")
        f.write("} ItemDef;\n\n")
        
        f.write("extern const ItemDef itemTable[NUM_ITEMS];\n\n")
        f.write("#endif\n")
        
    # Generate item_data.c
    print("Generating src/item_data.c...")
    with open('src/item_data.c', 'w') as f:
        f.write("#include \"item_data.h\"\n")
        f.write("#include \"action_ids.h\"\n\n")
        f.write("const ItemDef itemTable[NUM_ITEMS] = {\n")
        for i in items:
            eq = "true" if i['isEquippable'] else "false"
            ki = "true" if i['isKeyItem'] else "false"
            f.write(f"    {{\"{i['name']}\", {i['type']}, {i['attackBonus']}, {i['defenseBonus']}, {i['price']}, {i['useActionId']}, {eq}, {ki}}},\n")
        f.write("};\n")
        
    return NEXT_ACTION_ID, item_name_to_id
