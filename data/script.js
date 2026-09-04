/* Bobcat 743 · cockpit v3
 * Drag key (direct manipulation, optimistic), simple/advanced detail, sticky STOP,
 * override with hold-to-arm. Talks to the 2026 /api contract; falls back to an
 * in-page simulator when no controller answers (or with ?sim). */
'use strict';

const POLL_MS = 400, RAW_MS = 2000, HB_MS = 300, OV_HOLD_MS = 2000, OV_WINDOW_S = 60;
const POS = { off: { a: -30, n: 0 }, on: { a: 0, n: 1 }, glow: { a: 45, n: 2 }, start: { a: 90, n: 3 } };
const ORDER = ['off', 'on', 'glow', 'start'];
const $ = id => document.getElementById(id);

/* ---------- simulator (same contract as firmware 6dd0128) ---------- */
class Sim {
  constructor() {
    this.s = { kp: 0, held: false, hb: 0, running: false, maint: false, glow0: null, crank0: null, lights: false, runSince: null };
    this.temp = 24; this.hours = 12.3; this.t0 = Date.now();
  }
  now() { return Date.now() / 1000; }
  tick() {
    const s = this.s, now = this.now();
    if (s.held && now - s.hb > 1.0) { s.held = false; s.kp = 1; s.crank0 = null; }          // dead-man
    if (s.kp === 3 && s.held && s.maint) {
      if (s.crank0 == null) s.crank0 = now;
      if (now - s.crank0 >= 2.2) { s.running = true; s.runSince = now; s.held = false; s.kp = 1; s.crank0 = null; }
    }
    if (!(s.kp === 3 && s.held)) s.crank0 = null;
    const target = s.running ? 88 : 24;
    this.temp += (target - this.temp) * 0.004;
  }
  status() {
    this.tick();
    const s = this.s, now = this.now();
    let seq, name;
    if (s.kp === 0) { seq = 0; name = 'OFF'; }
    else if (s.kp === 3 && s.held) { seq = 3; name = 'START'; }
    else if (s.kp === 2) { seq = 2; name = 'GLOW'; }
    else if (s.running) { seq = 4; name = 'RUNNING'; }
    else { seq = 1; name = 'ON'; }
    const glowActive = s.kp === 2 || (s.kp === 3 && s.held);
    const cd = glowActive && s.glow0 ? Math.max(0, Math.round(20 - (now - s.glow0))) : 0;
    const cranking = s.kp === 3 && s.held && s.maint && s.crank0 != null && now - s.crank0 < 10;
    const wob = Math.sin(now * 1.3) * 0.05;
    const batt = cranking ? 10.6 + wob : s.running ? 13.9 + wob : 12.55 + wob;
    const oil = s.running ? 34 + Math.sin(now) * 2 : 0;
    const hyd = s.running ? 1420 + Math.sin(now * 0.7) * 60 : 0;
    if (s.running) this.hours += POLL_MS / 3600000;
    const d = {
      state: name, seq, maintenance: s.maint, engineHours: this.hours, version: 'v3-demo',
      hydraulic: { v: Math.round(hyd), min: 800, max: 3000, unit: 'psi' },
      engineTemp: { v: Math.round(this.temp), min: 40, max: 120, warn: 104, crit: 110, unit: 'C' },
      oil: { v: Math.round(oil), min: 15, max: 80, unit: 'psi' },
      battery: { v: Math.round(batt * 10) / 10, min: 11.5, max: 14.8, unit: 'V' },
      fuel: { v: 63, low: 15, unit: '%' },
      outputs: { power: s.kp >= 1, glow: glowActive, starter: cranking, lights: s.lights },
      faults: { oil: s.running && oil < 15, temp: this.temp > 104, battery: batt < 11.5, fuel: false },
      glow: { active: glowActive, countdown: cd },
      net: { wifi: true, ip: '192.168.1.177', rssi: Math.round(-84 + Math.sin(now / 3) * 3), apClients: 1, mqtt: true, clock: '--:--' }
    };
    if (seq === 3 && !s.maint) d.crankBlock = 'starter blocked - enable Maintenance or wire seat+neutral';
    return d;
  }
  control(b) {
    const s = this.s, now = this.now();
    switch (b.action) {
      case 'key': {
        if (b.position === 3) return { ok: false, error: 'position must be 0..2, crank via hold', _code: 400 };
        if (b.position === 2 && s.kp !== 2) s.glow0 = now;
        if (b.position === 0) s.running = false;
        s.kp = b.position; s.held = false; return { ok: true };
      }
      case 'crank':
        if (b.held) {
          if (!s.held) { s.kp = 3; s.held = true; if (s.glow0 == null || now - s.glow0 > 20) s.glow0 = now; }
          s.hb = now;
          return s.maint ? { ok: true, starter: true } : { ok: true, starter: false, note: 'interlocks not satisfied (seat bar + neutral)' };
        }
        s.held = false; if (s.kp >= 3) s.kp = 1; return { ok: true };
      case 'stop': s.held = false; s.kp = 1; s.running = false; s.glow0 = null; return { ok: true };
      case 'lights': s.lights = !s.lights; return { ok: true };
      case 'maintenance': s.maint = !!b.enabled; return { ok: true };
    }
    return { ok: false, error: 'unknown action', _code: 400 };
  }
  raw() {
    const st = this.status();
    return { battery_raw: Math.round(st.battery.v / 0.00526), temperature_raw: Math.round((150 - st.engineTemp.v) / 0.04),
      oil_raw: Math.round(st.oil.v / 0.0195), hydraulic_raw: Math.round(st.hydraulic.v / 0.7326), fuel_raw: 200 + Math.round(36 * st.fuel.v),
      battery_calculated: st.battery.v, temperature_calculated: st.engineTemp.v, oil_calculated: st.oil.v,
      hydraulic_calculated: st.hydraulic.v, fuel_calculated: st.fuel.v };
  }
  settings() { return { engine: { glow: 20, crank: 10 }, thresholds: { maxTemp: 104, minOil: 15, minHyd: 800, minV: 11.5, maxV: 14.8 } }; }
}

