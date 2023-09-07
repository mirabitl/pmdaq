async function spyneCommand(orig, command,pdict) {
    url = orig + "/" + command
    if (pdict!=null)
    {
        url=url+"?"
        for (const [key, value] of Object.entries(pdict)) {
            url=url+key+"="+value+"&"
            //console.log(`${key} ${value}`); // "a 5", "b 7", "c 9"
          }
          url=url.substring(0, url.length-1);
    }
    alert(url);
    try {
        let res = await fetch(url);
        let jmsg = await res.json();
        //console.log(jmsg);
        vcm = command.split("?")
        let jcnt = jmsg[vcm[0] + "Response"][vcm[0] + "Result"][0];
        return JSON.parse(jcnt);
    } catch (error) {
        console.log(error);
    }
}

var daqname=null;
var pnsdaq=null;

async function getConfigurations() {
    let mghost = document.getElementById("mg_host").value;
    let mgport = document.getElementById("mg_port").value;
    /*
    let ptest={
	"db":JSON.stringify({"name":"toto","version":2})
    };
    urk="http://toto:1234/TITU";
    if (ptest!=null)
    {
        urk=urk+"?"
        for (const [key, value] of Object.entries(ptest)) {
            urk=urk+key+"="+value+"&"
            //console.log(`${key} ${value}`); // "a 5", "b 7", "c 9"
          }
          urk=urk.substring(0, urk.length-1);
    }
    alert(urk);
    */
    const orig = 'http://' + mghost + ':' + mgport;
    //const url = orig+'/CONFIGURATIONS'

    let jlist = await spyneCommand(orig, "CONFIGURATIONS",null)
    console.log(jlist["content"].length)
    var list_conf = document.createElement("select");
    list_conf.name = "sel_configs";
    list_conf.id = "sel_configs"

    for (const val of jlist["content"]) {
        var conf_option = document.createElement("option");
        conf_option.value = val;
        conf_option.text = val[0] + "  |  " + val[1] + " => " + val[2];
        //console.log(conf_option.text)
        list_conf.appendChild(conf_option);
    }

    var label = document.createElement("label");
    label.innerHTML = "Choose your configuration: "
    label.htmlFor = "configs";
    var bSetConfig = document.createElement('input');
    bSetConfig.type = 'button';
    bSetConfig.id = 'setconfig';
    bSetConfig.value = 'Set';
    bSetConfig.className = 'btn';

    bSetConfig.onclick = async function () {
        // …
        theConf = list_conf.options[list_conf.selectedIndex].value
        vc = theConf.split(",")
        daqname=vc[0]+":"+vc[1];
        alert(vc[0] + " version " + vc[1] +" -> "+daqname);
        let pdict=
        {
            name:vc[0],
            version:vc[1]
        };
        let jconf = await spyneCommand(orig, "CONFIGURATION",pdict)
        console.log(jconf["content"])
        if ( "pns" in jconf["content"])
            pnsdaq=jconf["content"]["pns"];
        else
            pnsdaq=prompt("Enter the PMDAQ name server","lyocmsmu03")
        // create the daq in webdaq
        let daqhost = document.getElementById("daq_host").value;
        let daqport = document.getElementById("daq_port").value;
    
    
        const origdaq = 'http://' + daqhost + ':' + daqport;
        let pdaq={
            daqmongo:daqname,
            pnsname:pnsdaq
        }
        let jdaq = await spyneCommand(origdaq, "REGISTERDAQ",pdaq);
        console.log(jdaq);
	document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";
    };

    document.getElementById("configsel").appendChild(label).appendChild(list_conf)
    document.getElementById("configsel").append(bSetConfig);


    


}

async function CreateDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    
    let pdaq = {
        daq: daqname,
    }
    let jdaq = await spyneCommand(origdaq, "CREATE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";

}
function onConnect() {
    //topic =  document.getElementById("topic_s").value;
    pico_location = document.getElementById("pico_location").value;
    topic = pico_location + "/RUNNING"
    document.getElementById("messages").innerHTML += "<span> Subscribing to topic " + topic + "</span><br>";
    //console.log("in On connect")

    client.subscribe(topic);
    // Now lits subsystems
    brooks_head = {}
    brooksys = []
    zupsys = []
    genesyssys = []
    wienersys = []
    gas_list = {}
    topic = pico_location + "/LIST"
    publish_one_message(topic, "{}")
}



