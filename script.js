//variables for canvas
const drawing_canvas = document.getElementById("myCanvas");
const ctx = drawing_canvas.getContext("2d");
//variables for drawing
const brushSize = 14;
let mouseX = [];//storing X vals
let mouseY = [];//storing Y vals
//variables for mouse
let mouseDown = false;//variable for mouse and touch
let mousePos = { x: 0, y: 0 };
//variables for touch
let touchPos = { x: 0, y: 0 };
//variables for sending ESP32
let requestX = "X";
let requestY = "Y";
let requestMain = "";
//function for drawing a dot
function drawDot(x, y, s) {
    ctx.beginPath();
    ctx.arc(x, y, s, 0, 2 * Math.PI);
    ctx.fillStyle = "black";
    ctx.fill();
}
//functions for mouse events
function draw_mouseMove(e) {
    //console.log("move");
    mousePos.x = e.offsetX;//the mouse X relative to canvas
    mousePos.y = e.offsetY;//the mouse Y relative to canvas
    //drawing a dot at the mouse
    if (mouseDown) {
        drawDot(mousePos.x, mousePos.y, brushSize);
        //appending position arrays
        mouseX.push(mousePos.x);
        mouseY.push(mousePos.y);
    }
}
function draw_mouseDown(e) {
    mouseDown = true;
    //drawing point at mouse start
    draw_mouseMove(e);
    //console.log("down");
}
function draw_mouseUp() {
    mouseDown = false;
    //adding data to know when to lift pen
    mouseX.push(-1);
    mouseY.push(-1);
    //console.log("up");
}
//functions for touch events
function draw_touchMove(e) {
    //console.log("move");
    var touch = e.touches[0];
    touchPos.x = touch.pageX - touch.target.offsetLeft;//offsetting x position to line up with canvas
    touchPos.y = touch.pageY - touch.target.offsetTop;//offsetting x position to line up with canvas
    //drawing a dot at the touch
    drawDot(touchPos.x, touchPos.y, brushSize);
    //appending position arrays
    mouseX.push(touchPos.x);
    mouseY.push(touchPos.y);
    //prevent scrolling
    e.preventDefault();

}
function draw_touchStart(e){
    //drawing point at touch start
    draw_touchMove(e);
}
function draw_touchEnd(){
    //adding data to know when to lift pen
    mouseX.push(-1);
    mouseY.push(-1);
}
//function to clear the canvas
function clearCanvas() {
    ctx.clearRect(0, 0, drawing_canvas.width, drawing_canvas.height);
    //resetting position arrays
    mouseX.length = 0;
    mouseY.length = 0;
    //resetting request strings
    requestX = "X";
    requestY = "Y";
    requestMain = "";
}
//function to send X and Y info to drawing CNC
$.ajaxSetup({ timeout: 8000 });//in milliseconds
function sendTo() {
    //building href to send
    //requestX
    for (let i = 0; i < mouseX.length; i++) {
        requestX += mouseX[i].toString();//adding string of number to requestX
        requestX += ",";//dividing the positions
    }
    //requestY
    for (let i = 0; i < mouseY.length; i++) {
        requestY += mouseY[i].toString();//adding string of number to requestY
        requestY += ",";//dividing the positions
    }
    //storing the ammount of positions in between parenthenses at the beginning of the header
    requestMain = "(" + mouseX.length + ")" + requestX + requestY + "!";//building requestMain ! as ender
    console.log(mouseX);
    console.log(mouseY);
    $.get("/?values=" + requestMain);
}
//adding all event listeners
drawing_canvas.addEventListener("mousedown", draw_mouseDown);
drawing_canvas.addEventListener("mouseup", draw_mouseUp);
drawing_canvas.addEventListener("mousemove", draw_mouseMove);
//support for phone
drawing_canvas.addEventListener("touchmove", draw_touchMove);
drawing_canvas.addEventListener("touchstart", draw_touchStart);
drawing_canvas.addEventListener("touchend", draw_touchEnd);
