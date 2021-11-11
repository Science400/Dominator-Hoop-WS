/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
    initButtons();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    // document.getElementById('led').className = data.status;
    console.log(data);
    if (data.hasOwnProperty("cQR")) {
        cycleQuantityInitial = data.cQI;
        cycleQuantityRemaining = data.cQR;
        progressBarWidth = (cycleQuantityInitial - cycleQuantityRemaining)/cycleQuantityInitial;
        console.log(progressBarWidth);
        $('#progress-bar').css('width', progressBarWidth*100+'%');
        $('#cyclesCompletedSpan').text((cycleQuantityInitial - cycleQuantityRemaining));
        $('#cyclesInitialSpan').text(cycleQuantityInitial);
        if(progressBarWidth >= 1)
        {
            $('#progress-bar').toggleClass('progress-bar-animated');
        }
        if($('#cyclesOutput').is(".d-none"))
        {
            $('#cyclesInput').toggleClass('d-none')
            $('#cyclesOutput').toggleClass('d-none')
        }
    }

}

// ----------------------------------------------------------------------------
// Button handling
// ----------------------------------------------------------------------------

function initButtons() {
    document.getElementById('startButton').addEventListener('click', onStart);
    document.getElementById('stopButton').addEventListener('click', onStop);
}

function onStart(event) {
    $('#cyclesInput').toggleClass('d-none')
    $('#cyclesOutput').toggleClass('d-none')
    cycles = $('#cycleQuantity').val()
    websocket.send(JSON.stringify({ 'startCycle': 'true', 'cycleQuantity': cycles }));
    console.log(JSON.stringify({ 'startCycle': 'true', 'cycleQuantity': cycles }))
}

function onStop(event) {
    $('#cyclesInput').toggleClass('d-none')
    $('#cyclesOutput').toggleClass('d-none')
    websocket.send(JSON.stringify({ 'startCycle': 'false', 'cycleQuantity': '0' }));
    console.log(JSON.stringify({ 'startCycle': 'false' }))
}