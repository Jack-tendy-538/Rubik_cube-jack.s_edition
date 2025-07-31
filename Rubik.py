from ctypes import cdll

module = cdll.LoadLibrary("Rubik.dll")
# you are on your own from here
# you can use module.<function_name> to call functions from the DLLS