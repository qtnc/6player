function sendQuery (query, callback) {
callback = callback || null;
var url = 'http://' + window.location.host + '/action/' + query;
ajax('GET', url, null, callback, null);
}

if (!window.onloads) window.onloads = [];
window.onloads.push(function(){
$('#btnPlay', 'onclick', function(){ sendQuery(2001); });
$('#btnStop', 'onclick', function(){ sendQuery('stop'); });
$('#btnNext', 'onclick', function(){ sendQuery(2002); });
$('#btnPrev', 'onclick', function(){ sendQuery(2003); });
});

alert(1);