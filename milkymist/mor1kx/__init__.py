from migen.fhdl.structure import *
from migen.fhdl.specials import Instance
from migen.fhdl.module import Module
from migen.bus import wishbone

class MOR1KX(Module):
	def __init__(self):
		self.ibus = i = wishbone.Interface()
		self.dbus = d = wishbone.Interface()
		self.interrupt = Signal(32)
		self.ext_break = Signal()

		###
		cpu = "CAPPUCCINO"
		reset_pc = 0x00860000

		i_adr_o = Signal(32)
		d_adr_o = Signal(32)
		self.specials += Instance("mor1kx",
			Instance.Parameter("FEATURE_INSTRUCTIONCACHE",
					   "ENABLED"),
			Instance.Parameter("OPTION_ICACHE_BLOCK_WIDTH", 4),
			Instance.Parameter("OPTION_ICACHE_SET_WIDTH", 8),
			Instance.Parameter("OPTION_ICACHE_WAYS", 1),
			Instance.Parameter("OPTION_ICACHE_LIMIT_WIDTH", 31),
			Instance.Parameter("FEATURE_DATACACHE", "ENABLED"),
			Instance.Parameter("OPTION_DCACHE_BLOCK_WIDTH", 4),
			Instance.Parameter("OPTION_DCACHE_SET_WIDTH", 8),
			Instance.Parameter("OPTION_DCACHE_WAYS", 1),
			Instance.Parameter("OPTION_DCACHE_LIMIT_WIDTH", 31),
			Instance.Parameter("FEATURE_TIMER", "NONE"),
			Instance.Parameter("OPTION_PIC_TRIGGER", "LEVEL"),
			Instance.Parameter("FEATURE_SYSCALL", "NONE"),
			Instance.Parameter("FEATURE_TRAP", "NONE"),
			Instance.Parameter("FEATURE_RANGE", "NONE"),
			Instance.Parameter("FEATURE_OVERFLOW", "NONE"),
			Instance.Parameter("FEATURE_ADDC", "NONE"),
			Instance.Parameter("FEATURE_CMOV", "NONE"),
			Instance.Parameter("FEATURE_FFL1", "NONE"),
			Instance.Parameter("OPTION_CPU0", cpu),
			Instance.Parameter("OPTION_RESET_PC", reset_pc),
			Instance.Parameter("IBUS_WB_TYPE", "B3_REGISTERED_FEEDBACK"),
			Instance.Parameter("DBUS_WB_TYPE", "B3_REGISTERED_FEEDBACK"),

			Instance.Input("clk", ClockSignal()),
			Instance.Input("rst", ResetSignal()),

			Instance.Input("irq_i", self.interrupt),

			Instance.Output("iwbm_adr_o", i_adr_o),
			Instance.Output("iwbm_dat_o", i.dat_w),
			Instance.Output("iwbm_sel_o", i.sel),
			Instance.Output("iwbm_cyc_o", i.cyc),
			Instance.Output("iwbm_stb_o", i.stb),
			Instance.Output("iwbm_we_o", i.we),
			Instance.Output("iwbm_cti_o", i.cti),
			Instance.Output("iwbm_bte_o", i.bte),
			Instance.Input("iwbm_dat_i", i.dat_r),
			Instance.Input("iwbm_ack_i", i.ack),
			Instance.Input("iwbm_err_i", i.err),
			Instance.Input("iwbm_rty_i", 0),

			Instance.Output("dwbm_adr_o", d_adr_o),
			Instance.Output("dwbm_dat_o", d.dat_w),
			Instance.Output("dwbm_sel_o", d.sel),
			Instance.Output("dwbm_cyc_o", d.cyc),
			Instance.Output("dwbm_stb_o", d.stb),
			Instance.Output("dwbm_we_o", d.we),
			Instance.Output("dwbm_cti_o", d.cti),
			Instance.Output("dwbm_bte_o", d.bte),
			Instance.Input("dwbm_dat_i", d.dat_r),
			Instance.Input("dwbm_ack_i", d.ack),
			Instance.Input("dwbm_err_i", d.err),
			Instance.Input("dwbm_rty_i", 0))

		self.comb += [
			self.ibus.adr.eq(i_adr_o[2:]),
			self.dbus.adr.eq(d_adr_o[2:])
		]
