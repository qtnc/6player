function require (url) {
var s = document.createElement('script');
s.setAttribute('type', 'text/javascript');
s.setAttribute('src', url);
document.getElementsByTagName('head')[0].appendChild(s);
}

function log (str) {
if (window.console && console.log) console.log(str);
}

function ajax (method, url, data, success, failure) {
var xhr = null;
if (window.XMLHttpRequest) xhr = new XMLHttpRequest();
else if (window.ActiveXObject) xhr = new ActiveXObject('Microsoft.XMLHTTP');
if (!xhr) return null;
xhr.onreadystatechange = function () {
if (xhr.readyState==4) {
if (xhr.status==200) success(xhr.responseText, xhr.responseXML);
else failure(xhr.status, xhr);
}};
xhr.onerror = xhr.onabort = function () { 
failure(-1, xhr); 
};
xhr.open(method, url, true);
if (data) xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
xhr.send(data);
return xhr;
}

function addClass (o, name) {
var t = o.className.toString().split(' ');
if (t.indexOf(name)>=0) return;
t.push(name);
var s = t.join(' ');
o.className = s;
}

function removeClass (o, name) {
var t = o.className.toString().split(' ');
var i = t.indexOf(name);
if (i<0) return;
t.splice(i,1);
var s = t.join(' ');
o.className = s;
}

if (!Array.prototype.indexOf) Array.prototype.indexOf = function (o, start) {
if (typeof(start)!='number') start=0;
for (var i=start; i<this.length; i++) if (this[i]==o) return i;
return -1;
}

String.prototype.indexOfIgnoreCase = function (s) {
return this.toLowerCase().indexOf(s.toLowerCase());
}

String.prototype.startsWith = function (str) {
return this.indexOf(str)==0;
}

String.prototype.startsWithIgnoreCase = function (str) {
return this.indexOfIgnoreCase(str)==0;
}

String.prototype.trim = function () {
return this.replace(/^\s*/,'').replace(/\s*$/,'');
}

String.prototype.splitn = function (sep, lim) {
var t = this.split(sep);
if (t.length>lim) {
t.push(
t.splice(lim -1, t.length -lim +1)
.join(sep));
}
return t;
}

String.prototype.escapeHTML = function () {
return this
.split('&').join('&amp;')
.split('<').join('&lt;')
.split('>').join('&gt;');
}

String.prototype.stripHTML = function () {
return this.replace(/<.*?>/g, '');
}



function $ (selector, event, func) {
if (!document.querySelectorAll) return false;
var tab = document.querySelectorAll(selector);
for (var i=0; i<tab.length; i++) tab[i][event]=func;
}

function $create (o) {
var tn = o.tagName;
var x = document.createElement(tn);
for (var i in o) if (i!='tagName'&&typeof(o[i])!='function') x.setAttribute(i, o[i]);
return x;
}

window.onload = function () {
var html5 = ('header nav footer main article section aside figure figcaption mark time output').split(' ');
for (var i=0; i<html5.length; i++) document.createElement(html5[i]);
$('input', 'onfocus', function(){ this.select(); });
$('a[data-confirm]', 'onclick', function(){ return confirm(this.getAttribute('data-confirm')); });
$('a[data-popup]', 'onclick', function(){ return !window.open(this.href); });
if (window.onloads) for (var i=0; i<window.onloads.length; i++) window.onloads[i]();
};

