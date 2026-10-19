# OS StressLab — C Web Backend

A local educational CPU and memory workload dashboard. The web server and stress engine are both written in C; HTML/CSS/JavaScript provide the browser interface. No Python/Flask is required to run this version.

## Requirements (Ubuntu / WSL)

```bash
sudo apt update
sudo apt install build-essential
```

## Build and run

From this folder:

```bash
gcc main.c -o stress_engine -O2 -Wall -Wextra -pthread
gcc server.c -o stress_server -O2 -Wall -Wextra
./stress_server
```

Open `http://127.0.0.1:5000` in your browser. Keep the terminal open while using the website. Stop the server with Ctrl+C.

The test limits are 1–8 worker threads, 0–512 MB allocated memory, and 5–60 seconds. CPU/RAM percentages and memory totals describe the Linux/WSL environment, not only the stress process. Use moderate settings and stop the test if the system becomes sluggish.
