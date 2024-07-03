/** @file midi.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2024.06.06 -

	@brief [ MCS modoki (YM3802) ]
*/

#ifndef MIDI_MCS_MODOKI_H
#define MIDI_MCS_MODOKI_H

#include "../vm_defs.h"
#include "../device.h"
#include "../../fifo.h"

class EMU;

/**
	@brief MCS modoki (YM3802) - MIDI Communication and Service Controller

	@note
	Following functions are not implemented:
	* FSK
	* SYNC detector / SYNC controller
	* MIDI clock interpolation
	* Break detector on receiver / Break sender on transmitter
	* Recording / playback counter
*/
class MIDI : public DEVICE
{
public:
	/// @brief signals on MIDI I/O port
	enum SIG_MIDI_IDS {
		SIG_TXD = 1,
		SIG_TXF,
		SIG_RXD,
		SIG_RXF,
		SIG_EXI,		// External input
		SIG_EXO,		// External output
	};

private:
	/// @brief event ids
	enum EVENT_IDS {
		EVENT_MIDI_TXD = 0,
		EVENT_MIDI_RXD,
		EVENT_MIDI_CLKM,
		EVENT_MIDI_CLKF,
		EVENT_MIDI_END,
	};
	enum en_midi_messages {
		MIDI_EXCLUSIVE_START = 0xf0,
		MIDI_EXCLUSIVE_STOP  = 0xf7,
		MIDI_MSG_CLOCK = 0xf8,
		MIDI_MSG_START = 0xfa,
		MIDI_MSG_STOP  = 0xfb,
		MIDI_MSG_CONT  = 0xfc,
		MIDI_MSG_SENSE = 0xfe,
		MIDI_MSG_RESET = 0xff
	};
	enum en_reg_nums {
		MCS_IVR = 0,   // Interrupt Vector (read only)
		MCS_RGR,       // Register Group / System Control
		MCS_ISR,       // Interrupt Service (read only)
		MCS_ICR,       // Interrupt Clear (write only)

		MCS_IOR,       // Interrupt Vector Offset Request
		MCS_IMR,       // Interrupt Mode Control
		MCS_IER,       // Interrupt Enable Request

		MCS_DMR = 14,  // Real Time Message Control
		MCS_DCR,       // Real Time Message Request
		MCS_DSR,       // FIFO IRx Data
		MCS_DNR,       // FIFO IRx Control

		MCS_RRR = 24,  // Rx Rate
		MCS_RMR,       // Rx Mode
		MCS_AMR,       // Address Hunter Maker
		MCS_ADR,       // Address Hunter Device

		MCS_RSR = 34,  // FIFO Rx Buffer Status
		MCS_RCR,       // FIFO Rx Buffer Control
		MCS_RDR,       // FIFO Rx Data

		MCS_TRR = 44,  // Tx Rate
		MCS_TMR,       // Tx Mode

		MCS_TSR = 54,  // FIFO Tx Status
		MCS_TCR,       // FIFO Tx Control
		MCS_TDR,       // FIFO Tx Data

		MCS_FSR = 64,  // FSK status
		MCS_FCR,       // FSK control
		MCS_CCR,       // Click Counter Control
		MCS_CDR,       // Click Counter Data (7-bit)

		MCS_SRR = 74,  // Recording Counter current value
		MCS_SCR,       // Sequencer Control
		MCS_SPRL,      // Playback Counter (low 8-bits)
		MCS_SPRH,      // Playback Counter (high 7-bits)

		MCS_GTRL = 84, // General Timer (low 8-bits)
		MCS_GTRH,      // General Timer (high 6-bits)
		MCS_MTRL,      // MIDI Clock Timer (low 8-bits)
		MCS_MTRH,      // MIDI Clock Timer (high 6-bits)

		MCS_EDR = 94,  // External I/O Direction
		MCS_EOR,       // External I/O Output Data
		MCS_EIR,       // External I/O Input Data

		MCS_END
	};
	enum en_reg_index {
		REG_NUL = 0,  // Dummy

		REG_IVR,      // Interrupt Vector (read only)
		REG_RGR,      // Register Group / System Control
		REG_ISR,      // Interrupt Service (read only)
		REG_ICR,      // Interrupt Clear (write only)

		REG_IOR,      // Interrupt Vector Offset Request
		REG_IMR,      // Interrupt Mode Control
		REG_IER,      // Interrupt Enable Request

		REG_DMR,      // Real Time Message Control
		REG_DCR,      // Real Time Message Request
//		REG_DSR,      // FIFO IRx Data
//		REG_DNR,      // FIFO IRx Control

