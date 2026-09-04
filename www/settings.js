var settingsData={},pinsData=null;

function byId(id){return document.getElementById(id);}

function showToast(msg,ok){
  var t=byId('toast');
  if(!t){t=document.createElement('div');t.className='toast';t.id='toast';document.body.appendChild(t);}
  t.textContent=msg;t.style.background=ok?'var(--accent)':'var(--crit)';t.classList.add('show');
  setTimeout(function(){t.classList.remove('show');},3000);
}

function saveSetting(key,value){
  return fetch('/api/settings',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({key:key,value:String(value)})})
  .then(function(r){if(!r.ok)throw new Error('save failed');return r.json();});
}

function setInpVal(id,val){
  var el=byId(id);
  if(el){if(el.classList.contains('toggle')){el.classList.toggle('on',!!val);}else{el.value=val!=null?val:'';}}
}

function getInpVal(id){
  var el=byId(id);
  if(!el)return null;
  if(el.classList.contains('toggle'))return el.classList.contains('on');
  return el.value;
}

function applySettings(s){
  if(!s)return;
  setInpVal('sGlow',s.engine?s.engine.glow:null);
  setInpVal('sCrank',s.engine?s.engine.crank:null);
  setInpVal('sCooldown',s.engine?s.engine.cooldown:null);
  setInpVal('sTempScale',s.cal?s.cal.tempScale:null);
  setInpVal('sMinOil',s.thresholds?s.thresholds.minOil:null);
  setInpVal('sMinHyd',s.thresholds?s.thresholds.minHyd:null);
  setInpVal('sMinV',s.thresholds?s.thresholds.minV:null);
  setInpVal('sFuelLow',s.cal?s.cal.fuelLow:null);
  setInpVal('sMqttEnabled',s.mqtt?s.mqtt.enabled:null);
  setInpVal('sMqttHost',s.mqtt?s.mqtt.host:null);
  setInpVal('sMqttPort',s.mqtt?s.mqtt.port:null);
  setInpVal('sMqttTopic',s.mqtt?s.mqtt.topic:null);
  setInpVal('sNtp',s.time?s.time.ntp:null);
  setInpVal('sUtcOffset',s.time?s.time.utcOffset:null);
  setInpVal('sApSsid',s.wifi&&s.wifi.ap?s.wifi.ap.ssid:null);
  renderWifiNetworks(s.wifi?s.wifi.networks:null);
  updateSysStrip(s);
}

function updateSysStrip(s){
  if(!s||!s.wifi){byId('cfgWifiInfo').textContent='--';byId('cfgApInfo').textContent='--';byId('cfgMqttInfo').textContent='--';byId('cfgClockInfo').textContent='--';return;}
  byId('cfgWifiInfo').textContent=s.wifi.ip||'connected';byId('cfgApInfo').textContent=s.wifi.apClients>0?s.wifi.apClients+' client':'on';
  byId('cfgMqttInfo').textContent=s.mqtt&&s.mqtt.enabled?'up':'off';byId('cfgClockInfo').textContent=s.time&&s.time.ntp?'ok':'--';
}

function renderWifiNetworks(nets){
  var el=byId('wifiNetworks');el.innerHTML='';
  if(!nets||!nets.length)return;
  for(var i=0;i<nets.length;i++){
    var n=nets[i];var d=document.createElement('div');d.className='netitem';
    var label=n.ssid||n;var status=n.connected?' <span style="color:var(--accent)">· connected</span>':'';
    d.innerHTML='<span>'+label+status+'</span><span class="x" data-ssid="'+(n.ssid||n)+'">✕</span>';
    el.appendChild(d);
    d.querySelector('.x').addEventListener('click',function(){
      showToast('Save a new SSID above to change home WiFi',false);
    });
  }
}

