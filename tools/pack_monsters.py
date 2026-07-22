import os
import xml.etree.ElementTree as ET

# Helper to convert "15/16" or "1/16" into a percentage 0-100
def parse_fraction(val_str):
    if not val_str:
        return 0
    if "/" in val_str:
        parts = val_str.split("/")
        return int((float(parts[0]) / float(parts[1])) * 100)
    return int(float(val_str) * 100)

def main():
    game_xml_path = "python-reference/data/game.xml"
    if not os.path.exists(game_xml_path):
        print("game.xml not found!")
        return

    tree = ET.parse(game_xml_path)
    root = tree.getroot()

    monsters = []
    monster_indices = {}
    
    # Track unique sprite names to declare as extern
    sprite_names = []

    # Map Python reference spell types to our SpellEnum
    spell_map = {
        "Heal": "SPELL_HEAL",
        "Hurt": "SPELL_HURT",
        "Sleep": "SPELL_SLEEP",
        "Stopspell": "SPELL_STOPSPELL",
        "Healmore": "SPELL_HEALMORE",
        "Hurtmore": "SPELL_HURTMORE",
        "BreathFire": "SPELL_BREATH_FIRE",
        "BreathStrongFire": "SPELL_BREATH_STRONG_FIRE"
    }

    # Extract global monsters (children of Game, not inside a map or encounter)
    # Actually, in game.xml, they are under <Game> -> <Monster> or similar. Let's just find all <Monster> that have "strength" defined.
    for monster_node in root.findall(".//Monster"):
        if "strength" not in monster_node.attrib:
            continue # Skip map-instance monsters

        name = monster_node.attrib.get("name", "Unknown")
        image = monster_node.attrib.get("image", "slime.png")
        sprite_name = os.path.splitext(image)[0]
        if sprite_name not in sprite_names:
            sprite_names.append(sprite_name)
            
        strength = int(monster_node.attrib.get("strength", "0"))
        agility = int(monster_node.attrib.get("agility", "0"))
        
        hp_str = monster_node.attrib.get("hp", "0")
        if "-" in hp_str:
            hp_min = int(hp_str.split("-")[0])
            hp_max = int(hp_str.split("-")[1])
        else:
            hp_min = int(hp_str)
            hp_max = int(hp_str)

        xp = int(monster_node.attrib.get("xp", "0"))
        
        gp_str = monster_node.attrib.get("gp", "0")
        if "-" in gp_str:
            gp = int(gp_str.split("-")[0]) # Just take min GP for now
        else:
            gp = int(gp_str)
            
        allows_crit = "true" if monster_node.attrib.get("allowsCriticalHits", "yes") == "yes" else "false"
        
        sleep_resist = parse_fraction(monster_node.attrib.get("sleepResist", "0"))
        stopspell_resist = parse_fraction(monster_node.attrib.get("stopspellResist", "0"))
        hurt_resist = parse_fraction(monster_node.attrib.get("hurtResist", "0"))
        
        blockFactor_str = monster_node.attrib.get("blockFactor", "0.25")
        blockFactor64 = int(float(blockFactor_str) * 64)

        spellId1 = "SPELL_NONE"
        spellChance1 = 0
        spellId2 = "SPELL_NONE"
        spellChance2 = 0
        healthThreshold = 0

        action_rules = monster_node.findall("MonsterActionRule")
        for i, rule in enumerate(action_rules):
            action_type = rule.attrib.get("type")
            prob = int(float(rule.attrib.get("probability", "0")) * 100)
            thresh = int(float(rule.attrib.get("healthRatioThreshold", "0")) * 100)
            
            mapped_spell = spell_map.get(action_type, "SPELL_NONE")
            
            if i == 0:
                spellId1 = mapped_spell
                spellChance1 = prob
                healthThreshold = thresh
            elif i == 1:
                spellId2 = mapped_spell
                spellChance2 = prob

        monster_indices[name] = len(monsters)
        monsters.append({
            "name": name,
            "hp_min": hp_min,
            "hp_max": hp_max,
            "strength": strength,
            "agility": agility,
            "xp": xp,
            "gp": gp,
            "allows_crit": allows_crit,
            "sleep_resist": sleep_resist,
            "stopspell_resist": stopspell_resist,
            "hurt_resist": hurt_resist,
            "blockFactor64": blockFactor64,
            "spell1": spellId1,
            "chance1": spellChance1,
            "spell2": spellId2,
            "chance2": spellChance2,
            "thresh": healthThreshold,
            "sprite": sprite_name
        })

    print(f"Packed {len(monsters)} monsters.")

    # Parse MonsterSets
    monster_sets = []
    for mset_node in root.findall(".//MonsterSets/MonsterSet"):
        m_list = []
        for m_node in mset_node.findall("Monster"):
            m_name = m_node.attrib.get("name")
            if m_name in monster_indices:
                m_list.append(monster_indices[m_name])
        # Pad to 10 entries with 255 (0xFF)
        while len(m_list) < 10:
            m_list.append(255)
        monster_sets.append(m_list)

    # Generate src/monster_data.h
    with open("src/monster_data.h", "w") as f:
        f.write("#ifndef MONSTER_DATA_H\n")
        f.write("#define MONSTER_DATA_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include <stdbool.h>\n")
        f.write("#include <graphx.h>\n")
        f.write("#include \"spells.h\"\n\n")
        
        f.write("typedef struct {\n")
        f.write("    char name[24];\n")
        f.write("    uint8_t hp_min;\n")
        f.write("    uint8_t hp_max;\n")
        f.write("    uint8_t strength;\n")
        f.write("    uint8_t defense;\n")
        f.write("    uint8_t agility;\n")
        f.write("    uint16_t xp;\n")
        f.write("    uint16_t gp;\n")
        f.write("    bool allowsCriticalHits;\n")
        f.write("    uint8_t sleepResist;\n")
        f.write("    uint8_t stopspellResist;\n")
        f.write("    uint8_t hurtResist;\n")
        f.write("    uint8_t blockFactor64;\n\n")
        f.write("    SpellEnum spellId1;\n")
        f.write("    uint8_t spellChance1;\n")
        f.write("    SpellEnum spellId2;\n")
        f.write("    uint8_t spellChance2;\n")
        f.write("    uint8_t healthThreshold;\n\n")
        f.write("    void* sprite;\n")
        f.write("} MonsterDef;\n\n")
        
        f.write(f"#define NUM_MONSTERS {len(monsters)}\n")
        f.write("extern const MonsterDef monsterTable[NUM_MONSTERS];\n\n")
        f.write("extern const uint8_t monsterSets[20][10];\n\n")

        for idx, m in enumerate(monsters):
            # Create a clean macro name (e.g. MONSTER_SLIME, MONSTER_DRAGONLORD_S_TRUE_FORM)
            safe_name = m['name'].upper().replace("'", "").replace(" ", "_")
            f.write(f"#define MONSTER_{safe_name} {idx}\n")
        
        f.write("\n#endif // MONSTER_DATA_H\n")

    # Generate src/monster_data.c
    with open("src/monster_data.c", "w") as f:
        f.write("#include \"monster_data.h\"\n")
        f.write("#include \"gfx/gfx.h\"\n\n")
        
        f.write(f"const MonsterDef monsterTable[{len(monsters)}] = {{\n")
        for m in monsters:
            f.write(f"    {{\"{m['name']}\", {m['hp_min']}, {m['hp_max']}, {m['strength']}, {m['agility'] // 2}, {m['agility']}, {m['xp']}, {m['gp']}, {m['allows_crit']}, {m['sleep_resist']}, {m['stopspell_resist']}, {m['hurt_resist']}, {m['blockFactor64']}, {m['spell1']}, {m['chance1']}, {m['spell2']}, {m['chance2']}, {m['thresh']}, (void*){m['sprite']}}},\n")
        f.write("};\n\n")

        f.write("const uint8_t monsterSets[20][10] = {\n")
        for m_list in monster_sets:
            m_str = ", ".join(str(idx) for idx in m_list)
            f.write(f"    {{{m_str}}},\n")
        f.write("};\n")

if __name__ == "__main__":
    main()
