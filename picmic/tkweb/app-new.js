// Configuration
const API_BASE_URL = 'http://localhost:8000';
const API_ENDPOINT = '/picmic';
const STATUS_UPDATE_INTERVAL = 2000; // ms
const MAX_HISTORY = 50;

// Global state
let currentConfig = null;
let currentMethods = [];
let statusUpdateTimer = null;
let statusHistory = [];
let charts = {};
let mongoConfigs = [];

// Initialize on page load
document.addEventListener('DOMContentLoaded', async () => {
    console.log('Initializing PicoMic Web Control...');
    
    // Initialize charts
    initializeCharts();
    
    // Setup event listeners - Configuration Tab
    document.getElementById('btn-load-mongo-list').addEventListener('click', loadMongoConfigList);
    document.getElementById('btn-download-mongo-config').addEventListener('click', downloadMongoConfig);
    document.getElementById('btn-save-mongo-config').addEventListener('click', saveConfigToMongo);
    document.getElementById('btn-save-json-file').addEventListener('click', saveConfigAsJSON);
    document.getElementById('btn-apply-config').addEventListener('click', applyConfiguration);
    
    // Setup event listeners - Dashboard
    document.getElementById('btn-refresh-status').addEventListener('click', refreshStatus);
    document.getElementById('btn-clear-log').addEventListener('click', clearLog);
    document.getElementById('btn-transition-init').addEventListener('click', () => executeTransition('initialise'));
    document.getElementById('btn-transition-configure').addEventListener('click', () => executeTransition('configure'));
    document.getElementById('btn-transition-start').addEventListener('click', () => executeTransition('start'));
    document.getElementById('btn-transition-stop').addEventListener('click', () => executeTransition('stop'));
    
    // Setup event listeners - Calibration
    document.getElementById('btn-start-calib').addEventListener('click', startCalibration);
    
    // Setup event listeners - Advanced
    document.getElementById('btn-apply-advanced').addEventListener('click', applyAdvancedParams);
    
    // Setup event listeners - Methods
    document.getElementById('btn-execute-method').addEventListener('click', executeMethod);
    
    // Setup event listeners - Navigation
    document.getElementById('btn-docs-link').addEventListener('click', () => {
        window.open(`${API_BASE_URL}/docs`, '_blank');
    });
    
    // Load initial data
    await loadMethods();
    await refreshStatus();
    await loadAdvancedParams();
    
    // Try to load current config
    await loadCurrentConfig();
    
    // Start periodic updates
    startStatusUpdates();
});

// ============================================================================
// CONFIGURATION MANAGEMENT (NEW MONGODB SUPPORT)
// ============================================================================

async function loadMongoConfigList() {
    try {
        logOutput('Loading MongoDB configurations...', 'info');
        const response = await apiCall('/mongo/configs');
        
        mongoConfigs = response.configurations || [];
        
        if (mongoConfigs.length === 0) {
            logOutput('❌ No configurations found in MongoDB', 'warn');
            return;
        }
        
        // Populate dropdown
        const select = document.getElementById('mongo-config-select');
        select.innerHTML = '';
        mongoConfigs.forEach((config, idx) => {
            const option = document.createElement('option');
            option.value = idx;
            option.textContent = config.label;
            select.appendChild(option);
        });
        select.disabled = false;
        
        logOutput(`✓ Loaded ${mongoConfigs.length} configurations from MongoDB`, 'success');
    } catch (error) {
        logOutput('Failed to load MongoDB configurations', 'error');
    }
}

async function downloadMongoConfig() {
    const select = document.getElementById('mongo-config-select');
    const idx = parseInt(select.value);
    
    if (isNaN(idx) || !mongoConfigs[idx]) {
        logOutput('❌ Select a configuration first', 'error');
        return;
    }
    
    const config = mongoConfigs[idx];
    
    try {
        logOutput(`Downloading ${config.label}...`, 'info');
        const response = await apiCall('/mongo/config/download', 'POST', {
            name: config.name,
            version: config.version
        });
        
        currentConfig = response.config;
        renderConfigTree(currentConfig);
        logOutput(`✓ Configuration ${config.label} downloaded and loaded`, 'success');
    } catch (error) {
        logOutput(`Failed to download configuration: ${error}`, 'error');
    }
}

