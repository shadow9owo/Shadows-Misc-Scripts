import os

filename = "global.ini"

def getvalue(key: str):
    """Return the value for the given key, or None if not found."""
    if not filename or not os.path.isfile(filename):
        return None

    try:
        with open(filename, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if "=" not in line:
                    continue
                file_key, file_value = line.split("=", 1)
                if file_key == key:
                    return file_value
    except Exception as e:
        print(f"Error reading INI: {e}")
        return None

    return None


def setvalue(key: str, value: str) -> bool:
    """Set the value for a key in the INI file. Adds the key if it doesn't exist."""
    if not filename:
        return False

    lines = []
    found = False

    # Read existing lines
    if os.path.isfile(filename):
        try:
            with open(filename, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.rstrip("\n")
                    if "=" in line:
                        file_key = line.split("=", 1)[0]
                        if file_key == key:
                            lines.append(f"{key}={value}")
                            found = True
                        else:
                            lines.append(line)
                    else:
                        lines.append(line)
        except Exception as e:
            print(f"Error reading INI: {e}")
            return False

    if not found:
        lines.append(f"{key}={value}")

    try:
        with open(filename, "w", encoding="utf-8") as f:
            for line in lines:
                f.write(line + "\n")
        return True
    except Exception as e:
        print(f"Error writing INI: {e}")
        return False
