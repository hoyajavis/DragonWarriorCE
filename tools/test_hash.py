def stable_hash(s):
    h = 5381
    for c in s:
        h = ((h << 5) + h) + ord(c)
        h &= 0xFFFFFFFF
    return h % 512

print(f"PM_Defeated_Dragonlord: {stable_hash('PM_Defeated_Dragonlord')}")
print(f"PM_Not_Taken_Initial_Chest_1: {stable_hash('PM_Not_Taken_Initial_Chest_1')}")
