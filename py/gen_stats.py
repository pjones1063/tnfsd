import csv
import json
import urllib.request
import time
from collections import Counter

# --- CONFIGURATION ---
LOG_FILE = "tnfsd_stats.csv"   
OUTPUT_FILE = "stats.html"
IP_CACHE_FILE = "ip_cache.json"

# --- GEOIP LOOKUP ---
ip_country_cache = {}

def load_cache():
    global ip_country_cache
    try:
        with open(IP_CACHE_FILE, 'r') as f:
            ip_country_cache = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        ip_country_cache = {}

def save_cache():
    with open(IP_CACHE_FILE, 'w') as f:
        json.dump(ip_country_cache, f)

def get_country(ip):
    if ip.startswith(("127.", "192.168.", "10.", "172.16.")):
        return "Local"
    if ip in ip_country_cache:
        return ip_country_cache[ip]
    try:
        # Rate limiting for the free API
        time.sleep(0.1) 
        url = f"http://ip-api.com/json/{ip}?fields=country"
        with urllib.request.urlopen(url, timeout=2) as response:
            data = json.loads(response.read().decode())
            country = data.get("country", "Unknown")
            ip_country_cache[ip] = country
            return country
    except Exception:
        return "Unknown"

# --- MAIN GENERATOR ---
def generate_stats():
    load_cache()
    
    # 1. READ ALL DATA (For Summary Stats)
    # We read the entire file to get accurate totals for the "Headers"
    ip_mount_counts = Counter()
    unique_mount_files = set()
    processed_rows = []

    print(f"Reading {LOG_FILE}...")
    
    try:
        with open(LOG_FILE, 'r', encoding='utf-8', errors='replace') as f:
            reader = csv.reader(f)
            
            for row in reader:
                if len(row) < 3: continue 

                timestamp = row[0].strip()
                ip = row[1].strip()
                message = row[2].strip()

                # Parse Message
                if " " in message:
                    command, filename = message.split(" ", 1)
                else:
                    command = message
                    filename = ""

                # Filter 'phantom' to blanks
                if "phantom" in filename.lower():
                    display_filename = ""
                else:
                    display_filename = filename

                # Calculate Global Stats
                # Count as mount if command is [OPEN] or 0x00 and has a filename
                if ("[OPEN]" in command or "0x00" in command) and display_filename:
                    ip_mount_counts[ip] += 1
                    unique_mount_files.add(display_filename)

                # Fetch Country (cached)
                country = get_country(ip)

                processed_rows.append({
                    "time": timestamp,
                    "ip": ip,
                    "country": country,
                    "cmd": command,
                    "file": display_filename
                })
                
    except FileNotFoundError:
        print(f"Error: Could not find {LOG_FILE}.")
        return

    save_cache()

    # 2. PREPARE DATA SUBSETS
    # Global Stats (The "Headers") - calculated from everything
    total_mounts = sum(ip_mount_counts.values())
    total_unique_files = len(unique_mount_files)
    total_unique_users = len(ip_mount_counts)

    # Table 1: Top 20 Users (Limited)
    top_20_users = ip_mount_counts.most_common(20)

    # Table 2: Recent Activity (Limited to last 20)
    # Reverse to show newest first, then slice top 20
    recent_20_activity = list(reversed(processed_rows))[:20]

    print("Generating HTML report...")
    
    html = f"""
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>TNFS Server Statistics</title>
        <style>
            body {{ font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background: #f4f4f9; color: #333; margin: 0; padding: 20px; }}
            .container {{ max-width: 1200px; margin: 0 auto; }}
            h1 {{ text-align: center; color: #2c3e50; margin-bottom: 30px; }}
            
            /* Dashboard Headers */
            .dashboard {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-bottom: 40px; }}
            .card {{ background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); text-align: center; }}
            .card h2 {{ margin: 0; font-size: 1rem; color: #7f8c8d; text-transform: uppercase; letter-spacing: 1px; }}
            .card p {{ margin: 10px 0 0; font-size: 2.5rem; font-weight: 700; color: #2980b9; }}
            
            /* Tables */
            h2 {{ color: #34495e; border-bottom: 2px solid #34495e; padding-bottom: 10px; margin-top: 40px; }}
            table {{ width: 100%; border-collapse: collapse; background: white; border-radius: 8px; overflow: hidden; box-shadow: 0 2px 5px rgba(0,0,0,0.1); margin-bottom: 20px; }}
            th, td {{ padding: 12px 15px; text-align: left; border-bottom: 1px solid #ddd; }}
            th {{ background-color: #2c3e50; color: white; text-transform: uppercase; font-size: 0.8rem; letter-spacing: 0.05em; }}
            tr:hover {{ background-color: #f8f9fa; }}
            
            .rank-col {{ width: 50px; font-weight: bold; color: #7f8c8d; text-align: center; }}
            .num-col {{ text-align: right; font-weight: bold; color: #2980b9; }}
            td.empty {{ color: #ccc; font-style: italic; }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>TNFS Server Activity</h1>
            
            <div class="dashboard">
                <div class="card">
                    <h2>Total Mounts</h2>
                    <p>{total_mounts}</p>
                </div>
                <div class="card">
                    <h2>Unique Mounts</h2>
                    <p>{total_unique_files}</p>
                </div>
                <div class="card">
                    <h2>Active Users</h2>
                    <p>{total_unique_users}</p>
                </div>
            </div>

            <h2>Top 20 Active Users</h2>
            <table>
                <thead>
                    <tr>
                        <th class="rank-col">#</th>
                        <th>IP Address</th>
                        <th>Country</th>
                        <th class="num-col">Mounts</th>
                    </tr>
                </thead>
                <tbody>
    """

    for rank, (ip, count) in enumerate(top_20_users, 1):
        country = get_country(ip)
        html += f"""
                    <tr>
                        <td class="rank-col">{rank}</td>
                        <td>{ip}</td>
                        <td>{country}</td>
                        <td class="num-col">{count}</td>
                    </tr>
        """

    html += """
                </tbody>
            </table>

            <h2>Recent Activity Log (Last 20)</h2>
            <table>
                <thead>
                    <tr>
                        <th>Timestamp</th>
                        <th>IP Address</th>
                        <th>Country</th>
                        <th>Command</th>
                        <th>Filename</th>
                    </tr>
                </thead>
                <tbody>
    """

    for row in recent_20_activity:
        file_display = row['file'] if row['file'] else '<span class="empty">-</span>'
        html += f"""
                    <tr>
                        <td>{row['time']}</td>
                        <td>{row['ip']}</td>
                        <td>{row['country']}</td>
                        <td>{row['cmd']}</td>
                        <td>{file_display}</td>
                    </tr>
        """

    html += """
                </tbody>
            </table>
            
            <p style="text-align: center; margin-top: 40px; color: #999; font-size: 0.8rem;">
                Generated by TNFS Stats
            </p>
        </div>
    </body>
    </html>
    """

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write(html)
    
    print(f"Success! Stats written to {OUTPUT_FILE}")

if __name__ == "__main__":
    generate_stats()
    
