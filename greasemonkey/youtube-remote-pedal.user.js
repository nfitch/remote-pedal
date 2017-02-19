// ==UserScript==
// @name        youtube-remote-pedal
// @namespace   ytrp
// @description YouTube Remote Pedal
// @include     https://www.youtube.com/watch*
// @require  http://ajax.googleapis.com/ajax/libs/jquery/1.7.2/jquery.min.js
// @version     1
// @grant       none
// ==/UserScript==

/****************************
 * Before you start, the ws might be on http while YouTube is on https.
 * If you get a "SecurityError:", then you need to make your browser more
 * unsafe:
 *
 * http://stackoverflow.com/questions/11768221/firefox-websocket-security-issue
 * tl;dr
 * about:config
 * network.websocket.allowInsecureFromHTTPS -> toggle to true
 ****************************/

// DEBUG
var debug = false;
function debugLog(s) {
    if (debug) {
        unsafeWindow.console.log(s);
    }
}

// Hold some state...
var inProgress = false;
var previousBackSeconds = 0;
var previousNewTime = 0;
var continuedLoop = false;

// Map functions to buttons...
var fmap = {
    'left': function (data) {
        backAndPause(data);
    },
    'middle': function (data) {
        playVideo();
    },
    'right': function (data) {
        pauseVideo();
    }
}

function updateStatus(s) {
    document.getElementById("ytrp-status").innerHTML = s;
}

function connectws() {
    updateStatus('Connecting...');
    var url = document.getElementById("ytrp-endpoint").value;
    try {
        var socket = new WebSocket(url);
    } catch (e) {
        alert("Failed to connect: " + e);
    }
    socket.onopen = function () {
        updateStatus('Connected');
    }
    socket.onmessage = function (e) {
        var data = JSON.parse(e.data);
        debugLog(data);
        fmap[data.b].call(this, data);
    }
    socket.onclose = function () {
        updateStatus('Disconnected');
    }
}

function getVideo() {
    return document.getElementsByTagName("video")[0];
}

function playVideo() {
    continuedLoop = false;
    getVideo().play();
}

function pauseVideo() {
    getVideo().pause();
}

function backAndPause(data) {
    // TODO: We probably want to detect a "cancel" here if the timout is really
    // long.
    if (inProgress === true) {
        return;
    }
    var video = getVideo();
    var backSeconds = data.p;
    var duration = data.d;

    // How far back should we seek?  If we're a "continuation loop", meaning
    // this is a loop that we keep playing ove and over we want to keep seeking
    // past to a set point, rather than having jitter around where we seek back
    // to (since setTimeout isn't going to be precise).
    var newTime;
    // TODO: We'll need to work into some jitter with the === backSeconds here
    // when using the real pedal.
    if (continuedLoop && backSeconds === previousBackSeconds) {
        newTime = previousNewTime;
        debugLog("continued: " + newTime);
    } else {
        newTime = video.currentTime - backSeconds;
        newtime = (newTime < 0) ? newTime = 0 : newTime;
        debugLog("seeking: " + newTime);
    }

    // Kick off the loop
    inProgress = true;
    video.pause();
    video.currentTime = newTime;
    // If you call play too fast it won't pick up the seek
    setTimeout(function seekedNowPlay() {
        video.play();
        setTimeout(function donePlaying() {
            inProgress = false;
            video.pause();
            // Save state for the next time...
            previousBackSeconds = backSeconds;
            previousNewTime = newTime;
            debugLog("done: " + previousNewTime);
            // We assume a "continued loop" until they press play again.
            continuedLoop = true;
        }, backSeconds * 1000);
    }, 500);
}

// Add a bit of UI to the top... back to 1999, building it manually...
var d = document;
var ytrpDiv = d.createElement('div');
ytrpDiv.id = "ytrpDiv";

var ytrpTable = d.createElement('table');
ytrpTable.border = 1;
ytrpTable.width = '100%';
ytrpDiv.appendChild(ytrpTable);
var ytrpTableRow = ytrpTable.insertRow(0);

// Status Cell
var ytrpTdStatus = ytrpTableRow.insertCell(0);
ytrpTdStatus.appendChild(d.createTextNode("Remote Pedal Status: "));
var ytrpTdStatusSpan = d.createElement('span');
ytrpTdStatusSpan.id = 'ytrp-status';
ytrpTdStatusSpan.innerHTML = 'Disconnected';
ytrpTdStatus.appendChild(ytrpTdStatusSpan);

// Endpoint Cell
var ytrpTdEndpoint = ytrpTableRow.insertCell(1);
ytrpTdEndpoint.appendChild(d.createTextNode("Endpoint: "));
var ytrpEndpoint = d.createElement('input');
ytrpEndpoint.id = 'ytrp-endpoint';
ytrpEndpoint.type = 'text';
ytrpEndpoint.size = 30
ytrpEndpoint.value = 'ws://localhost:8090'
ytrpTdEndpoint.appendChild(ytrpEndpoint);
var ytrpConnect = d.createElement('input');
ytrpConnect.type = 'button';
ytrpConnect.value = 'connect';
ytrpConnect.addEventListener('click', connectws, false);
ytrpTdEndpoint.appendChild(ytrpConnect);

//Also tried these... but yt-masthead-container was the best.
//$('#early-body')
//$('#body-container')
$('#yt-masthead-container').append(ytrpDiv);

// If we're debugging we know we can automatically connect.
if (debug) {
    connectws();
}
