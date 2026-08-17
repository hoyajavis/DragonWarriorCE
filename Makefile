# ----------------------------
# Makefile Options
# ----------------------------

NAME = PYDW
DESCRIPTION = "Dragon Warrior Demake"
COMPRESSED = NO
ARCHIVED = YES

BSSHEAP_HIGH = 0xD19800

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(CEDEV)/meta/makefile.mk