/* ---------- backend selection ---------- */
const Backend = {
  sim: null, demo: false,
  async init() {
    if (location.search.includes('sim')) return this.useSim();
    try {
      const c = new AbortController(); const t = setTimeout(() => c.abort(), 1500);
      const r = await fetch('/api/status', { signal: c.signal, cache: 'no-store' }); clearTimeout(t);
      if (!r.ok) throw 0;
      const j = await r.json();                       // a portal login page is not a controller
      if (typeof j.seq !== 'number' || !j.battery) throw 0;
    } catch (e) { this.useSim(); }
  },
  useSim() { this.sim = new Sim(); this.demo = true; },
  async status() { if (this.sim) return this.sim.status(); const r = await fetch('/api/status'); if (!r.ok) throw 0; return r.json(); },
  async raw() { if (this.sim) return this.sim.raw(); const r = await fetch('/api/raw-sensors'); return r.ok ? r.json() : null; },
  async settings() { if (this.sim) return this.sim.settings(); const r = await fetch('/api/settings'); return r.ok ? r.json() : null; },
  async control(body) {
    if (this.sim) { const j = this.sim.control(body); if (!j.ok) toast(j.error); return { ok: j.ok, j }; }
    try {
      const r = await fetch('/api/control', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });
      const j = await r.json().catch(() => null);
      if (!r.ok) toast(j && j.error ? j.error : 'Command failed');
      return { ok: r.ok, j };
    } catch (e) { toast('No link to the controller'); return { ok: false }; }
  }
};

let toastT = null;
function toast(msg) { const t = $('toast'); t.textContent = msg; t.classList.add('show'); clearTimeout(toastT); toastT = setTimeout(() => t.classList.remove('show'), 2500); }
function vib(p) { if (navigator.vibrate) navigator.vibrate(p); }

