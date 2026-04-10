async function api(path, method='GET', body=null) {
  const res = await fetch(CONFIG.API_BASE + path, {
    method,
    headers: {'Content-Type':'application/json'},
    body: body ? JSON.stringify(body) : null
  });
  return await res.json();
}

// ---------- VERSIONS ----------
async function createApp() {
  const name = v_name.value;
  const version = v_version.value;

  const res = await api('/apps/', 'POST', {name, version});
  versions_output.textContent = JSON.stringify(res, null, 2);
}

async function deleteApp() {
  const name = d_name.value;
  const version = d_version.value;

  const res = await api(`/apps/${name}/versions/${version}`, 'DELETE');
  versions_output.textContent = JSON.stringify(res, null, 2);
}

async function configureApp() {
  const name = c_name.value;
  const version = c_version.value;
  const params = JSON.parse(c_params.value);

  const res = await api(`/apps/${name}/versions/${version}/configure`, 'POST', {params});
  versions_output.textContent = JSON.stringify(res, null, 2);
}

// ---------- FSM ----------
async function sendTransition() {
  const name = t_name.value;
  const version = t_version.value;
  const transition = t_transition.value;

  const res = await api(`/apps/${name}/versions/${version}/transitions`, 'POST', {
    name: transition
  });

  fsm_output.textContent = JSON.stringify(res, null, 2);
}

async function sendCommand() {
  const name = cmd_name.value;
  const version = cmd_version.value;
  const cmd = cmd_command.value;
  const params = JSON.parse(cmd_params.value);

  const res = await api(`/apps/${name}/versions/${version}/commands`, 'POST', {
    cmd,
    params
  });

  fsm_output.textContent = JSON.stringify(res, null, 2);
}