		REG_RRR,      // Rx Rate
		REG_RMR,      // Rx Mode
		REG_AMR,      // Address Hunter Maker
		REG_ADR,      // Address Hunter Device

		REG_RSR,      // FIFO Rx Buffer Status
		REG_RCR,      // FIFO Rx Buffer Control
//		REG_RDR,      // FIFO Rx Data

		REG_TRR,      // Tx Rate
		REG_TMR,      // Tx Mode

		REG_TSR,      // FIFO Tx Status
		REG_TCR,      // FIFO Tx Control
//		REG_TDR,      // FIFO Tx Data

		REG_FSR,      // FSK status
		REG_FCR,      // FSK control
		REG_CCR,      // Click Counter Control
		REG_CDR,      // Click Counter Data (7-bit)

//		REG_SRR,      // Recording Counter current value
		REG_SCR,      // Sequencer Control
		REG_SPRL,     // Playback Counter (low 8-bits)
		REG_SPRH,     // Playback Counter (high 7-bits)

		REG_GTRL,     // General Timer (low 8-bits)
		REG_GTRH,     // General Timer (high 6-bits)
		REG_MTRL,     // MIDI Clock Timer (low 8-bits)
		REG_MTRH,     // MIDI Clock Timer (high 6-bits)

		REG_EDR,      // External I/O Direction
		REG_EOR,      // External I/O Output Data
		REG_EIR,      // External I/O Input Data

