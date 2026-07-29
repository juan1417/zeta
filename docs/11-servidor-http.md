# 11. Servidor HTTP

`zeta_server` es un binario que expone el estado del intérprete Zeta a través de HTTP REST. Usa **Crow** (header-only, basado en Asio) como framework.

## Tabla de Contenidos

- [Resumen Rápido](#resumen-rápido)
- [11.1. Iniciar el Servidor](#111-iniciar-el-servidor)
- [11.2. Endpoints](#112-endpoints)
- [11.3. CORS](#113-cors)
- [11.4. Threading](#114-threading)
- [11.5. Ejemplo de Uso Programático (Python)](#115-ejemplo-de-uso-programático-python)
- [11.6. Seguridad](#116-seguridad)
- [11.7. Códigos de Error HTTP](#117-códigos-de-error-http)
- [11.8. Lifecycle: State entre Requests](#118-lifecycle-state-entre-requests)
- [11.9. El Flag `--script`](#119-el-flag---script)
- [11.10. Rutas Definidas por el Usuario](#1110-rutas-definidas-por-el-usuario)
- [11.11. Endpoints No Documentados / Deprecated](#1111-endpoints-no-documentados--deprecated)
- [11.12. Limitaciones Actuales](#1112-limitaciones-actuales)

---

## Resumen Rápido

| Endpoint | Método | Descripción |
|----------|--------|-------------|
| `GET /` | GET | UI HTML embebida (Tailwind + Chart.js) |
| `GET /api/datos` | GET | Variables globales serializadas como JSON |
| `GET /api/metricas` | GET | KPIs registrados con `metric(nombre, valor)` |
| `GET /api/dashboard` | GET | Config del dashboard HTML (`DashboardConfig`) |
| `GET /api/grafo` | GET | SceneSpec activa (consumida por renderers) |
| `POST /api/run` | POST | Ejecutar código Zeta arbitrario |
| `GET /u/*` | GET/POST/... | Rutas definidas por el usuario via `route()` |

---

## 11.1. Iniciar el Servidor

```bash
./zeta_server --port 8080
```

Flags:

| Flag | Descripción | Default |
|------|-------------|---------|
| `--port N` | Puerto de escucha | `8080` |
| `--script archivo.zl` | Script a ejecutar al iniciar | — |

Salida esperada:

```
=== Zeta Server ===
Puerto: 8080
URL: http://localhost:8080
API:  http://localhost:8080/api/datos
==================
```

El servidor corre **multithreaded** (Crow gestiona un thread pool).

---

## 11.2. Endpoints

### `GET /` — UI HTML embebida

Devuelve una página HTML con Tailwind + Chart.js que muestra métricas, gráficos y una consola para ejecutar código Zeta.

```bash
curl http://localhost:8080/
```

Abre esta URL en un browser para una UI interactiva.

### `GET /api/datos` — Variables globales como JSON

```bash
curl http://localhost:8080/api/datos
```

```json
{
  "datos": {
    "mes": [null, null, null, ...],
    "ventas": [1000.0, 1200.0, 1500.0, ...],
    "gastos": [500.0, 600.0, 700.0, ...]
  },
  "idx": [1.0, 2.0, 3.0, ...]
}
```

**Qué devuelve**: cada variable global del ámbito del intérprete, serializada según su tipo:

| Tipo Zeta | JSON |
|-----------|------|
| `num` | número |
| `null` (NaN) | `null` |
| `str` | string |
| `bool` | `true` / `false` |
| `vec` | array de números/nulls |
| `dict` | objeto |
| `df` | objeto con arrays por columna |
| `func`, `err`, `scene` | `"<no serializable>"` |

### `GET /api/metricas` — KPIs registrados

```bash
curl http://localhost:8080/api/metricas
```

```json
[
  {"nombre": "Ventas totales", "valor": 17700.0},
  {"nombre": "Promedio ventas", "valor": 1770.0}
]
```

Solo aparecen las métricas registradas con `metric(nombre, valor)`.

### `GET /api/dashboard` — Config del dashboard HTML

```bash
curl http://localhost:8080/api/dashboard
```

Devuelve el `DashboardConfig` creado con `dashboard(...)` y `add_*`. Si no se ha creado ninguno, retorna `{"titulo": "", "elementos": []}`.

### `GET /api/grafo` — Scene spec (lo que consumen los renderers)

```bash
curl http://localhost:8080/api/grafo
```

Devuelve la `SceneSpec` activa. Si no hay escena, retorna un esqueleto vacío:

```json
{
  "titulo": "",
  "autor": "",
  "layout": {"tipo": "grid", "cols": 2, "gap": 10, "background": "#1e1e1e"},
  "nodes": []
}
```

**Este es el endpoint principal** que `zeta_dashboard` y `zeta_term` consumen.

### `POST /api/run` — Ejecutar código Zeta

```bash
curl -X POST http://localhost:8080/api/run \
    -H "Content-Type: application/json" \
    -d '{"code": "$datos = load_csv(\"tests/datos.csv\")\nprint(len($datos))"}'
```

Body: `{"code": "..."}`. El código se ejecuta con un nuevo parser/intérprete (mismo `Interpreter` instance, mismo scope global).

**Importante**: el código se ejecuta sobre el estado existente. Las variables definidas se agregan al ámbito global, las escenas se modifican in-place.

Respuesta exitosa:

```json
{
  "output": "10\n",
  "datos": { ... },
  "metricas": [ ... ]
}
```

Respuesta con error:

```json
{"error": "division por cero"}
```

con HTTP code 500.

---

## 11.3. CORS

Todos los endpoints `GET` retornan el header `Access-Control-Allow-Origin: *`. Esto permite que un frontend en otro dominio consuma la API directamente.

---

## 11.4. Threading

Crow maneja un thread pool. Cada request se procesa en un thread separado. El `Interpreter` es **compartido** entre threads (un `shared_ptr<Interpreter>`).

**Cuidado**: como el `Interpreter` no tiene locks internos, las operaciones concurrentes que modifican estado (e.g., dos `POST /api/run` simultáneos) pueden causar condiciones de carrera. En la práctica, los scripts Zeta se ejecutan en serie en cada request.

Si necesitas concurrencia estricta, pon un lock alrededor de `interpreter->ejecutar(...)` en `server_main.cpp`.

---

## 11.5. Ejemplo de Uso Programático (Python)

```python
import requests

BASE = "http://localhost:8080"

# Ejecutar código
r = requests.post(f"{BASE}/api/run", json={
    "code": "$datos = load_csv('tests/datos.csv')\nprint('OK')"
})
print(r.json()["output"])

# Obtener escena
r = requests.get(f"{BASE}/api/grafo")
escena = r.json()
print(f"Escena con {len(escena['nodes'])} nodos")

# Stream de métricas
r = requests.get(f"{BASE}/api/metricas")
for m in r.json():
    print(f"{m['nombre']}: {m['valor']}")
```

---

## 11.6. Seguridad

**El servidor no tiene autenticación ni rate limiting**. Cualquiera con acceso al puerto puede ejecutar código arbitrario. Por diseño: es un servidor de desarrollo, no de producción.

Para producción:

1. Poner detrás de un reverse proxy (nginx, Caddy) con TLS y auth.
2. Limitar el `POST /api/run` a localhost o redes internas.
3. Sanitizar el código antes de ejecutarlo (no hay sandbox).

---

## 11.7. Códigos de Error HTTP

| Código | Cuándo |
|--------|--------|
| 200 | Todo OK |
| 400 | Body de `POST /api/run` no tiene `code` o JSON inválido |
| 500 | Error de parseo, error de runtime, error de import, etc. |

El body de error siempre es `{"error": "mensaje"}`.

---

## 11.8. Lifecycle: State entre Requests

El servidor mantiene un único `Interpreter` durante toda su vida. Las variables, métricas, escenas y dashboard definidos en un `POST /api/run` persisten para el siguiente.

```bash
# Request 1: definir variable
curl -X POST http://localhost:8080/api/run -d '{"code": "$x = 42"}'

# Request 2: usar la variable
curl -X POST http://localhost:8080/api/run -d '{"code": "print($x)"}'
# output: "42\n"
```

---

## 11.9. El Flag `--script`

Si pasas `--script archivo.zl`, el servidor ejecuta ese script al iniciar, antes de empezar a escuchar:

```bash
./zeta_server --port 8080 --script tests/dashboard_scene.zl
```

Esto es útil para **pre-cargar datos y escenas** sin tener que hacer un `POST /api/run` manual.

---

## 11.10. Rutas Definidas por el Usuario

Además de los 6 endpoints built-in, Zeta permite registrar rutas HTTP propias via la builtin `route()`. Esto te permite construir APIs completas desde un script Zeta sin tocar C++.

### Sintaxis

```zeta
fn mi_handler($req) {
    # $req es un string JSON con {method, path, body, query, headers}
    # Tu handler retorna un dict {status, body, headers}
    return {
        "status": 200,
        "body": "hola",
        "headers": {"Content-Type": "text/plain"}
    }
}

route("GET", "/api/saludo", mi_handler)
```

`route(metodo, path, handler)` registra una ruta:

| Parámetro | Tipo | Descripción |
|-----------|------|-------------|
| `metodo` | string | `"GET"`, `"POST"`, `"PUT"`, `"DELETE"`, o `"PATCH"` (case-sensitive) |
| `path` | string | Debe empezar con `/` |
| `handler` | función | Recibe el request como string JSON, retorna la respuesta |

Retorna el número de rutas registradas hasta ahora (1, 2, 3, ...).

### El Request

`$req` es un string JSON serializado (parsealo en el handler o pasalo a una biblioteca JSON). Estructura:

```json
{
    "method": "POST",
    "path": "/api/echo",
    "body": "{\"x\":1}",
    "query": {"limit": "10"},
    "headers": {"Content-Type": "application/json", "User-Agent": "..."}
}
```

### La Respuesta

El handler debe retornar un dict con tres claves opcionales:

| Clave | Tipo | Default | Descripción |
|-------|------|---------|-------------|
| `status` | int | `200` | Código de estado HTTP |
| `body` | string, number o null | `null` | Cuerpo de la respuesta |
| `headers` | dict | `{}` | Headers HTTP |

Si el handler retorna un string o un número en vez de un dict, se envuelve automáticamente como `{status: 200, body: <valor>}`.

### Exponer las Rutas

El server expone todas las rutas registradas bajo el prefijo `/u/`. Internamente:

```
GET  /u/api/saludo     -> busca ruta GET /api/saludo en interpreter->obtener_rutas()
POST /u/api/echo       -> busca ruta POST /api/echo
PUT  /u/api/recurso/1  -> busca ruta PUT /api/recurso/1
```

Esto evita conflictos con los 6 endpoints built-in (`/api/datos`, `/api/run`, etc).

### Flujo Completo

```bash
# 1. Inicia el server
./zeta_server --port 8080 &

# 2. Carga un script que defina rutas via /api/run
curl -X POST http://localhost:8080/api/run \
    -H "Content-Type: application/json" \
    -d @mi_api.json

# 3. Llama tu ruta
curl http://localhost:8080/u/api/saludo
curl -X POST http://localhost:8080/u/api/echo -d '{"x":1}'
```

### Ejemplo: API de Estadísticas

```zeta
# stats_api.zl

# Cargar datos
$datos = load_csv("tests/datos.csv")

fn get_media_ventas($req) {
    $m = mean($datos:ventas)
    return {
        "status": 200,
        "body": "{\"media\":" + str($m) + "}",
        "headers": {"Content-Type": "application/json"}
    }
}

fn get_total_ventas($req) {
    $t = sum($datos:ventas)
    return {
        "status": 200,
        "body": "{\"total\":" + str($t) + "}",
        "headers": {"Content-Type": "application/json"}
    }
}

fn get_count($req) {
    $n = len($datos)
    return {
        "status": 200,
        "body": "{\"count\":" + str($n) + "}",
        "headers": {"Content-Type": "application/json"}
    }
}

route("GET", "/api/stats/media", get_media_ventas)
route("GET", "/api/stats/total", get_total_ventas)
route("GET", "/api/stats/count", get_count)

print("API de stats lista en:")
print("  GET http://localhost:8080/u/api/stats/media")
print("  GET http://localhost:8080/u/api/stats/total")
print("  GET http://localhost:8080/u/api/stats/count")
```

```bash
# Cargar
curl -X POST http://localhost:8080/api/run -H "Content-Type: application/json" -d @stats_api.zl.json

# Consultar
curl http://localhost:8080/u/api/stats/media
# {"media":15500.0}

curl http://localhost:8080/u/api/stats/total
# {"total":155000.0}

curl http://localhost:8080/u/api/stats/count
# {"count":10}
```

### Validaciones de `route()`

`route()` retorna un error (en vez de registrar) si:

| Condición | Error |
|-----------|-------|
| Método no es `GET/POST/PUT/DELETE/PATCH` | `"metodo invalido 'FETCH'"` |
| Path no empieza con `/` | `"path debe empezar con /"` |
| Handler no es una función | `"handler debe ser una funcion"` |

```zeta
# Errores típicos:
route("FETCH", "/x", $h)        # Error: "metodo invalido 'FETCH'"
route("GET", "no-slash", $h)    # Error: "path debe empezar con /"
route("GET", "/x", 42)          # Error: "handler debe ser una funcion"
```

### Limitaciones de `route()`

| Limitación | Descripción |
|------------|-------------|
| Sin path parameters | `/api/users/$id` no funciona como plantilla. Parsea `path` con un split |
| Sin regex matching | Solo match exacto |
| Sin streaming | La respuesta se envía completa |
| Sin middleware | No hay hooks `before_request` / `after_request`. Agrega un `CROW_ROUTE` adicional en `server_main.cpp` |
| Prefijo `/u/` | Las rutas user-defined viven bajo `/u/`. Edita el `CROW_ROUTE` en `server_main.cpp` para cambiarlo |

---

## 11.11. Endpoints No Documentados / Deprecated

| Endpoint | Estado |
|----------|--------|
| `GET /api/health` | No existe, pero es trivial agregar |
| `GET /api/version` | No existe |

Si los necesitas, agrégalos a `src/server_main.cpp`.

---

## 11.12. Limitaciones Actuales

| Limitación | Descripción |
|------------|-------------|
| Sin WebSockets | Las actualizaciones en vivo requieren polling |
| Sin upload de archivos | No hay endpoint para subir CSVs (usa `load_csv` con una ruta local) |
| Sin rate limiting | Ver §11.6 |
| Sin autenticación | Ver §11.6 |

Estas son limitaciones conscientes para mantener el server simple. Si las necesitas, son ~50 líneas de código cada una.
