import requests
import time
import os
import platform 
import subprocess
import random

version = "1.1.2"

print("checking for updates")
os.system("pip install bs4 datetime ctypes json requests tkinter subprocess platform random")

from bs4 import BeautifulSoup
import tkinter as tk
import tkinter.messagebox as messagebox
import ctypes
import json
import winreg

def ping(host): #icmp echo ping shit
    param = '-n' if platform.system().lower() == 'windows' else '-c'
    command = ['ping', param, '1', host]
    null_device = 'nul' if platform.system().lower() == 'windows' else '/dev/null'
    with open(null_device, 'w') as devnull:
        return subprocess.call(command, stdout=devnull, stderr=subprocess.STDOUT) == 0


def clear_console():
    if os.name == 'nt':
        os.system('cls')
    else:
        os.system('clear')
    return

url = None

if os.name == 'nt':
    os.system("pip install winreg")

clear_console()

def set_startup():
    if os.name == 'nt':
        try:
            key = winreg.CreateKey(winreg.HKEY_CURRENT_USER, r"Software\Microsoft\Windows\CurrentVersion\Run")
            winreg.SetValueEx(key, "steamtracker", 0, winreg.REG_SZ, f"python  {__file__}")
        except WindowsError as e:
            print("failed to add steamtracker to currentversion\\run")
    return

startup = None
hidewindow = None

logo = """
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒  _____ _______                _______ _____            _  ________ _____   ▒
▒ / ____|__   __|              |__   __|  __ \          | |/ /  ____|  __ \  ▒
▒| (___    | | ___  __ _ _ __ ___ | |  | |__) |__ _  ___| ' /| |__  | |__) | ▒
▒ \___ \   | |/ _ \/ _` | '_ ` _ \| |  |  _  // _` |/ __|  < |  __| |  _  /  ▒
▒ ____) |  | |  __/ (_| | | | | | | |  | | \ \ (_| | (__| . \| |____| | \ \  ▒
▒|_____/   |_|\___|\__,_|_| |_| |_|_|  |_|  \_\__,_|\___|_|\_\______|_|  \_\\ ▒
▒                                                                            ▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
 """

def checksetup():
    if os.path.isfile(os.getcwd() + "/config.json") != True:
        return False
    return True

def checkvalidlink(link):
    try:
        if requests.get(link).ok:
            return True
    except:
        return False
    return False

clear_console()
print(logo)

print("checking your internet connecting\n(this tool requires a wifi connection)")
isconnected = False
while isconnected == False:
    if ping("8.8.8.8") == True:
        print("connected.")
        isconnected = True
    else:
        print("are you sure youre connected as the icmp echo (aka ping) timedout?\n\npress any key to try again.")
        input()

if checksetup() != True:
    print("written by shadowdev")
    print(f"version - {version}")
    print("\n\n")
    print("setup required - press any key to continue")
    input()
    clear_console()
    with open("config.json", "w") as f:
        isvalid = False
        while isvalid == False:
            print("enter a a valid link (has to contain https://) to the steam profile to track")
            link = input()
            isvalid = checkvalidlink(link=link)
            clear_console()
        isvalid = False
        while isvalid == False:
          print("please enter a valid delay between checking for profile updates (in seconds milliseconds dont work) min 120 seconds")
          delay = input()
          if delay.isnumeric() and int(delay) >= 120:
            isvalid = True
          else:
            print("Invalid delay. Please enter a number greater than or equal to 120 seconds.")
        clear_console()
        if os.name == 'nt':
            print("do you want this program to run on startup?\n\n!!! you wont be able to change the location of this program without manually modifying the registry !!!\n Y | N")
            startup = input()
            if "y" in startup.lower():
                print("you have choosen to not run this app on startup\n\nif you didnt want to run this app on startup then modify the config.json")
                set_startup()
            elif "n" in startup.lower():
                print("you have choosen to not run this app on startup\n\nif you wanted to run this app on startup then modify the config.json")
            else:
                startup = "n"
            input()
            clear_console()
            if "y" in startup.lower():
                print("do you want to hide the application window on startup?\n Y | N")
                hidewindow = input()
                if "y" in hidewindow.lower():
                    print("you have choosen to keep the window always hidden when running to change this you will have to modify the config.json")
                if "n" in hidewindow.lower():
                    print("you have choosen to keep the window always open when running to change this you will have to modify the config.json")
                else:
                    hidewindow = "n"
                input()
        clear_console()
        print("setup has been completed thank you for using shadowdevs steamtracker")
        f.write(json.dumps({"link": link, "startupopt": startup, "hidewindow": hidewindow,"delay":delay})) 
        f.close()
