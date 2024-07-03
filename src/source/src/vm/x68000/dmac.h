/** @file dmac.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2022.02.22-

	@brief [ HD63450 modoki (DMAC) ]
*/

#ifndef DMAC_H
#define DMAC_H

#include "../vm_defs.h"
//#include "../emu.h"
#include "../device.h"


#ifdef USE_DEBUGGER
class DEBUGGER;
class DEBUGGER_BUS;
#endif

/**
	@brief HD63450 modoki - DMA controller
*/
class DMAC : public DEVICE
{
public:
	enum en_signals {
		SIG_REQ_0 = 0,
		SIG_REQ_1,
		SIG_REQ_2,
		SIG_REQ_3,
		SIG_BG,
		SIG_IACK,
		SIG_BUSERR,
	};

private:
	enum en_csr_flags {
		CSR_COC = 0x80,	// operation complete
		CSR_BTC = 0x40,	// block transfer complete
		CSR_NDT = 0x20,	// normal device termination
		CSR_ERR = 0x10,	// error bit
		CSR_ACT = 0x08,	// channel active
		CSR_DIT = 0x04,	// /done input transition
		CSR_PCT = 0x02,	// /pcl transition
		CSR_PCS = 0x01,	// state of /pcl line
	};
	enum en_cer_codes {
		CER_NO_ERROR = 0x00,
		CER_CONFIGURATION = 0x01,
		CER_OPERATION_TIMING = 0x02,
		CER_ADDRERR = 0x04,
		CER_ADDRERR_IN_MAR = 0x05,
		CER_ADDRERR_IN_DAR = 0x06,
		CER_ADDRERR_IN_BAR = 0x07,
		CER_BUSERR = 0x08,
		CER_BUSERR_IN_MAR = 0x09,
		CER_BUSERR_IN_DAR = 0x0a,
		CER_BUSERR_IN_BAR = 0x0b,
		CER_COUNTERR = 0x0c,
		CER_COUNTERR_IN_MTC = 0x0d,
		CER_COUNTERR_IN_BTC = 0x0f,
		CER_EXTERNAL_ABORT = 0x10,
		CER_SOFTWARE_ABORT = 0x11,
		CER_MASK = 0x1f,
	};
	enum en_dcr_flags {
		DCR_XRM = 0xc0,	// external request mode
		DCR_XRM_UNDEF = 0x40,	// external request undefined mode
		DCR_DTYP = 0x30, // device type
		DCR_DTYP_SINGLE = 0x20, // device type single mode
		DCR_DPS = 0x08, // device port size 0:8bits 1:16bits
		DCR_PCL = 0x03, // peripheral control line
	};
	enum en_ocr_flags {
		OCR_DIR = 0x80,	// direction
		OCR_BTD = 0x40,	// multi block transfer
		OCR_SIZE = 0x30, // operand size
		OCR_SIZE_8BITS = 0x00, // operand size 8bits(byte)
		OCR_SIZE_16BITS = 0x10, // operand size 16bits(word)
		OCR_SIZE_32BITS = 0x20, // operand size 32bits(long word)
		OCR_SIZE_UP8BITS = 0x30, // operand size 8bits unpack
		OCR_CHAIN = 0x0c, // chaining
		OCR_CHAIN_UNDEF = 0x04, // chaining undefined
		OCR_ARRAY_CHAIN = 0x08, // array / link array chaining
		OCR_REQG = 0x03, // dma requset method
		OCR_REQG_EXT = 0x02, // dma requset method using REQ line
	};
	enum en_scr_flags {
		SCR_MAC = 0x0c,	// memory access count method
		SCR_MAC_UNDEF = 0x0c,	// memory access count method (undefined value)
		SCR_DAC = 0x03,	// device access count method
		SCR_DAC_UNDEF = 0x03,	// device access count method (undefined value)
	};
	enum en_ccr_flags {
		CCR_STR = 0x80,	// start operation
		CCR_CNT = 0x40,	// continue operation
		CCR_HLT = 0x20,	// halt operation
		CCR_SAB = 0x10,	// software abort
		CCR_OPE_ALL = 0xf0,	// all operation
		CCR_INT = 0x08,	// interrupt enable
	};
	enum en_gcr_flags {
		GCR_BR = 0x03,	// band width
		GCR_BT = 0x0c,	// burst time
		GCR_BT_SFT = 2,
	};
	enum en_reg_data_mask {
		CSR_MASK = 0xf6,
		CSR_INTMASK = 0xf2,
		GCR_MASK = 0x0f,
		DCR_MASK = 0xfb,
		SCR_MASK = 0x0f,
		CCR_TOGGLE_MASK = 0x68,
		CCR_OPERATE_MASK = 0x90,
		CPR_MASK = 0x03,
		FC_MASK = 0x07,
	};
	enum en_dmac_events {
		EVENT_TRANSFER_0 = 0,
		EVENT_TRANSFER_1,
		EVENT_TRANSFER_2,
		EVENT_TRANSFER_3,
		EVENT_PROCESSED_0,
		EVENT_PROCESSED_1,
		EVENT_PROCESSED_2,
		EVENT_PROCESSED_3,
		EVENT_REQUEST_0,
		EVENT_REQUEST_1,
		EVENT_REQUEST_2,
		EVENT_REQUEST_3,
		END_OF_EVENT_IDS,
	};

private:
	DEVICE* d_memory;
#ifdef USE_DEBUGGER
	DEBUGGER *d_debugger;
#endif