/* ---------- dial geometry ---------- */
const CX = 136, CY = 136, R = 112;
const pol = (r, a) => { const t = a * Math.PI / 180; return [CX + r * Math.sin(t), CY - r * Math.cos(t)]; };
function arcPath(r, a0, a1) { const [x0, y0] = pol(r, a0), [x1, y1] = pol(r, a1); return `M ${x0} ${y0} A ${r} ${r} 0 ${a1 - a0 > 180 ? 1 : 0} 1 ${x1} ${y1}`; }
function buildDial() {
  const NS = 'http://www.w3.org/2000/svg';
  const ticks = $('ticks'), labels = $('labels');
  for (const k of ORDER) {
    const [x, y] = pol(R, POS[k].a);
    const c = document.createElementNS(NS, 'circle'); c.setAttribute('cx', x); c.setAttribute('cy', y); c.setAttribute('r', 5);
    c.setAttribute('class', 'tick'); c.setAttribute('fill', 'var(--ground)'); c.dataset.k = k; ticks.appendChild(c);
    const a = POS[k].a, anchor = a >= 60 ? 'start' : a <= -20 ? 'end' : 'middle';
    const [lx, ly] = pol(anchor === 'middle' ? R + 22 : R + 14, a);
    const t = document.createElementNS(NS, 'text'); t.setAttribute('x', lx); t.setAttribute('y', ly); t.setAttribute('class', 'lbl'); t.setAttribute('text-anchor', anchor);
    t.textContent = k.toUpperCase(); t.dataset.k = k; labels.appendChild(t);
  }
  const glow = $('glowarc'); glow.setAttribute('d', arcPath(R, POS.glow.a, POS.start.a));
  const len = R * (POS.start.a - POS.glow.a) * Math.PI / 180;
  glow.style.strokeDasharray = len; glow.style.strokeDashoffset = len; glow.dataset.len = len;
  $('runarc').setAttribute('d', arcPath(R, POS.off.a, POS.start.a));
}
function paintDial(st, uiState) {
  const heating = st.glow && st.glow.active, cranking = !!(st.outputs && st.outputs.starter);
  document.querySelectorAll('#ticks .tick, #labels .lbl').forEach(el => {
    const k = el.dataset.k;
    el.classList.toggle('on', k === uiState);
    el.classList.toggle('heat', k === 'glow' && heating);
    el.classList.toggle('crank', k === 'start' && cranking);
  });
  const g = $('glowarc'), len = +g.dataset.len;
  const total = (glowSeconds || 20);
  const frac = heating ? 1 - Math.min(st.glow.countdown, total) / total : 0;
  g.style.strokeDashoffset = len * (1 - frac);
  $('dial').classList.toggle('running', st.seq === 4);
  const u = $('under'); u.className = 'under';
  if (st.seq === 4) { u.textContent = st.state === 'RUNNING' ? 'Running' : st.state === 'LOW_OIL' ? 'Running · low oil' : 'Running · hot'; u.classList.add('run'); }
  else if (cranking) { u.textContent = 'Cranking'; u.classList.add('crank'); }
  else if (heating) { u.textContent = st.glow.countdown > 0 ? `Preheating ${st.glow.countdown} s` : 'Preheated'; u.classList.add('heat'); }
  else u.textContent = '';
}