		REG_DUMMY1,	  // Dummy
		REGS_END
	};
	enum en_ivr_masks {
		IVR_VEC  = 0x1e,	// Vector (cause interrupt)
	};
	enum en_rgr_masks {
		RGR_GRP  = 0x0f,	// Group number
		RGR_IC   = 0x80,	// Internal Clear Request
	};
	enum en_isr_masks {	
		ISR_MM   = 0x01,	// MIDI real-time message detected
		ISR_CC   = 0x02,	// Click Counter / MIDI-clock detected
		ISR_PC   = 0x04,	// Play-back Counter
		ISR_RC   = 0x08,	// Recording Counter
		ISR_OL   = 0x10,	// Off-line detected / Break detected
		ISR_RX   = 0x20,	// FIFO-Rx Ready
		ISR_TX   = 0x40,    // FIFO-Tx Empty
		ISR_GT   = 0x80,	// General Timer
		ISR_MASK = 0xff
	};
	enum en_ior_masks {
		IOR_MASK = 0xe0		// Interrupt Vector
	};
	enum en_imr_masks {
		IMR_VM   = 0x01,	// Vector output timing
		IMR_VE   = 0x02,	// Enable vector output
		IMR_OB   = 0x04,	// Select IRQ-4(ISR bit4) source
		IMR_CT   = 0x08,	// Select IRQ-1(ISR bit1) source
		IMR_MASK = 0x0f
	};
	enum en_ier_masks {
		IER_MASK = 0xff
	};
	enum en_dmr_masks {
		DMR_MCFS = 0x03,	// Select MIDI-clock for FIFO-IRx
		DMR_MCFS_MESSAGE = 0x01,	// from MIDI message detector 
		DMR_MCFS_SYNC    = 0x02,	// from SYNC detector 
		DMR_MCFS_TIMER   = 0x03,	// from MIDI-clock timer 
		DMR_MCDS = 0x04,	// Select MIDI-clock distributer
		DMR_CDE  = 0x08,	// Enable MIDI-control $F9 - $FD detection
		DMR_MCE  = 0x10,	// Enable MIDI-control $F8 output
		DMR_ASE  = 0x20,	// Enable MIDI-control $FE output
		DMR_MASK = 0x3f
	};
	enum en_dcr_masks {
		DCR_MSG  = 0x07,	// Real Time Message $F8 - $FF
		DCR_RC   = 0x08,	// For Recording Counter
		DCR_PC   = 0x10,	// For Play-back Counter
		DCR_CC   = 0x20,	// For Click Counter
		DCR_SYNC = 0x40,	// For Sync Counter
		DCR_ITX  = 0x80,	// For FIFO-ITx
		DCR_MASK = 0xff
	};
	enum en_dnr_masks {
		DNR_CLK  = 0x01,	// FIFO-IRx increment closk
		DNR_MASK = 0x01
	};
	enum en_rrr_masks {
		RRR_CLK   = 0x10,	// Rx Rate Select CLK
		RRR_RM32  = 0x08,	// Rx Rate CLKM/32 if set
		RRR_RF32  = 0x08,	// Rx Rate CLKF/32 if not set
		RRR_RFSL  = 0x07,	// Rx Rate CLKF/64 - /8192
		RRR_D_F  = 0x20,	// Select RXD or RXF
		RRR_MASK = 0x3f
	};
	enum en_rmr_masks {
		RMR_ST   = 0x01,	// Select stop-bit type in 2bits length
		RMR_SL   = 0x02,	// Stop-bit length 0:1bit 1:2bits
		RMR_EO   = 0x04,	// Parity-bit 0:Even 1:Odd
		RMR_PL   = 0x08,	// Parity-bit length
		RMR_PE   = 0x10,	// Parity-bit enable
		RMR_CL   = 0x20,	// Data-bit length 0:8bits 1:7bits
		RMR_MASK = 0x3f
	};
	enum en_amr_masks {
		AMR_MAKER_ID = 0x7f,	// Maker-ID
		AMR_IDCL = 0x80,	// Check Device-ID
		AMR_MASK = 0xff
	};
	enum en_adr_masks {
		ADR_DEVICE_ID = 0x7f,	// Device-ID
		ADR_BDRE = 0x80,	// Enable broadcast
		ADR_MASK = 0xff
	};
	enum en_rsr_masks {
		RSR_BSY  = 0x01,	// FIFO-Rx Busy
		RSR_ABSY = 0x02,	// FIFO-Rx Address-hunter Busy
		RSR_OL   = 0x04,	// FIFO-Rx Off-line
		RSR_BRK  = 0x08,	// FIFO-Rx Break detected
		RSR_PE   = 0x10,	// FIFO-Rx Parity error
		RSR_FE   = 0x20,	// FIFO-Rx Flaming error
		RSR_OV   = 0x40,	// FIFO-Rx Overflow detected
		RSR_RDY  = 0x80,	// FIFO-RX Ready (data in buffer)
		RSR_MASK = 0xff
	};
	enum en_rcr_masks {
		RCR_ENA  = 0x01,	// FIFO-Rx Enable receiver
		RCR_AHE  = 0x02,	// FIFO-Rx Enable Address-hunter
		RCR_OLC  = 0x04,	// FIFO-Rx Clear Off-line Flag
		RCR_BRKC = 0x08,	// FIFO-Rx Clear Break	
		RCR_FLTE = 0x10,	// FIFO-Rx Enable MIDI-clock filter
		RCR_OVC  = 0x40,	// FIFO-Rx Clear Overflow Flag
		RCR_CLE  = 0x80,	// FIFO-Rx Clear Buffer
		RCR_MASK = 0xdf
	};
	enum en_trr_masks {
		TRR_CLK   = 0x10,	// Tx Rate Select CLK
		TRR_RM32  = 0x08,	// Tx Rate CLKM/32 if set
		TRR_RF32  = 0x08,	// Tx Rate CLKF/32 if not set
		TRR_RFSL  = 0x07,	// Tx Rate CLKF/64 - /8192
		TRR_D_F  = 0x20,	// Select TXD or TXF
		TRR_TXRX = 0x40,	// TxD Connection
		TRR_MASK = 0x7f
	};
	enum en_tmr_masks {
		TMR_ST   = 0x01,	// Select stop-bit type in 2bits length
		TMR_SL   = 0x02,	// Stop-bit length 0:1bit 1:2bits
		TMR_EO   = 0x04,	// Parity-bit 0:Even 1:Odd
		TMR_PL   = 0x08,	// Parity-bit length
		TMR_PE   = 0x10,	// Parity-bit enable
		TMR_CL   = 0x20,	// Data-bit length 0:8bits 1:7bits
		TMR_MASK = 0x3f
	};
	enum en_tsr_masks {
		TSR_BSY  = 0x01,	// FIFO-Tx Busy
		TSR_IDL  = 0x04,	// FIFO-Tx Idle
		TSR_RDY  = 0x40,	// FIFO-Tx Ready (enable write)
		TSR_EMP  = 0x80,	// FIFO-TX Empty (no data in buffer)
		TSR_MASK = 0xc5
	};
	enum en_tcr_masks {
		TCR_ENA  = 0x01,	// FIFO-Tx Enable Transfer
		TCR_IDLC = 0x04,	// FIFO-Tx Clear Idle Flag
		TCR_BRKE = 0x08,	// FIFO-Tx Send Break	
		TCR_CLE  = 0x80,	// FIFO-Tx Clear Buffer
		TCR_MASK = 0x8d
	};
	enum en_fsr_masks {
		FSR_PDF  = 0x01,	// FSK Polarity detected flag
		FSR_PS   = 0x02,	// FSK Polarity status
		FSR_CFF  = 0x10,	// FSK Carrier fast detected flag
		FSR_CSF  = 0x20,	// FSK Carrier slow detected flag
		FSR_SS   = 0x40,	// FSK Demodulated signal status
		FSR_RXF  = 0x80,	// FSK RxF pin status
		FSR_MASK = 0xf3
	};
	enum en_fcr_masks {
		FCR_PDFC = 0x01,	// FSK Clear polarity detected flag
		FCR_P_N  = 0x02,	// FSK Set polarity by manual
		FCR_APD  = 0x04,	// FSK Disable auto polarity
		FCR_DE   = 0x08,	// FSK Enable Demodulator
		FCR_CFC  = 0x10,	// FSK Clear carrier S/F detected flag
		FCR_ME   = 0x80,	// FSK Enable modulator
		FCR_MASK = 0x9f
	};
	enum en_ccr_masks {
		CCR_OUTE = 0x01,	// Enable CLICK output
		CCR_CLKM = 0x02,	// Select CLKM freq.
		CCR_MASK = 0x03
	};
	enum en_cdr_masks {
		CDR_DATA = 0x7f,	// Click Counter load value
		CDR_LD   = 0x80,	// Load immediately
	};
	enum en_scr_masks {
		SCR_RATE = 0x0f,	// Interpolation rate
		SCR_CLR  = 0x10,	// Clear play-back counter
		SCR_ADD  = 0x20,	// Add play-back counter
		SCR_MASK = 0x3f
	};
	enum en_spr_masks {
		SPRL_DATA = 0xff,
		SPRH_DATA = 0x7f
	};
	enum en_gtr_masks {
		GTRL_DATA = 0xff,
		GTRH_DATA = 0x3f,
		GTRH_LD   = 0x80,	// Load immediately	
	};
	enum en_mtr_masks {
		MTRL_DATA = 0xff,
		MTRH_DATA = 0x3f,
		MTRH_LD   = 0x80,	// Load immediately	
	};
	enum en_return_value {
//		INVALID_VALUE = 0x00,
		INVALID_VALUE = 0xff,	// HiZ
	};

private:
	uint8_t m_regs[REGS_END];
	int m_register_id[EVENT_MIDI_END];	///< for event

