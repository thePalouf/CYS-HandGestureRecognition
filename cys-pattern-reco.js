// Global handle to module
var ImageProcModule = null;

// Global status message
var statusText = 'NO-STATUS';
var lastClick = null;

var isRunning = false;
var isReadyToReceive = false;

//var samplePeriod = 40; // ms

var startTime;
var endTime;
var averageFramePeriod;// = samplePeriod;
var ewmaSmooth = 0.3;

var view;
var imageData;

var skinPressed = false;
var contourPressed = false;
var backgroundPressed = false;

function pageDidLoad() {
  var src = make_image_source();
  view = make_image_source_view(src);
  view.init("live", "display", "camera"); 
  var camera = document.getElementById("camera");
  camera.hidden = true;

  var listener = document.getElementById("listener");
  listener.addEventListener("load", moduleDidLoad, true );
  listener.addEventListener("message", handleMessage, true );
  if ( ImageProcModule == null ) {
    updateStatus( 'LOADING...' );
  } else {
    updateStatus();
  }
}

function draw(v,c) {
  c.drawImage(v, 0, 0);
  setTimeout( draw, samplePeriod, v, c);
}

function sendImage() {
  if ( isRunning && isReadyToReceive ) {
    imData = view.getImageData();

    var theCommand = "process"; //"echo"; // test, process
	if (skinPressed) {theCommand = "askskin";}
	if (contourPressed) {theCommand = "askcontour";}
	if (backgroundPressed) {theCommand = "background";}
    var cmd = { cmd: theCommand,  
      width: imData.width, 
      height: imData.height, 
      data: imData.data };
    startTime = performance.now();
    ImageProcModule.postMessage( cmd );
    isReadyToReceive = false; // Don't send any more frames until ready
  } else if (isRunning ) {
    // Do nothing
  } else { 
    updateStatus( 'Stopped' );
  }
  setTimeout( sendImage, view.getSamplePeriod() );
}

function drawImage(pixels){
    var processed = document.getElementById("processed");
    var ctx = processed.getContext( "2d" );
    var imData = ctx.getImageData(0,0,processed.width,processed.height);
    var buf8 = new Uint8ClampedArray( pixels );
    imData.data.set( buf8 );
    ctx.putImageData( imData, 0, 0);
}


function moduleDidLoad() {
  ImageProcModule = document.getElementById( "cys-pattern-reco" );
  updateStatus( "OK" );
  var bt_background = document.getElementById( "bt_background" );
  var bt_execute = document.getElementById( "bt_execute" );
  var bt_stop = document.getElementById( "bt_stop" );
  var bt_showskin = document.getElementById( "bt_showskin" );
  var bt_showcontour = document.getElementById( "bt_showcontour" );

  bt_background.onclick = startSendingBackground;
  bt_execute.onclick = startSending;
  bt_stop.onclick = stopSending;
  bt_showskin.onclick = startAskingSkin;
  bt_showcontour.onclick = startAskingContour;

  bt_background.disableb = false;
  bt_stop.disabled = true;
  bt_execute.disabled = true;
  bt_showskin.disabled = true;
  bt_showcontour.disabled = true;

  bt_stop.hidden = false;
  bt_background.hidden = false;
  bt_execute.hidden = false;
  bt_showskin.hidden = false;
  bt_showcontour.hidden = false;
}

function startSending() {
  isRunning = true;
  isReadyToReceive = true;
  var bt_background = document.getElementById( "bt_background" );
  bt_background.disabled = true;
  var bt_execute = document.getElementById( "bt_execute" );
  bt_execute.disabled = true;
  var bt_stop = document.getElementById( "bt_stop" );
  bt_stop.disabled = false;
  var bt_showskin = document.getElementById( "bt_showskin" );
  bt_showskin.disabled = false;
  var bt_showcontour = document.getElementById( "bt_showcontour" );
  bt_showcontour.disabled = false;
  skinPressed = false;
  contourPressed = false;
  backgroundPressed = false;
  sendImage();
}

