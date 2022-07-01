#ifndef DOS_DOS_ENVIRONMENT_H
#define DOS_DOS_ENVIRONMENT_H

#include "support/types.h"

#include <optional>

class ibm5160_t;

class dos_environment_t {
public:
	// static dos_environment_t create_in_mcb(ibm5160_t *machine, mcb_t mcb);
	static dos_environment_t create_in_seg(ibm5160_t *machine, uint16_t seg, uint16_t capacity);
	static dos_environment_t from_seg(ibm5160_t *machine, uint16_t seg);

	void hexdump();

	bool set(const char *s);

private:
	dos_environment_t() {};

	ibm5160_t *machine;

	uint16_t seg;
	uint16_t capacity;
	uint16_t size; // Size includes the final zero

	struct env_entry_t {
		uint16_t begin;
		uint16_t equal_sign_pos;
		uint16_t end;

		uint16_t length() { return end - begin; }
	};

	struct set_string_t {
		const char *begin;
		const char *equal_sign_pos;
		const char *end;

		size_t key_length() { return equal_sign_pos - begin; }
		size_t length() { return end - begin; }
	};

	bool                       set_entry(set_string_t s);
	std::optional<env_entry_t> get_entry(set_string_t s);
	void                       remove_entry(env_entry_t e);
};

#endif
