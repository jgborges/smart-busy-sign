const char *indexHtml = R"====(
<html>
  <head>
    <title>Smart Busy Sign - Home</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body onload="load()">
    <h1>Sign Status</h1>
    <form>
      <input type="button" value="Refresh" onclick="loadStatus()" />
    </form>
    <ul id="status"></ul>
    <h1>WiFi Configuration</h1>
    <form id="wifi" method="post" enctype="application/x-www-form-urlencoded" action="/admin/wifi">
      <label>WiFi Mode:</label><br>
      <input type="radio" id="sta" name="mode" value="sta"> <label for="sta">WiFi Client (recommended)</label><br>
      <input type="radio" id="ap" name="mode" value="ap"> <label for="ap">Access Point</label><br>

      <label for="css">Network Name (SSID)</label><br>
      <input type="text" id="ssid" name="ssid" value="" maxlength="32"><br>
      <label for="password">WiFi Key (Password)</label><br>
      <input type="text" id="password" name="password" value="" maxlength="64"><br>
      <input type="submit" value="Submit"><br>
      <span><i>* Device will reboot after WiFi configuration changes. If client mode fails to connect, it will start again in Access Point (AP) mode (default settings)</i></span>
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

      const text = await response.text(); // read response body as text
      const data = JSON.parse(text);
      const statusDiv = document.querySelector("#status");
      statusDiv.innerHTML = ""; // clear
      (data || []).panels.forEach(p => {
        const li = document.createElement('li');
        li.textContent = `Panel ${p.name}: color=${p.color}, state=${p.state}, intensity=${p.intensity}`;
        statusDiv.appendChild(li);
      });
      console.info("Sign status loaded!");
    };

    async function loadWifiSettings() {
      console.info("Fetching Wifi Settings...");
      const response = await fetch('/admin/wifi', {
        method: 'GET',
        headers: { 
          'Content-Type': 'application/json',
        },
      });

      const text = await response.text(); // read response body as text
      const data = JSON.parse(text);

      document.getElementById(data.mode).checked = true;
      document.getElementById('ssid').value = data.ssid;
      document.getElementById('password').value = data.password;

      console.info("Wifi Settings loaded!");
    }

    async function reboot() {

    }

    async function factoryReset() {
      
    }
  </script>
</html>
)====";