var CONFIG_HTML = [
'<!DOCTYPE html>',
'<html><head><meta charset="utf-8">',
'<meta name="viewport" content="width=device-width, initial-scale=1">',
'<title>Rotating Lines Face</title>',
'<style>',
'body{font-family:-apple-system,Roboto,sans-serif;background:#f5f5f5;color:#222;margin:0;padding:20px}',
'h1{font-size:20px;margin:0 0 16px}',
'.row{background:#fff;padding:14px 16px;margin-bottom:10px;border-radius:8px;display:flex;justify-content:space-between;align-items:center}',
'.row label{font-size:16px}',
'input[type=color]{width:50px;height:32px;border:none;background:none;padding:0}',
'button{width:100%;padding:14px;font-size:16px;background:#f93;color:#fff;border:none;border-radius:8px;margin-top:12px}',
'</style></head><body>',
'<h1>Rotating Lines Face</h1>',
'<div class="row"><label for="bg">Background</label><input type="color" id="bg" value="#000000"></div>',
'<div class="row"><label for="line">Line</label><input type="color" id="line" value="#ffffff"></div>',
'<div class="row" style="flex-wrap:wrap"><label for="smooth" style="width:100%;margin-bottom:8px">Smooth animation above <span id="sval">80</span>% battery</label><input type="range" id="smooth" min="0" max="100" step="10" value="80" style="width:100%"></div>',
'<div class="row" style="flex-wrap:wrap"><label for="secthresh" style="width:100%;margin-bottom:8px">Hide seconds below <span id="secval">60</span>% battery (0 = always show)</label><input type="range" id="secthresh" min="0" max="100" step="10" value="60" style="width:100%"></div>',
'<div class="row" style="flex-wrap:wrap"><label for="batshow" style="width:100%;margin-bottom:8px">Show battery below <span id="batval">50</span>% (0 = never show)</label><input type="range" id="batshow" min="0" max="100" step="10" value="50" style="width:100%"></div>',
'<div class="row"><label for="linerot">Phase-offset line rotation</label><input type="checkbox" id="linerot" checked style="width:24px;height:24px"></div>',
'<div class="row"><label for="showdate">Show date</label><input type="checkbox" id="showdate" checked style="width:24px;height:24px"></div>',
'<div class="row"><label for="showmarkers">Show time markers</label><input type="checkbox" id="showmarkers" checked style="width:24px;height:24px"></div>',
'<div class="row" id="extendrow" style="display:none"><label for="extendlines">Extend second lines past screen edge</label><input type="checkbox" id="extendlines" style="width:24px;height:24px"></div>',
'<div class="row" style="flex-wrap:wrap"><label style="width:100%;margin-bottom:8px">Hour lines start at <span id="hourpctval">17</span>% radius</label><input type="range" id="hourpct" min="0" max="90" step="1" value="17" style="width:100%"></div>',
'<div class="row" style="flex-wrap:wrap"><label style="width:100%;margin-bottom:8px">Minute lines start at <span id="minpctval">50</span>% radius</label><input type="range" id="minpct" min="0" max="90" step="1" value="50" style="width:100%"></div>',
'<div class="row" style="flex-wrap:wrap"><label style="width:100%;margin-bottom:8px">Second lines start at <span id="secpctval">75</span>% radius</label><input type="range" id="secpct" min="0" max="95" step="1" value="75" style="width:100%"></div>',
'<button id="save">Save</button>',
'<script>(function(){',
'var params={};location.search.replace(/^\\?/,"").split("&").forEach(function(p){if(!p)return;var i=p.indexOf("=");params[decodeURIComponent(p.slice(0,i))]=decodeURIComponent(p.slice(i+1));});',
'var returnTo=params.return_to||"pebblejs://close#";',
'var smoothEl=document.getElementById("smooth");var svalEl=document.getElementById("sval");smoothEl.addEventListener("input",function(){svalEl.textContent=smoothEl.value;});',
'var secEl=document.getElementById("secthresh");var secvalEl=document.getElementById("secval");secEl.addEventListener("input",function(){secvalEl.textContent=secEl.value;});',
'var batEl=document.getElementById("batshow");var batvalEl=document.getElementById("batval");batEl.addEventListener("input",function(){batvalEl.textContent=batEl.value;});',
'var markersEl=document.getElementById("showmarkers");var extendRowEl=document.getElementById("extendrow");var extendEl=document.getElementById("extendlines");function updateExtendVisibility(){extendRowEl.style.display=markersEl.checked?"none":"flex";}markersEl.addEventListener("change",updateExtendVisibility);',
'var hourPctEl=document.getElementById("hourpct");var hourPctValEl=document.getElementById("hourpctval");hourPctEl.addEventListener("input",function(){hourPctValEl.textContent=hourPctEl.value;});',
'var minPctEl=document.getElementById("minpct");var minPctValEl=document.getElementById("minpctval");minPctEl.addEventListener("input",function(){minPctValEl.textContent=minPctEl.value;});',
'var secPctEl=document.getElementById("secpct");var secPctValEl=document.getElementById("secpctval");secPctEl.addEventListener("input",function(){secPctValEl.textContent=secPctEl.value;});',
'try{var saved=JSON.parse(localStorage.getItem("rlf_settings")||"{}");if(saved.bg_color)document.getElementById("bg").value=saved.bg_color;if(saved.line_color)document.getElementById("line").value=saved.line_color;if(saved.smooth_threshold!=null){smoothEl.value=saved.smooth_threshold;svalEl.textContent=saved.smooth_threshold;}if(saved.seconds_threshold!=null){secEl.value=saved.seconds_threshold;secvalEl.textContent=saved.seconds_threshold;}if(saved.battery_show_threshold!=null){batEl.value=saved.battery_show_threshold;batvalEl.textContent=saved.battery_show_threshold;}if(saved.line_rotation!=null){document.getElementById("linerot").checked=!!saved.line_rotation;}if(saved.show_date!=null){document.getElementById("showdate").checked=!!saved.show_date;}if(saved.show_markers!=null){markersEl.checked=!!saved.show_markers;}if(saved.extend_second_lines!=null){extendEl.checked=!!saved.extend_second_lines;}if(saved.hour_start_pct!=null){hourPctEl.value=saved.hour_start_pct;hourPctValEl.textContent=saved.hour_start_pct;}if(saved.minute_start_pct!=null){minPctEl.value=saved.minute_start_pct;minPctValEl.textContent=saved.minute_start_pct;}if(saved.second_start_pct!=null){secPctEl.value=saved.second_start_pct;secPctValEl.textContent=saved.second_start_pct;}updateExtendVisibility();}catch(e){}',
'document.getElementById("save").addEventListener("click",function(){var s={bg_color:document.getElementById("bg").value,line_color:document.getElementById("line").value,smooth_threshold:parseInt(smoothEl.value,10),seconds_threshold:parseInt(secEl.value,10),battery_show_threshold:parseInt(batEl.value,10),line_rotation:document.getElementById("linerot").checked?1:0,show_date:document.getElementById("showdate").checked?1:0,show_markers:markersEl.checked?1:0,extend_second_lines:extendEl.checked?1:0,hour_start_pct:parseInt(hourPctEl.value,10),minute_start_pct:parseInt(minPctEl.value,10),second_start_pct:parseInt(secPctEl.value,10)};try{localStorage.setItem("rlf_settings",JSON.stringify(s));}catch(e){}document.location=returnTo+encodeURIComponent(JSON.stringify(s));});',
'})();</script></body></html>'
].join('');