async function loadCurrentConfig() {
    try {
        const response = await apiCall('/config/current');
        if (response.config) {
            currentConfig = response.config;
            renderConfigTree(currentConfig);
            logOutput('✓ Current configuration loaded', 'success');
        }
    } catch (error) {
        console.log('No configuration loaded yet');
    }
}

async function applyConfiguration() {
    if (!currentConfig) {
        logOutput('❌ No configuration loaded', 'error');
        return;
    }
    
    try {
        logOutput('Applying configuration and creating DAQ...', 'info');
        
        // First update the config
        await apiCall('/config/update', 'POST', currentConfig);
        
        // Then create the DAQ with the configuration
        // Extract name and version from config, default to current name
        const name = currentConfig.name || 'default';
        const version = currentConfig.version || 0;
        
        const response = await apiCall('/create', 'POST', {
            name: name,
            version: version
        });
        
        logOutput('✓ DAQ created successfully', 'success');
        logOutput(`Status: ${JSON.stringify(response)}`, 'info');
    } catch (error) {
        logOutput(`❌ Failed to apply configuration: ${error}`, 'error');
    }
}

async function saveConfigToMongo() {
    const name = document.getElementById('config-save-name').value.trim();
    const comment = document.getElementById('config-save-comment').value.trim();
    
    if (!name) {
        logOutput('❌ Configuration name is required', 'error');
        return;
    }
    
    if (!currentConfig) {
        logOutput('❌ No configuration loaded to save', 'error');
        return;
    }
    
    try {
        logOutput(`Saving configuration "${name}" to MongoDB...`, 'info');
        
        // Save current config to MongoDB
        const response = await apiCall('/mongo/config/save', 'POST', {
            name: name,
            comment: comment
        });
        
        logOutput(`✓ Configuration saved to MongoDB`, 'success');
        document.getElementById('config-save-name').value = '';
        document.getElementById('config-save-comment').value = '';
        
        // Reload the list
        await loadMongoConfigList();
    } catch (error) {
        logOutput(`❌ Failed to save configuration: ${error}`, 'error');
    }
}

async function saveConfigAsJSON() {
    if (!currentConfig) {
        logOutput('❌ No configuration loaded', 'error');
        return;
    }
    
    try {
        const name = document.getElementById('config-save-name').value.trim() || 'config';
        const jsonStr = JSON.stringify(currentConfig, null, 2);
        const blob = new Blob([jsonStr], { type: 'application/json' });
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.href = url;
        a.download = `${name}.json`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        logOutput(`✓ Configuration saved as ${name}.json`, 'success');
    } catch (error) {
        logOutput(`Failed to save JSON: ${error}`, 'error');
    }
}

// ============================================================================
// JSON TREE VIEW
// ============================================================================

function renderConfigTree(config, depth = 0) {
    const treeDiv = document.getElementById('config-tree');
    if (!treeDiv) return;
    
    treeDiv.innerHTML = '';
    
    if (!config) {
        treeDiv.innerHTML = '<p class="text-muted">No configuration loaded</p>';
        return;
    }
    
    const root = document.createElement('div');
    root.className = 'tree-node';
    
    renderTreeNode(root, 'configuration', config, []);
    treeDiv.appendChild(root);
}