/* ---------- drag key ---------- */
class Key {
  constructor(el) {
    this.el = el; this.state = 'off'; this.angle = POS.off.a; this.drag = false; this.start = 0; this.busy = false; this.hb = null;
    el.addEventListener('pointerdown', e => this.down(e));
    document.addEventListener('pointermove', e => this.move(e));
    document.addEventListener('pointerup', e => this.up(e));
    document.addEventListener('pointercancel', e => this.up(e));
    el.addEventListener('contextmenu', e => e.preventDefault());
    el.addEventListener('keydown', e => this.keydown(e));
    el.addEventListener('keyup', e => this.keyup(e));
    this.paint();
  }
  center() { const r = this.el.getBoundingClientRect(); return [r.left + r.width / 2, r.top + r.height / 2]; }
  ang(x, y) { const [cx, cy] = this.center(); let a = Math.atan2(y - cy, x - cx) * 180 / Math.PI + 90; while (a < 0) a += 360; while (a >= 360) a -= 360; return a; }
  down(e) {
    if (this.busy) return; e.preventDefault();
    try { this.el.setPointerCapture(e.pointerId); } catch (_) {}
    this.drag = true; this.el.style.cursor = 'grabbing';
    this.start = this.ang(e.clientX, e.clientY) - this.angle; vib(10);
  }
  move(e) {
    if (!this.drag || this.busy) return; e.preventDefault();
    let t = this.ang(e.clientX, e.clientY) - this.start; while (t < 0) t += 360; while (t >= 360) t -= 360; if (t > 180) t -= 360;
    const i = ORDER.indexOf(this.state);
    let lo = POS.off.a, hi = POS[ORDER[Math.min(i + 1, 3)]].a;
    if (t < this.angle) { lo = POS.off.a; hi = this.angle; }
    t = Math.max(lo, Math.min(hi, t));
    if (t === this.angle) return;
    this.angle = t; this.paint();
    const s = t < -15 ? 'off' : t < 22.5 ? 'on' : t < 67.5 ? 'glow' : 'start';
    if (s !== this.state) { this.state = s; this.send(s); vib(15); paintDial(lastStatus || emptyStatus(), s); }
  }
  up() { if (!this.drag) return; this.drag = false; this.el.style.cursor = 'grab'; this.snap(); }
  snap() {
    this.busy = true;
    let best = 'off', d0 = 1e9;
    for (const k of ORDER) { const d = Math.abs(this.angle - POS[k].a); if (d < d0) { d0 = d; best = k; } }
    const changed = best !== this.state;
    this.angle = POS[best].a; this.state = best;
    this.el.classList.add('transitioning'); this.paint();
    if (changed) this.send(best);
    if (best === 'start') setTimeout(() => this.spring(), 300);
    else setTimeout(() => { this.el.classList.remove('transitioning'); this.busy = false; }, 200);
  }
  spring() {
    this.angle = POS.on.a; this.state = 'on'; this.paint();
    this.stopHb(); Backend.control({ action: 'crank', held: false });
    setTimeout(() => { this.el.classList.remove('transitioning'); this.busy = false; }, 200);
  }
  paint() { this.el.style.transform = `rotate(${this.angle}deg)`; this.el.setAttribute('aria-valuenow', POS[this.state].n); }
  send(s) {
    if (s === 'start') {
      Backend.control({ action: 'crank', held: true }).then(res => { if (res.ok && res.j && res.j.starter === false) blockReason = res.j.note || 'interlocks'; });
      this.startHb();
    } else { this.stopHb(); Backend.control({ action: 'key', position: POS[s].n }); }
  }
  startHb() { this.stopHb(); this.hb = setInterval(() => Backend.control({ action: 'crank', held: true }), HB_MS); }
  stopHb() { if (this.hb) { clearInterval(this.hb); this.hb = null; } }
  sync(seq) {
    if (this.drag || this.busy) return;
    const s = ['off', 'on', 'glow', 'start', 'on'][Math.min(seq, 4)];
    if (s === this.state) return;
    this.state = s; this.angle = POS[s].a;
    this.el.classList.add('transitioning'); this.paint();
    setTimeout(() => this.el.classList.remove('transitioning'), 200);
  }
  keydown(e) {
    if (e.repeat) return;
    const i = ORDER.indexOf(this.state);
    if (e.key === 'ArrowRight' && i < 2) { this.state = ORDER[i + 1]; this.angle = POS[this.state].a; this.el.classList.add('transitioning'); this.paint(); this.send(this.state); }
    else if (e.key === 'ArrowLeft' && i > 0 && i < 3) { this.state = ORDER[i - 1]; this.angle = POS[this.state].a; this.el.classList.add('transitioning'); this.paint(); this.send(this.state); }
    else if (e.key === ' ' && i >= 1) { e.preventDefault(); this.state = 'start'; this.angle = POS.start.a; this.el.classList.add('transitioning'); this.paint(); this.send('start'); this.busy = true; }
  }
  keyup(e) { if (e.key === ' ' && this.state === 'start') { this.spring(); } }
}

/* ---------- rendering ---------- */
let lastStatus = null, blockReason = '', glowSeconds = 20, key = null, ovArmed = false, ovLeft = 0, ovTimer = null, linkLost = false;
function emptyStatus() { return { seq: 0, state: 'OFF', glow: { active: false, countdown: 0 }, outputs: {}, faults: {}, net: {} }; }

