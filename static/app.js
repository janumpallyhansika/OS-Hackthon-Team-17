const $ = id => document.getElementById(id);
let last = [];
async function api(url, options = {}) {
  const r = await fetch(url, { cache: 'no-store', ...options, headers: { 'Content-Type': 'application/json', ...(options.headers || {}) } });
  const d = await r.json();
  if (!r.ok) throw new Error(d.error || 'Request failed');
  return d;
}
async function startTest() {
  try {
    await api('/api/start', { method: 'POST', body: JSON.stringify({ workers: +$('workers').value, memory: +$('memory').value, duration: +$('duration').value }) });
    $('notice').textContent = 'C stress test started.';
    await refresh();
  } catch (e) { $('notice').textContent = e.message; }
}
async function stopTest() {
  try { await api('/api/stop', { method: 'POST', body: '{}' }); $('notice').textContent = 'Test stopped.'; await refresh(); }
  catch (e) { $('notice').textContent = e.message; }
}
function chart(a) {
  const c = $('chart'); if (!c) return;
  const x = c.getContext('2d'), w = c.width, h = c.height; x.clearRect(0,0,w,h);
  for (let v=0; v<=100; v+=25) { const y=h-25-v/100*(h-45); x.strokeStyle='#29354c'; x.beginPath(); x.moveTo(35,y); x.lineTo(w-8,y); x.stroke(); x.fillStyle='#94a0b6'; x.font='11px sans-serif'; x.fillText(v+'%',2,y+4); }
  if (!a.length) { x.fillStyle='#94a0b6'; x.fillText('Start a test to collect measurements',100,h/2); return; }
  const mx=Math.max(1,...a.map(s=>s.elapsed));
  for (const [key,color] of [['cpu','#ffb16b'],['ram','#b5a1ff']]) { x.strokeStyle=color; x.lineWidth=2; x.beginPath(); a.forEach((s,i)=>{const px=35+s.elapsed/mx*(w-45), py=h-25-Math.max(0,Math.min(100,s[key]))/100*(h-45); if(i)x.lineTo(px,py);else x.moveTo(px,py);}); x.stroke(); }
}
async function refresh() {
  try {
    const d=await api('/api/status');
    $('status').textContent=d.running?'TEST RUNNING':'SYSTEM READY';
    $('start').disabled=!!d.running; $('stop').disabled=!d.running;
    last=(d.samples||[]).map(s=>({elapsed:Number(s.elapsed ?? s.second ?? 0),cpu:Number(s.cpu||0),ram:Number(s.ram||0)}));
    $('cpu').textContent=Number(d.cpu||0).toFixed(1); $('ram').textContent=Number(d.ram||0).toFixed(1);
    $('cpubar').style.width=Math.max(0,Math.min(100,Number(d.cpu||0)))+'%'; $('rambar').style.width=Math.max(0,Math.min(100,Number(d.ram||0)))+'%';
    $('peakcpu').textContent=Number(d.peak_cpu||0).toFixed(1); $('peakram').textContent=Number(d.peak_ram||0).toFixed(1);
    $('used').textContent=Number(d.used_mb||0).toLocaleString(); $('avail').textContent=Number(d.available_mb||0).toLocaleString();
    $('wl').textContent=$('workers').value; $('ml').textContent=$('memory').value;
    $('rw').textContent=$('workers').value; $('rm').textContent=$('memory').value+' MB'; $('rd').textContent=$('duration').value+' sec'; $('rs').textContent=last.length; $('samples').textContent=last.length+' samples';
    chart(last); $('notice').textContent=d.running?'Collecting system measurements from the C backend.':'C backend connected.';
  } catch(e) { $('notice').textContent='C backend unavailable: '+e.message; }
}
$('start').addEventListener('click',startTest); $('stop').addEventListener('click',stopTest);
$('workers').addEventListener('input',()=>{$('wl').textContent=$('workers').value;$('rw').textContent=$('workers').value;});
$('memory').addEventListener('input',()=>{$('ml').textContent=$('memory').value;$('rm').textContent=$('memory').value+' MB';});
$('duration').addEventListener('change',()=>{$('rd').textContent=$('duration').value+' sec';});
refresh(); setInterval(refresh,1000);
