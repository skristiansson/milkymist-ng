from migen.fhdl.structure import *
from migen.fhdl.module import Module
from migen.genlib.fifo import AsyncFIFO
from migen.genlib.record import layout_len
from migen.bank.description import AutoCSR
from migen.actorlib import structuring, dma_asmi, spi

from milkymist.dvisampler.edid import EDID
from milkymist.dvisampler.clocking import Clocking
from milkymist.dvisampler.datacapture import DataCapture

class RawDVISampler(Module, AutoCSR):
	def __init__(self, pads, asmiport):
		self.submodules.edid = EDID(pads)
		self.submodules.clocking = Clocking(pads)

		invert = False
		try:
			s = getattr(pads, "data0")
		except AttributeError:
			s = getattr(pads, "data0_n")
			invert = True
		self.submodules.data0_cap = DataCapture(8, invert)
		self.comb += [
			self.data0_cap.pad.eq(s),
			self.data0_cap.serdesstrobe.eq(self.clocking.serdesstrobe)
		]

		fifo = AsyncFIFO(10, 256)
		self.add_submodule(fifo, {"write": "pix", "read": "sys"})
		self.comb += [
			fifo.din.eq(self.data0_cap.d),
			fifo.we.eq(1)
		]

		pack_factor = asmiport.hub.dw//16
		self.submodules.packer = structuring.Pack([("word", 10), ("pad", 6)], pack_factor)
		self.submodules.cast = structuring.Cast(self.packer.source.payload.layout, asmiport.hub.dw)
		self.submodules.dma = spi.DMAWriteController(dma_asmi.Writer(asmiport), spi.MODE_SINGLE_SHOT, free_flow=True)
		self.comb += [
			self.packer.sink.stb.eq(fifo.readable),
			fifo.re.eq(self.packer.sink.ack),
			self.packer.sink.payload.word.eq(fifo.dout),
			self.packer.source.connect(self.cast.sink, match_by_position=True),
			self.cast.source.connect(self.dma.data, match_by_position=True)
		]