function onConnectionLost(responseObject) {
    document.getElementById("messages").innerHTML += "<span> ERROR: Connection is lost.</span><br>";
    if (responseObject != 0) {
        document.getElementById("messages").innerHTML += "<span> ERROR:" + responseObject.errorMessage + "</span><br>";
    }
}

function onMessageArrived(message) {
    console.log("OnMessageArrived: " + message.payloadString);
    document.getElementById("messages").innerHTML += "<span> Topic:" + message.destinationName + "| Message : " + message.payloadString + "</span><br>";
    jmsg = JSON.parse(message.payloadString);
    console.log(jmsg);
    //console.log(message.destinationName);
    //console.log(pico_location+"/RUNNING");
    //console.log(jmsg["devices"]);
    /// List of RUNNING devices
    if (message.destinationName == (pico_location + "/RUNNING")) {
        id_brooks = jmsg["devices"].findIndex((element) => element === "brooks");
        id_genesys = jmsg["devices"].findIndex((element) => element === "genesys");
        id_zup = jmsg["devices"].findIndex((element) => element === "zup");
        id_bme = jmsg["devices"].findIndex((element) => element === "bme");
        id_hih = jmsg["devices"].findIndex((element) => element === "hih");
        id_pico = jmsg["devices"].findIndex((element) => element === "rp2040");
        id_wiener = jmsg["devices"].findIndex((element) => element === "wiener");
        // Create brooks_head object
        if (id_brooks >= 0) {
            var s_sub = jmsg["subsystem"];
            brooksys.push(s_sub);
            brooks_head[s_sub] = {}
            // Query existing gas
            topic_gas = pico_location + "/" + s_sub + "/brooks/GAS/#"
            brooks_head[s_sub]["gas"] = {}
            brooks_head[s_sub]["gas"]["infos"] = {}
            brooks_head[s_sub]["gas"]["read"] = {}
            if (document.getElementById('brooks-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'brooks-' + s_sub;
                iDiv.className = 'brooks-' + s_sub;
                iDiv.innerHTML = "<H2> Brooks system " + s_sub + "</H2>"
                document.getElementById("Brooks").appendChild(iDiv);
                addGasTable(iDiv.className);
            }

            client.subscribe(topic_gas);
            //createListBrooks();
        }
        // Create genesys_head object
        if (id_genesys >= 0) {
            var s_sub = jmsg["subsystem"];
            genesyssys.push(s_sub);
            // Query existing gas
            topic_genesys = pico_location + "/" + s_sub + "/genesys/#"
            if (document.getElementById('genesys-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'genesys-' + s_sub;
                iDiv.className = 'genesys-' + s_sub;
                iDiv.innerHTML = "<H2> Lambda system " + s_sub + "</H2>"
                document.getElementById("Genesys").appendChild(iDiv);
                console.log("Add Genesys Table");
                addGenesysTable(iDiv.className);
            }

            client.subscribe(topic_genesys);
            //createListBrooks();
        }
        // Create genesys_head object
        if (id_zup >= 0) {
            var s_sub = jmsg["subsystem"];
            zupsys.push(s_sub);
            // Query existing gas
            topic_zup = pico_location + "/" + s_sub + "/zup/#"
            if (document.getElementById('genesys-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'zup-' + s_sub;
                iDiv.className = 'zup-' + s_sub;
                iDiv.innerHTML = "<H2> Zup system " + s_sub + "</H2>"
                document.getElementById("Zup").appendChild(iDiv);
                addGenesysTable(iDiv.className);
            }

            client.subscribe(topic_zup);
            //createListBrooks();
        }
        if (id_bme >= 0) {
            var s_sub = jmsg["subsystem"];

            // Query existing gas
            topic_bme = pico_location + "/" + s_sub + "/bme/#"
            if (document.getElementById('bme-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'bme-' + s_sub;
                iDiv.className = 'bme-' + s_sub;
                iDiv.innerHTML = "<H2> BME280 system " + s_sub + "</H2>"
                document.getElementById("Status").appendChild(iDiv);
                addBmeTable(iDiv.className);
            }

            client.subscribe(topic_bme);
            //createListBrooks();
        }
        if (id_hih >= 0) {
            var s_sub = jmsg["subsystem"];

            // Query existing gas
            topic_hih = pico_location + "/" + s_sub + "/hih/#"
            if (document.getElementById('hih-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'hih-' + s_sub;
                iDiv.className = 'hih-' + s_sub;
                iDiv.innerHTML = "<H2> Humidity HIH system " + s_sub + "</H2>"
                document.getElementById("Status").appendChild(iDiv);
                addHihTable(iDiv.className);
            }

            client.subscribe(topic_hih);
            //createListBrooks();
        }

        if (id_pico >= 0) {
            var s_sub = jmsg["subsystem"];

            // Query existing gas
            topic_pico = pico_location + "/" + s_sub + "/rp2040/#"
            if (document.getElementById('pico-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'pico-' + s_sub;
                iDiv.className = 'pico-' + s_sub;
                iDiv.innerHTML = "<H2> Pico board " + s_sub + "</H2>"
                document.getElementById("Status").appendChild(iDiv);
                addPicoTable(iDiv.className);
            }

            client.subscribe(topic_pico);
            //createListBrooks();
        }

        if (id_wiener >= 0) {
            var s_sub = jmsg["subsystem"];

            // Query existing gas
            topic_wiener = pico_location + "/" + s_sub + "/wiener/#"
            if (document.getElementById('hih-' + s_sub) == null) {
                var iDiv = document.createElement('div');
                iDiv.id = 'wiener-' + s_sub;
                iDiv.className = 'wiener-' + s_sub;
                iDiv.innerHTML = "<H2> Wiener system " + s_sub + "</H2>"
                document.getElementById("Wiener").appendChild(iDiv);
                addWienerTable(iDiv.className);
            }

            client.subscribe(topic_wiener);
            //createListBrooks();
        }



        console.log(brooksys)

    }
    /// List of gases
    v_t = message.destinationName.split("/");
    //console.log(v_t);
    /// Check gas infos
    if (v_t.length >= 5) {
        if (v_t[2] == "brooks" && v_t[3] == "GAS") {
            s_gas = v_t[4]
            s_sub = v_t[1];
            console.log("Gaz found " + s_gas + " in " + s_sub);

            gas_list[s_gas] = jmsg;

            console.log(gas_list);
            brooks_head[s_sub]["gas"]["infos"][s_gas] = jmsg;
            addGasRow('brooks-' + s_sub, jmsg);
            /*    
            createListGas()
            var label = document.createElement("label");
            label.innerHTML ="<p> "+ v_t[4]+" ID "+jmsg["device_id"]+"  Max range "+jmsg["gas_flow_range"]+" l/h"
            label.htmlFor = "gases info";
            */
            var topic_g = pico_location + "/" + s_sub + "/brooks/" + s_gas;
            client.subscribe(topic_g)

            //document.getElementById("brooks-gas-info").appendChild(label)
            return

        }
    }
    // Brooks Gas readout
    if (v_t.length == 4) {
        if (v_t[2] == "brooks") {
            var fset = document.getElementById("brooks-" + v_t[1] + "-table-flowset-" + v_t[3]);
            if (fset != null)
                fset.innerHTML = jmsg["setpoint_selected"].toFixed(5);
            var fread = document.getElementById("brooks-" + v_t[1] + "-table-flowread-" + v_t[3]);
            if (fread != null)
                fread.innerHTML = jmsg["primary_variable"].toFixed(5);
            return
        }
    }
    // Genesys
    //Detection and subscribe
    if (v_t.length == 3)
        if (v_t[2] == "genesys")
            addGenesysRow("genesys-" + v_t[1], jmsg);
    if (v_t.length == 3)
        if (v_t[2] == "zup")
            addGenesysRow("zup-" + v_t[1], jmsg);
    // BME
    if (v_t.length == 3)
        if (v_t[2] == "bme")
            addBmeRow("bme-" + v_t[1], jmsg);
    // HIH
    if (v_t.length == 3)
        if (v_t[2] == "hih")
            addHihRow("hih-" + v_t[1], jmsg);
    // PICO
    if (v_t.length == 3)
        if (v_t[2] == "rp2040")
            addPicoRow("pico-" + v_t[1], jmsg);
    // WIENER
    if (v_t.length == 3)
        if (v_t[2] == "wiener")
            addWienerRows("wiener-" + v_t[1], jmsg);

}

function startDisconnect() {
    client.disconnect();
    document.getElementById("messages").innerHTML += "<span> Disconnected. </span><br>";




}

function publishMessage() {
    msg = document.getElementById("Message").value;
    topic = document.getElementById("topic_p").value;

    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = topic;

    client.send(Message);
    document.getElementById("messages").innerHTML += "<span> Message to topic " + topic + " is sent </span><br>";
}
function publish_one_message(topic, msg) {


    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = topic;

    client.send(Message);
    document.getElementById("messages").innerHTML += "<span> Message to topic " + topic + " is sent </span><br>";
}

/*
function createListBrooks() {

    console.log("on va creer la liste")
    console.log(brooksys)

    create_widget = false;
    var select = document.getElementById("brooks-subsys-list-id");
    if (select == null) {
        select = document.createElement("select");
        select.name = "brooks-subsys-list";
        select.id = "brooks-subsys-list-id"
        create_widget = true;
    }
    removeOptions(select);

    for (const val of brooksys) {
        var option = document.createElement("option");
        option.value = val;
        option.text = val.charAt(0).toUpperCase() + val.slice(1);
        select.appendChild(option);
    }
    if (create_widget) {
        var label = document.createElement("label");
        label.innerHTML = "Choose the subsystem: "
        label.htmlFor = "subsytem";

        document.getElementById("brooks-subsystem-select").appendChild(label).appendChild(select);
    }
}
*/
function removeOptions(selectElement) {
    console.log(selectElement);
    if (selectElement != null) {
        var i, L = selectElement.options.length - 1;
        for (i = L; i >= 0; i--) {
            selectElement.remove(i);
        }
    }
}
/*
function createListGas() {

    console.log("on va creer la liste")
    console.log(gas_list)
    create_widget = false;

    var select = document.getElementById("brooks-gas-list-id");
    if (select == null) {
        select = document.createElement("select");
        select.name = "brooks-gas-list";
        select.id = "brooks-gas-list-id";
        create_widget = true;
    }
    console.log(select.options.length);
    removeOptions(select);

    for (let key in gas_list) {
        var option = document.createElement("option");
        option.value = key;
        option.text = key.charAt(0).toUpperCase() + key.slice(1);

        select.appendChild(option);
    }

    if (create_widget) {
        var label = document.createElement("label");
        label.innerHTML = "Choose the Gas: "
        label.htmlFor = "gases";


        document.getElementById("brooks-gas-select").appendChild(label).appendChild(select);
    }
}
function getGasList() {

    console.log("subscription to GAS info")
    //
    subsystem = document.getElementById("brooks-subsys-list-id").value
    console.log(pico_location)
    console.log(subsystem)
    topic_gas = pico_location + "/" + subsystem + "/brooks/GAS/#"
    gas_list = {}
    document.getElementById("brooks-gas-info").innerHTML = "";
    client.subscribe(topic_gas);
    console.log(topic_gas)

}
*/
function setFlow() {
    topic_cmd = pico_location + "/" + subsystem + "/CMD"
    jmsg = {}
    jmsg["device"] = "brooks"
    jmsg["command"] = "SETFLOW"
    jmsg["params"] = {}
    jmsg["params"]["device_id"] = gas_list[document.getElementById("brooks-gas-list-id").value]["device_id"]
    var req_flow = parseFloat(document.getElementById("gas_flow").value) * 100. / gas_list[document.getElementById("brooks-gas-list-id").value]["gas_flow_range"];
    if (req_flow > 100) req_flow = 100;

    jmsg["params"]["flow"] = req_flow;
    console.log(topic_cmd + "|" + JSON.stringify(jmsg));
    publish_one_message(topic_cmd, JSON.stringify(jmsg));
}

function addGasTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);

        var headers = ["Gas", "Id", "Max l/h", "Flow set (l/h)", "Flow read", "New value (l/h)", " ", "view"];
        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);

}
function addGasRow(div_name, g_obj) {
    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];
    var table = document.getElementById(div_name + '-table-id');
    var req_row = document.getElementById(div_name + "-table-" + g_obj["gas_type"]);
    if (req_row != null) return;
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-" + g_obj["gas_type"];
    row.insertCell(0).innerHTML = g_obj["gas_type"];
    row.insertCell(1).innerHTML = g_obj["device_id"];
    row.insertCell(2).innerHTML = g_obj["gas_flow_range"];

    var c_set = row.insertCell(3);
    c_set.id = div_name + "-table-flowset-" + g_obj["gas_type"]
    c_set.innerHTML = "None"
    var c_read = row.insertCell(4);
    c_read.id = div_name + "-table-flowread-" + g_obj["gas_type"]
    c_read.innerHTML = "None"
    var c_query = row.insertCell(5);
    var x = document.createElement("INPUT");
    x.id = div_name + "-table-flowquery-" + g_obj["gas_type"]
    x.setAttribute("type", "number");
    c_query.appendChild(x);

    var c_btn = row.insertCell(6);
    let btn = document.createElement("button");
    btn.innerHTML = "Set";
    btn.onclick = function () {
        var percent = document.getElementById(x.id).value * 100 / g_obj["gas_flow_range"];
        var s_sub = div_name.split("-")[1];
        //alert(div_name+" "+s_sub);
        alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(x.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        jmsg = {}
        jmsg["device"] = "brooks"
        jmsg["command"] = "SETFLOW"
        jmsg["params"] = {}
        jmsg["params"]["device_id"] = g_obj["device_id"]
        if (percent > 100) percent = 100;

        jmsg["params"]["flow"] = percent;
        console.log(topic_cmd + "|" + JSON.stringify(jmsg));
        publish_one_message(topic_cmd, JSON.stringify(jmsg));
    };
    c_btn.appendChild(btn);

    var c_btn_view = row.insertCell(7);
    var btn_view = document.createElement("button");
    btn_view.innerHTML = "VIEW";
    btn_view.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        var topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = "brooks"
        j_msg["command"] = "VIEW"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        //alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_view.appendChild(btn_view);



}

function addGenesysTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["V Set", "I Set ", "V read", "I read", "Status", "V Req.  ", " ", "I Req.", " ", " ", " ", " View "];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Genesis table "+div_name+" "+headers.length);
}

function addGenesysRow(div_name, g_obj) {

    var table = document.getElementById(div_name + '-table-id');
    var row_name = div_name + "-table-0";
    var req_row = document.getElementById(row_name);
    if (req_row != null) {
        var c_vset = document.getElementById(row_name + "-vset");
        c_vset.innerHTML = g_obj["vset"].toFixed(3);
        var c_iset = document.getElementById(row_name + "-iset");
        c_iset.innerHTML = g_obj["iset"].toFixed(3);
        var c_vout = document.getElementById(row_name + "-vout");
        c_vout.innerHTML = g_obj["vout"].toFixed(3);
        var c_iout = document.getElementById(row_name + "-iout");
        c_iout.innerHTML = g_obj["iout"].toFixed(3);
        var c_status = document.getElementById(row_name + "-status");
        c_status.innerHTML = g_obj["status"];
        return;
    }
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-0";

    var c_vset = row.insertCell(0);
    c_vset.id = row.id + "-vset";
    c_vset.innerHTML = g_obj["vset"].toFixed(3);
    var c_iset = row.insertCell(1);
    c_iset.id = row.id + "-iset";
    c_iset.innerHTML = g_obj["iset"].toFixed(3);
    var c_vout = row.insertCell(2);
    c_vout.id = row.id + "-vout";
    c_vout.innerHTML = g_obj["vout"].toFixed(3);
    var c_iout = row.insertCell(3);
    c_iout.id = row.id + "-iout";
    c_iout.innerHTML = g_obj["iout"].toFixed(3);
    var c_status = row.insertCell(4);
    c_status.id = row.id + "-status";
    c_status.innerHTML = g_obj["status"];
    var c_nvset = row.insertCell(5);
    var x_vset = document.createElement("INPUT");
    x_vset.id = row.id + "-nvset";
    x_vset.setAttribute("type", "number");
    c_nvset.appendChild(x_vset);

    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];

    var c_btn_vset = row.insertCell(6);
    let btn_vset = document.createElement("button");
    btn_vset.innerHTML = "Set Voltage";
    btn_vset.onclick = function () {
        var v_vset = document.getElementById(x_vset.id).value;

        topic_cmd = pico_location + "/" + s_sub + "/CMD";
        var j_msg = {};
        j_msg["device"] = s_mod;
        j_msg["command"] = "SETVOLTAGE";
        j_msg["params"] = {};
        j_msg["params"]["vset"] = parseFloat(v_vset);
        alert(topic_cmd + "|" + JSON.stringify(j_msg));
        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_vset.appendChild(btn_vset);

    var c_niset = row.insertCell(7);
    var x_iset = document.createElement("INPUT");
    x_iset.id = row.id + "-niset";
    x_iset.setAttribute("type", "number");
    c_niset.appendChild(x_iset);

    var c_btn_iset = row.insertCell(8);
    let btn_iset = document.createElement("button");
    btn_iset.innerHTML = "Set Max Current";
    btn_iset.onclick = function () {
        var v_iset = document.getElementById(x_iset.id).value;

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "SETCURRENT"
        j_msg["params"] = {}
        j_msg["params"]["iset"] = parseFloat(v_iset)
        alert(topic_cmd + "|" + JSON.stringify(j_msg));
        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_iset.appendChild(btn_iset);

    var c_btn_on = row.insertCell(9);
    let btn_on = document.createElement("button");
    btn_on.innerHTML = "ON";
    btn_on.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "SETON"
        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_on.appendChild(btn_on);

    var c_btn_off = row.insertCell(10);
    let btn_off = document.createElement("button");
    btn_off.innerHTML = "OFF";
    btn_off.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "SETOFF"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_off.appendChild(btn_off);

    var c_btn_view = row.insertCell(11);
    let btn_view = document.createElement("button");
    btn_view.innerHTML = "VIEW";
    btn_view.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "VIEW"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        //alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_view.appendChild(btn_view);



}
function addBmeTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["P (mbar) ", "T (C)", "H (%)", " View "];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Genesis table "+div_name+" "+headers.length);
}

function addBmeRow(div_name, g_obj) {
    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];

    var table = document.getElementById(div_name + '-table-id');
    var row_name = div_name + "-table-0";
    var req_row = document.getElementById(row_name);
    if (req_row != null) {
        var c_P = document.getElementById(row_name + "-P");
        c_P.innerHTML = g_obj["P"].toFixed(3);
        var c_T = document.getElementById(row_name + "-T");
        c_T.innerHTML = g_obj["T"].toFixed(3);
        var c_H = document.getElementById(row_name + "-H");
        c_H.innerHTML = g_obj["H"].toFixed(3);
        return;
    }
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-0";

    var c_P = row.insertCell(0);
    c_P.id = row.id + "-P";
    c_P.innerHTML = g_obj["P"].toFixed(3);
    var c_T = row.insertCell(1);
    c_T.id = row.id + "-T";
    c_T.innerHTML = g_obj["T"].toFixed(3);
    var c_H = row.insertCell(2);
    c_H.id = row.id + "-H";
    c_H.innerHTML = g_obj["H"].toFixed(3);

    var c_btn_view = row.insertCell(3);
    let btn_view = document.createElement("button");
    btn_view.innerHTML = "VIEW";
    btn_view.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "VIEW"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        //alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_view.appendChild(btn_view);



}

function addHihTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["T (C)", "H (%)", " View "];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Genesis table "+div_name+" "+headers.length);
}

