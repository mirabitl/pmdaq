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
var daqloc=null;
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
        console.log(jconf["content"]);
        if ( "pns" in jconf["content"])
            pnsdaq=jconf["content"]["pns"];
        else
            pnsdaq=prompt("Enter the PMDAQ name server","lyocmsmu03");
        daqloc=prompt("Enter the setup name","???");
        // create the daq in webdaq
        let daqhost = document.getElementById("daq_host").value;
        let daqport = document.getElementById("daq_port").value;
    
    
        const origdaq = 'http://' + daqhost + ':' + daqport;
        let pdaq={
            daqmongo:daqname,
            pnsname:pnsdaq,
            location:daqloc
        }
        let jdaq = await spyneCommand(origdaq, "REGISTERDAQ",pdaq);
        console.log(jdaq);
	document.getElementById("messages").innerHTML += "<span> Connecting to " + daqhost + "on port " + daqport + "</span><br>";
    };

    document.getElementById("configsel").appendChild(label).appendChild(list_conf)
    document.getElementById("configsel").append(bSetConfig);


    


}
async function getState()
{
      // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let pdaq = {
	daq:daqname
    };
    let jstate = await spyneCommand(origdaq, "STATE", pdaq);
    document.getElementById("daqstate").value=jstate["state"];
    return jstate["state"];
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
    document.getElementById("messages").innerHTML += "<span> CREATE on " + daqname + "/" + daqloc +  "</span><br>";

}
async function InitDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;
    let rdelay = document.getElementById("delay_reset").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    
    let pdaq = {
        daq: daqname,
        delay:rdelay
    }
    let jdaq = await spyneCommand(origdaq, "INITIALISE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> INITIALISE on " + daqname + "/" + daqloc +  "</span><br>";


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
    document.getElementById("messages").innerHTML += "<span> CONFIGURE on " + daqname + "/" + daqloc +  "</span><br>";


}
async function StartDaq() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let rcom=prompt("Enter a comment for this run","Chez les papous y'a des papous a pou...");
    let pdaq = {
        daq: daqname,
	comment:rcom
        }
    let jdaq = await spyneCommand(origdaq, "CONFIGURE", pdaq);
    console.log(jdaq);
    document.getElementById("messages").innerHTML += "<span> CONFIGURE on " + daqname + "/" + daqloc +  "</span><br>";


}
async function LutCalib() {

    // create the daq in webdaq
    let daqhost = document.getElementById("daq_host").value;
    let daqport = document.getElementById("daq_port").value;


    const origdaq = 'http://' + daqhost + ':' + daqport;
    let pdaq = {
	daq:daqname
    };
    let jstate = await spyneCommand(origdaq, "STATE", pdaq);
    console.log(jstate)
    if (jstate["state"]!="INITIALISED")
    {
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
    document.getElementById("messages").innerHTML += "<span> LUTCALIB on " + daqname + "/" + daqloc +  "</span><br>";


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
    document.getElementById("messages").innerHTML += "<span> LUTMASK on " + daqname + "/" + daqloc +  "</span><br>";


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
