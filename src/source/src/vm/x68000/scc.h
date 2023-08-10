/** @file scc.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ scc modoki (Z8530) ]
*/

#ifndef SCC_MODOKI_H
#define SCC_MODOKI_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief scc modoki (Z8530) - Serial Communication Controller

	@note A data receive/send using parallel 8bits.

*/
class SCC : public DEVICE
{
public:
	/// @brief signals on SCC I/O port
	enum SIG_SCC_IDS {
		SIG_TXDA = 1,
		SIG_TXDB,
		SIG_RXDA,
		SIG_RXDB,
		SIG_IACK,
		SIG_RTSA,
		SIG_RTSB,
		SIG_DTRA,
		SIG_DTRB,
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_SCC_TXDA = 0,
		EVENT_SCC_TXDB,
		EVENT_SCC_END
	};
	enum en_scc_names {
		SCC_WR0 = 0,
		SCC_WR1,
		SCC_WR2,	// A only
		SCC_WR3,
		SCC_WR4,
		SCC_WR5,
		SCC_WR6,
		SCC_WR7,
		SCC_WR8,
		SCC_WR9,	// A only
		SCC_WR10,
		SCC_WR11,
		SCC_WR12,
		SCC_WR13,
		SCC_WR14,
		SCC_WR15,
		SCC_RR0,
		SCC_RR1,
		SCC_RR2,
		SCC_RR3,	// A only
		SCC_RR8,
		SCC_RR10,
		SCC_REGS_END,

		SCC_IUS = SCC_RR3
	};
	enum en_scc_channels {
		SCC_CH_COMMON = 0,
		SCC_CH_HIDDEN,
		SCC_CH_A = 0,
		SCC_CH_B,
	};
	enum en_wr1_masks {
		WR1_EX_IE = 0x01,
		WR1_TX_IE = 0x02,
		WR1_PE_IE = 0x04,
		WR1_RX_IE = 0x18,
		WR1_RX_IE_SP  = 0x18,
		WR1_RX_IE_ALL = 0x10,
		WR1_RX_IE_AC  = 0x08,
		WR1_REQ_DIR = 0x20,
		WR1_REQ_DMA = 0x40,
		WR1_REQ_EN  = 0x80
	};
	enum en_wr0_masks {
		WR0_REG_SEL = 0x0f,
		WR0_CLEAR   = 0x38,
		WR0_CLEAR_U = 0x30,
	};
	enum en_wr3_masks {
		WR3_RX_CHAR_BITS = 0xc0,
		WR3_RX_EN = 0x01,
	};
	enum en_wr4_masks {
		WR4_PARITY_EN = 0x01,
		WR4_STOP_BITS = 0x0c,
		WR4_CLK_MODE = 0xc0,
	};
	enum en_wr5_masks {
		WR5_RTS = 0x02,
		WR5_TX_EN = 0x08,
		WR5_TX_SEND_BREAK = 0x10,
		WR5_TX_CHAR_BITS = 0x60,
		WR5_DTR = 0x80,
	};
	enum en_wr9_masks {
		WR9_VIS = 0x01,
		WR9_NV  = 0x02,
		WR9_DLC = 0x04,
		WR9_MIE = 0x08,
		WR9_STS = 0x10,
	};
	enum en_wr11_masks {
		WR11_RX_CLK = 0x60,
		WR11_RX_CLK_BAUDG = 0x40,
		WR11_TX_CLK = 0x18,
		WR11_TX_CLK_BAUDG = 0x10,
	};
	enum en_rr0_masks {
		RR0_RXC_AVAIL = 0x01,
		RR0_ZERO_CNT  = 0x02,
		RR0_TXB_EMPTY = 0x04,
		RR0_DCD       = 0x08,
		RR0_SYNC_HUNT = 0x10,
		RR0_CTS       = 0x20,
		RR0_TX_UN_EOM = 0x40,
		RR0_BREAK     = 0x80
	};
	enum en_rr1_masks {
		RR1_ALL_SENT  = 0x01,
		RR1_RES_CODE  = 0x0e,
		RR1_PARITYERR = 0x10,
		RR1_RX_OVRERR = 0x20,
		RR1_CRC_FRERR = 0x40,
		RR1_EOF       = 0x80,
		RR1_RX_ERRALL = 0xf0
	};
	enum en_rr3_masks {
		RR3_CB_EXT_IP = 0x01,
		RR3_CB_TX_IP  = 0x02,
		RR3_CB_RX_IP  = 0x04,
		RR3_CA_EXT_IP = 0x08,
		RR3_CA_TX_IP  = 0x10,
		RR3_CA_RX_IP  = 0x20
	};
	enum en_rr10_masks {
		RR10_ONLOOP   = 0x02,
		RR10_LOOPSEND = 0x10,
		RR10_TC_MISS  = 0x40,
		RR10_OC_MISS  = 0x80
	};

private:
	DEVICE *d_devices[2];	///< devices

