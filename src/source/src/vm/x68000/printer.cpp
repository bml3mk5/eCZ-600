/** @file printer.cpp

	SHARP X68000 Emulator 'eCZ-600'
	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ printer ]
*/

#include "printer.h"
#include <stdlib.h>
#include "../../emu.h"
#include "../../fileio.h"
#include "../../fifo.h"
#include "../../config.h"
#include "../../utility.h"


#define PRINTER_PEND_SIZE	64
//#define PRINTER_DELAY_TIME (CPU_CLOCKS/19200)
#define PRINTER_DELAY_SEC  (CPU_CLOCKS)

void PRINTER::cancel_my_event(int &id)
{
	if(id != -1) {
		cancel_event(this, id);
		id = -1;
	}
}

void PRINTER::register_my_event(double wait, int &id)
{
	cancel_my_event(id);
	register_event(this, SIG_PRINTER_CLOCK, wait, false, &id);
}

void PRINTER::initialize()
{
	h_fio = new FILEIO();
	p_data_buffer = new FIFOBYTE();
	m_buffer_overflow = false;
	m_send = false;
	m_strobe = false;
	m_busdata = 0;

	m_client_ch = -1;
	m_connect = DISCONNECT;
	m_send_type = SEND_TYPE_FILEONLY;
	m_send_size = 0;
#ifdef USE_PRINTER_PENDSIZE
	m_pend_size = 0;
#endif
	p_send_buff = NULL;
	memset(m_recv_buff, 0x00, sizeof(m_recv_buff));

	m_register_id = -1;
}

void PRINTER::reset()
{
	p_data_buffer->clear();
	m_buffer_overflow = false;
	m_strobe = false;
	m_busdata = 0;
	write_signals(&outputs_busy, 0);	// busy off 
}

void PRINTER::release()
{
	if (p_data_buffer) {
		delete p_data_buffer;
		p_data_buffer = NULL;
	}
	delete h_fio;
	h_fio = NULL;
}

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 1) {
	case 0:
		// data
		m_busdata = data;
		break;
	case 1:
		// strobe (negative logic)
		if (!m_strobe && !(data & 1)) {
			if (pConfig->printer_online[m_cfg_num]) {
				register_my_event(pConfig->printer_delay[m_cfg_num] * 1000.0, m_register_id);
				write_signals(&outputs_busy, 0xffffffff);	// busy 

//				logging->out_debugf("printer reg_id:%d i:%02d d:%02d m:%02d send:%s",register_id,id,data,mask,send ? "true" : "false");
			}
		}
		m_strobe = ((data & 1) == 0);
		break;
	}
}

void PRINTER::set_data(uint32_t data)
{
	if (p_data_buffer->write_pos() >= p_data_buffer->size()) {
		if (p_data_buffer->size() < PRNDATA_BUFFER_SIZE_MAX) {
			if (!p_data_buffer->allocate(p_data_buffer->size() + PRNDATA_BUFFER_SIZE)) {
				logging->out_logf(LOG_ERROR, _T("%s%d: Realloc failed."), this_class_name, m_cfg_num);
			}
		} else {
			m_buffer_overflow = true;
			logging->out_logf(LOG_ERROR, _T("%s%d: Buffer overflow."), this_class_name, m_cfg_num);
		}
	}
	if (m_buffer_overflow != true) {
		p_data_buffer->write(data & 0xff);
	}

#ifdef USE_SOCKET
	if (pConfig->printer_direct[m_cfg_num]) {
		if (!connect_socket()) {
			return;
		}
		// request sending printer data to server
		m_send_type = SEND_TYPE_DIRECT;
		m_send_size = 1;
		m_send_retry = 0;
		p_send_buff = p_data_buffer->data(p_data_buffer->write_pos()-1);
		if (m_connect == CONNECTED) {
			emu->send_data_tcp(m_client_ch);
#ifdef USE_PRINTER_PENDSIZE
			m_pend_size++;
#endif
		}
	}
#endif
//	logging->out_debugf("printer w %04x=%02x",addr,data);
}

void PRINTER::write_signal(int id, uint32_t data, uint32_t mask)
{
}

bool PRINTER::save_printer(const _TCHAR* filename)
{
	close_printer();

	if(h_fio->Fopen(filename, FILEIO::WRITE_BINARY)) {
		save_image();
	} else {
		return false;
	}
	close_printer();
	return true;
}

void PRINTER::close_printer()
{
	// close file
	h_fio->Fclose();
}

void PRINTER::save_image()
{
	uint8_t d[2];

	d[1]=0;
	// binary image
	for(int i=0; i<p_data_buffer->write_pos(); i++) {
		d[0] = p_data_buffer->peek(i);
//		d[0] = ~d[0];
		h_fio->Fwrite(d, sizeof(uint8_t), 1);
	}
}