	struct st_dma_regs {
		DEVICE *device;
		uint8_t csr;
		uint8_t cer;
		uint8_t dcr;
		uint8_t ocr;
		uint8_t scr;
		uint8_t ccr;
		uint16_t mtc;
		uint16_t btc;
		uint32_t mar;
		uint32_t dar;
		uint32_t bar;
		uint8_t niv;
		uint8_t eiv;
		uint8_t cpr;
		uint8_t mfc;
		uint8_t dfc;
		uint8_t bfc;
		uint16_t pack_data;		// use packing mode
		uint16_t pack_width;	// bit flags in packing mode
		uint32_t next_pointer;	// next pointer on linked array chaining mode
		int		 next_clock;	// next request clock on limited rate speed mode
		outputs_t outputs_ack;
#ifdef USE_DEBUGGER
		DEBUGGER_BUS *d_dbg;
#endif
	} m_dma[4];
	uint8_t m_gcr;

//	struct st_dma_params {
//	} m_param[4];

	enum en_busreq_flags {
		BUSREQ0 = 0x01,
		BUSREQ_MASK = 0x0f,
		REQLINE0 = 0x10,
		REQLINE_MASK = 0xf0,
	};
	uint8_t m_busreq;	 ///< asserting bus request (bit3-0: each asserting channel, b7-b4:request signal)
	uint8_t m_interrupt; ///< asserting interrupt (bit3-0)
	uint8_t m_intr_mask; ///< interrupt mask INT on CCR (bit3-0)
//	bool m_now_iack;	 ///< receiving IACK
	uint8_t m_now_err;	///< receiving BUSERR
	enum en_err_flags {
		NOW_ERR_REASON	= 0xe0,
		NOW_ERR_PHASE	= 0x03,
		NOW_ERR_HALT	= 0x20,
		NOW_ERR_BUSERR	= 0x40,
		NOW_ERR_RETRY	= 0x60,
		NOW_ERR_ADDRERR = 0xc0,
		NOW_ERR_RESET	= 0xe0,
		NOW_ERR_IN_MAR	= 0x01,
		NOW_ERR_IN_DAR	= 0x02,
		NOW_ERR_IN_BAR	= 0x03,
	};

	int m_register_id[END_OF_EVENT_IDS];

	outputs_t outputs_busreq;
	outputs_t outputs_irq;

	//for resume
#pragma pack(1)
	struct vm_state_st {
		struct {
			// 0x00
			uint8_t csr;
			uint8_t cer;
			uint8_t dcr;
			uint8_t ocr;
			uint8_t scr;
			uint8_t ccr;
			uint8_t niv;
			uint8_t eiv;
			uint16_t mtc;
			uint16_t btc;
			uint32_t mar;
			// 0x10
			uint32_t dar;
			uint32_t bar;
			uint8_t cpr;
			uint8_t mfc;
			uint8_t dfc;
			uint8_t bfc;
			uint16_t pack_data;
			uint16_t pack_width;
			// 0x20
		} m_dma[4];

