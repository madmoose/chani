#ifndef EMU_EMU_H
#define EMU_EMU_H

#include <cstdint>
#include <functional>

enum class address_space_t {
	_IO,
	_MEM
};

#define IO address_space_t::_IO
#define MEM address_space_t::_MEM

enum class width_t {
	_W8,
	_W16,
};

#define W8 width_t::_W8
#define W16 width_t::_W16

typedef std::function<uint16_t(address_space_t, uint32_t, width_t)> read_cb_t;
typedef std::function<void(address_space_t, uint32_t, width_t, uint16_t)> write_cb_t;

#define THIS_READ_CB(f)  [this](address_space_t s, uint32_t addr, width_t w) { return f(s, addr, w); };
#define THIS_WRITE_CB(f) [this](address_space_t s, uint32_t addr, width_t w, uint16_t v) { f(s, addr, w, v); }

typedef std::function<void()> callback_t;

#endif