int PRINTER::get_buffer_size() const
{
	return p_data_buffer->write_pos();
}

uint8_t* PRINTER::get_buffer() const
{
	return p_data_buffer->data(0);
}

bool PRINTER::set_direct_mode()
{
	if (!pConfig->printer_direct[m_cfg_num]) {
		m_send_size = 0;
		p_send_buff = get_buffer();

		if (!connect_socket()) {
			return false;
		}
		pConfig->printer_direct[m_cfg_num] = true;

	} else {
		disconnect_socket();
		m_send_type = SEND_TYPE_FILEONLY;
		pConfig->printer_direct[m_cfg_num] = false;

	}
	return true;
}

bool PRINTER::print_printer()
{
	if (m_send_type == SEND_TYPE_DIRECT) {
		return false;
	}

	m_send_type = SEND_TYPE_INDIRECT;
	m_send_size = get_buffer_size();
	p_send_buff = get_buffer();

	if (!connect_socket()) {
		return false;
	}
#ifdef USE_SOCKET
	if (m_connect == CONNECTED) {
		// already connected and writeable to socket
		emu->send_data_tcp(m_client_ch);
#ifdef USE_PRINTER_PENDSIZE
		m_pend_size = 0;
#endif
	}
#endif
	return true;
}

/// toggle on/offline to printer
void PRINTER::toggle_printer_online()
{
	pConfig->printer_online[m_cfg_num] = !pConfig->printer_online[m_cfg_num];

	if (m_strobe && pConfig->printer_online[m_cfg_num]) {
		register_my_event(pConfig->printer_delay[m_cfg_num] * 1000.0, m_register_id);
	}
}

uint8_t* PRINTER::get_sendbuffer(int ch, int* size, int* flags)
{
	if (ch == m_client_ch) {
		*size = m_send_size;
		return p_send_buff;
	} else {
		*size = 0;
		return NULL;
	}
}
void PRINTER::inc_sendbuffer_ptr(int ch, int size)
{
	if (ch == m_client_ch) {
		if (m_send_type == SEND_TYPE_INDIRECT) {
			m_send_size = (get_buffer_size() - size);
		} else if (m_send_type == SEND_TYPE_DIRECT) {
			if (size == 0) {
				// retry to send data later
				m_send_retry++;
				if (m_send_retry > 10) {
					// sending abort
					size = 1;
				}
			} else {
				m_send_retry = 0;
			}
			m_send_size = (1 - size);
		} else {
			m_send_size = 0;
		}

		p_send_buff += size;

		if (m_send_size <= 0) {
			if (m_send_type == SEND_TYPE_INDIRECT) {
				register_event_by_clock(this, SIG_PRINTER_DISCONNECT, PRINTER_DELAY_SEC, false, &m_register_id_2);
				m_send_type = SEND_TYPE_FILEONLY;
			}
			m_send_size = 0;
			p_send_buff = get_buffer();
		}
	}
}
uint8_t* PRINTER::get_recvbuffer0(int ch, int* size0, int* size1, int* flags)
{
	if (ch == m_client_ch) {
		*size0 = 4;
		*size1 = 0;
		return m_recv_buff;
	} else {
		*size0 = 0;
		*size1 = 0;
		return NULL;
	}
}

uint8_t* PRINTER::get_recvbuffer1(int ch)
{
	if (ch == m_client_ch) {
		return m_recv_buff;
	} else {
		return NULL;
	}
}

void PRINTER::inc_recvbuffer_ptr(int ch, int size)
{
#ifdef USE_PRINTER_PENDSIZE
	if (ch == m_client_ch) {
		if (size > 0 && m_pend_size >= PRINTER_PEND_SIZE && m_send_type == 2) {
			// receive ACK signal from mpprinter
			// and send ACK signal to computer
			register_my_event(1, m_register_id);
			m_pend_size = 0;
		}
	}
#endif
}

