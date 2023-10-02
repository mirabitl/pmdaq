function typeComment(e, divname) {  // Execute this function on enter key
    var _comment = document.createElement('pre'); //create a pre tag 
    _comment.textContent = e; //put the value inside pre tag
    //console.log(_comment)
    var _commentDiv = document.getElementById(divname);
    if (_commentDiv.firstChild === null) { // Will validate if any comment is there
        // If no comment is present just append the child
        document.getElementById(divname).appendChild(_comment)
    }
    else {
        // If a comment is present insert the new 
        // comment before the present first child in comment section
        document.getElementById(divname).insertBefore(_comment, _commentDiv.firstChild);
    }

}
function setComment(e, divname) {
    document.getElementById(divname).innerHTML = "<pre>" + e + "</pre>";

}
function showlog(jcnt) {
    typeComment(jcnt, "logmessages");
    //document.getElementById("logmessages").innerHTML += "<span>============================================></span><br>";
    //document.getElementById("logmessages").innerHTML += "<span>"+jcnt+ "</span><br>";
}
var verboselog = true;
var last_status = 0;
async function spyneCommand(orig, command, pdict) {
    url = orig + "/" + command
    if (pdict != null) {
        url = url + "?"
        for (const [key, value] of Object.entries(pdict)) {
            url = url + key + "=" + value + "&"
            //console.log(`${key} ${value}`); // "a 5", "b 7", "c 9"
        }
        url = url.substring(0, url.length - 1);
    }
    //alert(url);
    if (verboselog)
        showlog("Calling " + url);

    try {
        let res = await fetch(url);
        let jmsg = await res.json();

        vcm = command.split("?")
        let jcnt = jmsg[vcm[0] + "Response"][vcm[0] + "Result"][0];
        if (verboselog)
            showlog(jcnt);
        return JSON.parse(jcnt);
    } catch (error) {
        console.log(error);
    }
}
var daqname = null;
var registered = null;
var pnsdaq = null;
var daqloc = null;
var daqinfo = {
    name: null,
    state: null,
    version: 0,
    pns: null,
    config: null,
    location: null,
    url: null,
    registered: false
};

