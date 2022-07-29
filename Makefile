TARGET  = libcc.a
CLASSES = \
	cc_jobq       \
	cc_list       \
	cc_log        \
	cc_map        \
	cc_memory     \
	cc_multimap   \
	cc_mumurhash3 \
	cc_timestamp  \
	cc_workq
ifeq ($(CC_USE_MATH),1)
	CLASSES += \
		math/cc_pow2n       \
		math/cc_orientation \
		math/cc_quaternion  \
		math/cc_plane       \
		math/cc_float       \
		math/cc_fplane      \
		math/cc_ray3f       \
		math/cc_sphere      \
		math/cc_stack4f     \
		math/cc_rect12f     \
		math/cc_mat3f       \
		math/cc_mat4f       \
		math/cc_vec2f       \
		math/cc_vec3f       \
		math/cc_vec4f
endif
SOURCE  = $(CLASSES:%=%.c)
OBJECTS = $(SOURCE:.c=.o)
HFILES  = $(CLASSES:%=%.h)
OPT     = -O2 -Wall
CFLAGS  = $(OPT)
LDFLAGS = -lm
AR      = ar

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) *~ \#*\# $(TARGET)

$(OBJECTS): $(HFILES)
