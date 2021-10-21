#include "dos.h"

#include "binaries/exe_mz_header.h"
#include "emu/i8086.h"
#include "emu/ibm5160.h"
#include "support/file_reader.h"
#include "support/mem_writer.h"
#include "support/types.h"

void dos_t::build_psp(uint16_t psp_segment) {
	byte *image = &machine->memory[0x10 * psp_segment];
	mem_writer_t psp(image, 256);

	psp.writebyte(0xcd);
	psp.writebyte(0x20);
	psp.writele16(0x9fff);

	psp.seek_set(0x80);
	psp.writebyte(0x00);
	psp.writebyte(0x0d);
}

bool dos_t::exec(file_reader_t &rd) {
	uint16_t load_segment = 0x01ed; // TODO: Hacked to match DOSBox

	exe_mz_header_t head;
	if (!head.load(rd)) {
		puts("Head load failed.");
		return false;
	}

	uint32_t image_size = 512 * head.e_cp - 16 * head.e_cparhdr;
	if (head.e_cblp)
		image_size += head.e_cblp - 512;

	rd.seek_set(16 * head.e_cparhdr);

	uint16_t psp_segment = load_segment - 0x10;
	build_psp(psp_segment);

	byte *image = &(machine->memory[0x10 * load_segment]);
	rd.read(image, image_size);

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
		uint16_t seg = rd.readle16();

		byte *p = &image[0x10 * seg + ofs];
		writele16(p, readle16(p) + load_segment);
	}

	cpu->cx = 0xff;
	cpu->dx = 0x1dd;
	cpu->di = 0x3cbc;
	cpu->bp = 0x91c;
	cpu->set_if(true);

	cpu->ds = psp_segment;
	cpu->es = psp_segment;
	cpu->ss = head.e_ss + load_segment;
	cpu->sp = head.e_sp;
	cpu->cs = head.e_cs + load_segment;
	cpu->ip = head.e_ip;

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
