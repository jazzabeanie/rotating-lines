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
'<button id="save">Save</button>',
'<script>(function(){',
'var params={};location.search.replace(/^\\?/,"").split("&").forEach(function(p){if(!p)return;var i=p.indexOf("=");params[decodeURIComponent(p.slice(0,i))]=decodeURIComponent(p.slice(i+1));});',
'var returnTo=params.return_to||"pebblejs://close#";',
'try{var saved=JSON.parse(localStorage.getItem("rlf_settings")||"{}");if(saved.bg_color)document.getElementById("bg").value=saved.bg_color;if(saved.line_color)document.getElementById("line").value=saved.line_color;}catch(e){}',
'document.getElementById("save").addEventListener("click",function(){var s={bg_color:document.getElementById("bg").value,line_color:document.getElementById("line").value};try{localStorage.setItem("rlf_settings",JSON.stringify(s));}catch(e){}document.location=returnTo+encodeURIComponent(JSON.stringify(s));});',
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
  Pebble.sendAppMessage({
    BG_COLOR: hexStringToInt(settings.bg_color),
    LINE_COLOR: hexStringToInt(settings.line_color)
  });
});
