import glob
import os

files = [os.path.splitext(os.path.basename(f))[0] for f in sorted(glob.glob('python-reference/data/maps/legacy/*.dat'))]
for i, f in enumerate(files, 1):
    # Map alefgard -> overworld for backwards compatibility
    name = 'overworld' if f == 'alefgard' else f
    xml = f'map_{f}_legacy.xml'
    # Overworld in python ref uses map_alefgard_legacy.xml
    print(f"    '{name}': ('PYDW{i:03d}', 'legacy/{f}.dat', '{xml}'),")
