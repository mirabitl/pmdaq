# Deployment Guide for PicoMic Web Interface

## Quick Start

### Option 1: Direct Python (Development)

```bash
# 1. Navigate to services directory
cd /opt/pmdaq/picmic/services

# 2. Install dependencies
pip install fastapi uvicorn pydantic

# 3. Start the server
uvicorn main:app --host 0.0.0.0 --port 8000

# 4. Open browser to http://localhost:8000
# 5. Open the web interface:
#    - Open /opt/pmdaq/picmic/tkweb/index.html in your browser
#    - OR if served by Nginx: http://localhost
```

### Option 2: Using the Start Script

```bash
# Make it executable
chmod +x /opt/pmdaq/picmic/tkweb/start-server.sh

# Run with default settings (0.0.0.0:8000)
/opt/pmdaq/picmic/tkweb/start-server.sh

# Or with custom host/port
/opt/pmdaq/picmic/tkweb/start-server.sh 8000 127.0.0.1
```

### Option 3: Docker (Production)

```bash
# Build the Docker image
cd /opt/pmdaq/picmic
docker build -t picmic:latest -f tkweb/Dockerfile .

# Run the container
docker run -d \
  --name picmic \
  -p 8000:8000 \
  -p 80:80 \
  -v /tmp/results:/tmp/results \
  -v /dev/shm:/dev/shm \
  picmic:latest
```

### Option 4: Docker Compose (Recommended for Production)

```bash
cd /opt/pmdaq/picmic

# Start services
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f picmic-api

# Stop services
docker-compose down
```

## Full Deployment Setup

### 1. Install System Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y python3-pip python3-venv nginx

# CentOS/RHEL
sudo yum install -y python3-pip nginx
```

### 2. Create Python Virtual Environment

```bash
cd /opt/pmdaq/picmic

# Create venv
python3 -m venv venv
source venv/bin/activate  # Linux/Mac
# or
venv\Scripts\activate     # Windows

# Upgrade pip
pip install --upgrade pip

# Install dependencies
pip install fastapi uvicorn pydantic numpy matplotlib transitions
```

### 3. Configure Nginx (Optional)

```bash
# Copy example configuration
sudo cp /opt/pmdaq/picmic/tkweb/nginx.conf.example /etc/nginx/sites-available/picmic

# Create symlink
sudo ln -s /etc/nginx/sites-available/picmic /etc/nginx/sites-enabled/

# Remove default site
sudo rm /etc/nginx/sites-enabled/default

# Test configuration
sudo nginx -t

# Restart Nginx
sudo systemctl restart nginx
```

### 4. Create Systemd Service (Linux)

Create `/etc/systemd/system/picmic-api.service`:

```ini
[Unit]
Description=PicoMic DAQ API Server
After=network.target

[Service]
Type=simple
User=picmic
WorkingDirectory=/opt/pmdaq/picmic/services
Environment="PATH=/opt/pmdaq/picmic/venv/bin"
ExecStart=/opt/pmdaq/picmic/venv/bin/uvicorn main:app --host 0.0.0.0 --port 8000
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Then:

```bash
# Create user
sudo useradd -r -s /bin/bash picmic

# Set permissions
sudo chown -R picmic:picmic /opt/pmdaq/picmic

# Enable service
sudo systemctl daemon-reload
sudo systemctl enable picmic-api
sudo systemctl start picmic-api

# Check status
sudo systemctl status picmic-api
```

## Network Configuration

### Local Network Access

If you want to access the interface from other machines on the network:

1. **Find your IP address**:
   ```bash
   hostname -I  # Linux
   ipconfig     # Windows
   ```

2. **Update `app.js` in tkweb**:
   ```javascript
   const API_BASE_URL = 'http://YOUR_IP:8000';
   ```

3. **Access from another machine**:
   ```
   http://YOUR_IP/
   # or
   http://YOUR_IP:8000/docs (API docs)
   ```

### Firewall Rules

```bash
# Open port 8000 (FastAPI)
sudo ufw allow 8000/tcp

# Open port 80 (Nginx)
sudo ufw allow 80/tcp

# Open port 443 (HTTPS, if configured)
sudo ufw allow 443/tcp
```

## SSL/TLS Certificate (HTTPS)

