/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId])
/******/ 			return installedModules[moduleId].exports;
/******/
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			exports: {},
/******/ 			id: moduleId,
/******/ 			loaded: false
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.loaded = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "";
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports, __webpack_require__) {

	__webpack_require__(1);
	module.exports = __webpack_require__(2);


/***/ }),
/* 1 */
/***/ (function(module, exports) {

	/**
	 * Copyright 2024 Google LLC
	 *
	 * Licensed under the Apache License, Version 2.0 (the "License");
	 * you may not use this file except in compliance with the License.
	 * You may obtain a copy of the License at
	 *
	 *     http://www.apache.org/licenses/LICENSE-2.0
	 *
	 * Unless required by applicable law or agreed to in writing, software
	 * distributed under the License is distributed on an "AS IS" BASIS,
	 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	 * See the License for the specific language governing permissions and
	 * limitations under the License.
	 */
	
	(function(p) {
	  if (!p === undefined) {
	    console.error('Pebble object not found!?');
	    return;
	  }
	
	  // Aliases:
	  p.on = p.addEventListener;
	  p.off = p.removeEventListener;
	
	  // For Android (WebView-based) pkjs, print stacktrace for uncaught errors:
	  if (typeof window !== 'undefined' && window.addEventListener) {
	    window.addEventListener('error', function(event) {
	      if (event.error && event.error.stack) {
	        console.error('' + event.error + '\n' + event.error.stack);
	      }
	    });
	  }
	
	})(Pebble);


/***/ }),
/* 2 */
/***/ (function(module, exports) {

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


/***/ })
/******/ ]);
//# sourceMappingURL=pebble-js-app.js.map