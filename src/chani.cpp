#include "emu/ibm5160.h"
#include "emu/i8086.h"
#include "dos/dos.h"

int main(int argc, char const *argv[])
{
	ibm5160_t *machine = new ibm5160_t;
	dos_t     *dos     = new dos_t;

	machine->dos = dos;
	dos->machine = machine;

	raw_ifstream_t exe("DNCDPRG.EXE");
	if (!exe.good()) {
		return -1;
	}
	dos->exec(exe);

	for (;;) {
		machine->cpu->step();
	}

	return 0;
}