bool PRINTER::connect_socket()
{
#ifdef USE_SOCKET
	if (m_connect == DISCONNECT) {
		// get empty socket
		m_client_ch = emu->get_socket_channel();
		if (m_client_ch < 0) {
			return false;
		}
		// connect
		if (!emu->init_socket_tcp(m_client_ch, this)) {
			logging->out_log(LOG_ERROR, _T("Network socket initialize failed."));
			return false;
		}
		if (!emu->connect_socket(m_client_ch, pConfig->printer_server_host[m_cfg_num].Get(), pConfig->printer_server_port[m_cfg_num])) {
			logging->out_log(LOG_ERROR, _T("Cannot connect to printer server."));
			return false;
		}
		m_connect |= CONNECTABLE;
	}
	return true;
#else
	return false;
#endif
}
void PRINTER::disconnect_socket()
{
#ifdef USE_SOCKET
	if (!pConfig->printer_direct[m_cfg_num]) {
		// disconnect
		emu->disconnect_socket(m_client_ch);
	}
#endif
}
void PRINTER::network_connected(int ch)
{
	if (ch == m_client_ch) {
		m_connect |= CONNECTABLE;
	}
}
void PRINTER::network_disconnected(int ch)
{
	if (ch == m_client_ch) {
		m_connect = DISCONNECT;
		m_send_type = SEND_TYPE_FILEONLY;
		m_client_ch = -1;
	}
}
void PRINTER::network_writeable(int ch)
{
	if (ch == m_client_ch) {
		m_connect |= CONNWRITEABLE;
	}
}
void PRINTER::network_readable(int ch)
{
}

// ----------------------------------------------------------------------------
// event handler
// ----------------------------------------------------------------------------

void PRINTER::event_frame()
{
}

void PRINTER::event_callback(int event_id, int err)
{
//	logging->out_debugf("printer event send:%s",send ? "true" : "false");
	switch(event_id) {
		case SIG_PRINTER_CLOCK:
			write_signals(&outputs_busy, 0);	// idle 
			if (m_send_type == SEND_TYPE_DIRECT) {
				if (m_send_retry > 0) {
					// retry to send
#ifdef USE_SOCKET
					emu->send_data_tcp(m_client_ch);
#endif
					if (pConfig->printer_online[m_cfg_num]) {
						register_my_event(pConfig->printer_delay[m_cfg_num] * 1000.0, m_register_id);
					}
				} else {
#ifdef USE_PRINTER_PENDSIZE
					if (pend_size < PRINTER_PEND_SIZE)
#endif
					{
						// set data
						set_data(m_busdata);

						// send ACK signal
//						d_ctrl->write_signal(m_id_cb1,1,1);
//						d_ctrl->write_signal(m_id_cb1,0,1);

						cancel_my_event(m_register_id);
						m_send_type = SEND_TYPE_FILEONLY;
						m_strobe = false;
					}
				}
			} else {
				// set data
				set_data(m_busdata);

				// send ACK signal
//				d_ctrl->write_signal(m_id_cb1,1,1);
//				d_ctrl->write_signal(m_id_cb1,0,1);

				cancel_my_event(m_register_id);
				m_strobe = false;
			}
			break;
		case SIG_PRINTER_DISCONNECT:
			disconnect_socket();
			cancel_event(this, m_register_id_2);
			m_register_id_2 = -1;
			break;
	}
}

// ----------------------------------------------------------------------------
void PRINTER::save_state(FILEIO *fp)
{
	struct vm_state_st vm_state;

	//
	vm_state_ident.version = Uint16_LE(1);
	vm_state_ident.size = Uint32_LE(sizeof(vm_state_ident) + sizeof(vm_state));

	// copy values
	memset(&vm_state, 0, sizeof(vm_state));

	vm_state.m_register_id = Int32_LE(m_register_id);
	vm_state.m_register_id_2 = Int32_LE(m_register_id_2);

	fp->Fwrite(&vm_state_ident, sizeof(vm_state_ident), 1);
	fp->Fwrite(&vm_state, sizeof(vm_state), 1);
}

bool PRINTER::load_state(FILEIO *fp)
{
	vm_state_ident_t vm_state_i;
	struct vm_state_st vm_state;

	READ_STATE_CHUNK(fp, vm_state_i, vm_state);

	// copy values
	m_register_id = Int32_LE(vm_state.m_register_id);
	m_register_id_2 = Int32_LE(vm_state.m_register_id_2);

	return true;
}

// ----------------------------------------------------------------------------
#ifdef USE_DEBUGGER
uint32_t PRINTER::debug_read_io8(uint32_t addr)
{
	uint32_t data = 0xffff;
	switch(addr & 1) {
	case 0:
		// data
		data = m_busdata;
		break;
	case 1:
		// strobe
		data = (m_strobe ? 0 : 0xff);
		break;
	}
	return data;
}
bool PRINTER::debug_write_reg(const _TCHAR *reg, uint32_t data)
{
	return false;
}
bool PRINTER::debug_write_reg(uint32_t reg_num, uint32_t data)
{
	if (reg_num < 2) {
		write_io8(reg_num, data);
		return true;
	}
	return false;
}
void PRINTER::debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	buffer[0] = _T('\0');
	UTILITY::tcscat(buffer, buffer_len, _T("PRINTER:\n"));
	UTILITY::sntprintf(buffer, buffer_len, _T(" 00(DATA):%02X 01(STROBE):%02X"), m_busdata, m_strobe ? 0 : 0xff);
}
#endif
