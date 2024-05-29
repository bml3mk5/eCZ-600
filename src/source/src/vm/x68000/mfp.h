/** @file mfp.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22 -

	@brief [ mfp modoki (mc68901) ]
*/

#ifndef MFP_MODOKI_H
#define MFP_MODOKI_H

#include "../vm_defs.h"
#include "../device.h"

class EMU;

/**
	@brief mfp modoki (mc68901) - MultiFunction Peripheral

	@note USART receive/send a parallel 8bits data.
	      Synchronous mode is not implemented.
*/
class MFP : public DEVICE
{
public:
	/// @brief signals on MFP I/O port
	enum SIG_MFP_IDS {
		SIG_GPIP = 1,
		SIG_TAI,
		SIG_TBI,
		SIG_SI,
		SIG_SO,
		SIG_TXC,
		SIG_IACK,
		SIG_RR,
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_MFP_TIMER_A	= 0,
		EVENT_MFP_TIMER_B,
		EVENT_MFP_TIMER_C,
		EVENT_MFP_TIMER_D
	};
	enum en_mfp_names {
		MFP_GPDR = 0,
		MFP_AER,
		MFP_DDR,
		MFP_IERA,
		MFP_IERB,
		MFP_IPRA,
		MFP_IPRB,
		MFP_ISRA,
		MFP_ISRB,
		MFP_IMRA,
		MFP_IMRB,
		MFP_VR,
		MFP_TACR,
		MFP_TBCR,
		MFP_TCDCR,
		MFP_TADR,
		MFP_TBDR,
		MFP_TCDR,
		MFP_TDDR,
		MFP_SCR,
		MFP_UCR,
		MFP_RSR,
		MFP_TSR,
		MFP_UDRI,	// buffer to receive
		MFP_UDRO,	// buffer to send
		MFP_REGS_END
	};
	enum en_intr_masks {
		IRA_GPIP7 = 0x80,
		IRA_GPIP6 = 0x40,
		IRB_GPIP5 = 0x80,
		IRB_GPIP4 = 0x40,
		IRB_GPIP3 = 0x08,
		IRB_GPIP2 = 0x04,
		IRB_GPIP1 = 0x02,
		IRB_GPIP0 = 0x01,
		IRA_TIMERA = 0x20,
		IRA_TIMERB = 0x01,
		IRB_TIMERC = 0x20,
		IRB_TIMERD = 0x10,
		IRA_RCVFUL = 0x10,
		IRA_RCVERR = 0x08,
		IRA_XMTEMP = 0x04,
		IRA_XMTERR = 0x02,
	};
	enum en_rsr_masks {
		RSR_BF = 0x80,
		RSR_OE = 0x40,
		RSR_PE = 0x20,
		RSR_FE = 0x10,
		RSR_FSB = 0x08,
		RSR_MCIP = 0x04,
		RSR_SS = 0x02,
		RSR_RE = 0x01,
	};
	enum en_tsr_masks {
		TSR_BE = 0x80,
		TSR_UE = 0x40,
		TSR_AT = 0x20,
		TSR_END = 0x10,
		TSR_B = 0x08,
		TSR_H = 0x04,
		TSR_L = 0x02,
		TSR_TE = 0x01,
	};
	enum en_vr_masks {
		VR_VEC = 0xf0,
		VR_S = 0x08,
	};

	/// priority on interrupt signal
	static const uint16_t c_intr_pri[16];
	/// prescaler on timer
	static const uint8_t c_prescaler[8];

private:
	uint8_t m_regs[MFP_REGS_END];
	uint8_t m_vector;	///< vector number on interrupt
//	bool m_now_iack;	///< receiving IACK

	int m_timer_clock;	///< Timer clock
	int m_timer_register_id[4];	///< for timer event
	uint8_t m_timer_counter[4];	///< count down timer
	double  m_timer_period[4];	///< timer event clock (us)
	uint8_t m_timer_output;		///< bit0 timer A, bit 1 timer B
	uint8_t m_timer_onoff;		///< timer on/off flag

	uint32_t m_timer_prev_input[2];	///< TAI and TBI diff 

	DEVICE *d_serial;		///< device to send data
//	int m_xmit_register_id;	///< interrupt after sent data
//	int m_xmit_clock;		///< clock for transmit
	int m_xmit_wait_count;	///< transmit wait
	uint32_t m_xmit_prev_edge; ///< transmit clock edge on/off

	uint8_t m_send_buf;	///< xmit buffer
	uint8_t m_recv_buf; ///< recv buffer
	enum en_buf_status {
		RX_BUF_FULL = 0x01,
		TX_BUF_FULL = 0x02,
	};
	uint8_t m_buf_status;

	outputs_t outputs_irq;
	outputs_t outputs_tmo[4];	// timer output

	int m_irq_register_id;	///< delay interrupt
	int m_irq_ipr;			///< interrupt priority

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_regs[MFP_REGS_END];
		uint8_t m_vector;	///< vector number on interrupt
		uint8_t reserved0;	// m_now_iack;	///< receiving IACK
		uint8_t m_timer_output;		///< bit0 timer A, bit 1 timer B
		uint8_t m_timer_counter[4];	///< count down timer

		uint64_t m_timer_period[4];	///< timer event clock (us)

		int m_timer_clock;		///< Timer clock
		int m_timer_register_id[4];	///< for timer event
		uint32_t m_timer_prev_input[2];	///< TAI and TBI diff 

		uint8_t m_send_buf;	///< xmit buffer
		uint8_t m_recv_buf; ///< recv buffer
		uint8_t m_buf_status;
		char reserved;

		int m_irq_register_id;	///< delay interrupt
		int m_irq_ipr;			///< interrupt priority
		int m_xmit_wait_count;	///< transmit wait
		uint32_t m_xmit_prev_edge; ///< transmit clock edge on/off

		uint32_t reserved2[4];
	};
#pragma pack()

	void empty_xmit();
	void start_xmit();
	void xmit();
	inline void receive_data(uint8_t data);
	inline void store_received_data();
	inline void set_full_receive_buffer();
	inline void clear_full_receive_buffer();
	inline void update_irq_for_rx(uint32_t maska);
//	void set_ca2(uint8_t val);
//	void set_cb2(uint8_t val);
	void update_irq();
	inline void assert_irq();
//	void set_irqb(bool val);
	void cancel_my_event(int &register_id);
//	void cancel_my_events();
	void count_down_timer_a_b(int n);
	void count_down_timer_c_d(int n);

public:
	MFP(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("MFP");
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_tmo[0]);
		init_output_signals(&outputs_tmo[1]);
		init_output_signals(&outputs_tmo[2]);
		init_output_signals(&outputs_tmo[3]);
		d_serial = NULL;
	}
	~MFP() {}

	// common functions
	void initialize();
	void reset();
	void warm_reset(bool por);

	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_external_data8(uint32_t addr);
	uint32_t read_io8(uint32_t addr);

	void write_signal(int id, uint32_t data, uint32_t mask);

	// unique function
	void set_context_irq(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_irq, device, id, mask);
	}
	void set_context_timer(int ch, DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_tmo[ch], device, id, mask);
	}
	void set_timer_clock(int clock) {
		m_timer_clock = clock;
	}
	void set_context_serial(DEVICE* device) {
		d_serial = device;
	}
//	void set_transmit_clock(int clock) {
//		m_xmit_clock = clock;
//	}

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

#endif /* MFP_MODOKI_H */