function zone(v, warn, crit, dir) {                  // dir: 'high' = bad when high, 'low' = bad when low
  if (dir === 'high') return v >= crit ? 'crit' : v >= warn ? 'warn' : '';
  return v <= crit ? 'crit' : v <= warn ? 'warn' : '';
}
function cell(id, val, unit, frac, z, tick, muted) {
  const c = $(id); c.className = 'cell' + (z ? ' ' + z : '') + (muted ? ' stale' : '');
  c.querySelector('.v').innerHTML = `${val}<small>${unit}</small>`;
  const i = c.querySelector('.bar i'); i.style.width = `${Math.max(0, Math.min(1, frac)) * 100}%`; i.className = z || (muted ? 'sage' : '');
  const em = c.querySelector('.bar em'); if (tick == null) em.style.display = 'none'; else { em.style.display = ''; em.style.left = `${tick * 100}%`; }
}
function renderPrime(st) {
  const b = st.battery, o = st.oil, t = st.engineTemp, f = st.fuel, running = st.seq === 4;
  cell('cBat', b.v.toFixed(1), 'V', (b.v - 9) / (15.5 - 9), b.v > b.max ? 'warn' : zone(b.v, b.min + 0.5, b.min, 'low'), (b.min - 9) / (15.5 - 9), false);
  cell('cOil', Math.round(o.v), 'psi', o.v / o.max, running ? zone(o.v, o.min + 5, o.min, 'low') : '', o.min / o.max, !running);
  cell('cTemp', Math.round(t.v), '°C', (t.v - t.min) / (t.max - t.min), zone(t.v, t.warn, t.crit, 'high'), (t.warn - t.min) / (t.max - t.min), false);
  cell('cFuel', Math.round(f.v), '%', f.v / 100, zone(f.v, f.low * 2, f.low, 'low'), f.low / 100, false);
}
function renderLamps(st) {
  const on = { oil: st.faults.oil, temp: st.faults.temp, battery: st.faults.battery, fuel: st.faults.fuel, charging: st.battery && st.battery.v > 13.0, running: st.seq === 4 };
  document.querySelectorAll('.lamp').forEach(l => l.classList.toggle('on', !!on[l.dataset.k]));
}
function renderStatus(st) {
  const el = $('status'); let txt, cls = '';
  const cranking = st.outputs && st.outputs.starter;
  switch (st.state) {
    case 'OFF': txt = 'Off'; break;
    case 'ON': txt = 'Standby'; cls = 'ready'; break;
    case 'GLOW': txt = st.glow.countdown > 0 ? `Preheating ${st.glow.countdown} s` : 'Preheated'; cls = 'heat'; break;
    case 'START': txt = cranking ? 'Cranking' : st.crankBlock ? 'Starter blocked' : 'Preheating'; cls = cranking ? 'crank' : st.crankBlock ? 'alert' : 'heat'; break;
    case 'RUNNING': txt = 'Running'; cls = 'run'; break;
    case 'LOW_OIL': txt = 'Low oil pressure'; cls = 'alert'; break;
    case 'HIGH_TEMP': txt = 'Engine too hot'; cls = 'alert'; break;
    default: txt = 'Fault'; cls = 'alert';
  }
  el.textContent = txt; el.className = 'status ' + cls;
}
function renderAlert(st) {
  const a = $('alert'); let txt = '', cls = '';
  if (linkLost) { txt = 'No link to the controller · retrying'; cls = 'clay'; }
  else if (st.maintenance) { txt = `Override armed · interlocks bypassed${ovLeft ? ' · ' + ovLeft + ' s' : ''} · tap to disarm`; cls = ''; }
  else if (st.seq === 3 && st.crankBlock) { txt = 'Starter blocked · seat bar or neutral · hold Override'; cls = 'clay'; }
  else if (st.faults.temp) { txt = `Engine too hot · ${Math.round(st.engineTemp.v)} °C`; cls = 'clay'; }
  else if (st.faults.oil) { txt = `Oil pressure low · ${Math.round(st.oil.v)} psi`; cls = 'clay'; }
  else if (st.faults.battery && st.seq > 0 && !(st.outputs && st.outputs.starter)) { txt = `Battery low · ${st.battery.v.toFixed(1)} V`; cls = ''; }
  else if (st.faults.fuel) { txt = `Fuel low · ${Math.round(st.fuel.v)} %`; cls = ''; }
  a.textContent = txt; a.className = 'alert' + (txt ? ' show' : '') + (cls ? ' ' + cls : '');
}