	int m_base_clock;		///< base clock

	uint8_t m_regs[2][SCC_REGS_END];
	int m_cur_cmd;		///< set command number to WR0

	uint8_t m_vector;	///< interrupt vector
	bool m_now_iack;	///< receiving IACK

	int m_register_id[EVENT_SCC_END];	///< interrupt after sent data

	enum en_stat_flags {
		FIRST_RECEIVED = 0x01,
	};
	enum en_buff_sizes {
		RECV_BUFF_SIZE = 3,
		XMIT_BUFF_SIZE = 1,
	};
	struct {
		uint8_t recv_buff[RECV_BUFF_SIZE + 1];	///< receive buffer
		uint8_t recv_wpos;
		uint8_t xmit_buff[XMIT_BUFF_SIZE + 1];	///< transmit buffer
		uint8_t xmit_wpos;
		uint8_t flags;
	} m_stat[2];

	outputs_t outputs_irq;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_regs[2][SCC_REGS_END];
		int m_cur_cmd;		///< set command number to WR0

		int m_base_clock;	///< base clock
		uint8_t m_vector;	///< interrupt vector
		uint8_t m_now_iack;	///< receiving IACK
		char reserved[2];
		union {
			struct {
				struct {
					uint8_t first_received;	///< first received data
					char reserved[3];
				} m_stat[2];

				int m_register_id[EVENT_SCC_END];	///< interrupt after sent data
				uint32_t reserved2[2];
			} v1;
			struct {
				struct {
					uint8_t recv_buff[RECV_BUFF_SIZE + 1];	///< receive buffer
					uint8_t reserved1[6 - RECV_BUFF_SIZE];
					uint8_t recv_wpos;
					uint8_t xmit_buff[XMIT_BUFF_SIZE + 1];	///< transmit buffer
					uint8_t reserved2[5 - XMIT_BUFF_SIZE];
					uint8_t xmit_wpos;
					uint8_t flags;
				} m_stat[2];

				int m_register_id[EVENT_SCC_END];	///< interrupt after sent data
			} v2;
		};
	};
#pragma pack()

	void process_wr0_cmd(int ch, uint8_t cmd);
	void start_xmit(int ch);
	inline void write_to_xmit_data(int ch, uint8_t data);
	inline void load_xmit_data_from_reg(int ch);

	inline void receive_data(int ch, uint32_t data, uint32_t mask);
	inline void store_received_data_to_reg(int ch);
	inline uint8_t read_received_data(int ch);

	inline void push_recv_buff(int ch, uint8_t data);
	inline void push_xmit_buff(int ch, uint8_t data);
	inline uint8_t shift_recv_buff(int ch);
	inline uint8_t shift_xmit_buff(int ch);

	inline void update_intr_flags_for_rx(int ch);
	inline void update_intr_flags_for_tx(int ch);

	void update_intr_for_tx(int ch);
	void assert_intr();
	void update_intr();
	void update_intr_vector();

	void register_my_event(int id, double usec);
	void cancel_my_event(int id);

	static const uint8_t c_intr_causes[];

public:
	SCC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("SCC");
		init_output_signals(&outputs_irq);
		d_devices[SCC_CH_A] = NULL;
		d_devices[SCC_CH_B] = NULL;
	}
	~SCC() {}

	// common functions
	void initialize();
	void reset();
	void warm_reset(bool por);

	void channel_reset(int ch);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);

	double get_send_speed_usec(int ch, bool is_byte);
	double get_recv_speed_usec(int ch, bool is_byte);

	// unique function
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_channel_a(DEVICE* device) {
		d_devices[SCC_CH_A] = device; 
	}
	void set_context_channel_b(DEVICE* device) {
		d_devices[SCC_CH_B] = device; 
	}
	void set_context_clock(int clock) {
		m_base_clock = clock;
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* SCC_MODOKI_H */