async function runningDaqs()
{
    let mghost = document.getElementById("mg_host").value;
    let mgport = document.getElementById("mg_port").value;
    const orig = 'http://' + mghost + ':' + mgport;
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;
    const origdaq = 'http://' + daqhost + ':' + daqport;
    daqinfo.url = origdaq;

    let jdaq = await spyneCommand(daqinfo.url, "DAQLIST", {});
    //console.log(jdaq);
    //console.log(jdaq["content"]);
    typeComment("Running DAQs " + jdaq["content"], "messages");
    var list_daq = document.createElement("select");
    list_daq.name = "sel_configs";
    list_daq.id = "sel_configs"

    for (const val of jdaq["content"]) {
        let conf_option = document.createElement("option");
        conf_option.value = val["name"];
        conf_option.text = val["name"] ;
        //console.log(conf_option.text)
        list_daq.appendChild(conf_option);
    }

    let label = document.createElement("label");
    label.innerHTML = "Choose your daq: "
    label.htmlFor = "daqs";
    let bSetDaq = document.createElement('input');
    bSetDaq.type = 'button';
    bSetDaq.id = 'setdaq';
    bSetDaq.value = 'Set';
    bSetDaq.className = 'btn';

    bSetDaq.onclick = async function () {
        // …
        theDaq = list_daq.options[list_daq.selectedIndex].value
        vc = theDaq.split(":")
        daqname = vc[0] + ":" + vc[1];
        alert(vc[0] + " version " + vc[1] + " -> " + daqname);
        let pdict =
        {
            name: vc[0],
            version: vc[1]
        };
        daqinfo.config = await spyneCommand(orig, "CONFIGURATION", pdict)
        console.log(daqinfo.config["content"]);
        if ("pns" in daqinfo.config["content"])
            pnsdaq = daqinfo.config["content"]["pns"];
        else
            pnsdaq = prompt("Enter the PMDAQ name server", "lyocmsmu03");
        daqloc = prompt("Enter the setup name", "???");
        daqinfo.name = daqname;
        daqinfo.state = vc[0];
        daqinfo.version = parseInt(vc[1]);
        daqinfo.pns = pnsdaq;
        daqinfo.location = daqloc;
        // create the daq in webdaq
        let daqhost = document.getElementById("daq_host").value;
        let daqport = document.getElementById("daq_port").value;


        const origdaq = 'http://' + daqhost + ':' + daqport;
        daqinfo.url = origdaq;

        let pdaq = {
            daqmongo: daqinfo.name,
            pnsname: daqinfo.pns,
            location: daqloc
        }
        let jdaq = await spyneCommand(daqinfo.url, "REGISTERDAQ", pdaq);
        console.log(jdaq);
        typeComment(" Connecting to " + daqhost + "on port " + daqport, "messages");
        //document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";
        for (app of daqinfo.config["content"]["apps"]) {
            typeComment("Application " + app["name"] + " instance " + app["instance"] + " found", "messages");
            //document.getElementById("messages").innerHTML += "<span> Application " + app["name"] + " instance " + app["instance"] + " found </span><br>"; 
            mqtt_topic = daqinfo.state + "/" + app["name"] + "/" + app["instance"] + "/status";
            typeComment("One can subscribe " + mqtt_topic, "messages");
            //document.getElementById("messages").innerHTML += "<span> One can subscribe "+mqtt_topic+ "  </span><br>"; 

        }
        subscribeMqtt();
        registered = daqname;
        daqinfo.registered = true;
    };
    document.getElementById("daqsel").innerHTML="";
    document.getElementById("daqsel").appendChild(label).appendChild(list_daq)
    document.getElementById("daqsel").append(bSetDaq);

}

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

    let jlist = await spyneCommand(orig, "CONFIGURATIONS", null)
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
        daqname = vc[0] + ":" + vc[1];
        alert(vc[0] + " version " + vc[1] + " -> " + daqname);
        let pdict =
        {
            name: vc[0],
            version: vc[1]
        };
        daqinfo.config = await spyneCommand(orig, "CONFIGURATION", pdict)
        console.log(daqinfo.config["content"]);
        if ("pns" in daqinfo.config["content"])
            pnsdaq = daqinfo.config["content"]["pns"];
        else
            pnsdaq = prompt("Enter the PMDAQ name server", "lyocmsmu03");
        daqloc = prompt("Enter the setup name", "???");
        daqinfo.name = daqname;
        daqinfo.state = vc[0];
        daqinfo.version = parseInt(vc[1]);
        daqinfo.pns = pnsdaq;
        daqinfo.location = daqloc;
        // create the daq in webdaq
        let daqhost = document.getElementById("daq_host").value;
        let daqport = document.getElementById("daq_port").value;


        const origdaq = 'http://' + daqhost + ':' + daqport;
        daqinfo.url = origdaq;

        let pdaq = {
            daqmongo: daqinfo.name,
            pnsname: daqinfo.pns,
            location: daqloc
        }
        let jdaq = await spyneCommand(daqinfo.url, "REGISTERDAQ", pdaq);
        console.log(jdaq);
        typeComment(" Connecting to " + daqhost + "on port " + daqport, "messages");
        //document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";
        for (app of daqinfo.config["content"]["apps"]) {
            typeComment("Application " + app["name"] + " instance " + app["instance"] + " found", "messages");
            //document.getElementById("messages").innerHTML += "<span> Application " + app["name"] + " instance " + app["instance"] + " found </span><br>"; 
            mqtt_topic = daqinfo.state + "/" + app["name"] + "/" + app["instance"] + "/status";
            typeComment("One can subscribe " + mqtt_topic, "messages");
            //document.getElementById("messages").innerHTML += "<span> One can subscribe "+mqtt_topic+ "  </span><br>"; 

        }
        subscribeMqtt();
        registered = daqname;
        daqinfo.registered = true;
    };

    document.getElementById("configsel").appendChild(label).appendChild(list_conf)
    document.getElementById("configsel").append(bSetConfig);





}
var last_refresh = 0;
async function refreshStatus(deadline) {
    if (registered == null) {
        requestIdleCallback(refreshStatus);
        return;
    }
    let t_now = Date.now();
    if (t_now - last_refresh < 10000) {

        requestIdleCallback(refreshStatus);
        return;
    }
    if (deadline.timeRemaining() <= 5) {
        // This will take more than 5ms so wait until we
        // get called back with a long enough deadline.
        requestIdleCallback(refreshStatus);
        return;
    }
    verboselog = false;
    //console.log("Refreshing ... ");
    let state = await getState();
    if (state == "RUNNING" && (t_now - last_status) > 10000)
    {
        await spyneCommand(daqinfo.url, "BUILDERSTATUS", { daq: daqinfo.name });
        last_status =t_now; 
    }
    last_refresh = Date.now();
    verboselog = true;
    requestIdleCallback(refreshStatus);
}
async function getState() {
    // create the daq in webdaq
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        return "NONE";
    }
    let jstate = await spyneCommand(daqinfo.url, "STATE", { daq: daqinfo.name });
    //console.log(jstate)
    if (jstate.hasOwnProperty("state")) {
        document.getElementById("daqstate").innerHTML = jstate["state"];
        return jstate["state"];
    }
    else {
        document.getElementById("daqstate").innerHTML = "NONE";
        return "NONE";
    }
}
async function CreateDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "CREATE", { daq: daqinfo.name });
    console.log(jdaq);

    typeComment("CREATE on " + daqname + "/" + daqloc, "messages");
    await getState();
}
async function InitDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }
    // create the daq in webdaq
    let rdelay = document.getElementById("delay_reset").value;
    let jdaq = await spyneCommand(daqinfo.url, "INITIALISE", { daq: daqinfo.name, delay: rdelay });
    console.log(jdaq);
    typeComment("INITIALISE on " + daqname + "/" + daqloc, "messages");
    await getState();

}
async function ConfigureDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "CONFIGURE", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("CONFIGURE on " + daqname + "/" + daqloc, "messages");
    await getState();


}
async function StartDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }
    let rcom = prompt("Enter a comment for this run", "Chez les papous y'a des papous a poux...");
    let jdaq = await spyneCommand(daqinfo.url, "START", { daq: daqinfo.name, comment: rcom });
    console.log(jdaq);
    typeComment("START on " + daqname + "/" + daqloc, "messages");
    await getState();


}

