<div align="center">
  <img src="logo.svg" alt="Dollar Price Checker" width="280" height="80">
</div>

# Dollar Price Checker

Consulta las cotizaciones del dólar en Argentina desde la terminal. Rápido, seguro y mínimo.

```bash
dollar
```

```
Dólar Blue
-----------
USD
  COMPRA: 1395.00
  VENTA:  1415.00
  24/04/2026 16:08
```

## Instalación

### One-liner (Linux)

```bash
curl -sSL https://raw.githubusercontent.com/sasax741/dollar_price/main/scripts/install.sh | sh
```

### Dependencias

**Debian/Ubuntu:**
```bash
sudo apt-get install build-essential libcurl4-openssl-dev libjansson-dev
```

**macOS (Homebrew):**
```bash
brew install curl jansson
```

### Compilar e instalar

```bash
make
sudo make install
dollar
```

Para instalar sin sudo:
```bash
make install PREFIX=~/.local
```

## Uso

```bash
dollar                    # Tabla con colores
dollar --format=json      # JSON machine-readable
dollar --no-colors        # Sin colores (pipelines)
dollar --no-cache         # Ignorar cache
dollar --timeout=15       # Timeout en segundos
dollar --cache-ttl=600    # Cache por 10 min
dollar --help             # Ayuda completa
```

### Variables de entorno

| Variable | Descripción |
|---|---|
| `DOLLAR_NO_COLOR` | Desactivar colores |
| `DOLLAR_TIMEOUT` | Timeout en segundos |
| `DOLLAR_CACHE_TTL` | TTL del cache en segundos |

### JSON output

```bash
dollar --format=json | jq '.[0].venta'
# 1415.00
```

## Arquitectura

```
├── include/          Headers públicos
│   ├── cache.h       Cache offline con TTL
│   ├── config.h      Constantes y códigos de error
│   ├── network.h     Cliente HTTP con retry
│   ├── parser.h      Parser JSON con schema validation
│   └── security.h    Validaciones de seguridad
├── src/              Módulos core
│   ├── cache.c       Cache en ~/.dollar/cache.json
│   ├── main.c        Main loop e integración
│   ├── network.c     HTTP + retry exponencial
│   ├── parser.c      JSON parsing + validación
│   └── security.c    Límites de buffer, rangos, sanitización
└── tests/            Tests unitarios (CMocka)
    ├── test_parser.c     8 tests
    └── test_security.c   15 tests
```

### Seguridad

- Límite de 1MB en respuestas
- Timeout configurable (defecto 30s)
- Retry exponencial (1s → 2s → 4s, máximo 3 intentos)
- Cache offline en fallos de API
- Validación de rangos de precios (0.01–10000)
- Sanitización de strings JSON
- SSL verification activa (anti-MITM)

## Tests

```bash
make test        # Compila y ejecuta todos los tests
make test-parser    # Solo parser
make test-security  # Solo security
```

23 tests, 0 fallos.

## API

Usa [dolarapi.com](https://dolarapi.com) — endpoint público gratuito.

## Licencia

MIT
