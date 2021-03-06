from operator import itemgetter
import re

def get_macros(filename):
	f = open(filename, "r")
	r = {}
	for line in f:
		match = re.match("\w*#define\s+(\w+)\s+(.*)", line, re.IGNORECASE)
		if match:
			r[match.group(1)] = match.group(2)
	return r

def _get_rw_functions(reg_name, reg_base, size):
	r = ""
	if size > 8:
		raise NotImplementedError("Register too large")
	elif size > 4:
		ctype = "unsigned long long int"
	elif size > 2:
		ctype = "unsigned int"
	elif size > 1:
		ctype = "unsigned short int"
	else:
		ctype = "unsigned char"

	r += "static inline "+ctype+" "+reg_name+"_read(void) {\n"
	if size > 1:
		r += "\t"+ctype+" r = MMPTR("+hex(reg_base)+");\n"
		for byte in range(1, size):
			r += "\tr <<= 8;\n\tr |= MMPTR("+hex(reg_base+4*byte)+");\n"
		r += "\treturn r;\n}\n"
	else:
		r += "\treturn MMPTR("+hex(reg_base)+");\n}\n"

	r += "static inline void "+reg_name+"_write("+ctype+" value) {\n"
	for byte in range(size):
		shift = (size-byte-1)*8
		if shift:
			value_shifted = "value >> "+str(shift)
		else:
			value_shifted = "value"
		r += "\tMMPTR("+hex(reg_base+4*byte)+") = "+value_shifted+";\n"
	r += "}\n"
	return r

def get_csr_header(csr_base, bank_array, interrupt_map):
	r = "#ifndef __HW_CSR_H\n#define __HW_CSR_H\n#include <hw/common.h>\n"
	for name, csrs, mapaddr, rmap in bank_array.banks:
		r += "\n/* "+name+" */\n"
		reg_base = csr_base + 0x800*mapaddr
		r += "#define "+name.upper()+"_BASE "+hex(reg_base)+"\n"
		for csr in csrs:
			nr = (csr.size + 7)//8
			r += _get_rw_functions(name + "_" + csr.name, reg_base, nr)
			reg_base += 4*nr
		try:
			interrupt_nr = interrupt_map[name]
		except KeyError:
			pass
		else:
			r += "#define "+name.upper()+"_INTERRUPT "+str(interrupt_nr)+"\n"
	r += "\n#endif\n"
	return r
