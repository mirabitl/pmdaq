#!/bin/bash
# Start script for PicoMic Web Interface

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVICES_DIR="$SCRIPT_DIR/../services"
PORT=${1:-8000}
HOST=${2:-"0.0.0.0"}

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}PicoMic Web Interface Startup${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Check if services directory exists
if [ ! -d "$SERVICES_DIR" ]; then
    echo -e "${RED}Error: Services directory not found at $SERVICES_DIR${NC}"
    exit 1
fi

# Check if main.py exists
if [ ! -f "$SERVICES_DIR/main.py" ]; then
    echo -e "${RED}Error: main.py not found at $SERVICES_DIR/main.py${NC}"
    exit 1
fi

echo -e "${YELLOW}Starting FastAPI server...${NC}"
echo "  Host: $HOST"
echo "  Port: $PORT"
echo ""

# Change to services directory
cd "$SERVICES_DIR"

# Check if uvicorn is installed
if ! python3 -c "import uvicorn" 2>/dev/null; then
    echo -e "${YELLOW}Installing uvicorn...${NC}"
    pip install uvicorn
fi

# Start the server
echo -e "${GREEN}Server starting at: http://localhost:$PORT${NC}"
echo -e "${GREEN}Web UI available at: http://localhost:$PORT/docs${NC}"
echo ""
echo -e "${YELLOW}Press Ctrl+C to stop the server${NC}"
echo ""

# Start uvicorn
exec uvicorn main:app --host "$HOST" --port "$PORT" --reload