function renderTreeNode(parent, key, value, path) {
    const nodeDiv = document.createElement('div');
    nodeDiv.className = 'tree-node';
    
    const itemDiv = document.createElement('div');
    itemDiv.className = 'tree-item';
    
    const newPath = [...path, key];
    const isObject = value !== null && typeof value === 'object' && !Array.isArray(value);
    const isArray = Array.isArray(value);
    const isPrimitive = !isObject && !isArray;
    
    // Toggle button
    if (isObject || isArray) {
        const toggle = document.createElement('span');
        toggle.className = 'tree-toggle expanded';
        toggle.addEventListener('click', (e) => {
            e.stopPropagation();
            toggle.classList.toggle('expanded');
            toggle.classList.toggle('collapsed');
            childrenDiv.classList.toggle('hidden');
        });
        itemDiv.appendChild(toggle);
    } else {
        const noToggle = document.createElement('span');
        noToggle.className = 'tree-toggle no-children';
        itemDiv.appendChild(noToggle);
    }
    
    // Key
    const keySpan = document.createElement('span');
    keySpan.className = 'tree-key';
    keySpan.textContent = key;
    itemDiv.appendChild(keySpan);
    
    // Value or summary
    const valueSpan = document.createElement('span');
    valueSpan.className = 'tree-value';
    
    if (isArray) {
        valueSpan.innerHTML = `<span class="array-summary">[${value.length} items]</span>`;
    } else if (isObject) {
        valueSpan.innerHTML = `<span class="object-summary">{${Object.keys(value).length} keys}</span>`;
    } else if (typeof value === 'string') {
        valueSpan.innerHTML = `<span class="string">"${escapeHtml(value)}"</span>`;
        const editBtn = document.createElement('button');
        editBtn.className = 'btn btn-xs btn-outline-primary tree-value-edit';
        editBtn.textContent = '✎';
        editBtn.addEventListener('click', () => editValue(newPath, value, 'string'));
        valueSpan.appendChild(editBtn);
    } else if (typeof value === 'number') {
        valueSpan.innerHTML = `<span class="number">${value}</span>`;
        const editBtn = document.createElement('button');
        editBtn.className = 'btn btn-xs btn-outline-primary tree-value-edit';
        editBtn.textContent = '✎';
        editBtn.addEventListener('click', () => editValue(newPath, value, 'number'));
        valueSpan.appendChild(editBtn);
    } else if (typeof value === 'boolean') {
        valueSpan.innerHTML = `<span class="boolean">${value ? 'true' : 'false'}</span>`;
        const editBtn = document.createElement('button');
        editBtn.className = 'btn btn-xs btn-outline-primary tree-value-edit';
        editBtn.textContent = '✎';
        editBtn.addEventListener('click', () => editValue(newPath, value, 'boolean'));
        valueSpan.appendChild(editBtn);
    } else if (value === null) {
        valueSpan.innerHTML = `<span class="null">null</span>`;
    }
    
    itemDiv.appendChild(valueSpan);
    nodeDiv.appendChild(itemDiv);
    
    // Children
    if (isObject || isArray) {
        const childrenDiv = document.createElement('div');
        childrenDiv.className = 'tree-children';
        
        const items = isArray ? value : Object.entries(value);
        
        (isArray ? items : items).forEach((item, idx) => {
            const [childKey, childValue] = isArray ? [idx, item] : item;
            renderTreeNode(childrenDiv, childKey, childValue, newPath);
        });
        
        nodeDiv.appendChild(childrenDiv);
    }
    
    parent.appendChild(nodeDiv);
}

function editValue(path, currentValue, type) {
    const input = prompt(`Edit value (${type}):`, currentValue);
    if (input === null) return;
    
    let newValue = input;
    try {
        if (type === 'number') newValue = parseFloat(input);
        if (type === 'boolean') newValue = input.toLowerCase() === 'true';
        
        setValueInConfig(currentConfig, path, newValue);
        renderConfigTree(currentConfig);
        logOutput(`✓ Value updated: ${path.join('.')}`, 'success');
    } catch (e) {
        logOutput(`❌ Invalid value: ${e}`, 'error');
    }
}

function setValueInConfig(obj, path, value) {
    let current = obj;
    for (let i = 0; i < path.length - 1; i++) {
        current = current[path[i]];
    }
    current[path[path.length - 1]] = value;
}

function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

// ============================================================================
// CHART INITIALIZATION
// ============================================================================

