#ifndef DOS_DOS_ALLOC_H
#define DOS_DOS_ALLOC_H

#include "support/types.h"

class ibm5160_t;

// DOS Memory Control Block
struct mcb_t {
	ibm5160_t *machine;
	uint16_t   seg;

	mcb_t(ibm5160_t *machine, uint16_t seg);
	static mcb_t build_mcb(ibm5160_t *machine, uint16_t seg);

	byte     signature();
	bool     has_valid_signature();
	bool     is_last();
	void     set_is_last(bool last = true);
	uint16_t get_owner_pid();
	void     set_owner_pid(uint16_t pid);
	bool     is_free();
	void     set_is_free();
	uint16_t size_in_paras();
	void     set_size_in_paras(uint16_t paras);
	uint16_t data_seg();
	mcb_t    next();

	void     split(uint16_t paras);
	void     combine();
	bool     coalesce_free_blocks();
};

#endif
