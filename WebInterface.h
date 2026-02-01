#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ro">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>R WDT81 Radio</title>
<link rel="icon" href="data:,">
<style>
  @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;800&display=swap');
  
  :root {
      --color-bg: #1a5f3f;
      --color-card-bg: #B3D9F2;
      --color-border: #5B9BD5;
      --color-primary: #1a5f3f;
      --color-secondary: #5B9BD5;
      --color-accent: #FFD700;
      --color-text: #000000;
  }

  body {
    margin: 0;
    padding: 20px;
    background-color: var(--color-bg);
    font-family: 'Segoe UI', Arial, sans-serif;
    color: var(--color-text);
    min-height: 100vh;
    display: flex;
    flex-direction: column;
    align-items: center;
  }

  .container {
    width: 100%;
    max-width: 500px;
    text-align: center;
  }

  .header-container {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    margin-bottom: 20px;
    padding: 0 10px;
  }
  
  .header-img {
    max-height: 100px;
    width: auto;
    border-radius: 10px;
    box-shadow: 0 4px 15px rgba(0,0,0,0.3);
    border: 4px solid var(--color-border);
  }
  
  .mini-title {
    font-size: 1.1em;
    font-weight: 700;
    color: var(--color-border);
    text-shadow: 1px 1px 2px rgba(0,0,0,0.3);
    margin: 0 5px 0 0;
    display: inline-block;
  }
  
  .mini-subtitle {
    font-size: 1.1em;
    color: var(--color-primary);
    font-weight: 700;
    margin: 0;
    display: inline-block;
  }

  .card {
    background: var(--color-card-bg);
    border: 4px solid var(--color-border);
    border-radius: 15px;
    padding: 20px;
    margin-bottom: 20px;
    box-shadow: 0 8px 30px rgba(0,0,0,0.3);
    text-align: left;
  }
  
  .card.compact {
    padding: 15px;
  }
  
  .card h3 {
    margin-top: 0;
    color: #1a5f3f;
    border-bottom: 2px solid rgba(0,0,0,0.1);
    padding-bottom: 8px;
    margin-bottom: 12px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 1.1em;
  }

  .card.compact h3 {
    font-size: 0.95em;
    padding-bottom: 2px;
    margin-bottom: 5px;
  }

  input, button {
    width: 100%;
    padding: 12px;
    margin: 8px 0;
    box-sizing: border-box;
    border-radius: 12px;
    border: 3px solid var(--color-border);
    font-family: inherit;
    font-size: 1rem;
  }

  input {
    background: white;
    color: black;
  }
  
  input:focus {
    outline: none;
    border-color: var(--color-primary);
  }

  button {
    background: #1a5f3f;
    color: white;
    border: none;
    font-weight: 700;
    cursor: pointer;
    transition: all 0.3s;
    box-shadow: 0 6px 20px rgba(0,0,0,0.3);
  }
  
  button:active { transform: translateY(2px); }
  button:hover { 
      background: var(--color-accent);
      color: black;
      transform: translateY(-2px);
  }
  
  button.play-btn { 
    width: auto; 
    padding: 8px 16px;
    margin: 0; 
    font-size: 0.9em; 
    background: var(--color-secondary);
    color: black;
    font-weight: 600;
  }
  
  button.play-btn:hover {
    background: var(--color-accent);
    color: black;
  }
  
  button.play-btn.playing {
    background: var(--color-accent);
    color: black;
    font-weight: 700;
  }
  
  button.del-btn { 
    width: auto; 
    padding: 8px 12px;
    margin: 0 0 0 5px; 
    font-size: 0.9em; 
    background: #1a5f3f;
    color: white; 
  }
  
  button.del-btn:hover {
    background: var(--color-accent);
    color: black;
  }

  .slider-container-compact label {
    font-size: 0.85em;
    display: inline-block;
    width: 60px;
    text-align: right;
    margin-right: 5px;
  }
  .slider-container-compact {
      display: flex;
      align-items: center;
      margin: 2px 0;
  }
  
  .slider { 
    -webkit-appearance: none; 
    width: 100%; 
    height: 6px;
    border-radius: 5px; 
    background: white; 
    outline: none; 
    margin: 4px 0;
    border: 2px solid var(--color-border);
  }
  
  .slider::-webkit-slider-thumb {
    -webkit-appearance: none; 
    appearance: none;
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: var(--color-accent);
    cursor: pointer;
    box-shadow: 0 0 5px rgba(0,0,0,0.5);
    border: 2px solid #fff;
  }
  
  .slider::-moz-range-thumb {
    width: 20px;
    height: 20px;
    border-radius: 50%;
    background: var(--color-accent);
    cursor: pointer;
    box-shadow: 0 0 5px rgba(0,0,0,0.5);
    border: 2px solid #fff;
  }

  .list-wrapper { 
    max-height: 550px;
    overflow-y: auto; 
    background: white; 
    border-radius: 10px; 
    border: 3px solid var(--color-border); 
  }
  
  .list-item {
    padding: 12px; 
    border-bottom: 1px solid #eee;
    display: flex; 
    justify-content: space-between; 
    align-items: center;
    transition: 0.2s;
    color: #000;
  }
  
  .list-item:last-child { border-bottom: none; }
  .list-item:hover { background: #f0f0f0; }
  .item-name { font-weight: 700; color: #1a5f3f; }
  .item-name.playing { color: #FFD700; font-weight: 800; }
  .item-info { color: #666; font-size: 0.8em; }
  
  .control-buttons {
    display: flex;
    gap: 10px;
    margin-top: 10px;
  }
  
  .control-buttons button {
    flex: 1;
    padding: 10px;
    margin: 0;
    font-size: 0.95em;
  }
</style>
</head>
<body>
  <div class="container">
    
    <div class="header-container">
       <img src="https://cristianiftodewdt81.github.io/ESP32-S3-RS485-CAN-waveshare-WDT81V1/palatiasi.png" 
            alt="Palatul Culturii" class="header-img" 
            onerror="this.style.display='none'">
       <img src="https://cristianiftodewdt81.github.io/ESP32-S3-RS485-CAN-waveshare-WDT81V1/semnatura.png" 
            alt="Semnatura" class="header-img" 
            onerror="this.style.display='none'">
       <img src="https://cristianiftodewdt81.github.io/ESP32-S3-RS485-CAN-waveshare-WDT81V1/logo.png" 
            alt="Logo" class="header-img logo" 
            onerror="this.style.display='none'">
    </div>

    <div class="card compact">
      <div style="margin-bottom: 8px;">
        <span class="mini-title">R WDT81</span>
        <span class="mini-subtitle">Radio Stations</span>
      </div>
      <h3 id="now_playing_header" style="font-size: 1em;">Press ▶ to start playing</h3>
      
       <div class="slider-container-compact">
        <label>🔊 <span id="volVal">75</span>%</label>
        <input type="range" min="0" max="100" value="75" class="slider" id="volRange" oninput="updateVol(this.value)">
      </div>

      <div class="slider-container-compact">
        <label>🎸 <span id="bassVal">10</span></label>
        <input type="range" min="0" max="15" value="10" class="slider" id="bassRange" oninput="updateEq()">
      </div>

      <div class="slider-container-compact">
        <label>🎼 <span id="trebleVal">7</span></label>
        <input type="range" min="0" max="7" value="7" class="slider" id="trebleRange" oninput="updateEq()">
      </div>
      
      <div class="control-buttons">
        <button onclick="playCurrentOrFirst()">▶ PLAY</button>
        <button onclick="stopStream()">⏹ STOP</button>
      </div>
    </div>

    <div class="card">
      <h3>📻 Stations</h3>
      <div id="stream-list" class="list-wrapper">Loading...</div>
      
      <div style="margin-top: 15px; border-top: 1px solid rgba(0,0,0,0.1); padding-top: 15px;">
        <input type="text" id="new_name" placeholder="Station Name">
        <input type="text" id="new_url" placeholder="Stream URL (http://...)">
        <button onclick="addStream()">+ Add Station</button>
      </div>
    </div>

    <div class="card">
      <h3>📡 WiFi Connection</h3>
      <div style="margin-bottom: 10px; font-size: 0.9em; color:#1a5f3f;">
         IP: <span id="ip" style="color:#000;">...</span> | Signal: <span id="rssi" style="color:#1a5f3f; font-weight:bold;">...</span>
      </div>
      <button onclick="scanNetworks()">Scan Networks 🔄</button>
      <div id="net-list" class="list-wrapper" style="margin: 10px 0; display:none;"></div>
      
      <input type="text" id="ssid" placeholder="SSID Name">
      <input type="password" id="pass" placeholder="Password">
      <button onclick="connectWifi()">Connect ⚡</button>
    </div>

    <div class="card">
      <h3>⚠️ System</h3>
      <button onclick="resetDevice()">Reset to Defaults & Reboot 🔄</button>
    </div>

  </div>

<script>
let lastPlayedStation = null;
let currentPlayingStation = null;

function updateNowPlaying(stationName, isPlaying) {
    const header = document.getElementById("now_playing_header");
    if(isPlaying && stationName && stationName !== "Stopped") {
        header.innerHTML = "🎵 Now Playing: <strong>" + stationName + "</strong>";
        currentPlayingStation = stationName;
    } else {
        header.innerHTML = "Press ▶ to start playing";
        currentPlayingStation = null;
    }
    updatePlayButtons();
}

function updatePlayButtons() {
    const buttons = document.querySelectorAll('.play-btn');
    const stationNames = document.querySelectorAll('.item-name');
    
    buttons.forEach(btn => {
        btn.classList.remove('playing');
    });
    
    stationNames.forEach(name => {
        name.classList.remove('playing');
    });
    
    if(currentPlayingStation) {
        buttons.forEach((btn, index) => {
            const stationName = stationNames[index].textContent;
            if(stationName === currentPlayingStation) {
                btn.classList.add('playing');
                stationNames[index].classList.add('playing');
            }
        });
    }
}

function stopStream() {
    fetch('/stop').then(() => {
        updateNowPlaying("", false);
    });
}

function playCurrentOrFirst() {
    if(lastPlayedStation) {
        playStream(lastPlayedStation.name, lastPlayedStation.url);
    } else {
        fetch('/streams').then(r => r.json()).then(data => {
            if(data.length > 0) {
                playStream(data[0].name, data[0].url);
            }
        });
    }
}

function scanNetworks() {
  var list = document.getElementById("net-list");
  list.style.display = "block";
  list.innerHTML = "<div class='list-item'>Scanning...</div>";
  fetch('/scan').then(r => r.json()).then(data => {
    list.innerHTML = "";
    data.forEach(net => {
      var item = document.createElement("div");
      item.className = "list-item";
      item.innerHTML = `<span class="item-name">${net.ssid}</span> <span class="item-info">${net.rssi}dB</span>`;
      item.onclick = function() { 
        document.getElementById("ssid").value = net.ssid; 
        list.style.display = "none";
      };
      item.style.cursor = "pointer";
      list.appendChild(item);
    });
  }).catch(e => {
    list.innerHTML = "<div class='list-item'>Error scanning</div>";
  });
}

function connectWifi() {
  var s = document.getElementById("ssid").value;
  var p = document.getElementById("pass").value;
  if(!s) return alert("Enter SSID");
  fetch('/connect?ssid=' + encodeURIComponent(s) + '&pass=' + encodeURIComponent(p))
    .then(r => alert('Connecting... Check OLED display.'));
}

function updateVol(val) {
  document.getElementById("volVal").innerText = val;
  fetch('/volume?val=' + val);
}

function updateEq() {
  const t = document.getElementById("trebleRange").value;
  const b = document.getElementById("bassRange").value;
  document.getElementById("trebleVal").innerText = t;
  document.getElementById("bassVal").innerText = b;
  fetch('/eq?treble=' + t + '&bass=' + b);
}

function loadStreams() {
  fetch('/streams').then(r => r.json()).then(data => {
    var div = document.getElementById("stream-list");
    div.innerHTML = "";
    if(data.length === 0) div.innerHTML = "<div class='list-item'>No stations</div>";
    data.forEach((st, idx) => {
      var d = document.createElement("div");
      d.className = "list-item";
      d.innerHTML = `
        <div style="flex-grow:1; overflow:hidden;">
           <div class="item-name">${st.name}</div>
           <div class="item-info" style="font-size:0.7em;">${st.url}</div>
        </div>
        <div style="display: flex; gap: 5px;"> 
          <button class="play-btn" onclick="playStream('${st.name}', '${st.url}')">▶</button> 
          <button class="del-btn" onclick="delStream(${idx})">🗑</button> 
        </div>`;
      div.appendChild(d);
    });
    updatePlayButtons();
  }).catch(e => {
    div.innerHTML = "<div class='list-item'>Error loading</div>";
  });
}

function playStream(name, url) {
  lastPlayedStation = {name: name, url: url};
  fetch('/play?name=' + encodeURIComponent(name) + '&url=' + encodeURIComponent(url))
    .then(() => {
        updateNowPlaying(name, true);
    });
}

function addStream() {
  var n = document.getElementById("new_name").value;
  var u = document.getElementById("new_url").value;
  if(!n || !u) return alert("Fill Name and URL!");
  fetch('/streams?action=add&name=' + encodeURIComponent(n) + '&url=' + encodeURIComponent(u))
    .then(() => { 
       document.getElementById("new_name").value=""; 
       document.getElementById("new_url").value=""; 
       loadStreams(); 
    });
}

function delStream(idx) {
  if(confirm("Delete station?")) {
    fetch('/streams?action=del&index=' + idx).then(() => loadStreams());
  }
}

function resetDevice() {
  if(confirm("Reset to default 13 stations and WiFi settings?")) fetch('/reset');
}

function getStatus() {
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById("ip").innerText = d.ip;
    document.getElementById("rssi").innerText = d.rssi + "dB";
    
    // Sincronizăm slider-ul cu volumul real de pe server
    if(d.volume !== undefined) {
        document.getElementById("volRange").value = d.volume;
        document.getElementById("volVal").innerText = d.volume;
    }
    
    // Sync EQ
    if(d.treble !== undefined) {
        // Dacă e > 7 (modul cut), îl afișăm ca 0 sau 1 în slider? 
        // Userul vrea doar 0-7. Dacă e >7, e "pe dos".
        // Ignorăm valorile "stranii" > 7 la afișare, sau le clampăm la 7?
        let t = d.treble;
        if(t > 7) t = 7; 
        document.getElementById("trebleRange").value = t;
        document.getElementById("trebleVal").innerText = t;
    }
    if(d.bass !== undefined) {
        document.getElementById("bassRange").value = d.bass;
        document.getElementById("bassVal").innerText = d.bass;
    }
    
    // Update now playing from server
    if(d.playing && d.station && d.station !== "Stopped") {
        updateNowPlaying(d.station, true);
    } else {
        updateNowPlaying("", false);
    }
  }).catch(e => {
    console.log("Status error:", e);
  });
}

setInterval(getStatus, 2000);
loadStreams();
getStatus();
</script>
</body>
</html>
)rawliteral";

#endif
