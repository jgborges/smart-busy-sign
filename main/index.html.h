const char *indexHtml = R"====(
<html>
  <head>
    <title>Smart Busy Sign - Home</title>
    <!--<style>
      body { font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>-->
  </head>
  <body onload="load()">
    <h1>Sign Status</h1>
    <input type="button" value="Refresh" onclick="loadStatus()" />
    <table id="statusTable">
      <thead><tr>
        <td>Panel Name</td>
        <td>Color</td>
        <td>Status</td>
        <td>Light Itensity</td>
      </tr></thead>
      <tbody>
      </tbody>
    </table>
    <ul id="status"></ul>
    <h1>WiFi Configuration</h1>
    <h4><span id="wifi-warn" style="background-color: yellow;"></span></h4>
    <form id="wifi" onsubmit="updateWifi()">
      <label>WiFi Mode:</label><br>
      <input type="radio" id="sta" name="mode" value="sta"> <label for="sta">WiFi Client (recommended)</label><br>
      <input type="radio" id="ap" name="mode" value="ap"> <label for="ap">Access Point</label><br>

      <label for="css">Network Name (SSID)</label><br>
      <input type="text" id="ssid" name="ssid" value="" maxlength="32"><br>
      <label for="password">WiFi Key (Password)</label><br>
      <input type="password" id="password" name="password" value="" maxlength="64"><br>
      <input type="submit" name="action" value="Submit"><br>
      <span style="font-size: x-small; font-style: italic;">* Device will reboot after WiFi configuration changes. If client mode fails to connect, it will start again in Access Point (AP) mode (default settings)</span>
    </form>
    <h1>Tools</h1>
    <form>
    <input type="button" value="Reboot" onclick="reboot()" /> <input type="button" value="Factory Reset" onclick="factoryReset()" />
    </form>
  </body>
  <script>
    function load() {
      loadStatus();
      loadWifiSettings();
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

      const statusDiv = document.querySelector("#status");
      const tbody = document.querySelector("#statusTable > tbody");
      statusDiv.innerHTML = ""; // clear
      tbody.innerHTML = ""; // clear
      (data || []).panels.forEach(p => {
        const li = document.createElement('li');
        li.textContent = `Panel ${p.name}: color=${p.color}, state=${p.state}, intensity=${p.intensity}`;
        statusDiv.appendChild(li);

        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${p.name}</td><td>${p.color}</td><td>${p.state}</td><td>${p.intensity}</td>`;
        tbody.appendChild(tr);
      });
      console.info("Sign status loaded!");
    }

    async function loadWifiSettings() {
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
      document.getElementById('password').value = data.password;

      if (data.staConnectionFailed == true) {
        document.getElementById("wifi-warn").htmlContent = "Could not connect to configured WiFi network.<br>Check below fields are correct or if netowrk is available. <b>Default AP configuration was used</b>.";
      }
      console.info("Wifi Settings loaded!");
    }

    async function updateWifi(e) {
      e.preventDefault();

      const mode = document.querySelector('#wifi > input[name="mode"]:checked').value;
      const ssid = document.getElementById('ssid').value?.trim();
      const password = document.getElementById('password').value;
      if (!ssid || ssid.length > 32) {
        alert('SSID cannot be empty or larger than 32 characters');
        return;
      }
      if (!!password && (password.length > 64 || password.length < 8)) {
        alert('Passkey must contain at least 8 and at most 64 characters');
        return;
      }

      const response = await post('/admin/wifi', { mode, ssid, password });

      if (response.result == 200) {
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

    async function reboot(e) {
      e.preventDefault();
      console.info("Rebooting device...");
      const response = await post('/admin/reboot');
      console.info(response);
    }

    async function factoryReset(e) {
      e.preventDefault();
      console.info("Setting device to original factory settings...");
      const response = await post('/admin/factory-reset');
      console.info(response);
    }
  </script>
</html>
)====";