	FIFOBYTE m_tx_buffer;	///< FIFO-Tx Buffer (16bytes)
	FIFOBYTE m_itx_buffer;	///< FIFO-ITx Buffer (4bytes)

	double m_xmit_lamda;		///< transmit lamda (usec)
	double m_xmit_wait_period;	///< wait time to send next data

	FIFOWORD m_rx_buffer;	///< FIFO-Rx Buffer (128bytes)
	FIFOBYTE m_irx_buffer;	///< FIFO-IRx Buffer (4bytes)

	double m_recv_lamda;		///< receive lamda (usec)
	double m_recv_wait_period;	///< wait time to receive next data

	double  m_clkm_period;		///< clockm event (us)


	uint8_t  m_click_counter;		///< click counter
	uint8_t  m_record_counter;		///< recording counter
	 int16_t m_playback_counter;	///< play-back counter
	uint16_t m_general_counter;		///< general timer counter
	uint16_t m_midi_counter;		///< midi-clock counter

	uint8_t  m_click_counter_base;		///< click counter
	 int16_t m_playback_counter_base;	///< play-back counter
	uint16_t m_general_counter_base;	///< general timer counter reload value
	uint16_t m_midi_counter_base;		///< midi-clock counter

	uint8_t m_idle_counter;		///< transmission idle
	uint8_t m_offline_counter;	///< receiver off-line

	// for address hunter (status + maker ID + device ID if need)
	uint8_t m_ad_buffer[4];
	uint8_t m_ad_buffer_count;
	uint8_t m_ad_match_id;		///< match id (b0:make b1:device)

	enum {
		INTERPOLATE_BUFFER_MAX = 32
	};
	uint16_t m_interpolate_counter;	///< for interpolation
	uint16_t m_interpolate_deltas_idx;
	uint16_t m_interpolate_deltas_cnt;
	uint16_t m_interpolate_deltas[INTERPOLATE_BUFFER_MAX];

	// sequencer
	enum en_seq_states {
		SEQ_STATE_RECORD_WORKING = 0x08,
		SEQ_STATE_PLAY_WORKING   = 0x10,
		SEQ_STATE_CLICK_WORKING  = 0x20,
		SEQ_STATE_SYNC_WORKING   = 0x40,
	};
	uint8_t m_sequencer_state;

	outputs_t outputs_irq;
	outputs_t outputs_exo;		/// external output

