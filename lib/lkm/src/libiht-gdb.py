import os
import gdb
import ctypes

# quote the ctypes

class Clbr_stack_entry(ctypes.Structure):
    _fields_ = [
        ('from_', ctypes.c_ulonglong),
        ('to', ctypes.c_ulonglong)
    ]
    def __init__(self, from_, to):
        self.from_ = from_
        self.to = to

class Clbr_config(ctypes.Structure):
    _fields_ = [
        ('pid', ctypes.c_uint),
        ('lbr_select', ctypes.c_ulonglong)
    ]
    def __init__(self, pid, lbr_select):
        self.pid = pid
        self.lbr_select = lbr_select

class Clbr_data(ctypes.Structure):
    _fields_ = [
        ('lbr_tos', ctypes.c_ulonglong),
        ('entries', ctypes.POINTER(Clbr_stack_entry))
    ]
    def __init__(self, lbr_tos, entries):
        self.lbr_tos = lbr_tos
        self.entries = entries

class Clbr_ioctl_request(ctypes.Structure):
    _fields_ = [
        ('lbr_config', Clbr_config),
        ('buffer', ctypes.POINTER(Clbr_data))
    ]
    def __init__(self, lbr_config, buffer):
        self.lbr_config = lbr_config
        self.buffer = buffer

class Cbts_config(ctypes.Structure):
    _fields_ = [
        ('pid', ctypes.c_uint),
        ('bts_config', ctypes.c_ulonglong),
        ('bts_buffer_size', ctypes.c_ulonglong)
    ]
    def __init__(self, pid, bts_config, bts_buffer_size):
        self.pid = pid
        self.bts_config = bts_config
        self.bts_buffer_size = bts_buffer_size

class Cbts_record(ctypes.Structure):
    _fields_ = [
        ('from_', ctypes.c_ulonglong),
        ('to', ctypes.c_ulonglong),
        ('misc', ctypes.c_ulonglong)
    ]
    def __init__(self, from_, to, misc):
        self.from_ = from_
        self.to = to
        self.misc = misc

class Cbts_data(ctypes.Structure):
    _fields_ = [
        ('bts_buffer_base', ctypes.POINTER(Cbts_record)),
        ('bts_index', ctypes.POINTER(Cbts_record)),
        ('bts_interrupt_threshold', ctypes.c_ulonglong)
    ]
    def __init__(self, bts_buffer_size, bts_index, bts_interrupt_threshold):
        self.bts_buffer_size = bts_buffer_size
        self.bts_index = bts_index
        self.bts_interrupt_threshold = bts_interrupt_threshold

class Cbts_ioctl_request(ctypes.Structure):
    _fields_ = [
        ('bts_config', Cbts_config),
        ('buffer', ctypes.POINTER(Cbts_data))
    ]
    def __init__(self, bts_config, buffer):
        self.bts_config = bts_config
        self.buffer = buffer

# declare the return value
class LBRContent:
    def __init__(self, from_, to):
        self.from_ = from_
        self.to = to

class BTSContent:
    def __init__(self, from_, to, misc):
        self.from_ = from_
        self.to = to
        self.misc = misc
# import the dynamic link

script_dir = os.path.dirname(os.path.abspath(__file__))
lib_path = os.path.join(script_dir, "lib_api.so")
my_lib = ctypes.CDLL(lib_path)

# get the funcions and define the arguments

enable_lbr = my_lib.enable_lbr
disable_lbr = my_lib.disable_lbr
dump_lbr = my_lib.dump_lbr
select_lbr = my_lib.select_lbr

enable_lbr.restype = Clbr_ioctl_request
enable_lbr.argtypes = [ctypes.c_uint]
disable_lbr.argtypes = [Clbr_ioctl_request]
dump_lbr.argtypes = [Clbr_ioctl_request]
select_lbr.argtypes = [Clbr_ioctl_request]

enable_bts = my_lib.enable_bts
disable_bts = my_lib.disable_bts
dump_bts = my_lib.dump_bts
config_bts = my_lib.config_bts

enable_bts.restype = Cbts_ioctl_request
enable_bts.argtypes = [ctypes.c_uint]
disable_bts.argtypes = [Cbts_ioctl_request]
dump_bts.argtypes = [Cbts_ioctl_request]
config_bts.argtypes = [Cbts_ioctl_request]