function startSendingBackground() {
  isRunning = true;
  isReadyToReceive = true;
  var bt_background = document.getElementById( "bt_background" );
  bt_background.disabled = true;
  var bt_execute = document.getElementById( "bt_execute" );
  bt_execute.disabled = false;
  var bt_stop = document.getElementById( "bt_stop" );
  bt_stop.disabled = true;
  var bt_showskin = document.getElementById( "bt_showskin" );
  bt_showskin.disabled = true;
  var bt_showcontour = document.getElementById( "bt_showcontour" );
  bt_showcontour.disabled = true;
  skinPressed = false;
  contourPressed = false;
  backgroundPressed = true;
  sendImage();
}

function stopSending() {
  isRunning = false;
  isReadyToReceive = false;
  var bt_background = document.getElementById( "bt_background" );
  bt_background.disabled = false;
  var bt_stop = document.getElementById( "bt_stop" );
  stop.disabled = true;
  var bt_execute = document.getElementById( "bt_execute" );
  bt_execute.disabled = false;
  var bt_showskin = document.getElementById( "bt_showskin" );
  bt_showskin.disabled = true;
  var bt_showcontour = document.getElementById( "bt_showcontour" );
  bt_showcontour.disabled = true;
  skinPressed = false;
  contourPressed = false;
  backgroundPressed = false;
}


function startAskingSkin() {
  isRunning = true;
  isReadyToReceive = true;
  var bt_background = document.getElementById( "bt_background" );
  bt_background.disabled = true;
  var bt_execute = document.getElementById( "bt_execute" );
  bt_execute.disabled = false;
  var bt_stop = document.getElementById( "bt_stop" );
  bt_stop.disabled = false;
  var bt_showskin = document.getElementById( "bt_showskin" );
  bt_showskin.disabled = true;
  var bt_showcontour = document.getElementById( "bt_showcontour" );
  bt_showcontour.disabled = false;
  //askSkin();
  skinPressed = true;
  contourPressed = false;
  backgroundPressed = false;
  sendImage();
}

function startAskingContour() {
  isRunning = true;
  isReadyToReceive = true;
  var bt_background = document.getElementById( "bt_background" );
  bt_background.disabled = true;
  var bt_execute = document.getElementById( "bt_execute" );
  bt_execute.disabled = false;
  var bt_stop = document.getElementById( "bt_stop" );
  bt_stop.disabled = false;
  var bt_showskin = document.getElementById( "bt_showskin" );
  bt_showskin.disabled = false;
  var bt_showcontour = document.getElementById( "bt_showcontour" );
  bt_showcontour.disabled = true;
  //askContour();
  skinPressed = false;
  contourPressed = true;
  backgroundPressed = false;
  sendImage();
}

function handleMessage(message_event) {
  var res = message_event.data;
  if ( res.Type == "version" ) {
    updateStatus( res.Version );
  }
  if ( res.Type == "completed" ) {
    if ( res.Data ) {
      // updateStatus( "Received array buffer");
      // Display processed image    
      endTime = performance.now();
      if (typeof averageFramePeriod === 'undefined' ) {
        averageFramePeriod = view.getSamplePeriod();
      }
      averageFramePeriod = (1-ewmaSmooth)*averageFramePeriod + ewmaSmooth*(endTime-startTime);
      updateStatus( 'Frame rate is ' + (averageFramePeriod).toFixed(1) + 'ms per frame' );
      drawImage( res.Data );
	var left = document.getElementById("leftHand");
	left.innerHTML = res.Left;
	var right = document.getElementById("rightHand");
	right.innerHTML = res.Right;
      isReadyToReceive = true;
    } else {
      updateStatus( "Received something unexpected");
    }

    // Display processed image    
    //drawImage( res.Data );
  }
  if ( res.Type == "learning" ) {
    	//backgroundSent = true;
	updateStatus( "Learning background" );
	/*var bt_execute = document.getElementById( "bt_execute" );
  	bt_execute.disabled = false;*/
  }
  if ( res.Type == "status" ) {
     updateStatus( res.Message ); 
  }
}

function updateStatus( optMessage ) {
  if (optMessage)
    statusText = optMessage;
  var statusField = document.getElementById("statusField");
  if (statusField) {
    statusField.innerHTML = " : " + statusText;
  }
}

