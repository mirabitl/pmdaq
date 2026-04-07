#!/usr/bin/env bash

set -e

BASE_URL="http://lyocms09:8000"

NAME=""
VERSION=""
ACTION=""
TRANSITION=""
CMD=""
PARAMS="{}"

usage() {
  echo "Usage:"
  echo "  $0 -a ACTION -n name -v version [options]"
  echo ""
  echo "Actions:"
  echo "  create"
  echo "  delete"
  echo "  restart"
  echo "  configure"
  echo "  transition"
  echo "  command"
  echo ""
  echo "Options:"
  echo "  -n name"
  echo "  -v version"
  echo "  -t transition_name"
  echo "  -c command_name"
  echo "  -p params_json"
  echo ""
  echo "Examples:"
  echo "  $0 -a create -n tel_cms -v 4"
  echo "  $0 -a restart -n tel_cms -v 4"
  echo "  $0 -a configure -n tel_cms -v 4 -p '{\"params_dbname\":\"params2026\"}'"
  echo "  $0 -a transition -n tel_cms -v 4 -t start"
  echo "  $0 -a command -n tel_cms -v 4 -c status"
  exit 1
}

# -----------------------------
# Parse args
# -----------------------------
while getopts "a:n:v:t:c:p:h" opt; do
  case $opt in
    a) ACTION="$OPTARG" ;;
    n) NAME="$OPTARG" ;;
    v) VERSION="$OPTARG" ;;
    t) TRANSITION="$OPTARG" ;;
    c) CMD="$OPTARG" ;;
    p) PARAMS="$OPTARG" ;;
    h) usage ;;
    *) usage ;;
  esac
done

if [[ -z "$ACTION" ]]; then
  usage
fi

# -----------------------------
# Core function
# -----------------------------
call_api() {
  METHOD=$1
  URL=$2
  DATA=$3

  echo ">>> $METHOD $URL"
  curl -s -X "$METHOD" "$URL" \
    -H "Content-Type: application/json" \
    -d "$DATA"
  echo -e "\n"
}

# -----------------------------
# Actions
# -----------------------------
case $ACTION in

  create)
    call_api POST "$BASE_URL/apps/" "{
      \"name\":\"$NAME\",
      \"version\":\"$VERSION\"
    }"
    ;;
  
  delete)
    call_api DELETE "$BASE_URL/apps/$NAME/versions/$VERSION" "{}"
    ;;

  restart)
    call_api POST "$BASE_URL/apps/$NAME/versions/$VERSION/restart" "{
      \"params\": $PARAMS
    }"
    ;;

  configure)
    call_api POST "$BASE_URL/apps/$NAME/versions/$VERSION/configure" "{
      \"params\": $PARAMS
    }"
    ;;

  transition)
    if [[ -z "$TRANSITION" ]]; then
      echo "Missing transition name (-t)"
      exit 1
    fi

    call_api POST "$BASE_URL/apps/$NAME/versions/$VERSION/transitions" "{
      \"name\":\"$TRANSITION\"
    }"
    ;;

  command)
    if [[ -z "$CMD" ]]; then
      echo "Missing command (-c)"
      exit 1
    fi

    call_api POST "$BASE_URL/apps/$NAME/versions/$VERSION/commands" "{
      \"cmd\":\"$CMD\",
      \"params\": $PARAMS
    }"
    ;;

  *)
    echo "Unknown action"
    usage
    ;;
esac