lbr_req = None
lbr_enable = False

bts_req = None
bts_enable = False

def get_function_name(address):
    symbol_output = gdb.execute("info symbol " + str(address), to_string=True)
    lines = symbol_output.splitlines()
    if len(lines) > 1:
        function_name = lines[1]
        function_name = function_name.split(' ', 1)[1]
        return function_name
    return address

def get_gdb_pid():
    inferior = gdb.selected_inferior()
    if inferior is not None:
        return inferior.pid
    return None

class EnableLBR(gdb.Command):
    def __init__(self):
        super(EnableLBR, self).__init__("enable_lbr", gdb.COMMAND_USER)

    def invoke(self, args, from_tty):
        global lbr_req
        process_pid = get_gdb_pid()
        lbr_req = enable_lbr(process_pid)
        lbr_enable = True
        print("LIBIHT-GDB: enable lbr for pid :", lbr_req.lbr_config.pid)

class DisableLBR(gdb.Command):
    def __init__(self):
        super(DisableLBR, self).__init__("disable_lbr", gdb.COMMAND_USER)

    def invoke(self, args, from_tty):
        global lbr_req
        disable_lbr(lbr_req)
        lbr_enable = False
        print("LIBIHT-GDB: disable lbr for pid :", lbr_req.lbr_config.pid)

class DumpLBR(gdb.Command):
    def __init__(self):
        super(DumpLBR, self).__init__("dump_lbr", gdb.COMMAND_USER)
    
    def invoke(self, args, from_tty):
        global lbr_req
        print("LIBIHT-GDB: dump lbr for pid :", lbr_req.lbr_config.pid)
        dump_lbr(lbr_req)
        lbr_tos = lbr_req.buffer.contents.lbr_tos
        data_pointer = ctypes.cast(lbr_req.buffer.contents.entries, ctypes.POINTER(Clbr_stack_entry))
        lbr_content = []
        for i in range(0, lbr_tos):
            lbr_content.append(LBRContent(data_pointer[i].from_, data_pointer[i].to))
        print(lbr_tos)
        for i in range(0, lbr_tos):
            print("MSR_LBR_NHM_FROM[", i, "]: ", get_function_name(hex(lbr_content[i].from_)))
            print("MSR_LBR_NHM_TO  [", i, "]: ", get_function_name(hex(lbr_content[i].to)))
        return lbr_content

class EnableBTS(gdb.Command):
    def __init__(self):
        super(EnableBTS, self).__init__("enable_bts", gdb.COMMAND_USER)
    
    def invoke(self, args, from_tty):
        global bts_req
        process_pid = get_gdb_pid()
        bts_req = enable_bts(process_pid)
        bts_enable = True
        print("LIBIHT-GDB: enable bts for pid :", bts_req.bts_config.pid)

class DisableBTS(gdb.Command):
    def __init__(self):
        super(DisableBTS, self).__init__("disable_bts", gdb.COMMAND_USER)
    
    def invoke(self, args, from_tty):
        global bts_req
        disable_bts(bts_req)
        bts_enable = False
        print("LIBIHT-GDB: disable bts for pid :", bts_req.bts_config.pid)

class DumpBTS(gdb.Command):
    def __init__(self):
        super(DumpBTS, self).__init__("dump_bts", gdb.COMMAND_USER)
    
    def invoke(self, args, from_tty):
        global bts_req
        dump_bts(bts_req)
        bts_tos = 32
        data_pointer = ctypes.cast(bts_req.buffer.contents.bts_buffer_base, ctypes.POINTER(Cbts_record))
        bts_content = []
        for i in range (0, bts_tos):
            bts_content.append(BTSContent(data_pointer[i].from_, data_pointer[i].to, data_pointer[i].misc))
        for i in range (0, bts_tos):
            print("LIBIHT-GDB: BTS record ", i,": from ", hex(bts_content[i].from_), " to ", hex(bts_content[i].to), ".");

EnableLBR()
DisableLBR()
DumpLBR()

EnableBTS()
DisableBTS()
DumpBTS()