		struct {
			uint32_t next_pointer;	// next pointer on linked array chaining mode
			int		 next_clock;	// next request clock on limited rate speed mode
		} m_param[4];

		uint8_t m_gcr;
		uint8_t m_busreq;	 ///< asserting bus request (bit3-0: each asserting channel, b7-b4:request signal)
		uint8_t m_interrupt; ///< asserting interrupt 
		uint8_t reserved0;	// m_now_iack;	 ///< receiving IACK

		int m_register_id[15];
	};
#pragma pack()

	uint32_t read_via_debugger_data16(struct st_dma_regs *dma, uint32_t addr);
	void write_via_debugger_data_n(struct st_dma_regs *dma, uint32_t addr, uint32_t data, int width);
	uint32_t read_via_debugger_data_n(struct st_dma_regs *dma, uint32_t addr, int width);
	void write_via_debugger_io8(struct st_dma_regs *dma, uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_io8(struct st_dma_regs *dma, uint32_t addr);
	void write_via_debugger_io16(struct st_dma_regs *dma, uint32_t addr, uint32_t data);
	uint32_t read_via_debugger_io16(struct st_dma_regs *dma, uint32_t addr);

	void cancel_my_event(int id);
	void register_transfer_event(int channel, int clock);
	void register_processed_event(int channel, int clock);
	void register_request_event(int channel, int clock);
	void update_priority_channel();
	void request(int channel, bool onoff);
	void busack(int channel, bool onoff);
	inline bool prestart_transfer(struct st_dma_regs *dma, int channel);
	void start_transfer(int channel);
	void transfer(int channel);
	void finish_transfer(struct st_dma_regs *dma, int channel);
	inline void continue_transfer(struct st_dma_regs *dma, int channel, int clock);
	inline void next_transfer(struct st_dma_regs *dma, int channel, int clock);
	void abort_transfer(int channel);
	void error_transfer(int channel, uint8_t reason);
	void processed(int channel);
	void update_irq(int channel, bool onoff);
	void update_irq_mask(int channel, bool mask_onoff);
	inline void update_breq(int channel, bool onoff);
	inline void update_reqline(int channel, bool onoff);

public:
	DMAC(VM* parent_vm, EMU* parent_emu, const char* identifier) : DEVICE(parent_vm, parent_emu, identifier) {
		set_class_name(_T("DMAC"));

		for(int i = 0; i < 4; i++) {
			m_dma[i].device = NULL;
			init_output_signals(&m_dma[i].outputs_ack);
		}
		d_memory = NULL;
#ifdef USE_DEBUGGER
		d_debugger = NULL;
		for(int i = 0; i < 4; i++) {
			m_dma[i].d_dbg = NULL;
		}
#endif
		init_output_signals(&outputs_busreq);
		init_output_signals(&outputs_irq);
	}
	~DMAC() {}

	// common functions
	void initialize();
	void release();
	void reset();
	void warm_reset(bool por);
	void write_io8(uint32_t addr, uint32_t data);
	void write_io16(uint32_t addr, uint32_t data);
	void write_io_n(uint32_t addr, uint32_t data, int width);
	uint32_t read_external_data8(uint32_t addr);
	uint32_t read_io8(uint32_t addr);
	uint32_t read_io16(uint32_t addr);
	uint32_t read_io_n(uint32_t addr, int width);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);

	void save_state(FILEIO *fio);
	bool load_state(FILEIO *fio);

	// unique functions
	void set_context_memory(DEVICE* device)
	{
		d_memory = device;
	}
	void set_context_device(int channel, DEVICE* device)
	{
		m_dma[channel].device = device;
	}
	void set_context_busreq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_busreq, device, id, mask);
	}
	void set_context_ack(int channel, DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&m_dma[channel].outputs_ack, device, id, mask);
	}
	void set_context_irq(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs_irq, device, id, mask);
	}

#ifdef USE_DEBUGGER
	void debug_regs_info(_TCHAR *buffer, size_t buffer_len);
	void set_context_debugger(DEBUGGER* device)
	{
		d_debugger = device;
	}
	DEVICE *get_debugger()
	{
		return (DEVICE *)d_debugger;
	}
#endif
};

#endif /* DMAC_H */

