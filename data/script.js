var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

//Khởi tạo websocket khi page load
window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

//Hàm này có công dụng: Web sẽ gửi dữ liệu đến server (board ESP32)
function getReadings(){
    websocket.send("getReadings");
}

function setStateLed() {
    websocket.send("stateLed");
}

function setUserPass() {
    const username = document.getElementById("username").value;
    const password = document.getElementById("password").value;

    if (websocket.readyState === WebSocket.OPEN) {
        const loginData = {
            username: username,
            password: password
        };
        websocket.send(JSON.stringify(loginData));
        document.getElementById("statusLogin").innerText = " While sending...";
    } 
    else {
        document.getElementById("statusLogin").innerText = " Don't connect websocket.";
    }
}

//Hàm này có nhiệm vụ: Khởi tạo kết nối WebSocket giữa trình duyệt và ESP32, đồng thời gán các hàm xử lý sự kiện WebSocket (mở kết nối, nhận dữ liệu, mất kết nối,...)
function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getReadings();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

//Hàm này có công dụng: là Web sẽ nhận dữ liệu từ ESP32 sau khi nó gửi yêu cầu tới ESP32
function onMessage(event) {
    console.log("Raw data:", event.data);
    try {
        var myObj = JSON.parse(event.data);
        console.log("Parsed JSON:", myObj); // Log object sau parse

        // Cập nhật temperature nếu có
        if (myObj["temperature"] !== undefined) {
            const tempElement = document.getElementById("temperature");
            if (tempElement) {
                tempElement.innerHTML = myObj["temperature"];
                console.log(`Updated temperature to: ${myObj["temperature"]}`);
            } else {
                console.warn("Element with id 'temperature' not found");
            }
        }

        // Cập nhật LED status nếu có
        if (myObj["led"] !== undefined) {
            const ledStatus = document.getElementById("ledStatus");
            if (ledStatus) {
                ledStatus.innerText = myObj["led"] === "1" ? "ON" : "OFF";
                console.log(`Updated ledStatus to: ${myObj["led"] === "1" ? "ON" : "OFF"}`);
            } else {
                console.warn("Element with id 'ledStatus' not found");
            }
        }

        // Cập nhật statusLogin nếu có
        if (myObj["status"] === "saved") {
            const statusLogin = document.getElementById("statusLogin");
            if (statusLogin) {
                statusLogin.innerText = "Saved";
                console.log("Updated statusLogin to 'Saved'");
            } 
            else {
                console.error("Element with id 'statusLogin' not found");
            }
        }
    } catch (e) {
        console.error("Error parsing JSON:", e);
    }
}