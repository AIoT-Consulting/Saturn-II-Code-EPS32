// note R"KEYWORD( html page code )KEYWORD"; 
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R":::::(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">
<head>

<title>AIoT Pond Monitor</title>
<script src="https://aiot-consulting.github.io/Static-Assets/gauge.js" async defer></script>

  <style>
     body {
        position: relative;
        top: 50px;
        display:block;
        width: 100%;
        height: 100%;
        background-image: url('https://aiot-consulting.github.io/Static-Assets/pond_backgrd.jpg');
    }
    span {
        position: relative;
        font-size:12px;
        color: white;
        vertical-align:top;
        top: 3px;
    }
    h1 {
      margin:0;
      padding:0;
      font-family: 'Times New Roman', Times, serif;
      text-align:center;
      color:rgb(255, 255, 255);
      font-size:60px;
      padding: 20px 0;
      letter-spacing:2px;
      text-shadow: #34a198 3px 3px;
    }
    h2 {
      padding: 0px;
      margin:0px;
      font-family: arial;
      text-align:center;
      color:White;
      font-size:16px;
      letter-spacing:1px;
      text-shadow: #333 1px 1px;
    }
    h3 {
      margin:0;
      padding:0;
      font-family: arial;
      text-align:center;
      color:White;
      font-size:26px;
      padding: 0px 0;
      letter-spacing:2px;
      text-shadow: #34a198 2px 2px;
    }
    h4 {
      margin:0;
      padding:4px 4px;
      font-family: arial;
      color:black;
      font-size:12px;
    }
    table {
      position: relative;
      width:100%;
      border-spacing: 0px;
    }
    #table_G1{
      table-layout: auto;
      width: 60%;
      max-width: 900px;
      min-width: 680px;
      height: auto;
      border: 0px solid black;
      padding: 0px;
    }
    #table_G3{
      table-layout: auto;
      width: 60%;
      max-width: 800px;
      min-width: 680px;
      height: auto;
      border: 2px solid rgb(203, 202, 202);
      border-radius: 5px;
      background-color: white;
      padding: 0px;
    }
    #table_data{
      table-layout: auto;
      width: 60%;
      max-width: 800px;
      min-width: 680px;
      height: auto;
      border: 3px solid rgb(203, 202, 202);
      background-color: grey;
      border-radius: 5px;
      padding: 0px;
      vertical-align: middle;
    }
    #table_S{
      table-layout: auto;
      width: 60%;
      max-width: 800px;
      min-width: 670px;
      height: auto;
      border: 0px solid white;
      padding: 0px;
    }
    tr {
      border: 1px solid white;
      font-family: "Verdana", "Arial", sans-serif;
      font-size: 20px;
    }
    th {
      height: 20px;
      padding: 3px 15px;
      background-color: #343a40;
      color: #FFFFFF !important;
      }
    td {
      border: 0px solid white;
      white-space: nowrap;
      vertical-align: middle;
      text-align: center;
    }
    input[type="checkbox"] {
      margin: 2px;
      position:relative;
      vertical-align:middle;
      margin-top: 10px;
      width:96px;
      height:32px;
      -webkit-appearance: none;
      background: linear-gradient(0deg, #333, #000);
      opacity: 0.8;
      outline: none;
      cursor: pointer;
      border-radius: 16px;
      box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    input:hover {
        opacity: 1;
    }
    input:checked[type="checkbox"]:nth-of-type(1) {
      background: linear-gradient(0deg, #70a1ff, #1e90ff);
      box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    input[type="checkbox"]:before {
      content:'';
      position:absolute;
      top:0;
      left:0;
      width:64px;
      height:32px;
      background: linear-gradient(0deg, #000, #6b6b6b);
      border-radius: 16px;
      box-shadow: 0 0 0 1px #232323;
      transform: scale(.98,.96);
      transition:1.0s;
    }
    input:checked[type="checkbox"]:before {
      left:32px;
    }
    input[type="checkbox"]:after{
      content:'';
      position:absolute;
      top:calc(50% - 2px);
      left:56px;
      width:4px;
      height:4px;
      background: linear-gradient(0deg, #6b6b6b, #000);
      border-radius: 50%;
      transition:1.0s;
    }
    input:checked[type="checkbox"]:after {
      left:88px;
    }
    .switch1-holder {
        width: 130px;
        height:32px;
        padding: 10px 10px;
        border-radius: 10px;
        margin-bottom: 30px;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
        justify-content: space-between;
        align-items: center;
    }
    .switch1-toggle {
        height: 40px;
    }
    .switch1-toggle input[type="checkbox"] {
        position: absolute;
        opacity: 0;
        z-index: -2;
    }
    .switch1-toggle input[type="checkbox"] + label {
        position: relative;
        display: inline-block;
        width: 100px;
        height: 40px;
        vertical-align: bottom;
        align-items: center;
        border-radius: 20px;
        margin: 0;
        margin-top: 12px;
        cursor: pointer;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    .switch1-toggle input[type="checkbox"] + label::before {
        position: absolute;
        content: '';
        color: #fff;
        font-size: 14px;
        text-align: center;
        line-height: 30px;
        top: 8px;
        left: 8px;
        width: 88px;
        height: 28px;
        border-radius: 20px;
        background-color: #ce0000;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
        transition: .3s ease-in-out;
    }
    .switch1-toggle input[type="checkbox"]:checked + label::before {
        left: 4px;
        top: 4px;
        width: 88px;
        height: 28px;
        font-size:12px;
        content: 'RESETTING';
        color: #fff;
        font-weight:bold; 
        background-color: #b31b00;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    .switch2-holder {
        width: 130px;
        height:32px;
        padding: 10px 10px;
        border-radius: 10px;
        margin-bottom: 30px;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
        justify-content: space-between;
        align-items: center;
    }
    .switch2-toggle {
        height: 40px;
    }
    .switch2-toggle input[type="checkbox"] {
        position: absolute;
        opacity: 0;
        z-index: -2;
    }
    .switch2-toggle input[type="checkbox"] + label {
        position: relative;
        display: inline-block;
        width: 100px;
        height: 40px;
        vertical-align: bottom;
        align-items: center;
        border-radius: 20px;
        margin: 0;
        margin-top: 12px;
        cursor: pointer;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    .switch2-toggle input[type="checkbox"] + label::before {
        position: absolute;
        content: 'OFF';
        color: #fff;
        font-size: 14px;
        text-align: center;
        line-height: 30px;
        top: 8px;
        left: 8px;
        width: 90px;
        height: 30px;
        border-radius: 20px;
        background-color: #2caa0d;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
        transition: .3s ease-in-out;
    }
    .switch2-toggle input[type="checkbox"]:checked + label::before {
        left: 6px;
        top: 6px;
        width: 88px;
        height: 28px;
        content: 'OVERRIDE';
        color: #fff;
        font-weight:bold; 
        background-color: #b31b00;
        box-shadow: 0 0 0 4px #353535, 0 0 0 5px #3e3e3e, inset 0 0 10px rgba(0,0,0,1);
    }
    .container {
      display: block;
      margin-left: auto;
      margin-right: auto;
      background: rgba(225,225,225,0.75);
      border-radius: 50px;
      width: 50%;
      height: auto;
      text-align: center;
      min-width: 800px;
      max-width: 1000px;
    } 
  </style>
  </head>
  
  <body onload="process(); ">
    <main class="container" style="margin-top:30px">
    <center><table id="table_G1">
            <tr>
            <td><center><h1>Pond Dashboard</h1></center>
            <h3>Monitoring System</h2><br></td></center>   
      </table></center>
      <center><table id="table_G1">
      <tr>
      <!-- Pond Water Level gauge -->     
              <td align="left">
              <canvas id="gauge_1" data-type="linear-gauge"
                    data-width="115"
                    data-height="450"
                    data-border-radius="10"
                    data-bar-begin-circle="false"
                    data-units="gals"
                    data-font-units-size="30"
                    data-font-units-style="bolder"
                    data-title="Level"
                    data-font-title-style="bolder"
                    data-font-title-size="25"
                    data-color-title="black"
                    data-value="50"
                    data-value-Int="1"
                    data-value-Dec="1"
                    data-animate-on-init="true"
                    data-animated-value="true"
                    data-min-value="0"
                    data-max-value="1300"
                    data-major-ticks="0,100,200,300,400,500,600,700,800,900,1000,1110,1200,1300"
                    data-minor-ticks="10"
                    data-highlights='[
                    { "from": 0, "to": 900, "color": "rgba(193, 51, 51,.75)" },
                    { "from": 900, "to": 1100, "color": "rgba(255, 241, 41,.85)" },
                    { "from": 1100, "to": 1200, "color": "rgba(44, 201, 55,.75)" },
                    { "from": 1200, "to": 1300, "color": "rgba(255, 241, 41,.85)" }                      
                    ]'
                    data-color-needle-start="rgba(240, 128, 128, 1)"
                    data-color-needle-end="rgba(255, 160, 122, .9)"
                    data-value-box="true"
                    data-value-text-shadow="true"
                    data-animation-rule="linear"
                    data-animation-duration="500"
                    data-border-outer-width="3"
                    data-border-middle-width="2"
                    data-border-inner-width="0">
              </canvas>
              </td>
              <!-- Pond Water Temperature gauge -->     
              <td align="left">
                <canvas id="gauge_2" data-type="linear-gauge"
                    data-width="115"
                    data-height="450"
                    data-border-radius="10"
                    data-units="&#8451"
                    data-font-units-size="30"
                    data-font-units-style="bolder"
                    data-title="Water Temp"
                    data-font-title-style="bolder"
                    data-font-title-size="25"
                    data-color-title="black"
                    data-value="0"
                    data-value-Int="1"
                    data-value-Dec="1"
                    data-animate-on-init="true"
                    data-animated-value="true"
                    data-min-value="-10"
                    data-max-value="40"
                    data-major-ticks="-10,-5,0,5,10,15,20,25,30,35,40"
                    data-minor-ticks="5"
                    data-highlights='[
                    { "from": -10, "to": 0, "color": "rgba(193, 51, 51,.75)" },
                    { "from": 0, "to": 15, "color": "rgba(255, 241, 41,.85)" },
                    { "from": 15, "to": 25, "color": "rgba(44, 201, 55,.75)" },
                    { "from": 25, "to": 32, "color": "rgba(255, 241, 41,.85)" },
                    { "from": 32, "to": 40, "color": "rgba(193, 51, 51,.75)" }                     
                    ]'
                    data-color-needle-start="rgba(240, 128, 128, 1)"
                    data-color-needle-end="rgba(255, 160, 122, .9)"
                    data-value-box="true"
                    data-animation-rule="linear"
                    data-animation-duration="500"
                    data-border-outer-width="3"
                    data-border-middle-width="2"
                    data-border-inner-width="0">
                     </canvas>
                </td>
                <!-- Fish Food gauge -->
                <td align="center">
                <canvas id="gauge_3" data-type="radial-gauge"
                    data-width="225"
                    data-height="225"
                    data-units="&#xFF05"
                    data-font-title-style="bolder"
                    data-color-title="black"
                    data-title="Fish Food"
                    data-value="5"
                    data-value-Int="1"
                    data-value-Dec="0"
                    data-animate-on-init="true"
                    data-animated-value="true"
                    data-min-value="0"
                    data-max-value="100"
                    data-major-ticks="0,10,20,30,40,50,60,70,80,90,100"
                    data-minor-ticks="10"
                    data-highlights='[
                    { "from": 0, "to": 10, "color": "rgba(193, 51, 51,.75)" },
                    { "from": 10, "to": 30, "color": "rgba(255, 241, 41,.85)" },
                    { "from": 30, "to": 100, "color": "rgba(44, 201, 55,.75)" }                        
                    ]'
                    data-color-needle-start="rgba(240, 128, 128, 1)"
                    data-color-needle-end="rgba(255, 160, 122, .9)"
                    data-value-box="true"
                    data-animation-rule="linear"
                    data-animation-duration="500"
                    data-border-outer-width="8"
                    data-border-middle-width="5"
                    data-border-inner-width="3">
                </canvas><br>
                <!-- Air Pressure gauge -->
                <canvas id="gauge_4" data-type="radial-gauge"
                    data-width="225"
                    data-height="225"
                    data-units="psi"
                    data-title="Air Pump Press"
                    data-font-title-style="bolder"
                    data-color-title="black"
                    data-value="5"
                    data-value-Int="1"
                    data-value-Dec="1"
                    data-animate-on-init="true"
                    data-animated-value="true"
                    data-min-value="0"
                    data-max-value="5"
                    data-major-ticks="0,1,2,3,4,5"
                    data-minor-ticks="10"
                    data-highlights='[
                    { "from": 0, "to": 2, "color": "rgba(193, 51, 51,.75)" },
                    { "from": 2, "to": 4, "color": "rgba(255, 241, 41,.85)" },
                    { "from": 4, "to": 5, "color": "rgba(44, 201, 55,.75)" }                       
                    ]'
                    data-color-needle-start="rgba(240, 128, 128, 1)"
                    data-color-needle-end="rgba(255, 160, 122, .9)"
                    data-value-box="true"
                    data-animation-rule="linear"
                    data-animation-duration="500"
                    data-border-outer-width="8"
                    data-border-middle-width="5"
                    data-border-inner-width="3">
                </canvas>
                </td>
                <td style="vertical-align:bottom">
                    <center><h3>Status</h3><br>
                    <img src="https://aiot-consulting.github.io/Static-Assets/online.png" alt="Connection Status"  width="115" id="statusIndicator0">
                    <br>
                    <br><img src="https://aiot-consulting.github.io/Static-Assets/water_lvl_ok.png" alt="level_ok"  width="115" id="statusIndicator1">
                    <br><img src="https://aiot-consulting.github.io/Static-Assets/temp_ok.png" alt="temp_ok"  width="115" id="statusIndicator2">
                    <br><img src="https://aiot-consulting.github.io/Static-Assets/tds_ok.png" alt="tds_ok"  width="115" id="statusIndicator3">
                    <br><img src="https://aiot-consulting.github.io/Static-Assets/feed_ok.png" alt="feed_ok"  width="115" id="statusIndicator4"></center>
                </tr>
                </table>
                </center>
                <center>
                <table id="table_G3">
                    <tr>
                        <td align="center">
                            <!-- TDS Value gauge -->
                            <canvas id="gauge_5" data-type="linear-gauge"
                            data-width="646"
                            data-height="130"
                            data-bar-stroke-width="0"
                            data-bar-width="0"
                            data-needle-width="20"
                            data-needle-side="left"
                            data-border-radius="5"
                            data-bar-begin-circle="false"
                            data-ticks-width="20"
                            data-title="TDS (Total Disolved Solids)"
                            data-font-title-size="30"
                            data-font-title-Style="bolder"
                            data-color-title="black"
                            data-units="ppm"
                            data-font-units-size="25"
                            data-font-units-Style="bolder"
                            data-minor-ticks="10"
                            data-highlights='[ 
                            { "from": 0, "to": 50, "color": "rgba(206, 255, 255,.75)" },
                            { "from": 50, "to": 100, "color": "rgba(148, 203, 255,.75)" },
                            { "from": 100, "to": 200, "color": "rgba(16, 154, 222,.75)" },
                            { "from": 200, "to": 300, "color": "rgba(99, 81, 255,.75)" },
                            { "from": 300, "to": 400, "color": "rgba(153, 204, 0,.75)" },
                            { "from": 400, "to": 500, "color": "rgba(132, 130, 0,.75)" },
                            { "from": 500, "to": 600, "color": "rgba(123, 37, 0,.75)" }                 
                            ]'
                            data-value="500"
                            data-value-Int="1"
                            data-value-Dec="0"
                            data-min-value="0"
                            data-max-value="600"
                            data-major-ticks="0,100,200,300,500,500,600"
                            data-tick-side="right"
                            data-number-side="right"
                            data-animation-rule="bounce"
                            data-animation-duration="750"
                            data-value-text-shadow="true"
                            data-border-outer-width="0"
                            data-border-middle-width="0"
                            data-border-shadow-width="0"
                            data-border-inner-width="0">
                        </canvas>
                        </td> 
                    </tr>   
                </table>
                
            <table id="table_data">
            <tr><td style="text-align: left" width="12%";><h4>&nbsp;Water Temp:</h4><h4>&nbsp;Max Water Temp:</h4></td><td style="text-align: left" width="8%";><span id="dataTable1"> ...</span><span> &#8451</span><br><span id="dataTable2"> ...</span><span> &#8451</span></h4></td>
                <td width="2%";></td>
                <td style="text-align: left" width="12%";><h4>&nbsp;Filter Press:</h4><h4>&nbsp;Air Pump Press:</h4></td><td style="text-align: left" width="10%";><span id="dataTable3"> ...</span><span> psi</span><br><span id="dataTable4"> ...</span><span> psi</span></td>
                <td width="2%";></td>
                <td style="text-align: left" width="12%";><h4>&nbsp;Water Level:</h4><h4>&nbsp;TDS Value:</h4></td><td style="text-align: left" width="10%";><span id="dataTable5"> ...</span><span> gals</span><br><span id="dataTable6"> ...</span><span> ppm</span></td>
                <td width="2%";></td>
                <td style="text-align: left" width="12%";><h4>&nbsp;MCU Temp:</h4><h4>&nbsp;Timer:</h4></td><td style="text-align: left" width="18%";><span id="dataTable7"> ...</span><span> &#8451</span><br><span id="dataTable8"> ...</span></td>
            </tr>   
            </table>
            <table id="table_S">
            <tr>
            <form id="check_SWITCHES" name="SW_form">
            <td width="15%"><center><input type="checkbox" name="SW1" onclick="buttonPressOne()"></td></center>
            <td width="15%"></td>
            <td width="15%"><center><input type="checkbox" name="SW2" onclick="buttonPressTwo()"></td></center>
            <td width="10%"></td>
            <td width="15%" class="switch1-toggle"><center><input type="checkbox" id="SW3" name="SW3" onclick="buttonPressThree()" ><label for="SW3"></label></td></center>
            <td width="15%"></td>
            <td width="15%" class="switch2-toggle"><center><input type="checkbox" id="SW4" name="SW4" onclick="buttonPressFour()"><label for="SW4"></label></td></center>
            <tr>
            <td width="15%"><h2>Filter Pump</h2><td width="10%"></td><td width="15%"><h2>Air Pump</h2></td><td width="20%"></td><td width="15%"><h2>Reset</h2></td><td width="10%"></td><td width="15%"><h2>Override</h2></td></td></tr>
            </table>
            </center>   
  </main>

  <center><footer div class="foot" id = "temp" >Powered by AIoT Consulting - Firmware Vers. 1.0</div></footer></center>
  
  </body>


  <script type = "text/javascript">
    var offline = 0;
    var online = 0;
    // global variable visible to all java functions
    var xmlHttp=createXmlHttpObject();

    // function to create XML object
    function createXmlHttpObject(){
      if(window.XMLHttpRequest){
        xmlHttp=new XMLHttpRequest();
        }
      else{
        xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
      }
      return xmlHttp;
    }

   function buttonPressOne() {
      var xhttp = new XMLHttpRequest();
      xhttp.open("PUT", "buttonOne", false);
      xhttp.send();
    }

    function buttonPressTwo() {
      var xhttp = new XMLHttpRequest(); 
      xhttp.open("PUT", "buttonTwo", false);
      xhttp.send(); 
    }

    function buttonPressThree() {
      var xhttp = new XMLHttpRequest(); 
      xhttp.open("PUT", "buttonThree", false);
      xhttp.send(); 
    }

    function buttonPressFour() {
      var xhttp = new XMLHttpRequest(); 
      xhttp.open("PUT", "buttonFour", false);
      xhttp.send(); 
    }

// function to handle the response from the ESP
    function isonline() {
    var image0 = document.getElementById('statusIndicator0');
    if(xmlHttp.readyState==4) {
      online = 1;
      offline = 0;
      } else  {
      offline++;
      if (offline > 30)   {
        online = 0;
        }
      }
    if (online == 1) {          
      image0.src='https://aiot-consulting.github.io/Static-Assets/online.png';
      } else  {
      image0.src='https://aiot-consulting.github.io/Static-Assets/offline.png';
      }
    setTimeout("isonline()",500);
    }
    isonline();

    // function to handle the response from the ESP
    function response(){
      var message;
      var xmlResponse;
      var xmldoc;
      var image1 = document.getElementById('statusIndicator1');
      var image2 = document.getElementById('statusIndicator2');
      var image3 = document.getElementById('statusIndicator3');
      var image4 = document.getElementById('statusIndicator4');     
      // get the xml stream
      xmlResponse=xmlHttp.responseXML;
        
      // Gauge 1 Liquid Level
      xmldoc = xmlResponse.getElementsByTagName("G"); 
      message = xmldoc[0].firstChild.nodeValue;
      document.getElementById("gauge_1").setAttribute("data-value", message);
      document.getElementById("dataTable5").innerHTML = message;
                 
      // Gauge 2 Liquid Temperature
      xmldoc = xmlResponse.getElementsByTagName("G");
      message = xmldoc[1].firstChild.nodeValue;
      
      document.getElementById("gauge_2").setAttribute("data-value", message);
      document.getElementById("dataTable1").innerHTML = message;
      
      // Gauge 3 Feeder Level
      xmldoc = xmlResponse.getElementsByTagName("G");
      message = xmldoc[2].firstChild.nodeValue;
      document.getElementById("gauge_3").setAttribute("data-value", message);

      // Gauge 4 Pressure Level
      xmldoc = xmlResponse.getElementsByTagName("G");
      message = xmldoc[3].firstChild.nodeValue;
      document.getElementById("gauge_4").setAttribute("data-value", message);      
      
      // Gauge 5 TDS
      xmldoc = xmlResponse.getElementsByTagName("G");
      message = xmldoc[4].firstChild.nodeValue;
      document.getElementById("gauge_5").setAttribute("data-value", message);
      
      // Switch 1
      if (this.responseXML.getElementsByTagName('SW')[0].childNodes[0].nodeValue === "1") {         
                document.SW_form.SW1.checked = true;
              }
              else {
                document.SW_form.SW1.checked = false;
              }
         
      //SWITCH 2
      if (this.responseXML.getElementsByTagName('SW')[1].childNodes[0].nodeValue === "1") {         
                document.SW_form.SW2.checked = true;
              }
              else {
                document.SW_form.SW2.checked = false;
              }

      //SWITCH 3
      if (this.responseXML.getElementsByTagName('SW')[2].childNodes[0].nodeValue === "1") {         
                document.SW_form.SW3.checked = true;
              }
              else {
                document.SW_form.SW3.checked = false;
              }

      //SWITCH 4
      if (this.responseXML.getElementsByTagName('SW')[3].childNodes[0].nodeValue === "1") {         
                document.SW_form.SW4.checked = true;
              }
              else {
                document.SW_form.SW4.checked = false;
              }

      // Status Indicator #1
      xmldoc = xmlResponse.getElementsByTagName("SI"); 
      message = xmldoc[0].firstChild.nodeValue;
      if (message == 1) {          
        image1.src='https://aiot-consulting.github.io/Static-Assets/water_lvl_ok.png';
                }
                else {
        image1.src='https://aiot-consulting.github.io/Static-Assets/water_lvl_low.png';
                }
      
      // Status Indicator #2
      xmldoc = xmlResponse.getElementsByTagName("SI"); 
      message = xmldoc[1].firstChild.nodeValue;
      if (message == 1) {          
        image2.src='https://aiot-consulting.github.io/Static-Assets/temp_ok.png';
                }
                else {
        image2.src='https://aiot-consulting.github.io/Static-Assets/temp_high.png';
                }
      // Status Indicator #3
      xmldoc = xmlResponse.getElementsByTagName("SI"); 
      message = xmldoc[2].firstChild.nodeValue;
      if (message == 1) {          
        image3.src='https://aiot-consulting.github.io/Static-Assets/tds_ok.png';
                }
                else {
        image3.src='https://aiot-consulting.github.io/Static-Assets/tds_high.png';
                }

      // Status Indicator #4
      xmldoc = xmlResponse.getElementsByTagName("SI"); 
      message = xmldoc[3].firstChild.nodeValue;
      if (message == 1) {          
        image4.src='https://aiot-consulting.github.io/Static-Assets/feed_ok.png';
                }
                else {
        image4.src='https://aiot-consulting.github.io/Static-Assets/feed_low.png';
                }
      // Dashboard Table Data
      xmldoc = xmlResponse.getElementsByTagName("DT");
      message = xmldoc[0].firstChild.nodeValue;
      document.getElementById("dataTable2").innerHTML = message;
            
      xmldoc = xmlResponse.getElementsByTagName("DT");
      message = xmldoc[1].firstChild.nodeValue;
      document.getElementById("dataTable7").innerHTML = message;
      
      xmldoc = xmlResponse.getElementsByTagName("DT");
      message = xmldoc[2].firstChild.nodeValue;
      document.getElementById("dataTable8").innerHTML = message;
    }
  
    // general processing code for the web page to ask for an XML steam
    // this is actually why you need to keep sending data to the page to 
    // force this call with stuff like this
    // server.on("/xml", SendXML);
    // otherwise the page will not request XML from the ESP, and updates will not happen
    
    function process(){
     
     if(xmlHttp.readyState==0 || xmlHttp.readyState==4) {
        xmlHttp.open("PUT","xml",true);
        xmlHttp.onreadystatechange=response;
        xmlHttp.send(null);
     }      
        // you may have to play with this value, big pages need more porcessing time, and hence
        // a longer timeout
        setTimeout("process()",500);
     }
    
    </script>

</html>



):::::";
