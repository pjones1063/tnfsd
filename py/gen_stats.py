import csv
from collections import Counter
from datetime import datetime

LOG_FILE = 'tnfsd_stats.csv'
HTML_FILE = 'index.html'

def generate_html():
    ips = []
    files = []
    timestamps = []

    try:
        with open(LOG_FILE, 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                if len(row) < 3: continue
                timestamps.append(row[0])
                ips.append(row[1])
                files.append(row[2])
    except FileNotFoundError:
        print("No log file found yet.")
        return

    # --- CALCULATE STATS ---
    unique_ips = len(set(ips))
    total_mounts = len(files)
    
    # Top 10 Files
    top_files = Counter(files).most_common(10)
    
    # Top 10 Users (IPs)
    top_ips = Counter(ips).most_common(10)

    # --- GENERATE HTML ---
    html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>TNFS Server Stats - 13Leader.net </title>
        <style>
            body {{ font-family: sans-serif; background: #f4f4f4; padding: 20px; }}
            .container {{ max-width: 900px; margin: auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }}
            h1 {{ color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; }}
            .cards {{ display: flex; gap: 20px; margin-bottom: 30px; }}
            .card {{ flex: 1; background: #007bff; color: white; padding: 20px; border-radius: 5px; text-align: center; }}
            .card h2 {{ margin: 0; font-size: 3em; }}
            table {{ width: 100%; border-collapse: collapse; margin-bottom: 30px; }}
            th, td {{ padding: 12px; border-bottom: 1px solid #ddd; text-align: left; }}
            th {{ background-color: #f8f9fa; }}
        </style>
    </head>
    <body>
        <div class="container">
            <h1>TNFS Server Activity</h1>
            <p>Last Updated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>

            <div class="cards">
                <div class="card">
                    <h2>{total_mounts}</h2>
                    <p>Total Mounts</p>
                </div>
                <div class="card">
                    <h2>{unique_ips}</h2>
                    <p>Unique IPs</p>
                </div>
            </div>

            <h3> Top 10 Mounted Images</h3>
            <table>
                <tr><th>Rank</th><th>Filename</th><th>Count</th></tr>
                {''.join(f'<tr><td>{i+1}</td><td>{f}</td><td>{c}</td></tr>' for i, (f, c) in enumerate(top_files))}
            </table>

            <h3> Top 10 Active IPs</h3>
            <table>
                <tr><th>Rank</th><th>IP Address</th><th>Count</th></tr>
                {''.join(f'<tr><td>{i+1}</td><td>{ip}</td><td>{c}</td></tr>' for i, (ip, c) in enumerate(top_ips))}
            </table>
        </div>
    </body>
    </html>
    """

    with open(HTML_FILE, 'w', encoding='utf-8') as f:
        f.write(html)
    print(f"Stats generated: {HTML_FILE}")

if __name__ == "__main__":
    generate_html()
    