function initSettingsListeners(){
  var inputs=document.querySelectorAll('.card .inp');
  for(var i=0;i<inputs.length;i++){
    inputs[i].addEventListener('change',function(e){
      var map={
        sGlow:'glow',sCrank:'crank',sCooldown:'cooldown',
        sTempScale:'tempScale',sMinOil:'minOil',sMinHyd:'minHyd',sMinV:'minV',sFuelLow:'fuelLow',
        sMqttHost:'mqttHost',sMqttPort:'mqttPort',sMqttTopic:'mqttTopic',
        sUtcOffset:'utcOffset',sApSsid:'apSSID',sApPass:'apPassword'
      };
      var key=map[e.target.id];if(!key)return;
      saveSetting(key,e.target.value).then(function(r){if(r.ok)showToast('Saved',true);else showToast('Save failed',false);}).catch(function(){showToast('Save failed',false);});
    });
  }
  var toggles=document.querySelectorAll('.card .toggle');
  for(var i=0;i<toggles.length;i++){
    toggles[i].addEventListener('click',function(e){
      this.classList.toggle('on');
      var on=this.classList.contains('on');
      if(this.id==='sMaint'){
        fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({action:'maintenance',enabled:on})})
        .then(function(r){if(r.ok)showToast(on?'MAINTENANCE ON — interlocks bypassed':'Maintenance OFF',true);else showToast('Failed',false);})
        .catch(function(){showToast('Failed',false);});
        return;
      }
      var key=this.id==='sMqttEnabled'?'mqttEnabled':'ntp';
      saveSetting(key,on?'true':'false').then(function(r){if(!r.ok)showToast('Save failed',false);}).catch(function(){showToast('Save failed',false);});
    });
  }
  byId('btnAddNetwork').addEventListener('click',function(){
    var ssid=byId('wifiSsid').value.trim();
    var pass=byId('wifiPass').value;
    if(!ssid){showToast('Enter an SSID',false);return;}
    saveSetting('wifiSSID',ssid)
      .then(function(){return pass?saveSetting('wifiPassword',pass):{ok:true};})
      .then(function(){showToast('Home WiFi saved — joining…',true);byId('wifiSsid').value='';byId('wifiPass').value='';loadSettings();})
      .catch(function(){showToast('Save failed',false);});
  });

  var rf=byId('restoreFile');
  if(rf){rf.addEventListener('change',function(){
    var f=this.files[0];if(!f)return;
    var reader=new FileReader();
    reader.onload=function(){
      fetch('/api/restore',{method:'POST',headers:{'Content-Type':'application/json'},body:reader.result})
      .then(function(r){return r.json();})
      .then(function(r){if(r.ok){showToast('Config restored ('+r.applied+' sections) — reboot to apply pins/wifi',true);loadSettings();loadPins();}else showToast('Restore failed',false);})
      .catch(function(){showToast('Restore failed',false);});
    };
    reader.readAsText(f);this.value='';
  });}
}

function initPinmap(){
  var pm=byId('pinmap');pm.innerHTML='<div class="hint" style="text-align:center;color:var(--dim)">Loading pin map...</div>';
}

function renderPinmap(p){
  if(!p||!p.map)return;
  var pm=byId('pinmap');pm.innerHTML='';
  var used={};
  for(var i=0;i<p.map.length;i++){var m=p.map[i];if(!m.fixed)used[m.gpio]=(used[m.gpio]||0)+1;}
  for(var i=0;i<p.map.length;i++){
    var m=p.map[i];
    var row=document.createElement('div');row.className='pinrow';
    var pool,sensorType='';
    if(m.type==='relay')pool=p.relays.slice();
    else if(m.type==='adc'){pool=p.adc1.slice();sensorType='adc';}
    else pool=p.header.slice();
    if(pool.indexOf(m.gpio)===-1)pool.push(m.gpio);
    pool.sort(function(a,b){return a-b;});
    var opts='';var dup=!m.fixed&&used[m.gpio]&&used[m.gpio]>1;
    for(var j=0;j<pool.length;j++){
      var g=pool[j];var sel=g===m.gpio?' selected':'';
      var du=!m.fixed&&used[g]&&used[g]>1;
      opts+='<option value="'+g+'"'+sel+(du?' class="dup"':'')+'>GPIO '+g+'</option>';
    }
    var cls='pinsel'+(p.strap.indexOf(m.gpio)!==-1?' warn':'')+(dup?' dup':'');
    var sel='<select class="'+cls+'" data-func="'+m.func+'"'+(m.fixed?' disabled':'')+'>'+opts+'</select>';
    var note=m.type==='adc'?'ADC1':(m.type==='relay'?'onboard':'digital');
    if(p.strap.indexOf(m.gpio)!==-1)note+=' · strapping';
    if(dup)note+=' · <b class="c">dup</b>';
    row.innerHTML='<div class="fn"><b>'+m.label+'</b><small>'+note+'</small></div>'+sel;
    pm.appendChild(row);
    if(!m.fixed){
      row.querySelector('select').addEventListener('change',function(e){
        var func=this.getAttribute('data-func'),gpio=parseInt(this.value);
        savePin(func,gpio);
      });
    }
  }
}

function savePin(func,gpio){
  fetch('/api/pins',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({func:func,gpio:gpio})})
  .then(function(r){if(r.ok){showToast('Pin mapped',true);loadPins();}else showToast('Pin save failed',false);})
  .catch(function(){showToast('Network error',false);});
}

function loadSettings(){
  fetch('/api/settings').then(function(r){if(!r.ok)throw new Error('fetch');return r.json();})
  .then(function(s){settingsData=s;applySettings(s);})
  .catch(function(){showToast('Could not load settings',false);});
}

function loadPins(){
  fetch('/api/pins').then(function(r){if(!r.ok)throw new Error('fetch');return r.json();})
  .then(function(p){pinsData=p;renderPinmap(p);})
  .catch(function(){byId('pinmap').innerHTML='<div class="hint" style="text-align:center;color:var(--crit)">Could not load pin map</div>';});
}

document.addEventListener('DOMContentLoaded',function(){
  initPinmap();
  initSettingsListeners();
  loadSettings();
  loadPins();
  fetch('/api/status').then(function(r){return r.json();}).then(function(s){var m=byId('sMaint');if(m)m.classList.toggle('on',!!s.maintenance);}).catch(function(){});
});
