/* Bobcat 743 · settings page. Same /api contract as the dashboard. Values are sent with their
 * real JSON types (the firmware reads any string as true, so booleans must be booleans). */
'use strict';
const $ = id => document.getElementById(id);
let toastT = null;
function toast(msg, bad) { const t = $('toast'); t.textContent = msg; t.className = 'toast show' + (bad ? ' bad' : ''); clearTimeout(toastT); toastT = setTimeout(() => t.classList.remove('show'), 2200); }
function fetchT(url, opts, ms) { const c = new AbortController(); const t = setTimeout(() => c.abort(), ms || 5000); return fetch(url, Object.assign({ cache: 'no-store', signal: c.signal }, opts || {})).finally(() => clearTimeout(t)); }
function post(url, body) { return fetchT(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }).then(async r => { const j = await r.json().catch(() => null); if (!r.ok) throw new Error(j && j.error ? j.error : 'failed'); return j; }); }

// field id -> api key, and how to type the value
const NUM = { sGlow: 'glow', sCrank: 'crank', sCooldown: 'cooldown', sMaxTemp: 'maxTemp', sMinOil: 'minOil', sMinHyd: 'minHyd', sMinV: 'minV', sMaxV: 'maxV', sFuelLow: 'fuelLow',
  sBattDiv: 'batteryDivider', sTempScale: 'tempScale', sOilScale: 'oilScale', sHydScale: 'hydScale', sFuelEmpty: 'fuelEmpty', sFuelFull: 'fuelFull', sMqttPort: 'mqttPort' };
const STR = { sMqttHost: 'mqttHost', sMqttTopic: 'mqttTopic', sApSsid: 'apSSID', sApPass: 'apPassword' };
const BOOL = { sMqttEnabled: 'mqttEnabled' };

function save(key, value) {
  return post('/api/settings', { key, value }).then(() => toast('Saved')).catch(e => toast(e.message === 'failed' ? 'Not saved' : 'Not saved · ' + e.message, true));
}
function set(id, v) { const el = $(id); if (!el) return; if (el.classList.contains('toggle')) el.classList.toggle('on', !!v); else if (v != null && v !== '') el.value = v; }

function applySettings(s) {
  if (!s) return;
  const e = s.engine || {}, t = s.thresholds || {}, c = s.cal || {}, m = s.mqtt || {}, w = s.wifi || {};
  set('sGlow', e.glow); set('sCrank', e.crank); set('sCooldown', e.cooldown);
  set('sMaxTemp', t.maxTemp); set('sMinOil', t.minOil); set('sMinHyd', t.minHyd); set('sMinV', t.minV); set('sMaxV', t.maxV); set('sFuelLow', c.fuelLow);
  set('sTempScale', c.tempScale); set('sOilScale', c.oilScale); set('sHydScale', c.hydScale); set('sFuelEmpty', c.fuelEmpty); set('sFuelFull', c.fuelFull);
  set('sMqttEnabled', m.enabled); set('sMqttHost', m.host); set('sMqttPort', m.port); set('sMqttTopic', m.topic);
  set('sApSsid', w.ap && w.ap.ssid);
  const nets = $('wifiNetworks'); nets.innerHTML = '';
  (w.networks || []).forEach(n => { const d = document.createElement('div'); d.className = 'netitem'; d.innerHTML = `<span>${n.ssid || n}</span><span class="ok" id="netState">saved</span>`; nets.appendChild(d); });
}
function applyRaw(r) {
  if (!r) return;
  $('rawBatt').textContent = `${r.battery_raw} counts → ${(+r.battery_calculated).toFixed(2)} V`;
  $('rawTemp').textContent = `${r.temperature_raw} counts → ${Math.round(r.temperature_calculated)} °C`;
  $('rawOil').textContent = `${r.oil_raw} counts → ${Math.round(r.oil_calculated)} psi`;
  $('rawHyd').textContent = `${r.hydraulic_raw} counts → ${Math.round(r.hydraulic_calculated)} psi`;
  $('rawFuel').textContent = `${r.fuel_raw} counts → ${Math.round(r.fuel_calculated)} %`;
  if (!$('sBattDiv').value && r.battery_divider) $('sBattDiv').value = r.battery_divider;
}
function applyStatus(st) {
  if (!st) return;
  const n = st.net || {};
  $('live').textContent = `${st.state === 'OFF' ? 'Off' : st.state === 'ON' ? 'Standby' : st.state.toLowerCase()} · ${n.wifi ? n.ip + ' · ' + n.rssi + ' dBm' : 'AP only'}`;
  $('sMaint').classList.toggle('on', !!st.maintenance);
  const ns = $('netState'); if (ns) { ns.textContent = n.wifi ? 'connected · ' + n.ip : 'not connected'; ns.style.color = n.wifi ? '' : 'var(--sun)'; }
  $('fwKv').innerHTML = [['Firmware', st.version || 'no version string'], ['Engine hours', `${(st.engineHours || 0).toFixed(1)} h`], ['Access point clients', n.apClients || 0], ['Home Assistant', n.mqtt ? 'publishing' : 'off']]
    .map(([k, v]) => `<div><span>${k}</span><span>${v}</span></div>`).join('');
}

