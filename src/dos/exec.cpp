#include "dos.h"

#include "binaries/exe_mz_header.h"
#include "dos/dos_environment.h"
#include "emu/i8086.h"
#include "emu/ibm5160.h"
#include "support/file_reader.h"
#include "support/mem_writer.h"
#include "support/types.h"

void dos_t::build_psp(uint16_t psp_segment, uint16_t psp_size_paras) {
	printf("build_psp @ %04x\n", psp_segment);
	byte *psp_image = &machine->memory[0x10 * psp_segment];
	mem_writer_t psp(psp_image, psp_size_paras);

	psp.writebyte(0xcd);
	psp.writebyte(0x20);
	psp.writele16(0xa000);

	psp.writebyte(0x00);

	psp.writebyte(0xcd);
	psp.writebyte(0x20);

	psp.seek_set(0x80);
	psp.writebyte(0x00);
	psp.writebyte(0x0d);
}

bool dos_t::exec(file_reader_t &rd) {
	const int env_size_paras = 0x0a;
	const int psp_size_paras = 0x10;

	current_psp = 1;

	allocate_memory(110 + 0x633 - 1 - 59);

	// Allocate memory for, and create, the environment for the new process.
	uint16_t env_seg = allocate_memory(env_size_paras + 59);
	assert(env_seg);

	dos_environment_t env = dos_environment_t::create_in_seg(machine, env_seg, env_size_paras << 4);

	env.set("PATH=Z:\\");
	env.set("COMSPEC=Z:\\COMMAND.COM");
	env.set("PROMPT=$P$G");
	env.set("BLASTER=A220 I7 D1 H5 P330 T6");

	// Read the header of the executable.
	exe_mz_header_t head;
	if (!head.load(rd)) {
		puts("Head load failed.");
		return false;
	}

	bool load_high = head.e_maxalloc == 0;

	assert((head.e_cp << 5) > head.e_cparhdr);
	uint32_t image_size_paras = (head.e_cp << 5) - head.e_cparhdr;

	// Find the max available memory by asking for too much
	uint16_t max_memory_paras;
	bool r = allocate_memory(0xffff, &max_memory_paras);

	// Allocation should always fail
	assert(r == 0);

	uint32_t required_paras = psp_size_paras + image_size_paras + head.e_minalloc;

	if (required_paras > max_memory_paras) {
		printf("Not enough memory to load executable. (%x > %x)\n", required_paras << 4, max_memory_paras << 4);
		exit(1);
	}

	if (load_high) {
		required_paras = max_memory_paras;
	} else {
		required_paras = std::min(uint32_t(max_memory_paras), required_paras + head.e_maxalloc);
	}

	uint16_t load_seg = allocate_memory(required_paras);
	assert(load_seg);

	rd.seek_set(head.e_cparhdr << 4);

	uint16_t exe_seg = load_seg + psp_size_paras;

	// TODO: Handle load_high
	assert(load_high == 0);
	byte *image = &(machine->memory[0x10 * exe_seg]);

	uint32_t image_size = (image_size_paras << 4);
	if (head.e_cblp) {
		assert(head.e_cblp < 512);
		assert(image_size >= 512);
		image_size = image_size - 512 + head.e_cblp;
	}
	rd.read(image, image_size);

	uint16_t psp_segment = load_seg;
	build_psp(psp_segment, psp_size_paras);

	machine->mem_write16(psp_segment, 0x2c, env_seg);

	user_dta_seg = psp_segment;
	user_dta_ofs = 0x80;

#if 0
	printf("Loading %04x-%04x\n", load_segment, load_segment + (image_size / 0x10));
	for (int p = load_segment; p < load_segment + (image_size / 0x10); ++p) {
		printf("%04x:0000 ", p);
		for (int i = 0; i != 16; ++i) {
			if (i % 16 > 0) {
				printf(" ");
			}
			printf("%02x", machine.memory[0x10 * p + i]);
			if (i % 16 == 15) {
				printf("\n");
			}
		}
	}
#endif

	rd.seek_set(head.e_lfarlc);
	for (int i = 0; i != head.e_crlc; ++i) {
		uint16_t ofs = rd.readle16();
		uint16_t seg = rd.readle16() + exe_seg;

		uint16_t v = machine->mem_read16(seg, ofs);
		machine->mem_write16(seg, ofs, v + exe_seg);
	}

	cpu->cx = 0xff;
	cpu->dx = 0x0810;
	cpu->di = 0x1f40;
	cpu->bp = 0x91c;
	cpu->set_if(true);

	cpu->si = 0x0012;

	cpu->ds = psp_segment;
	cpu->es = psp_segment;
	cpu->ss = head.e_ss + exe_seg;
	cpu->sp = head.e_sp;
	cpu->cs = head.e_cs + exe_seg;
	cpu->ip = head.e_ip;

	if (!validate_mcb_chain()) {
		printf("%s:%d\n", __FILE__, __LINE__);
		exit(0);
	}

#if 0
	for (int i = 0; i != 64; ++i) {
		if (i % 16 > 0) {
			printf(" ");
		}
		printf("%02x", machine.memory[0x1138 + i]);
		if (i % 16 == 15) {
			printf("\n");
		}
	}
#endif

	return true;
}
