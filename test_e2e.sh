#!/bin/bash
# test_e2e.sh - End-to-end test completo
# Inicia server, ejecuta script via API, captura screenshot via dashboard

set -e
cd "$(dirname "$0")"

echo "=== Zeta E2E Test ==="
pkill -9 zeta_server zeta_dashboard 2>/dev/null || true
sleep 1

# 0. Build todo
echo "[0/4] Rebuilding all binaries..."
./build.sh all 2>&1 | tail -4

PORT=8095
SCREENSHOT=tests/e2e_screenshot.png
SCENE_JSON=tests/e2e_scene.json

# 1. Generar scene via CLI
echo "[1/4] Generando scene via zeta CLI..."
./zeta tests/dashboard_scene.zl > /dev/null
mv tests/dashboard_scene.json "$SCENE_JSON"
NODES=$(python3 -c "import json; d=json.load(open('$SCENE_JSON')); print(len(d['nodes']))")
echo "      OK - $NODES nodos en $SCENE_JSON"

# 2. Iniciar server
echo "[2/4] Iniciando zeta_server en puerto $PORT..."
./zeta_server --port $PORT > /tmp/e2e_server.log 2>&1 &
SRV_PID=$!
sleep 1
if ! kill -0 $SRV_PID 2>/dev/null; then
    echo "      FAIL - server no arranco"
    cat /tmp/e2e_server.log
    exit 1
fi
echo "      OK - server PID $SRV_PID"

# 3. Ejecutar script via API
echo "[3/4] Ejecutando script via POST /api/run..."
python3 -c "
import json
with open('tests/dashboard_scene.zl') as f:
    code = f.read()
with open('/tmp/e2e_request.json', 'w') as f:
    json.dump({'code': code}, f)
"
RESULT=$(curl -s -X POST http://localhost:$PORT/api/run \
    -H "Content-Type: application/json" \
    -d @/tmp/e2e_request.json)
HAS_ERROR=$(echo "$RESULT" | python3 -c "import json,sys; d=json.load(sys.stdin); print('yes' if d.get('has_error') else 'no')" 2>/dev/null || echo "parse_err")
echo "      OK - has_error: $HAS_ERROR"

# 4. Verificar /api/grafo
echo "[4/4] Verificando GET /api/grafo..."
GRAFO=$(curl -s http://localhost:$PORT/api/grafo)
API_NODES=$(echo "$GRAFO" | python3 -c "import json,sys; d=json.load(sys.stdin); print(len(d.get('nodes',[])))" 2>/dev/null || echo "0")
echo "      OK - /api/grafo reporta $API_NODES nodos"

# 5. Screenshot via dashboard contra el server
echo "[BONUS] Generando screenshot via dashboard contra server..."
./zeta_dashboard --host localhost --port $PORT --screenshot "$SCREENSHOT" --width 1600 --height 1400 --wait 10 2>&1 | tail -1

# 6. Terminal render via zeta_term contra el server
echo "[BONUS] Renderizando via terminal contra server..."
TERM_OUT=tests/e2e_term.txt
./zeta_term --host localhost --port $PORT --width 120 --no-color > "$TERM_OUT" 2>&1
TERM_LINES=$(wc -l < "$TERM_OUT" 2>/dev/null || echo "0")
TERM_TITLES=$(rg -o '\+[A-Za-z][^+]+\-+' "$TERM_OUT" 2>/dev/null | sort -u | wc -l)
echo "      OK - $TERM_LINES lineas, $TERM_TITLES titulos de nodo"

# 7. Cleanup
kill $SRV_PID 2>/dev/null || true
sleep 0.5
pkill -9 zeta_server 2>/dev/null || true

echo
echo "=== Resultado ==="
echo "Scene JSON:   $SCENE_JSON ($NODES nodos)"
echo "API nodes:    $API_NODES"
echo "Screenshot:   $SCREENSHOT"
echo "Terminal:     $TERM_OUT ($TERM_LINES lineas, $TERM_TITLES titulos)"
ls -la "$SCENE_JSON" "$SCREENSHOT" "$TERM_OUT" 2>/dev/null

if [ "$NODES" = "$API_NODES" ] && [ "$NODES" -gt 0 ] && [ -f "$SCREENSHOT" ] && [ "$TERM_TITLES" -gt 0 ]; then
    echo "PASS"
    exit 0
else
    echo "FAIL"
    exit 1
fi
