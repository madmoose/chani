#include "exe_mz_header.h"

#include "support/file_reader.h"

#include <cstdio>

#define SIG_MZ 0x4d5a
#define SIG_NE 0x4e45

bool exe_mz_header_t::load(file_reader_t &rd)
{
	// if (rd.rem() < 28) {
	// 	puts("Not a valid executable: Size too small");
	// 	return false;
	// }

	e_magic = rd.readbe16();

	if (e_magic != SIG_MZ) {
		puts("Not a valid executable: Header magic not MZ.");
		return false;
	}

	e_cblp      = rd.readle16();
	e_cp        = rd.readle16();
	e_crlc      = rd.readle16();
	e_cparhdr   = rd.readle16();
	e_minalloc  = rd.readle16();
	e_maxalloc  = rd.readle16();
	e_ss        = rd.readle16();
	e_sp        = rd.readle16();
	e_csum      = rd.readle16();
	e_ip        = rd.readle16();
	e_cs        = rd.readle16();
	e_lfarlc    = rd.readle16();
	e_ovno      = rd.readle16();

	if (e_lfarlc == 0x40) {
		if (rd.eof()) {
			return false;
		}

		for (int i = 0; i != 4; ++i) {
			e_res[i] = rd.readle16();
		}
		e_oemid   = rd.readle16();
		e_oeminfo = rd.readle16();
		for (int i = 0; i != 10; ++i) {
			e_res2[i] = rd.readle16();
		}
		e_lfanew = rd.readle32();
	}

	return !rd.eof();
}

void exe_mz_header_t::dump()
{
	printf("DOS header\n");
	printf("e_magic     = %04x\n", e_magic);
	printf("e_cblp      = %04x\n", e_cblp);
	printf("e_cp        = %04x\n", e_cp);
	printf("e_crlc      = %04x\n", e_crlc);
	printf("e_cparhdr   = %04x\n", e_cparhdr);
	printf("e_minalloc  = %04x\n", e_minalloc);
	printf("e_maxalloc  = %04x\n", e_maxalloc);
	printf("e_ss        = %04x\n", e_ss);
	printf("e_sp        = %04x\n", e_sp);
	printf("e_csum      = %04x\n", e_csum);
	printf("e_ip        = %04x\n", e_ip);
	printf("e_cs        = %04x\n", e_cs);
	printf("e_lfarlc    = %04x\n", e_lfarlc);
	printf("e_ovno      = %04x\n", e_ovno);

	if (e_lfarlc == 0x40) {
		for (int i = 0; i != 4; ++i)
			printf("e_res[%d]    = %04x\n", i, e_res[i]);
		printf("e_oemid     = %04x\n", e_oemid);
		printf("e_oeminfo   = %04x\n", e_oeminfo);
		for (int i = 0; i != 10; ++i)
			printf("e_res2[%d]   = %04x\n", i, e_res2[i]);
		printf("e_lfanew    = %04x\n", e_lfanew);
	}
	putchar('\n');
}
