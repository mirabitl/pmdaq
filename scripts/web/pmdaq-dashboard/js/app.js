function applyConfig() {
  STATE.name = global_name.value;
  STATE.version = global_version.value;

  if (!isConfigured()) {
    alert("Name/version required");
    return;
  }

  console.log("Session:", STATE.name, STATE.version);

  connectMQTT();
}

window.onload = () => {
  buildVersionsUI();
  buildFSMUI();
};