	//for resume
#pragma pack(1)
	struct vm_state_st {
		uint8_t m_regs[REGS_END];
		char    reserved1[40 - REGS_END];

		int     m_register_id[EVENT_MIDI_END];
		int     reserved2[6 - EVENT_MIDI_END];

		uint8_t m_tx_buffer[16];
		int     m_tx_buffer_cnt;
		int     m_tx_buffer_wpt;
		int     m_tx_buffer_rpt;
		int     m_tx_buffer_siz;

		uint8_t m_itx_buffer[4];
		int     m_itx_buffer_cnt;
		int     m_itx_buffer_wpt;
		int     m_itx_buffer_rpt;

		uint64_t m_xmit_lamda;		// double
		uint64_t m_xmit_wait_period;// double

		uint16_t m_rx_buffer[128];
		int     m_rx_buffer_cnt;
		int     m_rx_buffer_wpt;
		int     m_rx_buffer_rpt;
		int     m_rx_buffer_siz;

		uint8_t m_irx_buffer[4];
		int     m_irx_buffer_cnt;
		int     m_irx_buffer_wpt;
		int     m_irx_buffer_rpt;

		uint64_t m_recv_lamda;		// double
		uint64_t m_recv_wait_period;// double

		uint64_t m_clkm_period;		// double
		uint8_t  m_click_counter;
		uint8_t  m_record_counter;
		 int16_t m_playback_counter;
		uint16_t m_general_counter;
		uint16_t m_midi_counter;

		uint8_t  m_click_counter_base;
		char     reserved3;
		 int16_t m_playback_counter_base;
		uint16_t m_general_counter_base;
		uint16_t m_midi_counter_base;
		uint8_t m_idle_counter;
		uint8_t m_offline_counter;
		uint8_t m_ad_buffer[4];
		uint8_t m_ad_buffer_count;
		uint8_t m_ad_match_id;

		uint16_t m_interpolate_deltas[16];

		uint16_t m_interpolate_counter;
		uint16_t m_interpolate_deltas_idx;
		uint16_t m_interpolate_deltas_cnt;
		short    reserved4;
		uint8_t m_sequencer_state;
		char    reserved5[7];
	};
#pragma pack()

	void calc_xmit_rate();
//	void empty_xmit();
	void start_xmit();
	void stop_xmit();
	void clear_xmit();
	void xmit();

	void push_to_fifo_tx(uint8_t data);
//	uint8_t pop_from_fifo_tx();

	void calc_recv_rate();
	void start_recv();
	void stop_recv();
	void clear_recv();
	void recv(uint16_t data);

	void push_to_fifo_rx(uint16_t data);

	inline void clear_irq(uint8_t val);
	inline void update_irq_vector();
	void update_irq();
	inline uint8_t read_irq_vector();
	void register_my_event(int id, double usec, bool loop);
	void cancel_my_event(int id);

	void calc_clkf_timer();
	void calc_clkm_timer();
	void count_clkf_timer();
	void count_clkm_timer();


	inline void countdown_midi_timer();

	inline void countdown_general_timer();


	// midi clock distributor

	void midi_dist_check();
	void midi_dist_send_midi_clock();
	void midi_dist_send_message(uint8_t msg);

	// interpolation

	inline void countup_interpolate_counter();
	void interpolator_recv_midi_clock();
	void interpolator_count_clock();

	// sequencer

	inline void countup_recording_counter();
	inline void process_recording_counter(uint8_t msg);

	inline void countdown_playback_counter();
	inline void process_playback_counter(uint8_t msg);

	inline void countdown_click_counter();
	inline void process_click_counter(uint8_t msg);

#ifdef USE_DEBUGGER
	static const uint8_t c_reg_map[];
	static const _TCHAR *c_reg_names[];
#endif

public:
	MIDI(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name("MIDI");
		init_output_signals(&outputs_irq);
		init_output_signals(&outputs_exo);
	}
	~MIDI() {}

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
	void set_context_external_output(DEVICE* device, int id, uint32_t mask) {
		register_output_signal(&outputs_exo, device, id, mask);
	}

	// event callback
	void event_frame();
	void event_callback(int event_id, int err);

	// for user interface

	void save_state(FILEIO *fp);
	bool load_state(FILEIO *fp);

#ifdef USE_DEBUGGER
	uint32_t debug_read_io8(uint32_t addr);
	uint32_t debug_read_reg(uint32_t reg_num);
	bool debug_write_reg(const _TCHAR *reg, uint32_t data);
	bool debug_write_reg(uint32_t reg_num, uint32_t data);
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
#endif
};

#endif /* MIDI_MCS_MODOKI_H */
