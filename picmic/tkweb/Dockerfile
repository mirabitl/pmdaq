# Docker file for PicoMic Web Interface
# Build and run the complete PicoMic system in a container

FROM python:3.11-slim

WORKDIR /opt/pmdaq/picmic

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    curl \
    nginx \
    && rm -rf /var/lib/apt/lists/*

# Copy application files
COPY . .

# Install Python dependencies
RUN pip install --no-cache-dir \
    fastapi \
    uvicorn \
    pydantic \
    numpy \
    matplotlib \
    transitions

# Install PicoMic packages (adjust paths as needed)
RUN pip install --no-cache-dir -e ./

# Copy Nginx configuration
COPY tkweb/nginx.conf.example /etc/nginx/sites-available/default

# Expose ports
EXPOSE 80 8000

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=40s --retries=3 \
    CMD curl -f http://localhost:8000/picmic/ || exit 1

# Startup script
RUN echo '#!/bin/bash\n\
set -e\n\
echo "Starting PicoMic services..."\n\
\n\
# Start FastAPI in background\n\
cd /opt/pmdaq/picmic/services\n\
uvicorn main:app --host 0.0.0.0 --port 8000 &\n\
\n\
# Start Nginx\n\
nginx -g "daemon off;"\n\
' > /start.sh && chmod +x /start.sh

CMD ["/start.sh"]
