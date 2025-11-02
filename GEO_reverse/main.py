import json, sys, time, requests
import pywifi
from pywifi import const

GOOGLE_GEO_API_KEY = "..."
GOOGLE_GEO_URL = f"https://www.googleapis.com/geolocation/v1/geolocate?key={GOOGLE_GEO_API_KEY}"
NOMINATIM_URL = "https://nominatim.openstreetmap.org/reverse"
NOMINATIM_UA = "generic"
TOP_N_APS = 10

def scan_wifi(timeout=3.0):
    wifi = pywifi.PyWiFi()
    ifaces = wifi.interfaces()
    if not ifaces:
        raise RuntimeError("no wap found")
    iface = ifaces[0]
    iface.scan()
    time.sleep(timeout)
    results = iface.scan_results()
    seen = {}
    for r in results:
        mac = (r.bssid or "").lower()
        if not mac:
            continue
        ssid = r.ssid or ""
        try:
            rssi = int(r.signal)
        except Exception:
            rssi = None
        cur = seen.get(mac)
        if cur is None or (rssi is not None and (cur["signalStrength"] is None or rssi > cur["signalStrength"])):
            seen[mac] = {"macAddress": mac, "signalStrength": rssi, "ssid": ssid}
    aps = list(seen.values())
    aps.sort(key=lambda x: (x["signalStrength"] is not None, x["signalStrength"]), reverse=True)
    return aps

def query_google_geo(wifi_list):
    payload = {"wap": wifi_list}
    try:
        resp = requests.post(GOOGLE_GEO_URL, json=payload, timeout=10)
        resp.raise_for_status()
        return resp.json()
    except requests.RequestException as e:
        print("err", e, file=sys.stderr)
        return None

def reverse_geocode(lat, lon):
    params = {"format": "jsonv2", "lat": lat, "lon": lon, "zoom": 18, "addressdetails": 1}
    headers = {"User-Agent": NOMINATIM_UA}
    try:
        r = requests.get(NOMINATIM_URL, params=params, headers=headers, timeout=10)
        r.raise_for_status()
        return r.json()
    except requests.RequestException as e:
        print("err", e, file=sys.stderr)
        return None

def main():
    print("scanning da networks")
    try:
        aps = scan_wifi(timeout=3.0)
    except Exception as e:
        print("err", e, file=sys.stderr)
        return

    if not aps:
        print("no wap")
        return

    wifi_for_geo = []
    for a in aps[:TOP_N_APS]:
        entry = {"macAddress": a["macAddress"]}
        if a.get("signalStrength") is not None:
            entry["signalStrength"] = a["signalStrength"]
        wifi_for_geo.append(entry)

    print(f"\nFound {len(aps)} APs; sending top {len(wifi_for_geo)} to Google Geolocation API...")
    print(json.dumps(wifi_for_geo, indent=2))

    data = query_google_geo(wifi_for_geo)
    if not data or "location" not in data:
        print("endpoint err")
        return

    loc = data["location"]
    acc = data.get("accuracy")
    lat, lon = loc.get("lat"), loc.get("lng")

    print(f"est. location: {lat:.6f}, {lon:.6f}  (accuracy ≈ {acc} m)")

    place = reverse_geocode(lat, lon)
    if place:
        print(place.get("display_name"))
        if place.get("address"):
            print("\nAddress details:")
            print(json.dumps(place["address"], indent=2, ensure_ascii=False))
    else:
        print("err")

if __name__ == "__main__":
    main()
    input()