function addHihRow(div_name, g_obj) {
    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];

    var table = document.getElementById(div_name + '-table-id');
    var row_name = div_name + "-table-0";
    var req_row = document.getElementById(row_name);
    if (req_row != null) {
        var c_T = document.getElementById(row_name + "-T");
        c_T.innerHTML = g_obj["T"].toFixed(3);
        var c_H = document.getElementById(row_name + "-H");
        c_H.innerHTML = g_obj["H"].toFixed(3);
        return;
    }
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-0";

    var c_T = row.insertCell(0);
    c_T.id = row.id + "-T";
    c_T.innerHTML = g_obj["T"].toFixed(3);
    var c_H = row.insertCell(1);
    c_H.id = row.id + "-H";
    c_H.innerHTML = g_obj["H"].toFixed(3);

    var c_btn_view = row.insertCell(2);
    let btn_view = document.createElement("button");
    btn_view.innerHTML = "VIEW";
    btn_view.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "VIEW"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        //alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_view.appendChild(btn_view);



}
function addHihTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["T (C)", "H (%)", " View "];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Genesis table "+div_name+" "+headers.length);
}

function addHihRow(div_name, g_obj) {
    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];

    var table = document.getElementById(div_name + '-table-id');
    var row_name = div_name + "-table-0";
    var req_row = document.getElementById(row_name);
    if (req_row != null) {
        var c_T = document.getElementById(row_name + "-T");
        c_T.innerHTML = g_obj["T"].toFixed(3);
        var c_H = document.getElementById(row_name + "-H");
        c_H.innerHTML = g_obj["H"].toFixed(3);
        return;
    }
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-0";

    var c_T = row.insertCell(0);
    c_T.id = row.id + "-T";
    c_T.innerHTML = g_obj["T"].toFixed(3);
    var c_H = row.insertCell(1);
    c_H.id = row.id + "-H";
    c_H.innerHTML = g_obj["H"].toFixed(3);

    var c_btn_view = row.insertCell(2);
    let btn_view = document.createElement("button");
    btn_view.innerHTML = "VIEW";
    btn_view.onclick = function () {

        //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
        topic_cmd = pico_location + "/" + s_sub + "/CMD"
        var j_msg = {}
        j_msg["device"] = s_mod
        j_msg["command"] = "VIEW"

        console.log(topic_cmd + "|" + JSON.stringify(j_msg));
        //alert(topic_cmd + "|" + JSON.stringify(j_msg));
        publish_one_message(topic_cmd, JSON.stringify(j_msg));
    };
    c_btn_view.appendChild(btn_view);



}

function addPicoTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["T (C)"];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Genesis table "+div_name+" "+headers.length);
}

function addPicoRow(div_name, g_obj) {
    var s_mod = div_name.split("-")[0];
    var s_sub = div_name.split("-")[1];

    var table = document.getElementById(div_name + '-table-id');
    var row_name = div_name + "-table-0";
    var req_row = document.getElementById(row_name);
    if (req_row != null) {
        var c_T = document.getElementById(row_name + "-T");
        c_T.innerHTML = g_obj["T"].toFixed(3);
        return;
    }
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    row.id = div_name + "-table-0";

    var c_T = row.insertCell(0);
    c_T.id = row.id + "-T";
    c_T.innerHTML = g_obj["T"].toFixed(3);




}
function addWienerTable(div_name) {

    var myTableDiv = document.getElementById(div_name);
    var req_tab = document.getElementById(div_name + "-table-id");
    console.log("req_tab " + req_tab)
    if (req_tab != null) return;
    var table = document.createElement('TABLE');
    table.id = div_name + '-table-id';
    table.className = div_name + '-table';
    table.border = '1';
    var headers = ["Channel", "V Set (V)", "I Set (uA)", "Ramp Up", "V read (V)", "I read (uA)", "Status", "V Req. (V)  ", " ", "I Req. (uA)", " ", "Ramp req.  ", " ", " ", " ", " View "];

    var tableBody = document.createElement('TBODY');
    table.appendChild(tableBody);
    for (var i = 0; i < 1; i++) {
        var tr = document.createElement('TR');
        tableBody.appendChild(tr);


        for (var j = 0; j < headers.length; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);
    //alert("Wiener table "+div_name+" "+headers.length);
}

function addWienerRows(div_name, g_obj) {

    var table = document.getElementById(div_name + '-table-id');
    for (rch of g_obj["channels"]) {
        let ch = rch["id"];
        let row_name = div_name + "-table-" + ch;
        console.log(ch + " gives " + row_name);
        let req_row = document.getElementById(row_name);
        if (req_row != null) {
            let c_vset = document.getElementById(row_name + "-vset");
            c_vset.innerHTML = rch["vset"].toFixed(1);
            let c_iset = document.getElementById(row_name + "-iset");
            c_iset.innerHTML = (rch["iset"] * 1.E6).toFixed(3);
            let c_vout = document.getElementById(row_name + "-vout");
            c_vout.innerHTML = rch["vout"].toFixed(1);
            let c_iout = document.getElementById(row_name + "-iout");
            c_iout.innerHTML = (rch["iout"] * 1.E6).toFixed(3);
            let c_status = document.getElementById(row_name + "-status");
            c_status.innerHTML = rch["status"].split("=")[1];

            let c_ramp = document.getElementById(row_name + "-ramp");
            c_ramp.innerHTML = rch["rampup"].toFixed(1);

            console.log("Process " + ch + " " + (rch["iout"] * 1.E6))
            continue;
        }
        var rowCount = table.rows.length;
        var row = table.insertRow(rowCount);
        row.id = div_name + "-table-" + ch;

        var c_channel = row.insertCell(0);
        c_channel.id = row.id + "-channel";
        c_channel.innerHTML = rch["id"];

        var c_vset = row.insertCell(1);
        c_vset.id = row.id + "-vset";
        c_vset.innerHTML = rch["vset"].toFixed(3);
        var c_iset = row.insertCell(2);
        c_iset.id = row.id + "-iset";
        c_iset.innerHTML = (rch["iset"] * 1E6).toFixed(3);
        var c_ramp = row.insertCell(3);
        c_ramp.id = row.id + "-ramp";
        c_ramp.innerHTML = rch["rampup"].toFixed(3);
        var c_vout = row.insertCell(4);
        c_vout.id = row.id + "-vout";
        c_vout.innerHTML = rch["vout"].toFixed(3);
        var c_iout = row.insertCell(5);
        c_iout.id = row.id + "-iout";
        c_iout.innerHTML = (rch["iout"] * 1.E6).toFixed(3);
        var c_status = row.insertCell(6);
        c_status.id = row.id + "-status";
        c_status.innerHTML = rch["status"].split("=")[1];
        var c_nvset = row.insertCell(7);
        let x_vset = document.createElement("INPUT");
        x_vset.id = row.id + "-nvset";
        x_vset.setAttribute("type", "number");
        c_nvset.appendChild(x_vset);

        var s_mod = div_name.split("-")[0];
        var s_sub = div_name.split("-")[1];

        var c_btn_vset = row.insertCell(8);
        let btn_vset = document.createElement("button");
        btn_vset.innerHTML = "Set Voltage";
        btn_vset.onclick = function () {
            var v_vset = document.getElementById(x_vset.id).value;

            topic_cmd = pico_location + "/" + s_sub + "/CMD";
            var j_msg = {};
            j_msg["device"] = s_mod;
            j_msg["command"] = "VSET";
            j_msg["params"] = {};
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;
            j_msg["params"]["vset"] = parseFloat(v_vset);
            alert(topic_cmd + "|" + JSON.stringify(j_msg));
            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_vset.appendChild(btn_vset);

        var c_niset = row.insertCell(9);
        let x_iset = document.createElement("INPUT");
        x_iset.id = row.id + "-niset";
        x_iset.setAttribute("type", "number");
        c_niset.appendChild(x_iset);

        var c_btn_iset = row.insertCell(10);
        let btn_iset = document.createElement("button");
        btn_iset.innerHTML = "Set Max Current";
        btn_iset.onclick = function () {
            var v_iset = document.getElementById(x_iset.id).value;

            //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
            topic_cmd = pico_location + "/" + s_sub + "/CMD"
            var j_msg = {}
            j_msg["device"] = s_mod
            j_msg["command"] = "ISET"
            j_msg["params"] = {}
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;
            j_msg["params"]["iset"] = parseFloat(v_iset) * 1E-6
            alert(topic_cmd + "|" + JSON.stringify(j_msg));
            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_iset.appendChild(btn_iset);

        var c_nramp = row.insertCell(11);
        let x_ramp = document.createElement("INPUT");
        x_ramp.id = row.id + "-nramp";
        x_ramp.setAttribute("type", "number");
        c_nramp.appendChild(x_ramp);

        var c_btn_ramp = row.insertCell(12);
        let btn_ramp = document.createElement("button");
        btn_ramp.innerHTML = "Set Ramp";
        btn_ramp.onclick = function () {
            var v_ramp = document.getElementById(x_ramp.id).value;

            //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
            topic_cmd = pico_location + "/" + s_sub + "/CMD"
            var j_msg = {}
            j_msg["device"] = s_mod
            j_msg["command"] = "RAMPUP"
            j_msg["params"] = {}
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;
            j_msg["params"]["rampup"] = parseFloat(v_ramp)
            alert(topic_cmd + "|" + JSON.stringify(j_msg));
            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_ramp.appendChild(btn_ramp);

        var c_btn_on = row.insertCell(13);
        let btn_on = document.createElement("button");
        btn_on.innerHTML = "ON";
        btn_on.onclick = function () {

            //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
            topic_cmd = pico_location + "/" + s_sub + "/CMD"
            var j_msg = {}
            j_msg["device"] = s_mod
            j_msg["command"] = "ON";
            j_msg["params"] = {}
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;

            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            alert(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_on.appendChild(btn_on);

        var c_btn_off = row.insertCell(14);
        let btn_off = document.createElement("button");
        btn_off.innerHTML = "OFF";
        btn_off.onclick = function () {

            //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
            topic_cmd = pico_location + "/" + s_sub + "/CMD"
            var j_msg = {}
            j_msg["device"] = s_mod
            j_msg["command"] = "OFF"
            j_msg["params"] = {}
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;

            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            alert(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_off.appendChild(btn_off);

        var c_btn_view = row.insertCell(15);
        let btn_view = document.createElement("button");
        btn_view.innerHTML = "VIEW";
        btn_view.onclick = function () {

            //alert("Gas " + g_obj["gas_type"] + "\n setting " + document.getElementById(y.id).value + "l/h range " + g_obj["gas_flow_range"] + "/  " + percent + "% \n for ID " + g_obj["device_id"]);
            topic_cmd = pico_location + "/" + s_sub + "/CMD"
            var j_msg = {}
            j_msg["device"] = s_mod
            j_msg["command"] = "STATUS"
            j_msg["params"] = {}
            j_msg["params"]["first"] = ch;
            j_msg["params"]["last"] = ch;

            console.log(topic_cmd + "|" + JSON.stringify(j_msg));
            //alert(topic_cmd + "|" + JSON.stringify(j_msg));
            publish_one_message(topic_cmd, JSON.stringify(j_msg));
        };
        c_btn_view.appendChild(btn_view);

    }

}
