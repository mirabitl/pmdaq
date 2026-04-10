// -------- Tabs --------
function openTab(id) {
  document.querySelectorAll('.content').forEach(c => c.classList.remove('active'));
  document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));

  document.getElementById(id).classList.add('active');
  event.target.classList.add('active');
}

// -------- STATE COLOR --------
function stateColor(s) {
  if (s === "RUNNING") return "lime";
  if (s === "CONFIGURED") return "orange";
  if (s === "STOPPED") return "red";
  return "#6cf";
}

// -------- MQTT RENDER --------
function renderMQTT() {
  const container = document.getElementById('mqttData');
  container.innerHTML = '';

  Object.entries(STATE.data).forEach(([app, instances]) => {

    const appDiv = document.createElement('div');
    appDiv.className = 'app';
    appDiv.innerHTML = `<h3>${app}</h3>`;

    Object.entries(instances).forEach(([inst, data]) => {

      const state = data.state?.value || data.info?.STATE || "?";

      let html = `
        <div class="instance">
          <div>
            Instance: ${inst}
            <span class="state" style="color:${stateColor(state)}">
              ${state}
            </span>
          </div>
      `;

      // -------- COMMANDES DYNAMIQUES (FSM depuis MQTT) --------
      if (data.info?.ALLOWED) {
        html += `<div>`;
        data.info.ALLOWED.forEach(cmd => {
          const name = cmd.split('/').pop();
          html += `<button onclick="quickTransition('${name}')">${name}</button>`;
        });
        html += `</div>`;
      }

      // -------- DETAILS --------
      html += `
        <details>
          <summary>Details</summary>
          <pre>${JSON.stringify(data, null, 2)}</pre>
        </details>
      </div>
      `;

      const wrapper = document.createElement('div');
      wrapper.innerHTML = html;
      appDiv.appendChild(wrapper);
    });

    container.appendChild(appDiv);
  });
}

//
// ==========================
// VERSIONS UI
// ==========================
//
function buildVersionsUI() {
  const el = document.getElementById('versionsUI');

  el.innerHTML = `
    <div class="card">
      <h3>Create</h3>
      <button onclick="createApp()">Create</button>
    </div>

    <div class="card">
      <h3>Delete</h3>
      <button onclick="deleteApp()">Delete</button>
    </div>

    <div class="card">
      <h3>Configure</h3>
      <textarea id="c_params">{}</textarea>
      <button onclick="configureApp()">Configure</button>
    </div>

    <pre id="versions_output"></pre>
  `;
}

//
// ==========================
// FSM UI
// ==========================
//
function buildFSMUI() {
  const el = document.getElementById('fsmUI');

  el.innerHTML = `
    <div class="card">
      <h3>Transition</h3>
      <input id="t_transition" placeholder="transition">
      <button onclick="sendTransition()">Send</button>
    </div>

    <div class="card">
      <h3>Command</h3>
      <input id="cmd_command" placeholder="command">
      <textarea id="cmd_params">{}</textarea>
      <button onclick="sendCommand()">Send</button>
    </div>

    <pre id="fsm_output"></pre>
  `;
}