function initializeCharts() {
    // Monitor chart (Events over time)
    const monitorCtx = document.getElementById('chartMonitor')?.getContext('2d');
    if (monitorCtx) {
        charts.monitor = new Chart(monitorCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Events',
                        data: [],
                        borderColor: '#0d6efd',
                        backgroundColor: 'rgba(13, 110, 253, 0.1)',
                        tension: 0.1
                    },
                    {
                        label: 'Run',
                        data: [],
                        borderColor: '#198754',
                        backgroundColor: 'rgba(25, 135, 84, 0.1)',
                        tension: 0.1
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: { beginAtZero: true }
                },
                plugins: {
                    legend: { position: 'bottom' }
                }
            }
        });
    }

    // S-Curve chart
    const scurveCtx = document.getElementById('chartScurve')?.getContext('2d');
    if (scurveCtx) {
        charts.scurve = new Chart(scurveCtx, {
            type: 'scatter',
            data: {
                datasets: [
                    {
                        label: 'S-Curve Data',
                        data: [],
                        borderColor: '#fd7e14',
                        backgroundColor: 'rgba(253, 126, 20, 0.3)',
                        showLine: true,
                        borderWidth: 2,
                        pointRadius: 4
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: { type: 'linear', title: { display: true, text: 'Threshold' } },
                    y: { title: { display: true, text: 'Response' } }
                },
                plugins: {
                    legend: { position: 'bottom' }
                }
            }
        });
    }
}

// ============================================================================
// API CALLS
// ============================================================================

async function apiCall(endpoint, method = 'GET', body = null) {
    try {
        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            }
        };
        
        if (body) {
            options.body = JSON.stringify(body);
        }
        
        const response = await fetch(`${API_BASE_URL}${API_ENDPOINT}${endpoint}`, options);
        
        if (!response.ok) {
            let error = {};
            try {
                error = await response.json();
            } catch (e) {
                error = { detail: response.statusText };
            }
            throw new Error(error.detail || `HTTP ${response.status}`);
        }
        
        const contentType = response.headers.get('content-type');
        if (contentType && contentType.includes('application/json')) {
            return await response.json();
        }
        return null;
    } catch (error) {
        console.error('API Error:', error);
        logOutput(`❌ Error: ${error.message}`, 'error');
        updateConnectionStatus(false);
        throw error;
    }
}

function updateConnectionStatus(connected) {
    const statusEl = document.getElementById('connection-status');
    if (connected) {
        statusEl.className = 'badge bg-success';
        statusEl.textContent = 'Connected';
    } else {
        statusEl.className = 'badge bg-danger';
        statusEl.textContent = 'Disconnected';
    }
}

// ============================================================================
// METHODS
// ============================================================================

async function loadMethods() {
    try {
        const response = await apiCall('/methods');
        currentMethods = response.methods || [];
        updateConnectionStatus(true);
        renderMethods(currentMethods);
        logOutput('✓ Methods loaded successfully', 'success');
    } catch (error) {
        logOutput('Failed to load methods', 'error');
    }
}

function renderMethods(methods) {
    const container = document.getElementById('methods-list');
    
    if (!methods || methods.length === 0) {
        container.innerHTML = '<p class="text-muted">No methods available</p>';
        return;
    }
    
    const grouped = {};
    methods.forEach(method => {
        const category = method.name.split('_')[0] || 'Other';
        if (!grouped[category]) grouped[category] = [];
        grouped[category].push(method);
    });
    
    let html = '';
    Object.entries(grouped).forEach(([category, categoryMethods]) => {
        html += `<div class="mb-3">
                    <h6 class="text-secondary">${category}</h6>
                    <div class="button-group">`;
        
        categoryMethods.forEach(method => {
            html += `<button class="btn btn-sm btn-outline-primary" 
                            onclick="showMethodModal('${method.name}')">
                        ${method.name}
                    </button>`;
        });
        
        html += `</div></div>`;
    });
    
    container.innerHTML = html;
}

