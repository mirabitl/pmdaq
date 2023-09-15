function showlog(jcnt)
{
    document.getElementById("logmessages").innerHTML += "<span>============================================></span><br>";
    document.getElementById("logmessages").innerHTML += "<span>"+jcnt+ "</span><br>";
}
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
    showlog("Calling "+url);

    try {
        let res = await fetch(url);
        let jmsg = await res.json();
	
        vcm = command.split("?")
        let jcnt = jmsg[vcm[0] + "Response"][vcm[0] + "Result"][0];
	showlog(jcnt);
        return JSON.parse(jcnt);
    } catch (error) {
        console.log(error);
    }
}
var daqname = null;
var registered=null;
var pnsdaq = null;
var daqloc = null;

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
        let jconf = await spyneCommand(orig, "CONFIGURATION", pdict)
        console.log(jconf["content"]);
        if ("pns" in jconf["content"])
            pnsdaq = jconf["content"]["pns"];
        else
            pnsdaq = prompt("Enter the PMDAQ name server", "lyocmsmu03");
        daqloc = prompt("Enter the setup name", "???");
        // create the daq in webdaq
        let daqhost = document.getElementById("daq_host").value;
        let daqport = document.getElementById("daq_port").value;


        const origdaq = 'http://' + daqhost + ':' + daqport;
        let pdaq = {
            daqmongo: daqname,
            pnsname: pnsdaq,
            location: daqloc
        }
        let jdaq = await spyneCommand(origdaq, "REGISTERDAQ", pdaq);
        console.log(jdaq);
        document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";
	registered=daqname;
    };
    
    document.getElementById("configsel").appendChild(label).appendChild(list_conf)
    document.getElementById("configsel").append(bSetConfig);





}
var last_refresh=0;
async function refreshStatus(deadline)
{
    if (registered==null)
    {
	requestIdleCallback(refreshStatus);
	return;
    }
    if (Date.now()-last_refresh <10000){
	
	requestIdleCallback(refreshStatus);
	return;
    }
    if (deadline.timeRemaining() <= 5) {
    // This will take more than 5ms so wait until we
    // get called back with a long enough deadline.
	requestIdleCallback(refreshStatus);
	return;
    }
    console.log("Refreshing ... ");
    await getState();
    last_refresh=Date.now();
    requestIdleCallback(refreshStatus);
}
async function getState() {
    // create the daq in webdaq
    
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let pdaq = {
        daq: daqname
    };
    let jstate = await spyneCommand(origdaq, "STATE", pdaq);
    console.log(jstate)
    if (jstate.hasOwnProperty("state")){
	document.getElementById("daqstate").innerHTML = jstate["state"];
	return jstate["state"];
    }
    else
    {
	document.getElementById("daqstate").innerHTML ="NONE";
	return "NONE";
    }
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
    document.getElementById("messages").innerHTML += "<span> CREATE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());
}
async function InitDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;
    let rdelay = document.getElementById("delay_reset").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname,
        delay: rdelay
    }
    let jdaq = await spyneCommand(origdaq, "INITIALISE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> INITIALISE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function ConfigureDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "CONFIGURE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> CONFIGURE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function StartDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let rcom = prompt("Enter a comment for this run", "Chez les papous y'a des papous a pou...");
    let pdaq = {
        daq: daqname,
        comment: rcom
    }
    let jdaq = await spyneCommand(origdaq, "START", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> CONFIGURE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}

async function StopDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "STOP", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> STOP on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function DestroyDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "DESTROY", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> DESTROY on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function RemoveDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "JC_DESTROY", pdaq);
    console.log(jdaq);
    registered=null;
    document.getElementById("messages").innerHTML += "<span> REMOVE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}

async function ResumeDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "RESUME", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> RESUME on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}