async function StopDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "STOP", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("STOP on " + daqname + "/" + daqloc, "messages");
    await getState();
}
async function DestroyDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "DESTROY", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("DESTROY on " + daqname + "/" + daqloc, "messages");
    await getState();


}
async function RemoveDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "JC_DESTROY", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("REMOVE on " + daqname + "/" + daqloc, "messages");
    registered = null;
    daqinfo.registered = false;
    await getState();
}

async function ResumeDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "RESUME", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("RESUME on " + daqname + "/" + daqloc, "messages");
    await getState();

}


async function PauseDaq() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "PAUSE", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("PAUSE on " + daqname + "/" + daqloc, "messages");
    await getState();


}

async function builderStatus() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "BUILDERSTATUS", { daq: daqinfo.name });
    console.log(jdaq["status"]);
    typeComment("BUILDERSTATUS on " + daqname + "/" + daqloc, "messages");
    await getState();


}
async function sourceStatus() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "SOURCESTATUS", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("SOURCESTATUS on " + daqname + "/" + daqloc, "messages");
    await getState();


}
async function triggerStatus() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let jdaq = await spyneCommand(daqinfo.url, "TRIGGERSTATUS", { daq: daqinfo.name });
    console.log(jdaq);
    typeComment("TRIGGERSTATUS on " + daqname + "/" + daqloc, "messages");
    await getState();


}


