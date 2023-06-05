const char *indexHtml = R"====(
<html>
  <head>
    <title>Smart Busy Sign - Home</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h1>Update Panel Status</h1><br>
    <form method="post" enctype="application/json" action="/status">
      <input type="text" name='{ "panels": [{ "name": "busy", "state": "on-solid" }]}'><br>
      <input type="submit" value="Submit" onclick="window.location.href='/';">
    </form>
    <form method="post" enctype="application/json" action="/status">
      <input type="text" name='{ "panels": [{ "name": "busy", "state": "off" }]}'><br>
      <input type="submit" value="Submit" onclick="window.location.href='/';">
    </form>
    <h1>Update WiFi Configuration</h1><br>
    <form method="post" enctype="application/x-www-form-urlencoded" action="/admin/wifi">
      <input type="text" name="ssid" value=""><br>
      <input type="text" name="password" value=""><br>
      <input type="submit" value="Submit">
    </form>
  </body>
</html>
)====";