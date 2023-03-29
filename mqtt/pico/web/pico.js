function startConnect() {

    clientID = "clientID-" + parseInt(Math.random() * 100);

    host = document.getElementById("host").value;
    port = document.getElementById("port").value;
    pico_location = document.getElementById("pico_location").value;
    //subsystem = document.getElementById("subsystem").value;

    document.getElementById("messages").innerHTML += "<span> Connecting to " + host + "on port " + port + "</span><br>";
    document.getElementById("messages").innerHTML += "<span> Using the client Id " + clientID + " </span><br>";

    client = new Paho.MQTT.Client(host, Number(port), clientID);

    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    client.connect({ onSuccess: onConnect });


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
    lambdasys = []
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
        // Create brooks_head object
        if (id_brooks >= 0) {
            s_sub = jmsg["subsystem"];
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
        console.log(brooksys)
    }
    /// List of gases
    v_t = message.destinationName.split("/");
    console.log(v_t);
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
    if (v_t.length == 4) {
        if (v_t[2] == "brooks") {
            var fset = document.getElementById("brooks-" + v_t[1] + "-table-flowset-" + v_t[3]);
            if (fset != null)
                fset.innerHTML = jmsg["setpoint_selected"].toFixed(5);
            var fread = document.getElementById("brooks-" + v_t[1] + "-table-flowread-" + v_t[3]);
            if (fread != null)
                fread.innerHTML = jmsg["primary_variable"].toFixed(5);
            /*
        create_widget = false;

        var label = document.getElementById("brooks-gas-status-" + v_t[3]);
        if (label == null) {
            label = document.createElement("lable");
            label.name = "brooks-gas-status-" + v_t[3];
            label.id = "brooks-gas-status-" + v_t[3];
            create_widget = true;
        }

        label.innerHTML = "<p> " + v_t[3] + " Set " + jmsg["setpoint_selected"] + "  read  " + jmsg["primary_variable"] + " l/h";
        label.htmlFor = "gases status";
        if (create_widget)
            document.getElementById("brooks-gas-status").appendChild(label);
            */
            return
        }
    }
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

        var headers = ["Gas", "Id", "Max l/h", "Flow set (l/h)", "Flow read", "New value (l/h)", " "];
        for (var j = 0; j < 7; j++) {
            var td = document.createElement('TD');
            td.width = '75';
            td.appendChild(document.createTextNode(headers[j]));
            tr.appendChild(td);
        }

    }

    myTableDiv.appendChild(table);

}
function addGasRow(div_name, g_obj) {

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



}