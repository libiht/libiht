import os
import gdb
import ctypes

# quote the ctypes

class Clbr_stack_entry(ctypes.Structure):
    _fileds_ = [
        ('from_', ctypes.c_unsigned_long_long),
        ('to', ctypes.c_unsigned_long_long)
    ]

class Clbr_config(cyptes.Structure):
    _fields_ = [
        ('pid', ctypes.c_unsigned_int),
        ('lbr_select', ctypes.c_unsigned_long_long)
    ]

class Clbr_data(ctypes.Structure):
    _fields_ = [
        ('lbr_tos', ctypes.c_unsigned_long_long),
        ('entries', ctypes.POINTER(Clbr_stack_entry))
    ]

class Clbr_ioctl_request(ctypes.Structure):
    _fields_ = [
        ('lbr_config', Clbr_config),
        ('buffer', ctypes.POINTER(Clbr_data))
    ]

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
select_lbr = my_lib.select_lbr

disable_lbr.argtypes = [Clbr_ioctl_request]
dump_lbr.argtypes = [Clbr_ioctl_request]
select_lbr.argtypes = [Clbr_ioctl_request]

lbr_req = None

class DumpLBR(gdb.Function):
    def __init__(self):
        super(DumpLBR, self).__init__("dump_lbr", gdb.COMMAND_USER)
        lbr_req = enable_lbr()
    
    def invoke(self, args, from_tty):
        dump_lbr(lbr_req)
        lbr_tos = lbr_request.buffer.contents.lbr_tos
        data_pointer = ctypes.cast(lbr_request.buffer.contents.entries, ctypes.POINTER(Clbr_stack_entry))
        lbr_content = []
        for i in range(0, lbr_tos):
            lbr_content.append(LBRContent(data_pointer[i].from_, data_pointer[i].to))
        if from_tty is True:
            for i in range(0, lbr_tos):
                print("MSR_LBR_NHM_FROM[", i, "]: ", hex(lbr_content[i].from_))
                print("MSR_LBR_NHM_TO  [", i, "]: ", hex(lbr_content[i].to))
        return lbr_content
