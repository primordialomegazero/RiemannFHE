// RiemannFHE Node.js Bindings
// Noise-Free FHE on the Riemann Critical Line
// φΩ0 — Primordial Omega Zero

const { execSync } = require('child_process');
const path = require('path');

const BINARY = path.join(__dirname, '../../build/riemann_api');

class RiemannFHE {
  constructor() {
    this.baseUrl = 'http://localhost:8443';
  }

  async health() {
    return this._get('/health');
  }

  async encrypt(value) {
    return this._post('/encrypt', { value });
  }

  async decrypt(value) {
    return this._post('/decrypt', { value });
  }

  async add(a, b) {
    return this._post('/add', { a, b });
  }

  async multiply(a, b) {
    return this._post('/multiply', { a, b });
  }

  async multiKeyEncrypt(value) {
    return this._post('/multi/encrypt', { value });
  }

  async riemannEncrypt(value, zeroIndex = 15) {
    return this._post('/riemann/encrypt', { value, zero_index: zeroIndex });
  }

  async benchmark() {
    return this._get('/benchmark');
  }

  async audit() {
    return this._get('/audit');
  }

  async _get(endpoint) {
    const res = await fetch(`${this.baseUrl}${endpoint}`);
    return res.json();
  }

  async _post(endpoint, data) {
    const res = await fetch(`${this.baseUrl}${endpoint}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data)
    });
    return res.json();
  }
}

module.exports = { RiemannFHE };
