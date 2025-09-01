from enum import Enum
import os
from headers import ini
import platform

customscanroot = ""

class ErrorCodes(Enum):
    Success = 0
    Failed = 1
    Unknown_Error = 2
    Access_Error = 3

debug = False # switch this to get spammed :)

# doesnt support rivals because rivals uses .sav which is too common to differentiate it from other games and the save file itself doesnt have a flag which would specify it as a need for speed save file
# wont fix

validationkey_frostbite = b"NFSS" # Unbound , payback , heat , all frostbite games basically
validationkey_blackbox = b"20CMl" # NFSMW2004
validationkey_ghost = bytes([0xDB, 0x38, 0x64, 0x7E]) # rivals
validationkey_mw2012 = bytes([0x00, 0xD8, 0x03, 0x00]) # NFSMW2012
validationkey_hotpursuit = bytes([0x00, 0x00, 0x04, 0x00]) # NFSHPRM

class selectedaction(Enum):
    Nul = 0
    Delete = 1
    Backup = 2 

class selectedaction_scan(Enum):
    ALLDISKS = 0
    CUSTOMPATH = 1

selectedscantype = selectedaction_scan.ALLDISKS
selectedactionint = selectedaction.Nul

saves = []

def is_int(s):
    """Return True if s can be converted to int, else False."""
    try:
        int(s)
        return True
    except ValueError:
        return False

#nothing here matters as long as it returns success
def validate_savefile(file):
    try:
        if os.path.basename(file).lower().endswith(".bak"): #exclude backups
            return ErrorCodes.Failed
        with open(file, "rb") as f:
            header = f.read(len(validationkey_frostbite))  # read first 4 bytes
            if header == validationkey_frostbite:
                return ErrorCodes.Success
            f.seek(0)
            header = f.read(len(validationkey_blackbox))
            if header == validationkey_blackbox:
                return ErrorCodes.Success
            f.seek(0)
            header = f.read(len(validationkey_ghost)) 
            if header == validationkey_ghost:
                return ErrorCodes.Success
            f.seek(0)
            header = f.read(len(validationkey_mw2012)) 
            if header == validationkey_mw2012:
                return ErrorCodes.Success
            f.seek(0)
            header = f.read(len(validationkey_hotpursuit)) 
            if header == validationkey_hotpursuit:
                return ErrorCodes.Success
            else:
                return ErrorCodes.Failed
    except Exception as e:
        if (debug):
            print(f"{e}")
        return ErrorCodes.Access_Error
    
def loadpath():
    global saves
    if os.path.isfile(ini.filename):
        index = 1
        paths = []
        while True:
            key = f"Path{index}"
            value = ini.getvalue(key)
            if value is None:
                break
            paths.append(value)
            index += 1
        saves = paths
    else:
        return
    
def savepath():
    global saves
    if not saves:
        return False

    try:
        with open(ini.filename, "w", encoding="utf-8") as f:
            for index, path in enumerate(saves, start=1):
                f.write(f"Path{index}={path}\n")
        return True
    except Exception as e:
        print(f"{e}")
        return False

def search4saves():
    global saves
    saves = []
    found_set = set()

    if selectedscantype == selectedaction_scan.ALLDISKS:
        cwd = os.getcwd()
        print(f"starting in cwd: {cwd}", flush=True)

        os_name = platform.system().lower()
        if os_name == "windows":
            drives = [f"{d}:\\" for d in "ABCDEFGHIJKLMNOPQRSTUVWXYZ" if os.path.exists(f"{d}:\\")]
            search_dirs = drives if drives else [cwd]
        else:
            search_dirs = ["/"]

    elif selectedscantype == selectedaction_scan.CUSTOMPATH:
        if not customscanroot:
            print("invalid customroot :(", flush=True)
            return []

        if not os.path.isdir(customscanroot):
            print(f"{customscanroot}. isnt a valid dir.", flush=True)
            return []
        else:
            search_dirs = [customscanroot]

        print(f"starting {search_dirs[0]}", flush=True)
    else:
        cwd = os.getcwd()
        search_dirs = [cwd]
        print(f"the fuck???: {cwd}", flush=True)

    def onerror(err):
        if debug:
            print(f"accessing {getattr(err, 'filename', 'unknown')}: {err}", flush=True)

    try:
        for base_dir in search_dirs:
            print(f"scanning {base_dir}", flush=True)
            if not os.path.isdir(base_dir):
                if debug:
                    print(f"skipping {base_dir}", flush=True)
                continue

            for root, dirs, files in os.walk(base_dir, onerror=onerror, followlinks=False):
                print(f"scanning {root}", flush=True)
                for file in files:
                    filepath = os.path.join(root, file)
                    try:
                        if validate_savefile(filepath) == ErrorCodes.Success:
                            if filepath not in found_set:
                                found_set.add(filepath)
                                saves.append(filepath)
                                if debug:
                                    print(f"found {filepath}", flush=True)
                    except Exception as e:
                        if debug:
                            print(f"err ({filepath}): {e}", flush=True)
                        continue

            if selectedscantype == selectedaction_scan.CUSTOMPATH:
                return saves
            
    except Exception as e:
        if debug:
            print(f"err {e}", flush=True)

    return saves