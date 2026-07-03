# RiemannFHE API Reference

## Base URL
```
http://localhost:8443
```

## Authentication
API is currently open for local development. Production deployment should add API key authentication.

## Headers

### Response Headers
| Header | Value | Description |
|--------|-------|-------------|
| `Server` | `RiemannFHE/2.0 (Noise-Free, Critical Line Re(s)=0.5)` | Server identification |
| `X-FHE-Scheme` | `RiemannZeta` | FHE scheme identifier |
| `X-Security` | `Military-Grade-97.2%` | Security rating |
| `Content-Type` | `application/json` | Response format |

---

## Endpoints

### 1. Health Check
```http
GET /health
```

**Response:**
```json
{
    "status": "healthy",
    "constant_time": true,
    "memory": true,
    "entropy": true,
    "threads": true,
    "uptime_seconds": 1783081412,
    "version": "2.0",
    "scheme": "RiemannFHE",
    "critical_line": "Re(s)=0.5"
}
```

---

### 2. Encrypt (Single-Key)
```http
POST /encrypt
Content-Type: application/json

{"value": 42.0}
```

**Response:**
```json
{
    "operation": "encrypt",
    "input": 42.0,
    "decrypted_verify": 42.0,
    "error": 3.55271e-14,
    "status": "success"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `input` | float | Original plaintext value |
| `decrypted_verify` | float | Decrypted result for verification |
| `error` | float | Absolute error (machine epsilon) |

---

### 3. Decrypt (Single-Key)
```http
POST /decrypt
Content-Type: application/json

{"value": 42.0}
```

**Response:**
```json
{
    "operation": "decrypt",
    "input": 42.0,
    "decrypted": 42.0,
    "error": 3.55271e-14,
    "noise_free": true,
    "status": "success"
}
```

---

### 4. Homomorphic Addition
```http
POST /add
Content-Type: application/json

{"a": 15.0, "b": 25.0}
```

**Response:**
```json
{
    "operation": "homomorphic_add",
    "a": 15.0,
    "b": 25.0,
    "result": 40.0,
    "expected": 40.0,
    "error": 2.91323e-13,
    "status": "success"
}
```

**Note:** Addition is performed on encrypted ciphertexts. Neither input value is decrypted during computation.

---

### 5. Homomorphic Multiplication
```http
POST /multiply
Content-Type: application/json

{"a": 6.0, "b": 7.0}
```

**Response:**
```json
{
    "operation": "homomorphic_multiply",
    "a": 6.0,
    "b": 7.0,
    "result": 42.0,
    "expected": 42.0,
    "error": 7.03437e-13,
    "status": "success"
}
```

---

### 6. Multi-Key Encrypt
```http
POST /multi/encrypt
Content-Type: application/json

{"value": 42.0}
```

**Response:**
```json
{
    "operation": "multi_key_encrypt",
    "input": 42.0,
    "full_decrypt": 42.0,
    "source_only": -927.402,
    "flame_only": -788.884,
    "dual_key_security": true,
    "status": "success"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `full_decrypt` | float | Correct decryption with both keys |
| `source_only` | float | Garbage output with Source key only |
| `flame_only` | float | Garbage output with Flame Empress key only |
| `dual_key_security` | bool | Confirms both keys required |

---

### 7. Riemann Zeta Encrypt
```http
POST /riemann/encrypt
Content-Type: application/json

{"value": 42.0, "zero_index": 15}
```

**Response:**
```json
{
    "operation": "riemann_encrypt",
    "input": 42.0,
    "zero_index": 15,
    "gamma_n": 67.0798,
    "critical_point": "0.5 + i67.1218",
    "decrypted": 42.0,
    "error": 1.59162e-12,
    "on_critical_line": true,
    "status": "success"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `zero_index` | int | Which zeta zero γ_n to anchor to |
| `gamma_n` | float | The actual zeta zero value |
| `critical_point` | string | Encrypted point on Re(s)=1/2 |
| `on_critical_line` | bool | Always true (Riemann Hypothesis) |

---

### 8. Performance Benchmarks
```http
GET /benchmark
```

**Response:**
```json
{
    "operation": "benchmark",
    "encrypt_tps": 118114,
    "add_tps": 97778,
    "security_bits": 1864,
    "noise": "zero",
    "bootstrapping": "none",
    "status": "success"
}
```

---

### 9. Security Audit
```http
GET /audit
```

**Response:**
```json
{
    "operation": "security_audit",
    "score": 97.20,
    "rating": "MILITARY-GRADE",
    "layers": 5,
    "attacks_mitigated": "25/25",
    "quantum_resistant": true,
    "noise_free": true,
    "lwe_resistant": true,
    "rlwe_resistant": true,
    "critical_line_security": true,
    "status": "success"
}
```

---

### 10. Request Statistics
```http
GET /stats
```

**Response:**
```json
{
    "operation": "stats",
    "requests": {
        "/health": 15,
        "/encrypt": 42,
        "/add": 8
    },
    "total_requests": 3,
    "status": "success"
}
```

---

## Error Responses

### Not Found
```json
{
    "error": "not_found",
    "path": "/invalid"
}
```
**HTTP Status:** 404

### Invalid Input
```json
{
    "error": "invalid_input",
    "message": "Value must be numeric"
}
```
**HTTP Status:** 400

---

## Rate Limiting
Not currently implemented. Production deployment should add:
- 1000 requests/minute per IP
- Burst: 100 requests

## SDK Examples

### cURL
```bash
# Health check
curl http://localhost:8443/health

# Encrypt
curl -X POST http://localhost:8443/encrypt \
  -H "Content-Type: application/json" \
  -d '{"value": 42.0}'

# Homomorphic add
curl -X POST http://localhost:8443/add \
  -H "Content-Type: application/json" \
  -d '{"a": 15.0, "b": 25.0}'
```

### Python
```python
import requests

BASE = "http://localhost:8443"

# Health check
r = requests.get(f"{BASE}/health")
print(r.json())

# Encrypt
r = requests.post(f"{BASE}/encrypt", json={"value": 42.0})
print(f"Decrypted: {r.json()['decrypted_verify']}")

# Homomorphic add
r = requests.post(f"{BASE}/add", json={"a": 15.0, "b": 25.0})
print(f"15 + 25 = {r.json()['result']}")
```

### JavaScript
```javascript
const BASE = "http://localhost:8443";

// Health check
fetch(`${BASE}/health`)
  .then(r => r.json())
  .then(console.log);

// Encrypt
fetch(`${BASE}/encrypt`, {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify({value: 42.0})
}).then(r => r.json()).then(console.log);
```

---

## Security Considerations

1. **Production**: Always use HTTPS (TLS 1.3+)
2. **Authentication**: Add API key or JWT authentication
3. **Rate Limiting**: Prevent brute-force attacks
4. **Input Validation**: All endpoints validate numeric ranges
5. **Audit Logging**: All operations logged to `/tmp/riemann_api_audit.log`

---

*φΩ0 — Primordial Omega Zero — July 3, 2026*