async function PauseDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname
    }
    let jdaq = await spyneCommand(origdaq, "PAUSE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> PAUSE on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function processCommand(appname, methodname, parameters) {
    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname,
        method: methodname,
        app: appname,
        params: JSON.stringify(parameters)
    }
    jdaq = await spyneCommand(origdaq, "PROCESS", pdaq);
    console.log(jdaq);
    return jdaq;
}
async function SetTriggerMdcc() {

    let board = document.getElementById("trg_board").value;
    let jrep = await processCommand(board, "SPILLOFF", { nclock: document.getElementById("trg_spilloff").value });
    jrep = await processCommand(board, "SPILLON", { nclock: document.getElementById("trg_spillon").value });
    jrep = await processCommand(board, "SETEXTERNAL", { value: document.getElementById("trg_external").value });
    jrep = await processCommand(board, "SETSPILLREGISTER", { value: document.getElementById("trg_spillreg").value });


    if (board == "lyon_mbmdcc") {
        jrep = await processCommand(board, "ENABLE", { value: document.getElementById("trg_channels").value });
        jrep = await processCommand(board, "CHANNELON", { value: document.getElementById("trg_channels").value });
    }


    document.getElementById("messages").innerHTML += "<span> SetTriggerMdcc on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function SetFebv1Values() {

    let board = document.getElementById("febv1_board").value;
    // create the daq in webdaq
    const origdaq = 'http://' + document.getElementById("daq_host").value + ':' + document.getElementById("daq_port").value;
    let jdaq = await spyneCommand(origdaq, "SETTDCMODE", { daq: daqname, mode: document.getElementById("febv1_mode").value });
    console.log(jdaq);
    jdaq = await spyneCommand(origdaq, "SETTDCDELAYS", { daq: daqname, active: document.getElementById("febv1_duration").value, dead: document.getElementById("febv1_delay").value });
    console.log(jdaq);


    document.getElementById("messages").innerHTML += "<span> SetFebv1Values on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function SetDBValues() {

    let board = document.getElementById("db_board").value;
    // create the daq in webdaq
    const origdaq = 'http://' + document.getElementById("daq_host").value + ':' + document.getElementById("daq_port").value;
    let jdaq = await spyneCommand(origdaq, "DOWNLOADDB", { daq: daqname, app: board, state: document.getElementById("dbstate").value, version: document.getElementById("dbversion").value });
    console.log(jdaq);


    document.getElementById("messages").innerHTML += "<span> SetDBValues on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function SetVthTime() {
    let jrep = await processCommand("lyon_febv1", "SETVTHTIME", { value: document.getElementById("febv1_vthtime").value });
    document.getElementById("messages").innerHTML += "<span> SetVthTime on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}
async function LutCalib() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let pdaq = {
        daq: daqname
    };
    let jstate = await spyneCommand(origdaq, "STATE", pdaq);
    console.log(jstate)
    if (jstate["state"] != "INITIALISED") {
        alert("DAQ must be initialised to make FEBV1 Lut calibration");
        return;
    }
    pdaq = {
        daq: daqname,
        tdc: document.getElementById("tdc_number").value,
        nchannels: document.getElementById("tdc_channels").value
    }
    let jdaq = await spyneCommand(origdaq, "LUTCALIB", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> LUTCALIB on " + daqname + "/" + daqloc + "</span><br>";


}
async function LutMask() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;

    let pdaq = {
        daq: daqname,
        tdc: document.getElementById("tdc_number").value,
        mask: document.getElementById("tdc_mask").value,
        feb: document.getElementById("tdc_feb").value
    }
    let jdaq = await spyneCommand(origdaq, "LUTMASK", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> LUTMASK on " + daqname + "/" + daqloc + "</span><br>";


}
async function doCal_dac10febv1() {
    state=await getState();
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
        alert("The DAQ must be running "+state);
    }
    document.getElementById("messages").innerHTML += "<span> doCal_dac10febv1 on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}

async function doCal_b0pmr() {
    state=await getState();
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
    document.getElementById("messages").innerHTML += "<span> doCal_b0pmr on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

}


async function doCal_b0gric() {
    state=await getState();
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
    document.getElementById("messages").innerHTML += "<span> doCal_b0gric on " + daqname + "/" + daqloc + "</span><br>";
    console.log(await getState());

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
