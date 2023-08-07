#ifndef INDEX_HTML_H
#define INDEX_HTML_H

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
      .popup {
        position: fixed;
        z-index: 1;
        left: 0;
        top: 0;
        width: 100%;
        height: 100%;
        overflow: auto;
        background-color: rgba(0, 0, 0, 0.4);
        display: none;
      }
      .popup-content {
        background-color: white;
        margin: 10% auto;
        padding: 20px;
        border: 1px solid #888888;
        width: 20%;
      }
      .show {
        display: block;
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
            <input type="radio" id="sta" name="wifiMode" value="sta"> <label for="sta">WiFi Client (recommended)</label><br>
            <input type="radio" id="ap" name="wifiMode" value="ap"> <label for="ap">Access Point</label><br>
          </td>
        </tr>
        <tr>
          <td><label for="ssid">Network Name (SSID)</label></td>
          <td><input style="font-family:monospace" type="text" id="ssid" name="ssid" value="" maxlength="32"> <button type="button" id="wifi-scan">Scan</button> 
            <div id="ssidList" class="popup">
              <div class="popup-content">
                <h4>Available WiFi networks:</h4>
                <select name="selected-ssid" size="16" style="width:100%;"></select><p>
                <button type="button" id="wifi-select">Select</button> <button type="button" id="wifi-refresh">Refresh</button>
              </div>
            </div>
          </td>
        </tr>
        <tr>
          <td><label for="psk">Pre-shared Key (Password)</label></td>
          <td>
            <input style="font-family:monospace" type="password" id="psk" name="psk" value="" maxlength="64"><br>
            <input type="checkbox" onclick="togglePassword()">Show Password
          </td>
        </tr>
      </table>
      <input id="wifi-submit" type="submit" name="action" value="Submit"><br>
      <span style="font-size:small; font-style:italic;">Device will reboot after WiFi configuration changes. If client mode fails </br>to connect, it will start again in Access Point (AP) mode (default settings)</span>
    </form>
    <h1>Settings</h1>
    <h4><span id="settings-warn" style="background-color: yellow;"></span></h4>
    <form id="settings">
      <table><tbody>
        <tr>
          <td><label for="tzOffset">Timezone</label></td>
          <td>
            <input style="font-family:monospace" type="text" id="tzOffset" name="tzOffset" value="" size="6"> minutes<br>
            <span id="clock" style="font-size:small"></span>
          </td>
        </tr>
        <tr><td colspan="2"><hr><h3>Energy Saving</h3></td></tr>
        <tr>
          <td><label for="autoTurnOffPeriod">Automatic turn-off lights</label></td>
          <td><input style="font-family:monospace" type="text" id="autoTurnOffPeriod" name="autoTurnOffPeriod" value="" size="6"> minutes of inactivity</td>
        </tr>
        <tr>
          <td><label for="autoSleep">Automatic device sleep</label></td>
          <td>
            <input type="checkbox" id="autoSleepEnabled" name="autoSleepEnabled">Auto sleep enabled<br>
            <input style="font-family:monospace" type="text" id="autoSleepPeriod" name="autoSleepPeriod" value="" size="6"> minutes<br>
            Prevent sleep between 
            <input style="font-family:monospace" type="text" id="autoSleepActiveHourStart" name="autoSleepActiveHourStart" value="" maxlength="2" size="4"> and 
            <input style="font-family:monospace" type="text" id="autoSleepActiveHourEnd" name="autoSleepActiveHourEnd" value="" maxlength="2" size="4"> hours of the day<br>
            <span style="font-size:small">
              <input type="checkbox" id="sunday" name="sunday">Sun
              <input type="checkbox" id="monday" name="monday">Mon
              <input type="checkbox" id="tuesday" name="tuesday">Tue
              <input type="checkbox" id="wednesday" name="wednesday">Wed
              <input type="checkbox" id="thursday" name="thursday">Thu
              <input type="checkbox" id="friday" name="friday">Fri
              <input type="checkbox" id="saturday" name="saturday">Sat
            </span>
          </td>
        </tr>
      </table>
      <input type="submit" name="action" value="Submit"><br>
    </form>
    <h1>Tools</h1>
    <form>
    <input type="button" value="Reboot" onclick="reboot()" /> 
    <input type="button" value="Sleep" onclick="sleep()" /> 
    <input type="button" value="Factory Reset" onclick="factoryReset()" />
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

    var daysOfWeek = [ 'sunday', 'monday', 'tuesday', 'wednesday', 'thursday', 'friday', 'saturday' ];

    function load() {
      loadStatus();
      loadDeviceInfo();
      loadWifiConfig();
      loadUserSettings();

      // setup config form buttons
      document.querySelector("#wifi").addEventListener("submit", updateWifiConfig);
      document.querySelector("#settings").addEventListener("submit", updateUserSettings);

      // setup wifi scan buttons
      const ssidList = document.querySelector("#ssidList");
      document.querySelector("#wifi-scan").addEventListener("click", showWifiList);
      document.querySelector("#wifi-refresh").addEventListener("click", showWifiList);
      document.querySelector("#wifi-select").addEventListener("click", () => {
        const wifi = document.querySelector("#ssidList > div.popup-content > select");
        if (!!wifi.value) {
          document.querySelector("#ssid").value = wifi.value;
          ssidList.classList.remove("show");
        }
      });
      window.addEventListener("click", (event) => {
        if (event.target == ssidList) {
          ssidList.classList.remove("show");
        }
      });
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
      document.getElementById('clock').textContent = new Date(data.timestamp*1000).toISOString();
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
      console.info("Fetching Wifi Configuration...");
      const response = await fetch('/admin/wifi', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const data = await response.json(); // read response body as json

      document.getElementById(data.mode).checked = true; // data.mode will be sta or ap
      document.getElementById('ssid').value = data.ssid;
      document.getElementById('psk').value = data.psk;

      if (data.staConnectionFailed == true) {
        document.getElementById("wifi-warn").htmlContent = "Could not connect to configured WiFi network.<br>Check below fields are correct or if netowrk is available. <b>Default AP configuration was used</b>.";
      }
      console.info("Wifi Settings loaded!");
    }

    async function loadUserSettings() {
      console.info("Fetching User Settings...");
      const response = await fetch('/admin/settings', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const data = await response.json(); // read response body as json

      document.getElementById('tzOffset').value = data.tzOffset; // data.mode will be sta or ap
      document.getElementById('autoTurnOffPeriod').value = data.autoTurnOffPeriod;
      document.getElementById('autoSleepEnabled').checked = data.autoSleep?.enabled === true;
      document.getElementById('autoSleepPeriod').value = data.autoSleep?.period;
      document.getElementById('autoSleepActiveHourStart').value = data.autoSleep?.activeHourStart;
      document.getElementById('autoSleepActiveHourEnd').value = data.autoSleep?.activeHourEnd;

      for (const day of daysOfWeek) {
        if (data.autoSleep?.activeDaysOfWeek?.includes(day)) {
          document.getElementById(day).checked = true;
        }
      }

      console.info("User Settings loaded!");
    }

    async function updateWifiConfig(e) {
      e.preventDefault();
      document.getElementById("wifi-warn").textContent = "";

      const mode = document.querySelector('input[name="wifiMode"]:checked')?.value;
      const ssid = document.getElementById('ssid').value?.trim();
      const psk = document.getElementById('psk').value;
      if (!mode) {
        alert('Please select a WiFi mode (Client or AP)');
        return;
      }
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
        document.getElementById("wifi-warn").textContent = "Successfully updated WiFi configuration! Device will reboot";
      } else {
        document.getElementById("wifi-warn").textContent = "Failed to update WiFi configuration!";
      }
    }

    async function showWifiList() {
      const list = await fetchWifiList();
      if (list == null) {
        alert("Could not fetch list of WiFi networks. Try again");
        return;
      }
      document.querySelector("#ssidList > div.popup-content > select").innerHTML = list.map(ssid => `<option value="${ssid}">${ssid}</option>`).join('\n');
      document.querySelector("#ssidList").classList.add("show");
    }

    async function fetchWifiList() {
      console.info("Fetching Wifi Configuration...");
      const response = await fetch('/admin/scan', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      if (!response.ok) {
        return null;
      } else if (response.status == 202) {
        await wait(3000);
        return await fetchWifiList();
      } else {
        const data = await response.json()
        return [...new Set(
          data
            .sort((a, b) => b.rssi - a.rssi)
            .map(item => item.ssid)
        )];
      }
    }

    async function updateUserSettings(e) {
      e.preventDefault();

      document.getElementById("settings-warn").textContent = "";

      const tzOffset = document.getElementById('tzOffset').value?.trim();
      const autoTurnOffPeriod = document.getElementById('autoTurnOffPeriod').value?.trim();
      const autoSleepEnabled = document.getElementById('autoSleepEnabled').checked === true;
      const autoSleepPeriod = document.getElementById('autoSleepPeriod').value?.trim();
      const autoSleepActiveHourStart = document.getElementById('autoSleepActiveHourStart').value?.trim();
      const autoSleepActiveHourEnd = document.getElementById('autoSleepActiveHourEnd').value?.trim();
      const autoSleepActiveDaysOfWeek = daysOfWeek.filter(d => document.getElementById(d).checked);

      const body = {};
      if (!tzOffset || +tzOffset < (-11*60) || +tzOffset > (11*60)) {
        alert('Please select a timezone between -11h and +11h');
        return;
      } else {
        body.tzOffset = +tzOffset;
      }

      if (!autoTurnOffPeriod || +autoTurnOffPeriod <= 0 || +autoTurnOffPeriod > (24*60)) {
        alert('Please select a value for auto turn-off between 1 min and 1440 min (24h)');
        return;
      } else {
        body.autoTurnOffPeriod = +autoTurnOffPeriod;
      }

      if (autoSleepEnabled) {
        if (!autoSleepPeriod || +autoSleepPeriod <= 0 || +autoSleepPeriod > 720) {
          alert('Please select a value for the period for inactivity before sleep between 1 min and 720 min (12h)');
          return;
        } else if (!autoSleepActiveHourStart || +autoSleepActiveHourStart < 0 || +autoSleepActiveHourStart > 23) {
          alert('Please select a value for active time start hour in the interval 0h to 23h');
          return;
        } else if (!autoSleepActiveHourEnd || +autoSleepActiveHourEnd < 0 || +autoSleepActiveHourEnd > 23) {
          alert('Please select a value for active time end hour in the interval 0h to 23h');
          return;
        } else if (+autoSleepActiveHourStart == +autoSleepActiveHourEnd) {
          alert('Please select a value for active time start hour that is not equal to end hour');
          return;
        } else if (autoSleepActiveDaysOfWeek.lengrh == 0) {
          alert('Please select at least on day of the week for active time');
          return;
        }

        body.autoSleep = {
          enabled: true,
          period: +autoSleepPeriod,
          activeHourStart: +autoSleepActiveHourStart,
          activeHourEnd: +autoSleepActiveHourEnd,
          activeDaysOfWeek: autoSleepActiveDaysOfWeek,
        }
      } else {
        body.autoSleep = { enabled: false };
      }

      const response = await post('/admin/settings', body);

      if (response.ok) {
        document.getElementById("settings-warn").textContent = "Successfully updated settings!";
      } else {
        document.getElementById("settings-warn").textContent = "Failed to update settings!";
      }
    }

    async function post(url, data = {}) {
      return await fetch(url, {
        method: 'POST',
        headers: {
          'Accept': 'application/json, text/plain, */*',
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
      });
    }

    function wait(time) {
      return new Promise((resolve) => setTimeout(resolve, time))
    }

    async function reboot() {
      console.info("Rebooting device...");
      const response = await post('/admin/reboot');
      console.info(response);
    }

    async function sleep() {
      const time = prompt("Sleep will put your device into a low energy state. Presse reset button to wake-up.\n\nPlease select the time in minutes to sleep (0=indefinetely):", 15);
      if (time != null && +time >= 0) {
        console.info("Making device sleep...");
        const response = await post('/admin/sleep', { value: time });
        console.info(response);
      }    
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

#endif