function hexStringToInt(s) {
  return parseInt(s.replace('#', ''), 16);
}

Pebble.addEventListener('showConfiguration', function () {
  var url = 'data:text/html;charset=utf-8,' + encodeURIComponent(CONFIG_HTML);
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function (e) {
  if (!e || !e.response) return;
  var settings;
  try {
    settings = JSON.parse(decodeURIComponent(e.response));
  } catch (err) {
    return;
  }
  var msg = {
    BG_COLOR: hexStringToInt(settings.bg_color),
    LINE_COLOR: hexStringToInt(settings.line_color)
  };
  if (settings.smooth_threshold != null) {
    msg.SMOOTH_THRESHOLD = settings.smooth_threshold;
  }
  if (settings.seconds_threshold != null) {
    msg.SECONDS_THRESHOLD = settings.seconds_threshold;
  }
  if (settings.battery_show_threshold != null) {
    msg.BATTERY_SHOW_THRESHOLD = settings.battery_show_threshold;
  }
  if (settings.line_rotation != null) {
    msg.LINE_ROTATION = settings.line_rotation;
  }
  if (settings.show_date != null) {
    msg.SHOW_DATE = settings.show_date;
  }
  if (settings.show_markers != null) {
    msg.SHOW_MARKERS = settings.show_markers;
  }
  if (!settings.show_markers && settings.extend_second_lines != null) {
    msg.EXTEND_SECOND_LINES = settings.extend_second_lines;
  }
  if (settings.hour_start_pct != null) {
    msg.HOUR_START_PCT = settings.hour_start_pct;
  }
  if (settings.minute_start_pct != null) {
    msg.MINUTE_START_PCT = settings.minute_start_pct;
  }
  if (settings.second_start_pct != null) {
    msg.SECOND_START_PCT = settings.second_start_pct;
  }
  Pebble.sendAppMessage(msg);
});