function showMethodModal(methodName) {
    const method = currentMethods.find(m => m.name === methodName);
    if (!method) return;
    
    document.getElementById('methodModalTitle').textContent = `Execute: ${methodName}`;
    
    let modalBody = '';
    if (method.parameters.length === 0) {
        modalBody = '<p class="text-muted">This method takes no parameters</p>';
    } else {
        modalBody = '<form id="method-params-form">';
        method.parameters.forEach(param => {
            const inputType = getInputType(param.type);
            const defaultValue = param.default !== null ? param.default : '';
            
            modalBody += `
                <div class="mb-3">
                    <label for="param-${param.name}" class="form-label">
                        ${param.name}
                        <small class="text-muted">(${param.type})</small>
                    </label>
                    <input type="${inputType}" 
                           class="form-control" 
                           id="param-${param.name}" 
                           name="${param.name}"
                           value="${defaultValue}"
                           placeholder="Enter ${param.name}">
                </div>
            `;
        });
        modalBody += '</form>';
    }
    
    document.getElementById('methodModalBody').innerHTML = modalBody;
    document.getElementById('btn-execute-method').dataset.methodName = methodName;
    
    const modal = new bootstrap.Modal(document.getElementById('methodModal'));
    modal.show();
}

function getInputType(pythonType) {
    if (!pythonType) return 'text';
    const type = pythonType.toLowerCase();
    if (type.includes('int')) return 'number';
    if (type.includes('float')) return 'number';
    if (type.includes('bool')) return 'checkbox';
    return 'text';
}

async function executeMethod() {
    const methodName = document.getElementById('btn-execute-method').dataset.methodName;
    const form = document.getElementById('method-params-form');
    
    const params = {};
    if (form) {
        const formData = new FormData(form);
        for (let [key, value] of formData) {
            if (value === 'true') params[key] = true;
            else if (value === 'false') params[key] = false;
            else if (!isNaN(value) && value !== '') params[key] = Number(value);
            else params[key] = value;
        }
    }
    
    try {
        logOutput(`Executing ${methodName}...`, 'info');
        
        const response = await apiCall('/commands', 'POST', {
            name: methodName,
            params: params
        });
        
        logOutput(`✓ ${methodName} executed successfully`, 'success');
        if (response && response.result) {
            logOutput(`Result: ${JSON.stringify(response.result, null, 2)}`, 'info');
        }
        
        await refreshStatus();
        bootstrap.Modal.getInstance(document.getElementById('methodModal')).hide();
    } catch (error) {
        logOutput(`✗ Failed to execute ${methodName}`, 'error');
    }
}

// ============================================================================
// TRANSITIONS (State Machine)
// ============================================================================

async function executeTransition(transitionName) {
    try {
        logOutput(`Executing transition: ${transitionName}...`, 'info');
        
        const response = await apiCall('/transitions', 'POST', {
            name: transitionName,
            params: {}
        });
        
        logOutput(`✓ Transition ${transitionName} completed`, 'success');
        await refreshStatus();
    } catch (error) {
        logOutput(`✗ Transition ${transitionName} failed`, 'error');
    }
}

// ============================================================================
// STATUS & MONITORING
// ============================================================================

async function refreshStatus() {
    try {
        const response = await apiCall('/');
        
        if (response && response.status) {
            const status = response.status;
            document.getElementById('status-state').textContent = status.state || '--';
            document.getElementById('status-run').textContent = status.run || 0;
            document.getElementById('status-event').textContent = status.event || 0;
            
            const runningBadge = document.getElementById('status-running');
            if (status.running) {
                runningBadge.textContent = 'RUNNING';
                runningBadge.className = 'badge bg-success';
            } else {
                runningBadge.textContent = 'IDLE';
                runningBadge.className = 'badge bg-danger';
            }
            
            // Update chart
            updateMonitorChart(status);
        }
        
        updateConnectionStatus(true);
    } catch (error) {
        updateConnectionStatus(false);
    }
}

function updateMonitorChart(status) {
    if (!charts.monitor) return;
    
    const now = new Date().toLocaleTimeString();
    
    // Keep only last MAX_HISTORY entries
    if (charts.monitor.data.labels.length >= MAX_HISTORY) {
        charts.monitor.data.labels.shift();
        charts.monitor.data.datasets[0].data.shift();
        charts.monitor.data.datasets[1].data.shift();
    }
    
    charts.monitor.data.labels.push(now);
    charts.monitor.data.datasets[0].data.push(status.event || 0);
    charts.monitor.data.datasets[1].data.push(status.run || 0);
    charts.monitor.update('none');
}

