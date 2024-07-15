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

# declare the return value
class LBRContent:
    def __init__(self, from_, to):
        self.from_ = from_
        self.to = to

# import the dynamic link

script_dir = os.path.dirname(os.path.abspath(__file__))
lib_path = os.path.join(script_dir, "liblbr_api.so")
my_lib = ctypes.CDLL(lib_path)

# get the funcions and define the arguments

enable_lbr = my_lib.enable_lbr
disable_lbr = my_lib.disable_lbr
dump_lbr = my_lib.dump_lbr
config_lbr = my_lib.config_lbr

enable_lbr.restype = Clbr_ioctl_request
enable_lbr.argtypes = [ctypes.c_uint]
disable_lbr.argtypes = [Clbr_ioctl_request]
dump_lbr.argtypes = [Clbr_ioctl_request]
config_lbr.argtypes = [Clbr_ioctl_request]

lbr_req = None
lbr_enable = False

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
        global lbr_req, lbr_enable
        process_pid = get_gdb_pid()
        lbr_req = enable_lbr(process_pid)
        lbr_enable = True
        print("LIBIHT-GDB: enable lbr for pid :", lbr_req.lbr_config.pid)

class DisableLBR(gdb.Command):
    def __init__(self):
        super(DisableLBR, self).__init__("disable_lbr", gdb.COMMAND_USER)

    def invoke(self, args, from_tty):
        global lbr_req, lbr_enable
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
        for i in range(lbr_tos + 1, 32):
            lbr_content.append(LBRContent(data_pointer[i].from_, data_pointer[i].to))
        for i in range(lbr_tos + 1):
            lbr_content.append(LBRContent(data_pointer[i].from_, data_pointer[i].to))

        # Print the LBR content from the oldest to the newest 
        # PS: (not the order in the LBR stack)
        print(lbr_tos)
        lbr_content = lbr_content[::-1]
        for i in reversed(range(len(lbr_content))):
            print("Last [", i, "] branch record:")
            print("\t From: ", get_function_name(hex(lbr_content[i].from_)))
            print("\t To  : ", get_function_name(hex(lbr_content[i].to)))
        return lbr_content

EnableLBR()
DisableLBR()
DumpLBR()
