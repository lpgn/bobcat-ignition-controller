var NS="http://www.w3.org/2000/svg";
function el(n,a){var e=document.createElementNS(NS,n);for(var k in a)e.setAttribute(k,a[k]);return e;}
function polar(cx,cy,r,deg){var a=(deg-90)*Math.PI/180;return[cx+r*Math.cos(a),cy+r*Math.sin(a)];}
function arc(cx,cy,r,a0,a1){var p0=polar(cx,cy,r,a0),p1=polar(cx,cy,r,a1);var large=(a1-a0)%360>180?1:0;return"M "+p0[0]+" "+p0[1]+" A "+r+" "+r+" 0 "+large+" 1 "+p1[0]+" "+p1[1];}
var A0=225,SWEEP=270;
function gauge(size,val,min,max,zones,thick){
  var cx=size/2,cy=size/2,r=size/2-thick/2-1;
  var svg=el("svg",{width:size,height:size,viewBox:"0 0 "+size+" "+size});
  svg.appendChild(el("path",{d:arc(cx,cy,r,A0,A0+SWEEP),fill:"none",stroke:"#1b2430","stroke-width":thick,"stroke-linecap":"round"}));
  for(var i=0;i<zones.length;i++){var z=zones[i];var s=A0+SWEEP*(z.from-min)/(max-min),e=A0+SWEEP*(z.to-min)/(max-min);
    svg.appendChild(el("path",{d:arc(cx,cy,r,s,e),fill:"none",stroke:z.color,"stroke-width":thick,opacity:z.dim?0.32:0.9}));}
  for(var i=0;i<=8;i++){var a=A0+SWEEP*i/8;var t0=polar(cx,cy,r-thick/2-1,a),t1=polar(cx,cy,r-thick/2-4,a);
    svg.appendChild(el("line",{x1:t0[0],y1:t0[1],x2:t1[0],y2:t1[1],stroke:"#2c3646","stroke-width":1}));}
  var va=A0+SWEEP*Math.max(0,Math.min(1,(val-min)/(max-min)));var pp=polar(cx,cy,r,va);
  svg.appendChild(el("circle",{cx:pp[0],cy:pp[1],r:thick/2+1.5,fill:"#0b0e13",stroke:"#e9eef5","stroke-width":2}));
  return svg;
}

function makeGauge(size,min,max,zones,thick){
  var svg=gauge(size,min,min,max,zones,thick);
  var needle=svg.querySelector('circle');
  var cx=size/2,cy=size/2,r=size/2-thick/2-1;
  return{svg:svg,update:function(v){
    var va=A0+SWEEP*Math.max(0,Math.min(1,(v-min)/(max-min)));
    var p=polar(cx,cy,r,va);
    needle.setAttribute('cx',p[0]);needle.setAttribute('cy',p[1]);
  }};
}

function zonesFor(type,a,b,c,d){
  if(type==='hydraulic'){var min=a,max=b;var mid=min+(max-min)*0.13;
    return[{from:0,to:min,color:"#e0685a"},{from:min,to:mid,color:"#e0aa3a"},{from:mid,to:max,color:"#3ddc84"}];}
  if(type==='engineTemp'){var min=a,warn=b,crit=c,max=d;var blue=min+(warn-min)*0.5;
    return[{from:min,to:blue,color:"#57c7ff",dim:1},{from:blue,to:warn,color:"#3ddc84"},{from:warn,to:crit,color:"#e0aa3a"},{from:crit,to:max,color:"#e0685a"}];}
  if(type==='oil'){var min=a,max=b;var mid=min+10;
    return[{from:0,to:min,color:"#e0685a"},{from:min,to:mid,color:"#e0aa3a"},{from:mid,to:max,color:"#3ddc84"}];}
  if(type==='battery'){var min=a,max=b;
    return[{from:10,to:min,color:"#e0685a"},{from:min,to:min+0.9,color:"#e0aa3a"},{from:min+0.9,to:max-0.1,color:"#3ddc84"},{from:max-0.1,to:max,color:"#e0aa3a"}];}
  if(type==='fuel'){var low=a;
    return[{from:0,to:low,color:"#e0685a"},{from:low,to:low*2,color:"#e0aa3a"},{from:low*2,to:100,color:"#3ddc84"}];}
  return[];
}