function startStatusUpdates() {
    if (statusUpdateTimer) clearInterval(statusUpdateTimer);
    statusUpdateTimer = setInterval(refreshStatus, STATUS_UPDATE_INTERVAL);
}

// ============================================================================
// CALIBRATION
// ============================================================================

async function startCalibration() {
    const params = {
        thmin: parseInt(document.getElementById('calib-thmin').value) || 0,
        thmax: parseInt(document.getElementById('calib-thmax').value) || 1000,
        thstep: parseInt(document.getElementById('calib-thstep').value) || 50,
        dc_pa: parseInt(document.getElementById('calib-dcpa').value) || 0,
        mode: document.getElementById('calib-mode').value || undefined
    };
    
    try {
        logOutput('Starting calibration...', 'info');
        
        const response = await apiCall('/calibration', 'POST', params);
        
        logOutput('✓ Calibration started', 'success');
        logOutput(`Result: ${JSON.stringify(response, null, 2)}`, 'info');
        
        // Simulate loading S-curve data
        setTimeout(() => loadCalibrationData(), 2000);
    } catch (error) {
        logOutput('✗ Failed to start calibration', 'error');
    }
}

async function loadCalibrationData() {
    try {
        const response = await apiCall('/calibration-data');
        
        if (response.data) {
            // Update S-curve chart
            if (charts.scurve) {
                charts.scurve.data.datasets[0].data = response.data;
                charts.scurve.update();
            }
            logOutput('✓ S-curve data updated', 'success');
        }
    } catch (error) {
        logOutput('Could not load calibration data', 'warn');
    }
}

// ============================================================================
// ADVANCED PARAMETERS
// ============================================================================

async function loadAdvancedParams() {
    try {
        const response = await apiCall('/advanced-params');
        
        document.getElementById('adv-threshold').value = response.threshold || 800;
        document.getElementById('adv-dcpa').value = response.dc_pa || 0;
        document.getElementById('adv-filtering').checked = response.filtering || false;
        document.getElementById('adv-falling').checked = response.falling || false;
        document.getElementById('adv-valEvt').checked = response.val_evt || false;
        document.getElementById('adv-polarity').checked = response.pol_neg || false;
    } catch (error) {
        logOutput('Could not load advanced parameters', 'warn');
    }
}

async function applyAdvancedParams() {
    const params = {
        threshold: parseInt(document.getElementById('adv-threshold').value) || 800,
        dc_pa: parseInt(document.getElementById('adv-dcpa').value) || 0,
        filtering: document.getElementById('adv-filtering').checked,
        falling: document.getElementById('adv-falling').checked,
        val_evt: document.getElementById('adv-valEvt').checked,
        pol_neg: document.getElementById('adv-polarity').checked
    };
    
    try {
        logOutput('Applying advanced parameters...', 'info');
        
        const response = await apiCall('/advanced-params', 'POST', params);
        
        logOutput('✓ Advanced parameters applied', 'success');
    } catch (error) {
        logOutput('✗ Failed to apply parameters', 'error');
    }
}

// ============================================================================
// LOGGING
// ============================================================================

function logOutput(message, level = 'info') {
    const logEl = document.getElementById('output-log');
    
    if (logEl.textContent.includes('Waiting for events')) {
        logEl.innerHTML = '';
    }
    
    const timestamp = new Date().toLocaleTimeString();
    const className = `log-${level}`;
    
    const entry = document.createElement('div');
    entry.className = className;
    entry.innerHTML = `<code>[${timestamp}] ${escapeHtml(message)}</code>`;
    
    logEl.appendChild(entry);
    logEl.scrollTop = logEl.scrollHeight;
}

function clearLog() {
    document.getElementById('output-log').innerHTML = '<p class="text-muted">Log cleared</p>';
}
