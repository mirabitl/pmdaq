
// ---------- TABS ----------
function openTab(id) {
  document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));

  document.getElementById(id).classList.add('active');
  event.target.classList.add('active');
}

// ---------- MQTT ----------
function renderMQTT() {
  const container = document.getElementById('mqttData');
  container.innerHTML = '';

  Object.entries(mqttData).forEach(([topic, data]) => {
    const div = document.createElement('div');
    div.className = 'card';
    div.innerHTML = `<b>${topic}</b><pre>${JSON.stringify(data, null, 2)}</pre>`;
    container.appendChild(div);
  });
}

// ---------- VERSIONS UI ----------
function buildVersionsUI() {
  const el = document.getElementById('versionsUI');

  el.innerHTML = `
    <div class="card">
      <h3>Create</h3>
      <input id="v_name" placeholder="name">
      <input id="v_version" placeholder="version">
      <button onclick="createApp()">Create</button>
    </div>

    <div class="card">
      <h3>Delete</h3>
      <input id="d_name" placeholder="name">
      <input id="d_version" placeholder="version">
      <button onclick="deleteApp()">Delete</button>
    </div>

    <div class="card">
      <h3>Configure</h3>
      <input id="c_name" placeholder="name">
      <input id="c_version" placeholder="version">
      <textarea id="c_params">{}</textarea>
      <button onclick="configureApp()">Configure</button>
    </div>

    <pre id="versions_output"></pre>
  `;
}

// ---------- FSM UI ----------
function buildFSMUI() {
  const el = document.getElementById('fsmUI');

  el.innerHTML = `
    <div class="card">
      <h3>Transition</h3>
      <input id="t_name" placeholder="name">
      <input id="t_version" placeholder="version">
      <input id="t_transition" placeholder="transition">
      <button onclick="sendTransition()">Send</button>
    </div>

    <div class="card">
      <h3>Command</h3>
      <input id="cmd_name" placeholder="name">
      <input id="cmd_version" placeholder="version">
      <input id="cmd_command" placeholder="command">
      <textarea id="cmd_params">{}</textarea>
      <button onclick="sendCommand()">Send</button>
    </div>

    <pre id="fsm_output"></pre>
  `;
}
