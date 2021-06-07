#ifndef EMU_I8284_PIT
#define EMU_I8284_PIT

#include "emu/device.h"
#include "support/types.h"

#include <functional>
#include <vector>

class ibm5160_t;

struct i8254_counter_t {
	bool     activated;
	uint32_t counting_element;
	uint16_t output_latch;     // Latch for reading
	uint16_t count_register;   // Register for writing
	byte     is_latched:1;
	byte     r:2;
	byte     w:2;
	byte     mode:3;
	byte     bcd:1;
	byte     out:1;

	i8254_counter_t() :
		activated(false),
		counting_element(0x10000),
		output_latch(0),
		count_register(0),
		is_latched(false),
		r(0),
		w(0),
		mode(0),
		bcd(0),
		out(0)
	{}

	byte read();
	void write(byte v);

	uint64_t next_cycles();
	uint64_t run_cycles(uint64_t cycles);
};

class i8254_pit_t : public device_t {
	byte selected_counter = 0;
	enum {
		ST_CW,
		ST_W_LSB,
		ST_W_MSB,
		ST_W_LSBMSB,
	} write_state = ST_CW;
	enum {
		ST_R_NONE,
		ST_R_LSB,
		ST_R_MSB,
		ST_R_LSBMSB,
	} read_state = ST_R_NONE;

	i8254_counter_t counter[3];

public:
	i8254_pit_t();

	byte     read(byte addr);
	void     write(byte addr, byte cw);

	double   frequency_in_mhz() { return 1.1931818181818181; };
	uint64_t next_cycles();
	uint64_t run_cycles(uint64_t cycles);
};

#endif
