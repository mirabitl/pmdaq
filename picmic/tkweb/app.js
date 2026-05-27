// Configuration
const API_BASE_URL = 'http://localhost:8000';
const API_ENDPOINT = '/picmic';
const STATUS_UPDATE_INTERVAL = 2000; // ms
const MAX_HISTORY = 50;

// Global state
let currentMethods = [];
let statusUpdateTimer = null;
let statusHistory = [];
let charts = {};

// Initialize on page load
document.addEventListener('DOMContentLoaded', async () => {
    console.log('Initializing PicoMic Web Control...');
    
    // Initialize charts
    initializeCharts();
    
    // Setup event listeners - Dashboard
    document.getElementById('btn-refresh-status').addEventListener('click', refreshStatus);
    document.getElementById('btn-clear-log').addEventListener('click', clearLog);
    document.getElementById('btn-transition-init').addEventListener('click', () => executeTransition('initialise'));
    document.getElementById('btn-transition-configure').addEventListener('click', () => executeTransition('configure'));
    document.getElementById('btn-transition-start').addEventListener('click', () => executeTransition('start'));
    document.getElementById('btn-transition-stop').addEventListener('click', () => executeTransition('stop'));
    
    // Setup event listeners - Calibration
    document.getElementById('btn-start-calib').addEventListener('click', startCalibration);
    
    // Setup event listeners - Configuration
    document.getElementById('btn-save-config').addEventListener('click', saveConfiguration);
    document.getElementById('btn-refresh-configs').addEventListener('click', loadConfigurations);
    
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
    await loadConfigurations();
    await loadAdvancedParams();
    
    // Start periodic updates
    startStatusUpdates();
});

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
// CONFIGURATION MANAGEMENT
// ============================================================================

async function loadConfigurations() {
    try {
        const response = await apiCall('/config/list');
        const configs = response.configurations || [];
        
        const configList = document.getElementById('config-list');
        if (configs.length === 0) {
            configList.innerHTML = '<p class="text-muted">No saved configurations</p>';
            return;
        }
        
        let html = '<div class="list-group">';
        configs.forEach(config => {
            const date = new Date(config.modified * 1000).toLocaleString();
            html += `
                <div class="list-group-item d-flex justify-content-between align-items-center">
                    <div>
                        <strong>${config.name}</strong>
                        <br>
                        <small class="text-muted">${date}</small>
                    </div>
                    <div>
                        <button class="btn btn-sm btn-info" onclick="loadConfigFile('${config.name}')">
                            Load
                        </button>
                        <button class="btn btn-sm btn-danger" onclick="deleteConfigFile('${config.name}')">
                            Delete
                        </button>
                    </div>
                </div>
            `;
        });
        html += '</div>';
        
        configList.innerHTML = html;
    } catch (error) {
        logOutput('Failed to load configurations', 'error');
    }
}

async function saveConfiguration() {
    const name = document.getElementById('config-save-name').value;
    const desc = document.getElementById('config-save-desc').value;
    
    if (!name) {
        alert('Please enter a configuration name');
        return;
    }
    
    try {
        logOutput(`Saving configuration: ${name}...`, 'info');
        
        await apiCall(`/config/save?name=${name}&description=${encodeURIComponent(desc)}`, 'POST');
        
        logOutput(`✓ Configuration ${name} saved`, 'success');
        document.getElementById('config-save-name').value = '';
        document.getElementById('config-save-desc').value = '';
        
        await loadConfigurations();
    } catch (error) {
        logOutput('Failed to save configuration', 'error');
    }
}

async function loadConfigFile(configName) {
    try {
        logOutput(`Loading configuration: ${configName}...`, 'info');
        
        await apiCall(`/config/load/${configName}`);
        
        logOutput(`✓ Configuration ${configName} loaded`, 'success');
        await loadAdvancedParams();
    } catch (error) {
        logOutput('Failed to load configuration', 'error');
    }
}

async function deleteConfigFile(configName) {
    if (!confirm(`Delete configuration "${configName}"?`)) return;
    
    try {
        logOutput(`Deleting configuration: ${configName}...`, 'info');
        
        await apiCall(`/config/${configName}`, 'DELETE');
        
        logOutput(`✓ Configuration ${configName} deleted`, 'success');
        await loadConfigurations();
    } catch (error) {
        logOutput('Failed to delete configuration', 'error');
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

function escapeHtml(text) {
    const map = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#039;'
    };
    return text.replace(/[&<>"']/g, m => map[m]);
}

// ============================================================================
// CLEANUP
// ============================================================================

window.addEventListener('beforeunload', () => {
    if (statusUpdateTimer) clearInterval(statusUpdateTimer);
});
