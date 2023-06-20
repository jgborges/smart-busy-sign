const char *indexHtml = R"====(
<html>
  <head>
    <link rel="icon" type="image/png" sizes="32x32" href="/resources/favicon-32x32.png"> 
    <title>Smart Busy Sign - Home</title>
    <style>
      body { font-family: Arial, Helvetica, Sans-Serif }

      @keyframes blink {  
        50% {
          background-color: white;
        }
      }
      @-webkit-keyframes blink {
        50% {
          background-color: white;
        }
      }
      .blinking-fast {
        -webkit-animation: blink 0.3s linear infinite;
        -moz-animation: blink 0.3s linear infinite;
        animation: blink 0.3s step-start 0s infinite;
      }
      .blinking {
        -webkit-animation: blink 1s linear infinite;
        -moz-animation: blink 1s linear infinite;
        animation: blink 1s step-start 0s infinite;
      }
      .blinking-slow {
        -webkit-animation: blink 1.5s linear infinite;
        -moz-animation: blink 1.5s linear infinite;
        animation: blink 1.5s step-start 0s infinite;
      }
    </style>
  </head>
  <body onload="load()">
    <h1>Sign Status</h1>
    <table id="statusTable" cellpadding="3px">
      <thead><tr style="background-color: black; color: white">
        <td colspan="2">Panel</td>
        <td>Color</td>
        <td>Status</td>
        <td>Light Intensity</td>
      </tr></thead>
      <tbody>
      </tbody>
    </table>
    <span style="font-size:small; font-style:italic;">* Light intensity ranges from 0 to 255</span><br>
    <input type="button" value="Refresh" onclick="loadStatus()" />
    <h1>Device Information</h1>
    <form>
      <table>
       <tbody>
       <tr>
        <td><label for="signModel">Sign Model</label></td>
        <td><input style="font-family:monospace" type="text" id="signModel" name="signModel" value="" readonly></td>
      </tr>
      <tr>
        <td><label for="serialNumber">Serial Number</label></td>
        <td><input style="font-family:monospace" type="text" id="serialNumber" name="serialNumber" value="" readonly></td>
      </tr>
      <tr>
        <td><label for="manufacturingDate">Manufacturing Date</label></td>
        <td><input style="font-family:monospace" type="text" id="manufacturingDate" name="manufacturingDate" value="" readonly></td>
      </tr>
      <tr>
        <td><label for="boardModel">Board Model</label></td>
        <td><input style="font-family:monospace" type="text" id="boardModel" name="boardModel" size="25" value="" readonly></td>
      </tr>
      <tr>
        <td><label for="firmwareVersion">Firmware Version</label></td>
        <td><input style="font-family:monospace" type="text" id="firmwareVersion" name="firmwareVersion" value="" readonly></td>
      </tr>
    </table>
    </form>
    <h1>WiFi Configuration</h1>
    <h4><span id="wifi-warn" style="background-color: yellow;"></span></h4>
    <form id="wifi">
      <table><tbody>
        <tr>
          <td><label>WiFi Mode</label></td>
          <td>
            <input type="radio" id="sta" name="mode" value="sta"> <label for="sta">WiFi Client (recommended)</label><br>
            <input type="radio" id="ap" name="mode" value="ap"> <label for="ap">Access Point</label><br>
          </td>
        </tr>
        <tr>
          <td><label for="ssid">Network Name (SSID)</label></td>
          <td><input style="font-family:monospace" type="text" id="ssid" name="ssid" value="" maxlength="32"></td>
        </tr>
        <tr>
          <td><label for="psk">Pre-shared Key (Password)</label></td>
          <td>
            <input style="font-family:monospace" type="password" id="psk" name="psk" value="" maxlength="64"><br>
            <input type="checkbox" onclick="togglePassword()">Show Password
          </td>
        </tr>
      </table>
      <input type="submit" name="action" value="Submit"><br>
      <span style="font-size:small; font-style:italic;">Device will reboot after WiFi configuration changes. If client mode fails </br>to connect, it will start again in Access Point (AP) mode (default settings)</span>
    </form>
    <h1>Tools</h1>
    <form>
    <input type="button" value="Reboot" onclick="reboot()" /> <input type="button" value="Factory Reset" onclick="factoryReset()" />
    </form>
  </body>
  <script>
    var colorMap = {
      'red': 'LightPink',
      'white': 'GhostWhite',
      'blue': 'LightBlue',
      'yellow': 'Yellow',
      'green': 'LightGreen',
    };

    function load() {
      loadStatus();
      loadDeviceInfo();
      loadWifiConfig();

      document.querySelector("#wifi").addEventListener("submit", updateWifi);
    }

    function togglePassword() {
      const element = document.getElementById("psk");
      element.type = (element.type === "password") ? "text" : "password";
    }

    async function loadStatus() {
      console.info("Fetching sign status...");
      const response = await fetch('/status', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const data = await response.json(); // read response body as json

      const tbody = document.querySelector("#statusTable > tbody");
      tbody.innerHTML = ""; // clear
      (data || []).panels.forEach(p => {
        const tr = document.createElement('tr');
        const bgcolor = p.state === 'off' ? 'LightGray' : colorMap[p.color] || p.color;
        const blink = p.state.includes("blinking") ? p.state.replace("on-", "") : "";
        const intensityPercentage = Math.round(p.intensity * 100.0 / 255.0) + '%';
        tr.innerHTML = `
          <td><img src="/resources/${p.name}.png" height="32" style="background-color:${bgcolor}" class="${blink}"/></td>
          <td>${p.name}</td>
          <td>${p.color}</td>
          <td>${p.state}</td>
          <td><span title="${p.intensity}">${intensityPercentage}</span></td>`;
        tbody.appendChild(tr);
      });
      console.info("Sign status loaded!");
    }

    async function loadDeviceInfo() {
      console.info("Fetching Device Information...");
      const response = await fetch('/admin/device', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const data = await response.json(); // read response body as json

      document.getElementById('signModel').value = data.signModel;
      document.getElementById('serialNumber').value = data.serialNumber;
      document.getElementById('manufacturingDate').value = data.manufacturingDate;
      document.getElementById('boardModel').value = data.boardModel;
      document.getElementById('firmwareVersion').value = data.firmwareVersion;

      console.info("Device Information loaded!");
    }

    async function loadWifiConfig() {
      console.info("Fetching Wifi Settings...");
      const response = await fetch('/admin/wifi', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const data = await response.json(); // read response body as json

      document.getElementById(data.mode).checked = true;
      document.getElementById('ssid').value = data.ssid;
      document.getElementById('psk').value = data.psk;

      if (data.staConnectionFailed == true) {
        document.getElementById("wifi-warn").htmlContent = "Could not connect to configured WiFi network.<br>Check below fields are correct or if netowrk is available. <b>Default AP configuration was used</b>.";
      }
      console.info("Wifi Settings loaded!");
    }

    async function updateWifi(e) {
      e.preventDefault();

      const mode = document.querySelector('#wifi > input[name="mode"]:checked').value;
      const ssid = document.getElementById('ssid').value?.trim();
      const psk = document.getElementById('psk').value;
      if (!ssid || ssid.length > 32) {
        alert('SSID cannot be empty or larger than 32 characters');
        return;
      }
      if (!!psk && (psk.length > 64 || psk.length < 8)) {
        alert('Pass key must contain at least 8 and at most 64 characters');
        return;
      }

      const response = await post('/admin/wifi', { mode, ssid, psk });

      if (response.ok) {
        document.getElementById("wifi-warn").textContent = "Successfully updated WiFi settings! Device will reboot";
      } else {
        document.getElementById("wifi-warn").textContent = "Failed to update WiFi settings!";
      }
    }

    async function post(url, data = {}) {
      return await fetch(url, {
        method: 'POST',
        redirect: 'manual',
        headers: {
          Accept: 'application.json',
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
      });
    }

    async function reboot() {
      console.info("Rebooting device...");
      const response = await post('/admin/reboot');
      console.info(response);
    }

    async function factoryReset() {
      if (!confirm("Factory reset will erase all your configurations, including WiFi!\n\nAre you sure you want to continue?")) {
        return;
      }
      console.info("Setting device to original factory settings...");
      const response = await post('/admin/factory-reset');
      console.info(response);
    }
  </script>
</html>
)====";