/* advanced */
const NS = 'http://www.w3.org/2000/svg';
function svgEl(n, a) { const e = document.createElementNS(NS, n); for (const k in a) e.setAttribute(k, a[k]); return e; }
function arcGauge(size, thick, zones, min, max) {
  const cx = size / 2, cy = size / 2, r = size / 2 - thick / 2 - 1, A0 = 225, SW = 270;
  const p = (a) => { const t = (a - 90) * Math.PI / 180; return [cx + r * Math.cos(t), cy + r * Math.sin(t)]; };
  const arc = (a0, a1) => { const [x0, y0] = p(a0), [x1, y1] = p(a1); return `M ${x0} ${y0} A ${r} ${r} 0 ${a1 - a0 > 180 ? 1 : 0} 1 ${x1} ${y1}`; };
  const svg = svgEl('svg', { width: size, height: size, viewBox: `0 0 ${size} ${size}` });
  svg.appendChild(svgEl('path', { d: arc(A0, A0 + SW), fill: 'none', stroke: 'var(--line)', 'stroke-width': thick, 'stroke-linecap': 'round' }));
  for (const z of zones) {
    const s = A0 + SW * (z.from - min) / (max - min), e = A0 + SW * (z.to - min) / (max - min);
    if (e > s) svg.appendChild(svgEl('path', { d: arc(s, e), fill: 'none', stroke: z.c, 'stroke-width': thick, opacity: z.dim ? .35 : .9 }));
  }
  const dot = svgEl('circle', { r: thick / 2 + 1.5, fill: 'var(--cell)', stroke: 'var(--ink)', 'stroke-width': 2 });
  svg.appendChild(dot);
  return { svg, set(v) { const a = A0 + SW * Math.max(0, Math.min(1, (v - min) / (max - min))); const [x, y] = p(a); dot.setAttribute('cx', x); dot.setAttribute('cy', y); } };
}
let gauges = null;
function buildGauges(st) {
  const g = $('gauges'); g.innerHTML = ''; gauges = {};
  const defs = [
    ['temp', 'Engine temp', st.engineTemp.unit === 'C' ? '°C' : st.engineTemp.unit, st.engineTemp.min, st.engineTemp.max,
      [{ from: st.engineTemp.min, to: st.engineTemp.warn, c: 'var(--leaf)' }, { from: st.engineTemp.warn, to: st.engineTemp.crit, c: 'var(--sun)' }, { from: st.engineTemp.crit, to: st.engineTemp.max, c: 'var(--clay)' }], `warn ${st.engineTemp.warn}, hot ${st.engineTemp.crit}`],
    ['oil', 'Oil pressure', 'psi', 0, st.oil.max, [{ from: 0, to: st.oil.min, c: 'var(--clay)' }, { from: st.oil.min, to: st.oil.min + 10, c: 'var(--sun)' }, { from: st.oil.min + 10, to: st.oil.max, c: 'var(--leaf)' }], `min ${st.oil.min}`],
    ['batt', 'Battery', 'V', 9, 15.5, [{ from: 9, to: st.battery.min, c: 'var(--clay)' }, { from: st.battery.min, to: st.battery.min + 0.8, c: 'var(--sun)' }, { from: st.battery.min + 0.8, to: st.battery.max, c: 'var(--leaf)' }, { from: st.battery.max, to: 15.5, c: 'var(--sun)' }], `${st.battery.min} to ${st.battery.max}`],
    ['fuel', 'Fuel', '%', 0, 100, [{ from: 0, to: st.fuel.low, c: 'var(--clay)' }, { from: st.fuel.low, to: st.fuel.low * 2, c: 'var(--sun)' }, { from: st.fuel.low * 2, to: 100, c: 'var(--leaf)' }], `low at ${st.fuel.low} %`]
  ];
  for (const [k, label, unit, min, max, zones, note] of defs) {
    const d = document.createElement('div'); d.className = 'gauge';
    const ag = arcGauge(58, 7, zones, min, max); d.appendChild(ag.svg);
    const t = document.createElement('div'); t.className = 'g';
    t.innerHTML = `<div class="k">${label}</div><div class="v"><span id="gv_${k}">--</span><small>${unit}</small></div><div class="r">${note}</div>`;
    d.appendChild(t); g.appendChild(d); gauges[k] = ag;
  }
}
function renderAdvanced(st) {
  if (!document.getElementById('app').classList.contains('advanced')) return;
  if (!gauges) buildGauges(st);
  gauges.temp.set(st.engineTemp.v); $('gv_temp').textContent = Math.round(st.engineTemp.v);
  gauges.oil.set(st.oil.v); $('gv_oil').textContent = Math.round(st.oil.v);
  gauges.batt.set(st.battery.v); $('gv_batt').textContent = st.battery.v.toFixed(1);
  gauges.fuel.set(st.fuel.v); $('gv_fuel').textContent = Math.round(st.fuel.v);
  // hydraulic sweep
  const h = st.hydraulic, sw = $('hydSweep'); const N = 24;
  if (sw.childElementCount !== N) { sw.innerHTML = ''; for (let i = 0; i < N; i++) sw.appendChild(document.createElement('b')); }
  $('hydV').innerHTML = `${Math.round(h.v)}<small>psi</small>`; $('hydMin').textContent = h.min; $('hydMax').textContent = h.max; $('hydRange').textContent = `min ${h.min} · max ${h.max} psi`;
  const lit = Math.round(N * Math.max(0, Math.min(1, h.v / h.max)));
  [...sw.children].forEach((b, i) => { const mid = (i + 0.5) / N * h.max; const z = mid < h.min ? 'crit' : mid < h.min * 1.3 ? 'warn' : ''; b.className = (i < lit ? 'lit ' : 'dim ') + z; });
  // outputs
  document.querySelectorAll('.out').forEach(o => { const on = !!st.outputs[o.dataset.k]; o.classList.toggle('on', on); o.querySelector('.s').lastChild.textContent = on ? 'On' : 'Off'; });
  // connection
  const n = st.net || {};
  $('netKv').innerHTML = kv([
    ['WiFi', n.wifi ? `${n.ip} · ${n.rssi} dBm` : 'not joined', n.wifi ? 'ok' : 'off'],
    ['Access point', `${n.apClients || 0} connected`, ''],
    ['Home Assistant', n.mqtt ? 'publishing' : 'off', n.mqtt ? 'ok' : 'off'],
    ['Clock', n.clock || '--:--', ''],
  ]);
  $('sesKv').innerHTML = kv([
    ['Engine hours', `${(st.engineHours || 0).toFixed(1)} h`, ''],
    ['Glow preheat', `${glowSeconds} s`, ''],
    ['Crank timeout', `${crankSeconds} s`, ''],
    ['Override', st.maintenance ? 'armed' : 'off', st.maintenance ? 'ok' : 'off'],
    ['Firmware', st.version || 'unknown', ''],
    ['Controller', Backend.demo ? 'simulated' : 'live', Backend.demo ? '' : 'ok'],
  ]);
}
function kv(rows) { return rows.map(([k, v, c]) => `<div><span>${k}</span><span class="${c}">${v}</span></div>`).join(''); }
function renderRaw(r) {
  if (!r) { $('rawKv').innerHTML = kv([['Raw sensors', 'not available', 'off']]); return; }
  $('rawKv').innerHTML = kv([
    ['Battery', `${r.battery_raw} → ${(+r.battery_calculated).toFixed(2)} V`, ''],
    ['Engine temp', `${r.temperature_raw} → ${Math.round(r.temperature_calculated)} °C`, ''],
    ['Oil', `${r.oil_raw} → ${Math.round(r.oil_calculated)} psi`, ''],
    ['Hydraulic', `${r.hydraulic_raw} → ${Math.round(r.hydraulic_calculated)} psi`, ''],
    ['Fuel', `${r.fuel_raw} → ${Math.round(r.fuel_calculated)} %`, ''],
  ]);
}
function renderFoot(st) {
  const n = st.net || {};
  $('foot').innerHTML = `${(st.engineHours || 0).toFixed(1)} engine hours · ${n.wifi ? `WiFi ${n.rssi} dBm` : 'AP only'} · ${st.version || ''}${Backend.demo ? ' · simulated controller' : ''}<br><a href="settings.html">Settings</a>`;
  $('btnLights').classList.toggle('on', !!(st.outputs && st.outputs.lights));
  $('btnLights').querySelector('small').textContent = st.outputs && st.outputs.lights ? 'on' : 'off';
}
function render(st) {
  lastStatus = st;
  key.sync(st.seq);
  paintDial(st, key.state);
  renderStatus(st); renderAlert(st); renderPrime(st); renderLamps(st); renderAdvanced(st); renderFoot(st);
  if (st.maintenance && !ovArmed) armUi(); if (!st.maintenance && ovArmed) disarmUi();
}