else:
    with open(os.getcwd() + "/config.json", "r") as f:
        config_data = json.loads(f.readline().strip())
        if "y" in config_data["startupopt"]:
            set_startup()

time.sleep(2)
clear_console()

if os.path.isfile(os.getcwd() + "/logs.txt") != True:
    open(os.getcwd() + "/logs.txt", "w")

with open(os.getcwd() + "/config.json", "r") as f:
    config_data = json.loads(f.readline().strip())
    try:
        url = config_data["link"]
        hidewindow = config_data["hidewindow"]
    except:
        print(f"your config.json is most likely corrupted please delete your config.json\nand run {__file__} again")
        input()
        exit(1)
    try:
        if "y" in hidewindow.lower().strip():
            if os.name == 'nt':
                kernel32 = ctypes.WinDLL('kernel32')
                user32 = ctypes.WinDLL('user32')
    
                if kernel32.GetConsoleWindow():
                    user32.ShowWindow(kernel32.GetConsoleWindow(), 0)
    except: 
        pass
    f.close()

while True:
    response = requests.get(url)

    if response.status_code == 200:

        soup = BeautifulSoup(response.content, 'html.parser')

        errcheck = soup.find('div', class_='error_ctn')

        if errcheck:
            h3_tags = soup.find_all('h3')
            for h3_tag in h3_tags:
                if "The specified profile could not be found." in h3_tag.text.lower():
                    messagebox.showerror("steamtracker", f"this specific steam profile has not been found are you sure you have the right url\nthe user may have setted up or changed his custom account url.")
        else:
            profile_in_game_name_div = soup.find('div', class_='profile_in_game_name')
            online_status = soup.find('div', class_='profile_in_game_header')
    
            profile_desc_div = soup.select_one('div.header_real_name')
            profile_desc = profile_desc_div.find('bdi')
    
            username = soup.find('span', class_='actual_persona_name')
    
            status = game_name = online_status.get_text(strip=True)
            profile_description = profile_desc.get_text(strip=True) if profile_desc else ""
            usernametxt = username.get_text(strip=True)
    
            avatar_div = soup.find('div', class_='playerAvatarAutoSizeInner')
            if avatar_div:
                img_tag = avatar_div.find('img')
                if img_tag:
                   profilepicturelink = img_tag.get('src')
    
            acclevel = soup.find('span', class_='friendPlayerLevelNum')
            accleveltxt = acclevel.get_text(strip=True) if acclevel else ""
    
            if profile_in_game_name_div:
                game_name = profile_in_game_name_div.get_text(strip=True)

                friends_response = requests.get(url + "/friends")
                friends_soup = BeautifulSoup(friends_response.content, 'html.parser')
                
                steamids = []
    
                friend_blocks = friends_soup.find_all('div', id='search_results')
    
                for block in friend_blocks:
                    steamid_tags = block.find_all(attrs={"data-steamid": True})
    
                    for steamid_tag in steamid_tags:
                        steamid = steamid_tag['data-steamid']
                        steamids.append(steamid)
    
                steamids_str = '-'.join(steamids)
                
                with open(os.getcwd() + "/logs.txt", "a") as f:

                 f.write("\n" + json.dumps({"time": int(time.time()),
                                           "game_being_played": "none",
                                           "status": status,
                                           "profile_description": profile_description,
                                           "username": usernametxt,
                                           "profile_picture": profilepicturelink,
                                           "level": accleveltxt,
                                           "friends": steamids_str}))
            else:
             with open(os.getcwd() + "/logs.txt", "a") as f:
                 friends_response = requests.get(url + "/friends")
                 friends_soup = BeautifulSoup(friends_response.content, 'html.parser')
                 
                 steamids = []
                 
                 friend_blocks = friends_soup.find_all('div', id='search_results')
    
                 for block in friend_blocks:
                     
                     steamid_tags = block.find_all(attrs={"data-steamid": True})
    
                     for steamid_tag in steamid_tags:
                         steamid = steamid_tag['data-steamid']
                         steamids.append(steamid)
    
                 steamids_str = '-'.join(steamids)
                 
                 f.write("\n" + json.dumps({"time": int(time.time() ),
                                            "game_being_played": "none",
                                            "status": status,
                                            "profile_description": profile_description,
                                            "username": usernametxt,
                                            "profile_picture": profilepicturelink,
                                            "level": accleveltxt,
                                            "friends": steamids_str}))
     
    else:
        messagebox.showerror("steamtracker", f"failed to fetch profile: {response.status_code}")
    delay = int(config_data["delay"])
    time.sleep(delay)