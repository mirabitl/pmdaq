// Configuration
const API_BASE_URL = 'http://localhost:8000';
const API_ENDPOINT = '/picmic';
const STATUS_UPDATE_INTERVAL = 2000; // ms

// Global state
let currentMethods = [];
let statusUpdateTimer = null;

// Initialize on page load
document.addEventListener('DOMContentLoaded', async () => {
    console.log('Initializing PicoMic Web Control...');
    
    // Setup event listeners
    document.getElementById('btn-create-config').addEventListener('click', createConfiguration);
    document.getElementById('btn-refresh-status').addEventListener('click', refreshStatus);
    document.getElementById('btn-execute-method').addEventListener('click', executeMethod);
    document.getElementById('btn-clear-log').addEventListener('click', clearLog);
    
    // Load methods and start status updates
    await loadMethods();
    await refreshStatus();
    startStatusUpdates();
});

// Utility function: Make API calls
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
            const error = await response.json();
            throw new Error(error.detail || `HTTP ${response.status}`);
        }
        
        // Check if response has content
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

// Update connection status indicator
function updateConnectionStatus(connected) {
    const statusEl = document.getElementById('connection-status');
    if (connected) {
        statusEl.innerHTML = '<span class="badge bg-success">Connected</span>';
    } else {
        statusEl.innerHTML = '<span class="badge bg-danger">Disconnected</span>';
    }
}

// Load available methods from server
async function loadMethods() {
    try {
        const response = await apiCall('/methods');
        currentMethods = response.methods;
        
        updateConnectionStatus(true);
        renderMethods(currentMethods);
        logOutput('✓ Methods loaded successfully', 'success');
    } catch (error) {
        logOutput('Failed to load methods', 'error');
    }
}

// Render methods as buttons
function renderMethods(methods) {
    const container = document.getElementById('methods-list');
    
    if (!methods || methods.length === 0) {
        container.innerHTML = '<p class="text-muted">No methods available</p>';
        return;
    }
    
    // Group methods by category
    const grouped = {};
    methods.forEach(method => {
        const category = method.name.split('_')[0] || 'Other';
        if (!grouped[category]) {
            grouped[category] = [];
        }
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
        
        html += `   </div>
                </div>`;
    });
    
    container.innerHTML = html;
}

// Show method execution modal
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

// Get HTML input type from Python type annotation
function getInputType(pythonType) {
    if (!pythonType) return 'text';
    
    const type = pythonType.toLowerCase();
    if (type.includes('int')) return 'number';
    if (type.includes('float')) return 'number';
    if (type.includes('bool')) return 'checkbox';
    
    return 'text';
}

// Execute method with parameters
async function executeMethod() {
    const methodName = document.getElementById('btn-execute-method').dataset.methodName;
    const form = document.getElementById('method-params-form');
    
    // Collect parameters
    const params = {};
    if (form) {
        const formData = new FormData(form);
        for (let [key, value] of formData) {
            // Convert to appropriate type
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
        
        // Refresh status after command execution
        await refreshStatus();
        
        // Close modal
        bootstrap.Modal.getInstance(document.getElementById('methodModal')).hide();
    } catch (error) {
        logOutput(`✗ Failed to execute ${methodName}`, 'error');
    }
}

// Create configuration
async function createConfiguration() {
    const name = document.getElementById('config-name').value;
    const version = parseInt(document.getElementById('config-version').value);
    
    if (!name) {
        alert('Please enter a configuration name');
        return;
    }
    
    try {
        logOutput(`Creating configuration: ${name} v${version}...`, 'info');
        
        const response = await apiCall('/create', 'POST', {
            name: name,
            version: version
        });
        
        logOutput(`✓ Configuration created successfully`, 'success');
        await refreshStatus();
    } catch (error) {
        logOutput(`✗ Failed to create configuration`, 'error');
    }
}

// Refresh status
async function refreshStatus() {
    try {
        const response = await apiCall('/');
        
        if (response && response.status) {
            const status = response.status;
            document.getElementById('status-state').textContent = status.state || '--';
            document.getElementById('status-run').textContent = status.run || '--';
            document.getElementById('status-event').textContent = status.event || '--';
            
            const runningBadge = document.getElementById('status-running');
            if (status.running) {
                runningBadge.textContent = 'RUNNING';
                runningBadge.className = 'badge bg-success';
            } else {
                runningBadge.textContent = 'IDLE';
                runningBadge.className = 'badge bg-danger';
            }
        }
        
        updateConnectionStatus(true);
    } catch (error) {
        updateConnectionStatus(false);
    }
}

// Start automatic status updates
function startStatusUpdates() {
    if (statusUpdateTimer) {
        clearInterval(statusUpdateTimer);
    }
    statusUpdateTimer = setInterval(refreshStatus, STATUS_UPDATE_INTERVAL);
}

// Log output
function logOutput(message, level = 'info') {
    const logEl = document.getElementById('output-log');
    
    // Clear initial message on first log
    if (logEl.textContent.includes('Waiting for commands')) {
        logEl.innerHTML = '';
    }
    
    const timestamp = new Date().toLocaleTimeString();
    const className = `log-${level}`;
    
    const entry = document.createElement('div');
    entry.className = className;
    entry.innerHTML = `<code>[${timestamp}] ${escapeHtml(message)}</code>`;
    
    logEl.appendChild(entry);
    logEl.scrollTop = logEl.scrollHeight; // Auto scroll
}

// Clear log
function clearLog() {
    document.getElementById('output-log').innerHTML = '<p class="text-muted">Log cleared</p>';
}

// Escape HTML to prevent injection
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

// Cleanup on page unload
window.addEventListener('beforeunload', () => {
    if (statusUpdateTimer) {
        clearInterval(statusUpdateTimer);
    }
});