async function processCommand(appname, methodname, parameters) {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let pdaq = {
        daq: daqinfo.name,
        method: methodname,
        app: appname,
        params: JSON.stringify(parameters)
    }
    jdaq = await spyneCommand(daqinfo.url, "PROCESS", pdaq);
    console.log(jdaq);
    return jdaq;
}
async function SetTriggerMdcc() {

    let board = document.getElementById("trg_board").value;
    let jrep = await processCommand(board, "SPILLOFF", { nclock: document.getElementById("trg_spilloff").value });
    jrep = await processCommand(board, "SPILLON", { nclock: document.getElementById("trg_spillon").value });
    jrep = await processCommand(board, "CALIBON", { value: 0});
    jrep = await processCommand(board, "CALIBOFF", {});
    jrep = await processCommand(board, "SETCALIBCOUNT", { nclock: 0 });

    jrep = await processCommand(board, "SETEXTERNAL", { value: document.getElementById("trg_external").value });
    jrep = await processCommand(board, "SETSPILLREGISTER", { value: document.getElementById("trg_spillreg").value });


    if (board == "lyon_mbmdcc") {
        jrep = await processCommand(board, "ENABLE", { value: document.getElementById("trg_channels").value });
        jrep = await processCommand(board, "CHANNELON", { value: document.getElementById("trg_channels").value });
    }


    typeComment("SetTriggerMdcc on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}
async function SetFebv1Values() {

    let board = document.getElementById("febv1_board").value;
    // create the daq in webdaq
    const origdaq = 'http://' + document.getElementById("daq_host").value + ':' + document.getElementById("daq_port").value;
    let jdaq = await spyneCommand(origdaq, "SETTDCMODE", { daq: daqname, mode: document.getElementById("febv1_mode").value });
    console.log(jdaq);
    jdaq = await spyneCommand(origdaq, "SETTDCDELAYS", { daq: daqname, active: document.getElementById("febv1_duration").value, dead: document.getElementById("febv1_delay").value });
    console.log(jdaq);


    typeComment("SetFebv1Values on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}
async function SetDBValues() {

    let board = document.getElementById("db_board").value;
    // create the daq in webdaq
    const origdaq = 'http://' + document.getElementById("daq_host").value + ':' + document.getElementById("daq_port").value;
    let jdaq = await spyneCommand(origdaq, "DOWNLOADDB", { daq: daqname, app: board, state: document.getElementById("dbstate").value, version: document.getElementById("dbversion").value });
    console.log(jdaq);


    typeComment("SetDBValues on " + daqname + "/" + daqloc, "messages");
    console.log(await getState());

}
async function SetVthTime() {
    let jrep = await processCommand("lyon_febv1", "SETVTHTIME", { value: document.getElementById("febv1_vthtime").value });
    typeComment("SetVthTime on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}
async function LutCalib() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let read_state = await getState();
    //console.log(read_state)
    if (read_state != "INITIALISED") {
        alert("DAQ must be initialised to make FEBV1 Lut calibration");
        return;
    }
    pdaq = {
        daq: daqinfo.name,
        tdc: document.getElementById("tdc_number").value,
        nchannels: document.getElementById("tdc_channels").value
    }
    let jdaq = await spyneCommand(daqinfo.url, "LUTCALIB", pdaq);
    console.log(jdaq);
    typeComment("LUTCALIB on " + daqname + "/" + daqloc, "messages");


}
async function LutMask() {
    if (!daqinfo.registered) {
        document.getElementById("daqstate").innerHTML = "NONE";
        alert("No DAQ registered");
        return "NONE";
    }

    let pdaq = {
        daq: daqinfo.name,
        tdc: document.getElementById("tdc_number").value,
        mask: document.getElementById("tdc_mask").value,
        feb: document.getElementById("tdc_feb").value
    }
    let jdaq = await spyneCommand(daqinfo.url, "LUTMASK", pdaq);
    console.log(jdaq);
    typeComment("LUTMASK on " + daqname + "/" + daqloc, "messages");


}
async function doCal_dac10febv1() {
    state = await getState();
    if (state == "RUNNING") {
        let jrep = await processCommand(document.getElementById("calf1_board").value, "SCURVE", {
            first: document.getElementById("calf1_1st").value,
            last: document.getElementById("calf1_last").value,
            step: document.getElementById("calf1_step").value,
            channel: document.getElementById("calf1_channel").value,
            spillon: document.getElementById("calf1_spillon").value,
            spilloff: document.getElementById("calf1_spilloff").value,
            mask: document.getElementById("calf1_mask").value
        });
        console.log(jrep);
    }
    else {
        alert("The DAQ must be running " + state);
    }
    typeComment("doCal_dac10febv1 on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}

async function doCal_b0pmr() {
    state = await getState();
    if (state == "RUNNING") {
        let jrep = await processCommand(document.getElementById("calpmr_board").value, "SCURVE", {
            first: document.getElementById("calpmr_1st").value,
            last: document.getElementById("calpmr_last").value,
            step: document.getElementById("calpmr_step").value,
            channel: document.getElementById("calpmr_channel").value,
            window: document.getElementById("calpmr_window").value,
            ntrg: document.getElementById("calpmr_ntrg").value,
            level: document.getElementById("calpmr_level").value
        });
        console.log(jrep);
    }
    else {
        alert("The DAQ must be running");
    }
    typeComment("doCal_b0pmr on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}


async function doCal_b0gric() {
    state = await getState();
    if (state == "RUNNING") {
        let jrep = await processCommand(document.getElementById("calgric_board").value, "SCURVE", {
            first: document.getElementById("calgric_1st").value,
            last: document.getElementById("calgric_last").value,
            step: document.getElementById("calgric_step").value,
            channel: document.getElementById("calgric_channel").value,
            window: document.getElementById("calgric_window").value,
            ntrg: document.getElementById("calgric_ntrg").value,
            level: document.getElementById("calgric_level").value
        });
        console.log(jrep);
    }
    else {
        alert("The DAQ must be running");
    }
    typeComment("doCal_b0gric on " + daqname + "/" + daqloc, "messages");
    //console.log(await getState());

}


function removeOptions(selectElement) {
    console.log(selectElement);
    if (selectElement != null) {
        var i, L = selectElement.options.length - 1;
        for (i = L; i >= 0; i--) {
            selectElement.remove(i);
        }
    }
}
requestId = window.requestIdleCallback(refreshStatus);

function subscribeMqtt() {


    clientID = "clientID-" + parseInt(Math.random() * 100);

    client = new Paho.MQTT.Client("lyoilc07.in2p3.fr", 8080, clientID);

    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    client.connect({ onSuccess: onConnect });


}


function onConnect() {
    for (app of daqinfo.config["content"]["apps"]) {
        mqtt_topic = daqinfo.state + "/" + app["name"] + "/" + app["instance"] + "/status";

        typeComment("Subscribing to topic " + mqtt_topic, "messages");
        client.subscribe(mqtt_topic);

    }
    //console.log("in On connect")


}



function onConnectionLost(responseObject) {
    typeComment("ERROR: Connection is lost", "messages");
    if (responseObject != 0) {
        typeComment("ERROR:" + responseObject.errorMessage, "messages");
    }
}

function onMessageArrived(message) {
    //console.log("OnMessageArrived: " + message.payloadString);
    vc = message.destinationName.split("/");
    if (vc[1] == "lyon_febv1" && vc[3] == "status") {
        jfebs = JSON.parse(message.payloadString);
        let run_mode = jfebs[0]["mode"]
        if (run_mode != 0) {
            if (run_mode & 1) {
                let chan = (run_mode >> 4) & 0xFF;
                setComment("LUT calibration Channel " + chan + " on " + vc[1], "calmessages");
            }
            if (run_mode & 2) {
                let chan = (run_mode >> 4) & 0xFF;
                let step = (run_mode >> 12) & 0xFFF;
                setComment("SCURVE Channel " + chan + " step " + step + " on " + vc[1], "calmessages");

            }
        }
    }
    else
        if (((vc[1] == "lyon_pmr") || (vc[1] == "lyon_gricv0") || (vc[1] == "lyon_gricv1")) && vc[3] == "status") {
            jprms = JSON.parse(message.payloadString);
            let run_mode = jpmrs[0]["mode"]
            if (run_mode != 0) {
                if (run_mode & 2) {
                    let chan = (run_mode >> 4) & 0xFF;
                    let step = (run_mode >> 12) & 0xFFF;
                    setComment("SCURVE Channel " + chan + " step " + step + " on " + vc[1], "calmessages");

                }
                if (run_mode & 4) {
                    let chan = (run_mode >> 4) & 0xFF;
                    let thre = (run_mode >> 12) & 0xFFF;
                    let step = (run_mode >> 24) & 0xFFF;
                    setComment("Gain curve Channel " + chan + " Threshold " + thre + " step " + step + " on " + vc[1], "calmessages");

                }
            }
        }

        else
            if (vc[1] == "evb_builder" && vc[3] == "status") {
                last_status= Date.now();
                jevb = JSON.parse(message.payloadString);
                let run = jevb["answer"]["run"];
                let event = jevb["answer"]["event"];
                document.getElementById("daqrun").innerHTML = run;
                document.getElementById("daqevent").innerHTML = event;
                typeComment("Topic:" + message.destinationName + "| Message : " + message.payloadString, "messages");
            }
            else
                typeComment("Topic:" + message.destinationName + "| Message : " + message.payloadString, "messages");
    jmsg = JSON.parse(message.payloadString);
    //console.log(jmsg);
    //console.log(message.destinationName);

}

async function getStates() {
    let mghost = document.getElementById("mg_host").value;
    let mgport = document.getElementById("mg_port").value;
    const orig = 'http://' + mghost + ':' + mgport;


    let jlist = await spyneCommand(orig, "STATES", null)
    console.log(jlist["content"].length)
    var list_state = document.createElement("select");
    list_state.name = "sel_states";
    list_state.id = "sel_states"

    for (const val of jlist["content"]) {
        var state_option = document.createElement("option");
        state_option.value = val;
        state_option.text = val[0] + "  |  " + val[1] + " => " + val[2];
        //console.log(state_option.text)
        list_state.appendChild(state_option);
    }

    var label = document.createElement("label");
    label.innerHTML = "Choose your state: "
    label.htmlFor = "states";
    var bSetState = document.createElement('input');
    bSetState.type = 'button';
    bSetState.id = 'setstate';
    bSetState.value = 'Download DB';
    bSetState.className = 'btn';
    let board = document.getElementById("db_board").value;

    bSetState.onclick = async function () {
        // …
        let theState = list_state.options[list_state.selectedIndex].value
        vc = theState.split(",")
        var statename = vc[0] + ":" + vc[1];
        alert(vc[0] + " version " + vc[1] + " -> " + daqname);
        let jdaq = await spyneCommand(daqinfo.url, "DOWNLOADDB", { daq: daqinfo.name, app: board, state: vc[0], version: vc[1] });
        console.log(jdaq);


        typeComment("SetDBValues on " + statename, "messages");
    };

    document.getElementById("statesel").appendChild(label).appendChild(list_state)
    document.getElementById("statesel").append(bSetState);





}
