#include "dos/dos_environment.h"

#include "dos/dos_alloc.h"
#include "emu/ibm5160.h"

dos_environment_t dos_environment_t::create_in_seg(ibm5160_t *machine, uint16_t seg, uint16_t capacity) {
	dos_environment_t env;
	env.machine  = machine;
	env.seg      = seg;
	env.capacity = capacity;
	env.size     = 1;

	assert(env.capacity < 0x8000);

	for (int i = 0; i != env.capacity; ++i) {
		machine->mem_write8(seg, i, ' ');
	}

	machine->mem_write8(seg, 0, '\0');

	return env;
}

dos_environment_t dos_environment_t::from_seg(ibm5160_t *machine, uint16_t seg) {
	assert(seg > 0);

	mcb_t mcb(machine, seg - 1);
	assert(mcb.has_valid_signature());

	dos_environment_t env;
	env.machine  = machine;
	env.seg      = seg;
	env.capacity = 0x10 * mcb.size_in_paras();

	assert(env.capacity < 0x8000);

	int i = 0;
	int entry_len = 0;
	do {
		int entry_start = i;
		for (; env.machine->mem_read8(env.seg, i) != '\0'; ++i)
			;
		i++;
		entry_len = i - entry_start;
	} while (i < env.capacity && entry_len > 1);

	assert(i < env.capacity);
	env.size = i;

	// env.hexdump();

	return env;
}

void dos_environment_t::hexdump() {
	for (int i = 0; i != 10; ++i) {
		printf("%04x:%04x ", seg, 0x10 * i);
		for (int j = 0; j != 16; ++j) {
			printf("%02x ", machine->mem_read8(seg, i * 0x10 + j));
		}
		for (int j = 0; j != 16; ++j) {
			byte c = machine->mem_read8(seg, i * 0x10 + j);
			printf("%c", isprint(c) ? c : '.');
		}
		printf("\n");
	}
	printf("\n");
}

bool dos_environment_t::set(const char *s) {
	// Find and count equal signs in set string.
	int equal_signs = 0;
	const char *equal_sign_pos = nullptr;
	int len = 0;
	for (len = 0; s[len] != '\0'; ++len) {
		if (s[len] == '=') {
			equal_sign_pos = s + len;
			equal_signs++;
		}
	}
	if (equal_signs != 1) {
		return false;
	}

	if (len >= 256) {
		return false;
	}

	set_string_t ss{ s, equal_sign_pos, s + len };

	return set_entry(ss);
}

bool dos_environment_t::set_entry(set_string_t s) {
	if (auto opt_e = get_entry(s)) {
		remove_entry(*opt_e);
	}

	if (size + s.length() + 1 > capacity) {
		return false;
	}

	// Place p at the final '\0'
	uint16_t p = size - 1;
	for (int i = 0; i != s.length() + 1; ++i, ++p) {
		machine->mem_write8(seg, p, s.begin[i]);
	}
	size += s.length() + 1;

	// Add ending '\0'
	machine->mem_write8(seg, size - 1, '\0');

	return true;
}

std::optional<dos_environment_t::env_entry_t> dos_environment_t::get_entry(set_string_t s) {
	env_entry_t e{ 0, 0, 0 };

	enum {
		ENTRY_START,
		ENTRY_KEY,
		ENTRY_VALUE,
		LIST_END,
		ERROR,
	} state;

	int entry_count = 0;

	state = ENTRY_START;
	int i = 0, si = 0;
	bool matches = false;
	while (state != LIST_END && state != ERROR) {
		assert(i < size);
		byte c = machine->mem_read8(seg, i++);
		assert(i < size || c == '\0');

		switch (state) {
			case ENTRY_START:
				if (c == '\0') {
					state = LIST_END;
				} else if (c == '=') {
					state = ERROR;
				} else {
					e.begin = e.equal_sign_pos = e.end = i;
					state = ENTRY_KEY;
					matches = i < s.key_length() && c == s.begin[si++];
				}
				break;
			case ENTRY_KEY:
				e.end = i;
				if (c == '\0') {
					state = ERROR;
				} else if (c == '=') {
					e.equal_sign_pos = i;
					state = ENTRY_VALUE;
				} else {
					matches = matches && i < s.key_length() && c == s.begin[si++];
				}
				break;
			case ENTRY_VALUE:
				e.end = i;
				if (c == '\0') {
					entry_count++;
					if (matches) {
						return e;
					}
					state = ENTRY_START;
				} else if (c == '=') {
					state = ERROR;
				}
				break;
			case LIST_END:
			case ERROR:
				break;
		}
	}

	return {};
}

void dos_environment_t::remove_entry(env_entry_t e) {
}
