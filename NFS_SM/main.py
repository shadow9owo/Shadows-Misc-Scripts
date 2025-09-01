from headers import Utils, ini
import os
from enum import Enum
import shutil

found = False
running = True

def applyselectedaction(path):
    if selectedactionint == Utils.selectedaction.Backup:
        shutil.copyfile(path, os.path.join(os.getcwd(),"backups", os.path.basename(path) + ".bak"))
    elif selectedactionint == Utils.selectedaction.Delete:
        if path in Utils.saves:
            Utils.saves.remove(path)
        os.remove(path)
        Utils.savepath()

    return

usagetick = 0 # clear at 5 clearing is pretty heavy for the system

def ChoosePath():
    global selectedscantype
    __input = ""
    invalid = True
    print("Possible scan types:\n1. SCAN ALL DISKS (slow)\n2. CUSTOM SCAN (fast)")
    while invalid:
        try:
            _input = int(input("select an option >> "))
            if _input == 1:
                invalid = False
                Utils.selectedscantype = Utils.selectedaction_scan.ALLDISKS
                return
            elif _input == 2:
                selectedscantype = Utils.selectedaction_scan.CUSTOMPATH
                invalid = False
            else:
                return
        except ValueError:
            print("invalid input")

    invalid = True

    print("input CUSTOMPATH:\n")
    while invalid:
        __input = input("select an option >> ")
        if os.path.isdir(__input):
            Utils.customscanroot = __input
            Utils.selectedscantype = Utils.selectedaction_scan.CUSTOMPATH
            invalid = False
            print(Utils.customscanroot + " selected")
    input()

    return

def HandleInput(___input):
    invalid = True
    global running, selectedactionint # i hate python why cant everything be global by default

    if type(___input) != int:
        print("invalid input")
        return
    
    if ___input == 1:
        ChoosePath()

        Utils.saves = Utils.search4saves()
        print("the following was found:")
        if not Utils.saves:
            print("empty....")
            return
        else:
            Utils.savepath()
        for i, save in enumerate(Utils.saves):
            print(f"{i}. {save}")
        return
    elif ___input == 2:
        if not Utils.saves:
            print("empty....")
            return
        for i, save in enumerate(Utils.saves):
            print(f"{i}. {save}")
        return
    elif ___input == 3:
        if not Utils.saves:
            print("empty....")
            return
        for i, save in enumerate(Utils.saves):
            print(f"{i}. {save}")

        print("input \'*\' to backup everything")
        while invalid:
            try:
                _input = input("select an option >> ")
                if _input == "*":
                    invalid = False
                elif Utils.is_int(_input):
                    invalid = False
                elif type(_input) == int:
                    invalid = False
                else:
                    raise ValueError
            except ValueError:
                print("invalid input")
            
        selectedactionint = Utils.selectedaction.Backup
        if _input == "*":
            for save_file in Utils.saves:
             applyselectedaction(save_file)
        else:
            applyselectedaction(Utils.saves[int(_input)])
        return
    elif ___input == 4:
        if not Utils.saves:
            print("empty....")
            return
        for i, save in enumerate(Utils.saves):
            print(f"{i}. {save}")

        print("input \'*\' to delete everything")

        while invalid:
            try:
                _input = input("select an option >> ")
                if _input == "*":
                    invalid = False
                elif Utils.is_int(_input):
                    invalid = False
                elif type(_input) == int:
                    invalid = False
                else:
                    raise ValueError
            except ValueError:
                print("invalid input")
            
        selectedactionint = Utils.selectedaction.Delete
        if _input == "*":
            for save_file in Utils.saves:
             applyselectedaction(save_file)
        else:
            applyselectedaction(Utils.saves[int(_input)])
        return
    elif ___input == 5:
        running = False
        return

    return

def menu():
    global _input
    print("1 - search for save files")
    print("2 - list save files")
    print("3 - backup save file(s)")
    print("4 - delete save file(s)")
    print("5 - exit")

    try:
        _input = int(input("select an option >> "))
    except ValueError:
        print("invalid input")
        return

    
    HandleInput(_input)

    return

def main():
    global usagetick,selectedactionint
    found = os.path.isfile(ini.filename)
    
    if not os.path.exists(os.path.join(os.getcwd(),"backups")):
        os.mkdir("backups")

    print(
        "\nNFS_SFM\nneed for speed save file manager\nver 0.1\n"
        )

#Utils.search4saves
    if found:
        print("config.ini was found")
        Utils.loadpath()
    else:
        print("no config.ini was found")

    while running:
        if usagetick > 9:
            Utils.clearconsole()
        menu()
        usagetick = usagetick + 1
        selectedactionint = Utils.selectedaction.Nul

main()
