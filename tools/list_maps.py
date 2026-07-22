import glob
import os
import xml.etree.ElementTree as ET

for f in glob.glob('data/maps/*.xml'):
    try:
        tree = ET.parse(f)
        root = tree.getroot()
        maps = root.findall('.//Map')
        if not maps and root.tag == 'Map':
            maps = [root]
        for m in maps:
            print(f"{m.get('name')} : {m.get('tiles')} : {os.path.basename(f)}")
    except Exception as e:
        pass