/* ---------- override ---------- */
let ovStart = 0, ovRaf = null;
function ovDown(e) { e.preventDefault(); if (ovArmed) { disarm(); return; } ovStart = performance.now(); ovRaf = requestAnimationFrame(ovStep); }
function ovStep() { const p = Math.min((performance.now() - ovStart) / OV_HOLD_MS, 1); $('btnOv').style.setProperty('--p', p); if (p >= 1) { arm(); return; } ovRaf = requestAnimationFrame(ovStep); }
function ovUp() { if (ovRaf) cancelAnimationFrame(ovRaf); ovRaf = null; $('btnOv').style.setProperty('--p', 0); }
function arm() { ovUp(); Backend.control({ action: 'maintenance', enabled: true }); armUi(); vib([30, 40, 30]); }
function disarm() { Backend.control({ action: 'maintenance', enabled: false }); disarmUi(); }
function armUi() {
  ovArmed = true; ovLeft = OV_WINDOW_S; $('btnOv').classList.add('armed'); $('btnOv').querySelector('small').textContent = 'armed · tap to disarm';
  clearInterval(ovTimer); ovTimer = setInterval(() => { ovLeft--; if (ovLeft <= 0) disarm(); }, 1000);
}
function disarmUi() { ovArmed = false; ovLeft = 0; clearInterval(ovTimer); ovTimer = null; $('btnOv').classList.remove('armed'); $('btnOv').querySelector('small').textContent = 'hold 2 s'; }

