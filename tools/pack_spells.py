import xml.etree.ElementTree as ET
import os

def main():
    tree = ET.parse('python-reference/data/game.xml')
    root = tree.getroot()
    
    spells_node = root.find('Spells')
    if spells_node is None:
        print("Error: Could not find <Spells> node.")
        return
    
    spells = []
    
    # Add NONE
    spells.append({
        'name': 'None',
        'enum_name': 'SPELL_NONE',
        'mp': 0,
        'in_combat': False,
        'out_combat': False,
    })
    
    for spell_node in spells_node.findall('Spell'):
        name = spell_node.attrib.get('name')
        mp = int(spell_node.attrib.get('mp', 0))
        in_combat = spell_node.attrib.get('availableInCombat', 'no') == 'yes'
        out_combat = spell_node.attrib.get('availableOutsideCombat', 'no') == 'yes'
        
        enum_name = "SPELL_" + name.upper()
        
        spells.append({
            'name': name,
            'enum_name': enum_name,
            'mp': mp,
            'in_combat': in_combat,
            'out_combat': out_combat,
        })
    
    # We must also append monster-only breath attacks so they can use the same SpellEnum
    spells.append({
        'name': 'BreathFire',
        'enum_name': 'SPELL_BREATH_FIRE',
        'mp': 0,
        'in_combat': True,
        'out_combat': False,
    })
    spells.append({
        'name': 'BreathSFire',
        'enum_name': 'SPELL_BREATH_STRONG_FIRE',
        'mp': 0,
        'in_combat': True,
        'out_combat': False,
    })

    print("Generating src/spell_data.h...")
    with open('src/spell_data.h', 'w') as f:
        f.write("#ifndef SPELL_DATA_H\n")
        f.write("#define SPELL_DATA_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stdbool.h>\n\n")
        
        f.write(f"#define NUM_SPELLS {len(spells)}\n\n")
        
        f.write("typedef enum {\n")
        for s in spells:
            f.write(f"    {s['enum_name']},\n")
        f.write("} SpellEnum;\n\n")
        
        f.write("typedef struct {\n")
        f.write("    char name[12];\n")
        f.write("    uint8_t mp;\n")
        f.write("    bool availableInCombat;\n")
        f.write("    bool availableOutsideCombat;\n")
        f.write("} SpellDef;\n\n")
        
        f.write("extern const SpellDef spellTable[NUM_SPELLS];\n\n")
        f.write("#endif\n")
        
    print("Generating src/spell_data.c...")
    with open('src/spell_data.c', 'w') as f:
        f.write("#include \"spell_data.h\"\n\n")
        f.write("const SpellDef spellTable[NUM_SPELLS] = {\n")
        
        for s in spells:
            f.write("    { ")
            f.write(f"\"{s['name'][:11]}\", ")
            f.write(f"{s['mp']}, ")
            f.write(f"{'true' if s['in_combat'] else 'false'}, ")
            f.write(f"{'true' if s['out_combat'] else 'false'} ")
            f.write("},\n")
            
        f.write("};\n")

if __name__ == '__main__':
    main()
