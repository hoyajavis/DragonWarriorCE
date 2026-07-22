import struct

with open('obj/PYDW036.bin', 'rb') as f:
    data = f.read()
    
width = data[0]
height = data[1]
print(f"Map {width}x{height}")

offset = 2 + width * height
num_transitions = data[offset]
offset += 1
print(f"Transitions: {num_transitions}")
offset += num_transitions * 12

num_interactables = data[offset]
offset += 1
print(f"Interactables: {num_interactables}")

for i in range(num_interactables):
    if offset + 6 > len(data): break
    x, y, obj_type, dir_val, action_id = struct.unpack('<BBBBH', data[offset:offset+6])
    print(f"  [{i}] x={x}, y={y}, type={obj_type}, action_id={action_id}")
    offset += 6
