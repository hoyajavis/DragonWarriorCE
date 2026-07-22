import struct

def dump_action(data, offset):
    if offset == 0:
        return "EMPTY"
    idx = offset
    res = ""
    while idx < len(data):
        opcode = data[idx]
        idx += 1
        if opcode == 0x00: # OP_END
            res += "OP_END "
            break
        elif opcode == 0x01: # OP_TEXT
            text = bytearray()
            while data[idx] != 0:
                text.append(data[idx])
                idx += 1
            idx += 1
            res += f"OP_TEXT({text.decode('ascii')}) "
        elif opcode == 0x02: # OP_JUMP_IF_FLAG
            flag_id, jump = struct.unpack('<HH', data[idx:idx+4])
            idx += 4
            res += f"OP_JUMP_IF_FLAG({flag_id},{jump}) "
        elif opcode == 0x03: # OP_JUMP_IF_NOT_FLAG
            flag_id, jump = struct.unpack('<HH', data[idx:idx+4])
            idx += 4
            res += f"OP_JUMP_IF_NOT_FLAG({flag_id},{jump}) "
        elif opcode == 0x16: # OP_GOTO_COORDINATES
            res += "OP_GOTO_COORDINATES "
            break
        else:
            res += f"OP_UNK({hex(opcode)}) "
            break
    return res

with open('bin/PYDWTXT.bin', 'rb') as f:
    data = f.read()

num_actions = struct.unpack('<H', data[0:2])[0]
print(f"Num Actions: {num_actions}")

for i in range(1, num_actions + 1):
    offset = struct.unpack('<H', data[i*2+2:i*2+4])[0]
    out = dump_action(data, offset)
    if i in [134, 135, 138, 139]:
        print(f"Action {i} -> {out}")
