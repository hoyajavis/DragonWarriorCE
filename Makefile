# ----------------------------
# Makefile Options
# ----------------------------

NAME = PYDW
DESCRIPTION = "Dragon Warrior Demake"
COMPRESSED = NO
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(CEDEV)/meta/makefile.mk