function renderPinmap(p) {
  if (!p || !p.map) return;
  const pm = $('pinmap'); pm.innerHTML = '';
  const used = {}; p.map.forEach(m => { if (m.type !== 'relay') used[m.gpio] = (used[m.gpio] || 0) + 1; });
  p.map.forEach(m => {
    const fixed = m.type === 'relay';
    let pool = m.type === 'relay' ? p.relays.slice() : m.type === 'adc' ? p.adc1.slice() : p.header.slice();
    if (pool.indexOf(m.gpio) === -1) pool.push(m.gpio); pool.sort((a, b) => a - b);
    const dup = !fixed && used[m.gpio] > 1;
    const opts = pool.map(g => `<option value="${g}"${g === m.gpio ? ' selected' : ''}>GPIO ${g}</option>`).join('');
    const cls = 'pinsel' + (p.strap.indexOf(m.gpio) !== -1 ? ' warn' : '') + (dup ? ' dup' : '');
    let note = m.type === 'adc' ? 'sensor · ADC1' : m.type === 'relay' ? 'onboard relay · fixed' : m.type === 'digital-in' ? 'switch input' : 'output';
    if (p.strap.indexOf(m.gpio) !== -1) note += ' · strapping pin'; if (dup) note += ' · used twice';
    const row = document.createElement('div'); row.className = 'pinrow';
    row.innerHTML = `<div class="fn"><b>${m.label}</b><small>${note}</small></div><select class="${cls}" data-func="${m.func}"${fixed ? ' disabled' : ''}>${opts}</select>`;
    pm.appendChild(row);
    if (!fixed) row.querySelector('select').addEventListener('change', function () {
      post('/api/pins', { func: this.dataset.func, gpio: parseInt(this.value, 10) }).then(() => { toast('Pin saved · applies after power cycle'); loadPins(); }).catch(e => { toast(e.message, true); loadPins(); });
    });
  });
}

function loadSettings() { fetchT('/api/settings').then(r => r.json()).then(applySettings).catch(() => toast('Could not load settings', true)); }
function loadPins() { fetchT('/api/pins').then(r => r.json()).then(renderPinmap).catch(() => { $('pinmap').innerHTML = '<div class="hint">Could not load the pin map</div>'; }); }
function loadRaw() { fetchT('/api/raw-sensors', null, 3000).then(r => r.json()).then(applyRaw).catch(() => {}); setTimeout(loadRaw, 2000); }
function loadStatus() { fetchT('/api/status', null, 3000).then(r => r.json()).then(applyStatus).catch(() => { $('live').textContent = 'No link'; }); setTimeout(loadStatus, 2000); }

function setTheme(t) { document.documentElement.dataset.theme = t; $('btnTheme').textContent = t === 'day' ? 'Night theme' : 'Day theme'; try { localStorage.setItem('bobcat.theme', t); } catch (_) {} }

document.addEventListener('DOMContentLoaded', () => {
  let theme = 'night'; try { theme = localStorage.getItem('bobcat.theme') || theme; } catch (_) {}
  setTheme(theme);
  $('btnTheme').addEventListener('click', () => setTheme(document.documentElement.dataset.theme === 'day' ? 'night' : 'day'));

  Object.keys(NUM).forEach(id => { const el = $(id); if (el) el.addEventListener('change', () => { const v = parseFloat(el.value); if (isNaN(v)) return; save(NUM[id], v); }); });
  Object.keys(STR).forEach(id => { const el = $(id); if (el) el.addEventListener('change', () => save(STR[id], el.value)); });
  Object.keys(BOOL).forEach(id => { const el = $(id); if (el) el.addEventListener('click', () => { el.classList.toggle('on'); save(BOOL[id], el.classList.contains('on')); }); });

  $('sMaint').addEventListener('click', function () {
    const on = !this.classList.contains('on'); this.classList.toggle('on', on);
    post('/api/control', { action: 'maintenance', enabled: on }).then(() => toast(on ? 'Interlocks bypassed' : 'Interlocks active')).catch(e => toast(e.message, true));
  });
  $('btnAddNetwork').addEventListener('click', () => {
    const ssid = $('wifiSsid').value.trim(), pass = $('wifiPass').value;
    if (!ssid) { toast('Enter the network name', true); return; }
    post('/api/settings', { key: 'wifiSSID', value: ssid }).then(() => pass ? post('/api/settings', { key: 'wifiPassword', value: pass }) : null)
      .then(() => { toast('Saved · joining ' + ssid); $('wifiSsid').value = ''; $('wifiPass').value = ''; setTimeout(loadSettings, 800); }).catch(e => toast(e.message, true));
  });
  $('restoreFile').addEventListener('change', function () {
    const f = this.files[0]; if (!f) return; const rd = new FileReader();
    rd.onload = () => fetchT('/api/restore', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: rd.result }).then(r => r.json())
      .then(j => { if (j.ok) { toast(`Restored ${j.applied} sections · power cycle for pins and WiFi`); loadSettings(); loadPins(); } else toast('Restore failed', true); }).catch(() => toast('Restore failed', true));
    rd.readAsText(f); this.value = '';
  });
  $('btnResetCal').addEventListener('click', () => { if (!confirm('Reset all sensor calibration to factory defaults?')) return; post('/api/reset-calibration', {}).then(() => { toast('Calibration reset'); loadSettings(); }).catch(e => toast(e.message, true)); });
  $('btnFactory').addEventListener('click', () => {
    if (!confirm('Factory reset: calibration, pin map and farm WiFi are erased and the controller reboots on its own access point. Continue?')) return;
    if (!confirm('Really erase everything?')) return;
    post('/api/factory-reset', {}).then(() => toast('Rebooting…')).catch(() => toast('Rebooting…'));
  });

  loadSettings(); loadPins(); loadRaw(); loadStatus();
});