### Using Let's Encrypt with Nginx

```bash
# Install certbot
sudo apt-get install certbot python3-certbot-nginx

# Get certificate
sudo certbot certonly --nginx -d picmic.example.com

# Update Nginx configuration (auto-done by certbot)
sudo systemctl reload nginx
```

### Manual HTTPS Configuration

Add to nginx.conf:

```nginx
server {
    listen 443 ssl;
    server_name picmic.example.com;

    ssl_certificate /path/to/certificate.crt;
    ssl_certificate_key /path/to/private.key;

    # ... rest of configuration
}

# Redirect HTTP to HTTPS
server {
    listen 80;
    server_name picmic.example.com;
    return 301 https://$server_name$request_uri;
}
```

## Performance Tuning

### Uvicorn Workers

For production, use multiple workers:

```bash
uvicorn main:app --host 0.0.0.0 --port 8000 --workers 4
```

Or in systemd service:

```ini
ExecStart=/opt/pmdaq/picmic/venv/bin/gunicorn main:app \
  --workers 4 \
  --worker-class uvicorn.workers.UvicornWorker \
  --bind 0.0.0.0:8000
```

### Nginx Caching

Add to nginx.conf:

```nginx
# Cache static files
location ~* \.(js|css|png|jpg|jpeg|gif|svg)$ {
    expires 30d;
    add_header Cache-Control "public, immutable";
}

# Gzip compression
gzip on;
gzip_types text/css application/javascript application/json;
gzip_min_length 1000;
```

## Monitoring and Logging

### Check Service Status

```bash
# Systemd
sudo systemctl status picmic-api

# Docker
docker-compose logs -f picmic-api

# Manual
ps aux | grep uvicorn
```

### Log Files

```bash
# Systemd logs
sudo journalctl -u picmic-api -f

# Nginx logs
sudo tail -f /var/log/nginx/access.log
sudo tail -f /var/log/nginx/error.log
```

### Health Check

```bash
# Test API endpoint
curl http://localhost:8000/picmic/

# Test web interface
curl http://localhost/

# Monitor with watch
watch -n 2 curl http://localhost:8000/picmic/
```

## Troubleshooting

### Port Already in Use

```bash
# Find process using port 8000
lsof -i :8000
# or
netstat -tlnp | grep 8000

# Kill process
kill -9 <PID>
```

### Permission Denied

```bash
# Fix ownership
sudo chown -R picmic:picmic /opt/pmdaq/picmic

# Fix permissions
sudo chmod -R 755 /opt/pmdaq/picmic
sudo chmod -R 755 /tmp/results
```

### Connection Refused

1. Check if service is running: `systemctl status picmic-api`
2. Check firewall: `sudo ufw status`
3. Check listening ports: `netstat -tlnp`
4. Check logs: `journalctl -u picmic-api -n 50`

### Slow Performance

1. Check system resources: `top`, `free -h`
2. Increase Uvicorn workers
3. Enable Nginx caching
4. Check network latency: `ping localhost`

## Backup and Restore

### Backup Configuration

```bash
# Backup services directory
tar czf picmic_backup_$(date +%Y%m%d).tar.gz \
  /opt/pmdaq/picmic/services \
  /opt/pmdaq/picmic/tkweb

# Backup results
tar czf picmic_results_$(date +%Y%m%d).tar.gz /tmp/results
```

### Restore Configuration

```bash
# Extract backup
tar xzf picmic_backup_20240527.tar.gz -C /opt/pmdaq/

# Restart service
sudo systemctl restart picmic-api
```

## Uninstall

```bash
# Stop service
sudo systemctl stop picmic-api
sudo systemctl disable picmic-api

# Remove systemd service
sudo rm /etc/systemd/system/picmic-api.service
sudo systemctl daemon-reload

# Remove Nginx configuration
sudo rm /etc/nginx/sites-available/picmic
sudo rm /etc/nginx/sites-enabled/picmic
sudo systemctl reload nginx

# Cleanup files (optional)
sudo rm -rf /opt/pmdaq/picmic
```

## Support

For issues or questions:

1. Check logs: `journalctl -u picmic-api`
2. Check browser console: F12 → Console
3. Check API docs: http://localhost:8000/docs
4. Check README.md in tkweb directory