var lastStatus=null,reconnecting=false,pollTimer=null,initialized=false;
var heroGauge=null,vitalGauges=[];

function byId(id){return document.getElementById(id);}

function showToast(msg){
  var t=byId('toast');t.textContent=msg;t.classList.add('show');
  setTimeout(function(){t.classList.remove('show');},3000);
}

function setCell(el,on,cls){
  el.classList.toggle('on',on&&cls==='on');
  el.classList.toggle('warn',on&&cls==='warn');
  el.classList.toggle('crit',on&&cls==='crit');
}

function updateStrip(net){
  var w=byId('wifiInfo'),wd=byId('wifiDot'),a=byId('apInfo'),ad=byId('apDot');
  var m=byId('mqttInfo'),md=byId('mqttDot'),c=byId('clockInfo'),cd=byId('clockDot');
  if(!net||reconnecting){
    w.textContent='reconnecting';w.className='sv recon';wd.className='';
    a.textContent='--';a.className='sv';ad.className='';
    m.textContent='--';m.className='sv';md.className='';
    c.textContent='--';c.className='sv';cd.className='mut';
    return;
  }
  w.textContent=net.ip+' · '+net.rssi+'dB';w.className='sv ok';wd.className='ok';
  a.textContent=net.apClients+' client'+(net.apClients===1?'':'s');a.className='sv info';ad.className='info';
  m.textContent=net.mqtt?'HA · up':'down';m.className=net.mqtt?'sv ok':'sv';md.className=net.mqtt?'ok':'';
  c.textContent=net.clock||'--:--';c.className='sv';cd.className='mut';
}

function updateSequence(seq){
  var steps=document.querySelectorAll('#seq .step');
  for(var i=0;i<steps.length;i++){
    var s=steps[i],idx=parseInt(s.getAttribute('data-idx'));
    s.classList.toggle('done',idx<seq);
    s.classList.toggle('active',idx===seq);
  }
}

function initHero(hyd){
  var hero=byId('hero');hero.innerHTML='';
  var z=zonesFor('hydraulic',hyd.min,hyd.max);
  var g=makeGauge(220,hyd.min,hyd.max,z,11);
  hero.appendChild(g.svg);
  heroGauge=g.update;
  var hc=document.createElement('div');hc.className='hc';
  var maxStr=hyd.max.toLocaleString();
  hc.innerHTML='<div class="hl">Hydraulic</div><div class="hv" id="heroVal">0</div><div class="hu" id="heroUnit">'+hyd.unit+'</div><div class="hd" id="heroDesc"><b>nominal</b> · max '+maxStr+'</div>';
  hero.appendChild(hc);
}

function updateHero(hyd){
  if(!heroGauge)initHero(hyd);
  heroGauge(hyd.v);
  byId('heroVal').textContent=Math.round(hyd.v);
}

function initVital(container,type,min,max,z,unit,label,rText,rCls,idx){
  var d=document.createElement('div');d.className='gauge';
  var g=makeGauge(54,min,max,z,6);
  d.appendChild(g.svg);
  var gg=document.createElement('div');gg.className='gg';
  gg.innerHTML='<span class="k">'+label+'</span><span class="v"><span id="vVal'+idx+'">0</span><small id="vUnit'+idx+'">'+unit+'</small></span><span class="r '+rCls+'" id="vR'+idx+'">'+rText+'</span>';
  d.appendChild(gg);
  container.appendChild(d);
  return{update:g.update,valEl:byId('vVal'+idx),rEl:byId('vR'+idx),idx:idx};
}

