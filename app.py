from flask import Flask,render_template,request,jsonify,send_file
import os,csv,subprocess,signal,threading
ROOT=os.path.dirname(os.path.abspath(__file__)); BIN=os.path.join(ROOT,"os_stresslab"); CSV=os.path.join(ROOT,"os_stresslab_results.csv")
app=Flask(__name__); lock=threading.Lock(); proc=None; config={"workers":2,"memory":128,"duration":15}
def samples():
 out=[]
 try:
  with open(CSV,newline="") as f:
   for r in csv.DictReader(f): out.append({"elapsed":int(float(r["elapsed_seconds"])),"cpu":float(r["cpu_percent"]),"ram":float(r["ram_percent"]),"used":int(r["used_ram_mb"]),"available":int(r["available_ram_mb"])})
 except (OSError,ValueError,KeyError): pass
 return out[-120:]
@app.get("/")
def home(): return render_template("index.html")
@app.get("/api/status")
def status():
 global proc
 with lock:
  running=proc is not None and proc.poll() is None
  if proc is not None and not running: proc=None
 rows=samples()
 return jsonify(running=running,config=config,samples=rows,latest=rows[-1] if rows else None,peak_cpu=max((x["cpu"] for x in rows),default=0),peak_ram=max((x["ram"] for x in rows),default=0))
@app.post("/api/start")
def start():
 global proc,config
 d=request.get_json(silent=True) or {}
 try: w,m,t=int(d.get("workers",2)),int(d.get("memory",128)),int(d.get("duration",15))
 except (ValueError,TypeError): return jsonify(error="Enter valid numbers"),400
 if not(1<=w<=8 and 0<=m<=512 and 5<=t<=60): return jsonify(error="Use 1–8 workers, 0–512 MB, 5–60 seconds"),400
 if not os.path.isfile(BIN): return jsonify(error="C executable missing. Compile main.c first."),500
 with lock:
  if proc is not None and proc.poll() is None: return jsonify(error="A test is already running"),409
  config={"workers":w,"memory":m,"duration":t}
  proc=subprocess.Popen([BIN,"--workers",str(w),"--memory",str(m),"--duration",str(t),"--csv",CSV],cwd=ROOT,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 return jsonify(message="Test started")
@app.post("/api/stop")
def stop():
 global proc
 with lock:
  if proc is not None and proc.poll() is None:
   proc.send_signal(signal.SIGTERM)
   try: proc.wait(timeout=3)
   except subprocess.TimeoutExpired: proc.kill();proc.wait()
   proc=None
 return jsonify(message="Stop requested")
@app.get("/api/download")
def download():
 if not os.path.exists(CSV): return jsonify(error="Run a test first"),404
 return send_file(CSV,as_attachment=True,download_name="os_stresslab_results.csv")
if __name__=="__main__": app.run(host="127.0.0.1",port=5000,debug=False)
