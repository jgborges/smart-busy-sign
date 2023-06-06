const char *indexHtml = R"====(
<html>
  <head>
    <title>Smart Busy Sign - Home</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h1>Status Status</h1>
    <input type="button" value="Refresh" onclick="loadStatus()" />
    <ul id="status"></ul>
    <h1>Update WiFi Configuration</h1><br>
    <form method="post" enctype="application/x-www-form-urlencoded" action="/admin/wifi">
      <input type="text" name="ssid" value=""><br>
      <input type="text" name="password" value=""><br>
      <input type="submit" value="Submit">
    </form>
  </body>
  <script>
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
      console.info("Sign status updated!");
    };
  </script>
</html>
)====";