function initVitals(temp,oil,batt,fuel){
  var v=byId('vitals');v.innerHTML='';
  var tz=zonesFor('engineTemp',temp.min,temp.warn,temp.crit,temp.max);
  var oz=zonesFor('oil',oil.min,oil.max);
  var bz=zonesFor('battery',batt.min,batt.max);
  var fz=zonesFor('fuel',fuel.low);
  vitalGauges=[
    initVital(v,'temp',temp.min,temp.max,tz,temp.unit,'Engine Temp',temp.warn+'–'+temp.crit+' ok','ok',0),
    initVital(v,'oil',oil.min,oil.max,oz,oil.unit,'Oil Pressure','min '+oil.min,'ok',1),
    initVital(v,'batt',batt.min,batt.max,bz,batt.unit,'Battery','charging','ok',2),
    initVital(v,'fuel',0,100,fz,fuel.unit,'Fuel','low @ '+fuel.low+'%','ok',3)
  ];
}

function updateVitals(temp,oil,batt,fuel){
  if(!vitalGauges.length)initVitals(temp,oil,batt,fuel);
  vitalGauges[0].update(temp.v);byId('vVal0').textContent=Math.round(temp.v);
  vitalGauges[1].update(oil.v);byId('vVal1').textContent=Math.round(oil.v);
  vitalGauges[2].update(batt.v);byId('vVal2').textContent=batt.v.toFixed(1);
  vitalGauges[3].update(fuel.v);byId('vVal3').textContent=Math.round(fuel.v);
  var fR=byId('vR3');fR.textContent='low @ '+fuel.low+'%';fR.className='r'+(fuel.v<=fuel.low?' warn':' ok');
}

function updateOutputs(outs){
  var cells=document.querySelectorAll('#outputs .cell');
  for(var i=0;i<cells.length;i++){
    var c=cells[i],k=c.getAttribute('data-key'),on=!!outs[k];
    setCell(c,on,'on');c.querySelector('.cs').textContent=on?'ON':'OFF';
  }
}

function updateFaults(faults){
  var cells=document.querySelectorAll('#faults .cell');
  for(var i=0;i<cells.length;i++){
    var c=cells[i],k=c.getAttribute('data-key'),f=!!faults[k];
    setCell(c,f,'warn');c.querySelector('.cs').textContent=f?'FAULT':'OK';
  }
}

function updateGlow(glow,seq){
  var steps=document.querySelectorAll('#seq .step');
  for(var i=0;i<steps.length;i++){
    var s=steps[i];
    if(parseInt(s.getAttribute('data-idx'))===2){
      s.textContent=glow.active?'Glow ('+glow.countdown+'s)':'Glow';
    }
  }
}

function updateStateLabel(state){
  byId('stateLabel').textContent=state;
  var st=byId('state');
  st.style.color=state==='ERROR'||state==='LOW_OIL'||state==='HIGH_TEMP'?'var(--crit)':'var(--accent)';
}

function updateLightsBtn(outs){
  byId('btnLights').textContent='Lights · '+(outs.lights?'On':'Off');
}

function updateEngineHours(hours){
  byId('cap').textContent='Engine hours '+hours.toLocaleString(undefined,{minimumFractionDigits:1,maximumFractionDigits:1})+' h · stop drops every relay instantly';
}

function renderStatus(data){
  if(!data)return;
  lastStatus=data;
  updateStateLabel(data.state);
  updateStrip(data.net);
  updateSequence(data.seq);
  updateHero(data.hydraulic);
  updateVitals(data.engineTemp,data.oil,data.battery,data.fuel);
  updateOutputs(data.outputs);
  updateFaults(data.faults);
  updateGlow(data.glow,data.seq);
  updateLightsBtn(data.outputs);
  updateEngineHours(data.engineHours);
}

async function poll(){
  try{
    var r=await fetch('/api/status');
    if(!r.ok)throw new Error('status '+r.status);
    var data=await r.json();
    if(reconnecting){reconnecting=false;}
    renderStatus(data);
  }catch(e){
    if(!reconnecting){reconnecting=true;updateStrip(null);}
  }
  pollTimer=setTimeout(poll,1000);
}

function apiCtrl(body){
  fetch('/api/control',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
  .then(function(r){if(!r.ok)showToast('Command failed');})
  .catch(function(){showToast('Network error');});
}

document.addEventListener('DOMContentLoaded',function(){
  byId('btnLights').addEventListener('click',function(){apiCtrl({action:'lights'});});
  byId('btnStop').addEventListener('click',function(){apiCtrl({action:'stop'});});
  poll();
});