/* ---------- mode / theme ---------- */
function setMode(adv) {
  $('app').classList.toggle('advanced', adv); $('modeAdv').classList.toggle('on', adv); $('modeSimple').classList.toggle('on', !adv);
  try { localStorage.setItem('bobcat.mode', adv ? 'advanced' : 'simple'); } catch (_) {}
  if (adv && lastStatus) { gauges = null; renderAdvanced(lastStatus); }
}
function setTheme(t) { document.documentElement.dataset.theme = t; $('btnTheme').textContent = t === 'day' ? 'Night theme' : 'Day theme'; try { localStorage.setItem('bobcat.theme', t); } catch (_) {} }

/* ---------- loops ---------- */
async function poll() {
  try { const st = await Backend.status(); if (linkLost) { linkLost = false; } render(st); }
  catch (e) { if (!linkLost) { linkLost = true; renderAlert(lastStatus || emptyStatus()); $('status').textContent = 'No link'; $('status').className = 'status alert'; } }
  setTimeout(poll, POLL_MS);
}
async function pollRaw() {
  if ($('app').classList.contains('advanced')) { try { renderRaw(await Backend.raw()); } catch (e) { renderRaw(null); } }
  setTimeout(pollRaw, RAW_MS);
}
let crankSeconds = 10;
document.addEventListener('DOMContentLoaded', async () => {
  buildDial();
  key = new Key($('key'));
  $('btnStop').addEventListener('click', () => { key.stopHb(); Backend.control({ action: 'stop' }); vib(50); });
  $('btnLights').addEventListener('click', () => Backend.control({ action: 'lights' }));
  const ov = $('btnOv'); ov.addEventListener('pointerdown', ovDown); ov.addEventListener('pointerup', ovUp); ov.addEventListener('pointercancel', ovUp); ov.addEventListener('pointerleave', ovUp);
  $('alert').addEventListener('click', () => { if (lastStatus && lastStatus.maintenance) disarm(); });
  $('modeSimple').addEventListener('click', () => setMode(false)); $('modeAdv').addEventListener('click', () => setMode(true));
  $('btnTheme').addEventListener('click', () => setTheme(document.documentElement.dataset.theme === 'day' ? 'night' : 'day'));
  let mode = 'simple', theme = 'night';
  try { mode = localStorage.getItem('bobcat.mode') || mode; theme = localStorage.getItem('bobcat.theme') || theme; } catch (_) {}
  if (location.search.includes('advanced')) mode = 'advanced';
  if (location.search.includes('day')) theme = 'day';
  setTheme(theme); setMode(mode === 'advanced');
  await Backend.init();
  try { const s = await Backend.settings(); if (s && s.engine) { glowSeconds = s.engine.glow || 20; crankSeconds = s.engine.crank || 10; } } catch (_) {}
  poll(); pollRaw();
});
